#include "Constants.h"
#include <string>
#include <iostream>
#include <mpi.h>
#include <vector>

#define DEBUG true

using namespace std;

#pragma once
class Worker {

private:
    
    MPI_Request * sendRequest;
    MPI_Status * sendStatus;

    char * catVideo;
    int numChunks;

public:

    Worker(void) {

        //Will hold the file when finished.
        catVideo = new char[MOVIE_SIZE_IN_BYTES]();

        //Divide the data into segmented chunks.
        numChunks = (MOVIE_SIZE_IN_BYTES / CHUNK_SIZE_IN_BYTES) + 1;

	debug("Worker Created");
    }

    ~Worker(void) {
        delete[] catVideo;
    }

    void something() {

        for (int i = 0; i < numChunks; i++) {

            sendDataRequest(i);
            
            // The index and size of the chunk we requested.
            int desiredChunkSize = i != numChunks - 1 ? CHUNK_SIZE_IN_BYTES : MOVIE_SIZE_IN_BYTES % CHUNK_SIZE_IN_BYTES;
            char * desiredChunkIndex = catVideo + (CHUNK_SIZE_IN_BYTES * i);

            // Issue a receive for the chunk data.
            MPI_Request recvRequest;
            MPI_Status recvStatus;
            MPI_Irecv(desiredChunkIndex, desiredChunkSize, MPI_CHAR, MPI_ANY_SOURCE, TAG_DATA_REQUEST, MPI_COMM_WORLD, &recvRequest);

            while(true) {

                int dataFlag;
                MPI_Test(&recvRequest, &dataFlag, &recvStatus);

                // Test for incoming chunk data
                if (dataFlag != 0){
                    // We have received the chunk. Move onto the next one.
                    debug("Chunk Received");
                    break; 
                }

                // Probe for Work Orders.
                int workFlag;
                MPI_Status probeStatus;
                MPI_Iprobe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &workFlag, &probeStatus);

                // If we found a Work Order.
                if (workFlag != 0 && probeStatus.MPI_TAG != TAG_DATA_REQUEST) {
                    debug("Work Order received");
                    int chunkPosition = probeStatus.MPI_TAG;
                    int waitlistSize;
		    MPI_Get_count(&probeStatus, MPI_INT, &waitlistSize);
                    vector<int> addresses(waitlistSize);
                    MPI_Status workOrderStatus;
                    MPI_Recv(&addresses[0], waitlistSize, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &workOrderStatus);
                    int size = chunkPosition != numChunks - 1 ? CHUNK_SIZE_IN_BYTES : MOVIE_SIZE_IN_BYTES % CHUNK_SIZE_IN_BYTES;
                    char * dataIndex = catVideo + (CHUNK_SIZE_IN_BYTES * chunkPosition);
                    sendData(dataIndex, size, chunkPosition, addresses);
                }
            }
        }
        cout << "Worker finished streaming" << endl;
    }

    void sendData(char * data, int length, int chunkPosition, vector<int> addresses) {
        for (unsigned int i = 0; i < addresses.size(); i++) {
            MPI_Send(data, length, MPI_CHAR, addresses[i], TAG_DATA_REQUEST, MPI_COMM_WORLD);
        }
    }
 
    // True if there are pending messages to receive.
    bool testForChunkData(MPI_Request * request, MPI_Status * status) {
        int flag;
        MPI_Test(request, &flag, status);
        return flag != 0;
    }

    void sendDataRequest(int chunkNum) {
        MPI_Send(&chunkNum, 1, MPI_INT, 0, TAG_DATA_REQUEST, MPI_COMM_WORLD);
    }

    void debug(string message) {
        if (DEBUG) {cout << message << endl; };
    }

};

