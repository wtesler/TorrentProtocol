#include "Constants.h"
#include <string>
#include <iostream>
#include <mpi.h>
#include <vector>
#include <sstream>

#define DEBUG true

typedef int FLAG;
typedef char BYTE;

using namespace std;

#pragma once
class Worker {

private:

    //Holds the main received data
    BYTE * data;

    const int rank;

    vector<string> messageBuffer;

public:

    Worker(int rank) : rank(rank) {

        //Aggregates the data.
        data = new BYTE[DATA_SIZE_IN_BYTES]();

        stringstream ss;
        ss << "Worker " << rank << " created.";
        messageBuffer.push_back(ss.str());
    }

    ~Worker(void) {
        delete[] data;
    }

    // Join the swarm! Begin the process of uploading/downloading data.
    void start() {

        // Iterate over each chunk.
        for (int i = 0; i < DATA_SIZE_IN_CHUNKS; i++) {

            // Issue a blocking send to the Manager for a chunk of data.
            sendDataRequest(i);

            // The size of the chunk we requested.
            int size = i != DATA_SIZE_IN_CHUNKS - 1 ? CHUNK_SIZE_IN_BYTES : LEFTOVER_SIZE_IN_BYTES;

            // The place where the new chunk should go.
            BYTE * dataIndex = data + (CHUNK_SIZE_IN_BYTES * i);

            // Issue a new asynchronous receive for the chunk data.
            FLAG dataFlag = -1;
            MPI_Request dataRequest;
            MPI_Status dataStatus;

            FLAG workFlag = -1;
            MPI_Status workStatus;
            
            // IDLE LOOP
            // Continously test for incoming data, and break when it arrives.
            // While incoming data has NOT arrived, probe for work orders.
            while(true) {

                if (dataFlag != 0) {
                    MPI_Irecv(dataIndex, size, MPI_CHAR, MPI_ANY_SOURCE, TAG_DATA_REQUEST, MPI_COMM_WORLD, &dataRequest);
                    dataFlag = 0;
                }
                MPI_Test(&dataRequest, &dataFlag, &dataStatus);
                // If we found an incoming chunk
                if (dataFlag != 0){

                    stringstream ss;
                    ss << "Worker " << rank << " received chunk " << i;
                    messageBuffer.push_back(ss.str());

                    // Move onto the next chunk.
                    // This is the only place where the loop can exit.
                    dataFlag = -1;
                    if (dataStatus.MPI_SOURCE != -1) {
                        break; 
                    }
                }

                MPI_Iprobe(0, TAG_WORK_ORDER, MPI_COMM_WORLD, &workFlag, &workStatus);
                // If we found a Work Order.
                if (workFlag != 0) {
                    
                    stringstream ss;
                    ss << "Worker " << rank << " received work order";
                    messageBuffer.push_back(ss.str());

                    // Receive and process the work order
                    fulfillOrder(workStatus);
                }
            }
        }

        stringstream ss;
        ss << "WORKER " << rank << " FINISHED";
        messageBuffer.push_back(ss.str());

        for (string s : messageBuffer) {
            cout << s << endl;
        }

        // Tell manager that we are done.
        int dummy = rank;
        MPI_Send(&dummy, 1, MPI_INT, 0, TAG_TERMINATION_NOTICE, MPI_COMM_WORLD);
    }

    // Retrieves the work order from the manager and processes it.
    void fulfillOrder(MPI_Status & orderStatus) {

        // Peek at the size of the data.
        int messageDataLength;
        MPI_Get_count(&orderStatus, MPI_INT, &messageDataLength);

        // Construct a vector to hold the data.
        vector<int> addresses(messageDataLength);

        // Receive the data from the Manager
        MPI_Status workOrderStatus;
        MPI_Recv(&addresses[0], messageDataLength, MPI_INT, 0, TAG_WORK_ORDER, MPI_COMM_WORLD, &workOrderStatus);

        // First things first, get the chunk position off the back of the list. 
        // Every other int in the list is a rank address
        int chunkPosition = addresses.back();
        addresses.pop_back();

        // Decide the size of the chunk at that position.
        int size = chunkPosition != DATA_SIZE_IN_CHUNKS - 1 ? CHUNK_SIZE_IN_BYTES : LEFTOVER_SIZE_IN_BYTES;

        // Get the actual pointer to data
        BYTE * dataIndex = data + (CHUNK_SIZE_IN_BYTES * chunkPosition);

        // Send the data to the designated Workers, thus fulfilling the work order.
        sendChunk(dataIndex, size, chunkPosition, addresses);
    }

    // Send a chunk of data to each computer inside addresses.
    void sendChunk(char * data, int length, int chunkPosition, vector<int> addresses) {
        for (unsigned int i = 0; i < addresses.size(); i++) {
           
            stringstream ss;
            ss << "Worker " << rank << " is sending chunk " << chunkPosition << " to Worker " << addresses[i];
            messageBuffer.push_back(ss.str());
            
            MPI_Send(data, length, MPI_CHAR, addresses[i], TAG_DATA_REQUEST, MPI_COMM_WORLD);
        }
    }

    // True if there are pending messages to receive.
    bool testForChunkData(MPI_Request * request, MPI_Status * status) {
        int flag;
        MPI_Test(request, &flag, status);
        return flag != 0;
    }

    // Tell the Manager that you need the chunk at position chunkPosition
    void sendDataRequest(int chunkPosition) {
        // Send out a data request and wait for the certificate.
        MPI_Send(&chunkPosition, 1, MPI_INT, 0, TAG_DATA_REQUEST, MPI_COMM_WORLD);
    }
};

