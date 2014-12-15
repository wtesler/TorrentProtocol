#include "TorrentNode.h"
#include "TorrentNodeList.h"
#include "Constants.h"
#include <climits>
#include <iostream>
#include <mpi.h>

#define DEBUG true

typedef char BYTE;
typedef int BOOL;

#pragma once
class Manager {

private:

    // Contains the file that you want to share.
    BYTE * data;

    // Prioritized Linked List of segments of data.
    TorrentNodeList * list;

    int networkSize;

public:

    // Constructor
    Manager(int size) : networkSize(size) {

        //a bunch of randomly generated byte data for testing.
        data = new char[DATA_SIZE_IN_BYTES];

        // initialize the data to random values.
        for (unsigned int i = 0; i < DATA_SIZE_IN_BYTES; i++) {
            // rand actually is psuedo-random, but it gets the job done.
            data[i] = rand() % CHAR_MAX;
        }

        // Structure the data into prioritizable chunks
        list = new TorrentNodeList(data, DATA_SIZE_IN_BYTES, CHUNK_SIZE_IN_BYTES);
    }

    // Destructor
    ~Manager() {
        delete data;
        delete list;
    }

    // Send a chunk of data to each computer inside addresses.
    void sendChunk(BYTE * data, int length, int chunkPosition, vector<int> addresses) {
        for (unsigned int i = 0; i < addresses.size(); i++) {
            if (DEBUG) cout << "Manager sending chunk " << chunkPosition << " to Worker " << addresses[i] << endl;
            MPI_Send(data, length, MPI_CHAR, addresses[i], TAG_DATA_REQUEST, MPI_COMM_WORLD);
        }
    }

    // Sends a work order to a computer.
    // Tells that computer to send data to another computer.
    void sendWorkOrder(int dest, int chunkPosition, vector<int> addresses) {
        if (DEBUG) cout << "Sending Work Order to " << dest << endl;

        // We store the position of the chunk in the last index of the vector.
        // Every other index contains rank addresses.
        addresses.push_back(chunkPosition);

        MPI_Send(&addresses[0], addresses.size(), MPI_INT, dest, TAG_WORK_ORDER, MPI_COMM_WORLD);
    }

    // This kicks off the Manager and begins the entire uploading/downloading network (swarm)
    void start() {

        // Requests and Statuses used by the Manager.
        MPI_Request dataRequest, terminationRequest;
        MPI_Status dataStatus, terminationStatus;

        // Asynchronously receive data request.
        int chunkPosition;
        MPI_Irecv(&chunkPosition, 1, MPI_INT, MPI_ANY_SOURCE, TAG_DATA_REQUEST, MPI_COMM_WORLD, &dataRequest);

        // Asynchronously receive termination notice.
        int terminatingRank;
        MPI_Irecv(&terminatingRank, 1, MPI_INT, MPI_ANY_SOURCE, TAG_TERMINATION_NOTICE, MPI_COMM_WORLD, &terminationRequest);

        // Keeps track of how many workers have finished
        int numTerminations = 1;

        // Keeps track of how many times the main loop iterates (mod INT_MAX).
        // Useful for pacing the Manager.
        int i = 0;

        // MAIN LOOP. Will only exit when there are no more workers in the swarm.
        while(numTerminations < networkSize){

            // Test to see if a data request has been received.
            BOOL dataFlag;
            MPI_Test(&dataRequest, &dataFlag, &dataStatus);
            // If request has been received.
            if (dataFlag != 0) {
                //process the new request.
                processDataRequest(&dataStatus, chunkPosition);
                // Issue a new asynchronous receive for data requests.
                MPI_Irecv(&chunkPosition, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &dataRequest);
            } 

            // Test to see if a termination notice has been received.
            BOOL terminationFlag;
            MPI_Test(&terminationRequest, &terminationFlag, &terminationStatus);
            // If termination notice has been received.
            if (terminationFlag != 0) {
                numTerminations++;
                // Issue a new asynchronous receive for data requests.
                MPI_Irecv(&chunkPosition, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &dataRequest);
            } 

            // Manager helps out with the data every so often (as defined by MANAGER_SEND_FREQ).
            if (i % MANAGER_SEND_FREQ == 0) {
                // This chunk of data currently has the highest priority.
                TorrentNode * chunk = static_cast<TorrentNode*>(list->lead);
                // Synchronously send the data each computer on the waitlist.
                sendChunk(chunk->getData(), chunk->getDataLength(), chunk->getPosition(), chunk->getWaitlist());
                // Reset the priority and waitlist of sent chunk.
                chunk->reset();
            }

            // Prevents oveflow.
            if (i == INT_MAX) { i = 0;}
            i++;
        }
    }

    // 1. Extracts info from the request.
    // 2. Adjusts list to reflect info.
    // 3. Conditionally sends a work order to the source.
    void processDataRequest(const MPI_Status * status, const int chunkPosition) {

        // The computer that sent us the request.
        int source = status->MPI_SOURCE;

        if (DEBUG) cout << "Worker " << source << " wants chunk " << chunkPosition << endl;

        // This is the chunk that source wants
        TorrentNode * desiredChunk = static_cast<TorrentNode*>(&list->nodeAt(chunkPosition));

        // Adds source to the waitlist if not already on it.
        desiredChunk->add(source);

        // See if the worker can help out.
        createWorkRequest(source, chunkPosition, desiredChunk);

        // Raise priority of chunk to reflect source's message.
        list -> set(desiredChunk, desiredChunk->getPriority() + 1);

    }

    // If the source can help out, send it a work order.
    void createWorkRequest(const int source, const int chunkPosition, TorrentNode * desiredChunk) {

        // Start from the front, and we will make our way to the back.
        TorrentNode * iter = static_cast<TorrentNode*>(list->lead);

        // Loops as long as there is still a potential chunk that source can help out with.
        while (iter != nullptr && iter->getWaitlist().size() > 0) {

            if (DEBUG) cout << "Checking to see if comp " << source << " can help with chunk " << iter->getPosition() << endl;

            // If the source is ahead of the prioritized chunk
            if (chunkPosition >= iter->getPosition()){

                // Tell the source to do some work.
                sendWorkOrder(source, iter->getPosition(), iter->getWaitlist());

                // Reward the source for helping out.
                list -> set(desiredChunk, desiredChunk->getPriority() + iter->getWaitlist().size());

                // Reset the node's waitlist and priority.
                iter->reset();

                break;
            }

            // Try the next chunk
            iter = static_cast<TorrentNode*>(iter->back);
        }

    }

};
