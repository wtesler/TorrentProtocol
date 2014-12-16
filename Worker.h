#include "Constants.h"
#include <string>
#include <iostream>
#include <mpi.h>
#include <vector>
#include <sstream>

#define DEBUG false

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
            }
        }

        stringstream ss;
        ss << "WORKER " << rank << " FINISHED";
        messageBuffer.push_back(ss.str());

        if (DEBUG) {
            for (string s : messageBuffer) {
                cout << s << endl;
            }
        }

        // Tell manager that we are done.
        int dummy = rank;
        MPI_Send(&dummy, 1, MPI_INT, 0, TAG_TERMINATION_NOTICE, MPI_COMM_WORLD);
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

