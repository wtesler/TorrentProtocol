#include "TorrentNode.h"
#include "TorrentNodeList.h"
#include "Constants.h"
#include <climits>
#include <iostream>
#include <chrono>
#include <thread>
#include <mpi.h>

#pragma once
class Manager {

private:

    TorrentNodeList * list;
    char * catVideo;
    MPI_Request * sendRequest;
    MPI_Status  * sendStatus;

    static const bool DEBUG = true;

public:

    Manager() {

        //Just a bunch of randomly generated byte data.
        catVideo = new char[MOVIE_SIZE_IN_BYTES];
        for (unsigned int i = 0; i < MOVIE_SIZE_IN_BYTES; i++) {
            // rand actually is psuedo-random, but it gets the job done.
            catVideo[i] = rand() % CHAR_MAX;
        }

        // Organizes the video into prioritized chunks
        list = new TorrentNodeList(catVideo, MOVIE_SIZE_IN_BYTES, CHUNK_SIZE_IN_BYTES);

    }

    ~Manager() {
        delete catVideo;
        delete list;
    }

    void sendData(char * data, int length, int chunkPosition, vector<int> addresses) {
        for (unsigned int i = 0; i < addresses.size(); i++) {
            MPI_Send(data, length, MPI_CHAR, addresses[i], TAG_DATA_REQUEST, MPI_COMM_WORLD);
        }
    }

    void sendWorkOrder(int dest, int chunkPosition, vector<int> addresses) {
        MPI_Send(&addresses[0], addresses.size(), MPI_INT, dest, chunkPosition, MPI_COMM_WORLD);
    }

    // True if there are pending messages to receive.
    bool hasMessage(MPI_Request * request, MPI_Status * status) {
        int flag;
        MPI_Test(request, &flag, status);
        if (DEBUG) {
            if (flag != 0) {
                cout << "Manager has a message" << endl;
            } else {
                cout << "Manager has no messages at this time" << endl;
            }
        }
        return flag != 0;
    }

    void something() {

        MPI_Request recvRequest;
        MPI_Status recvStatus;

	int chunkPosition;
        MPI_Irecv(&chunkPosition, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &recvRequest); 
        for (int i = 0; i < 10; i++) {

            if (hasMessage(&recvRequest, &recvStatus) && i % MANAGER_SEND_FREQ != 0) {
                receive(&recvStatus, chunkPosition);
                MPI_Irecv(&chunkPosition, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &recvRequest);
            } else {
                TorrentNode * node = static_cast<TorrentNode*>(list->lead);
                sendData(node->getData(), node->getDataLength(), node->getPosition(), node->getWaitlist());
                node->reset();
            }

            this_thread::sleep_for(chrono::seconds(1));
        }

       // send(0, i, vector<int>(1, i));
    }

    void receive(MPI_Status * status, int chunkPosition) {

        // The computer that sent us the message.
        int source = status->MPI_SOURCE;

        if (DEBUG) cout << "Comp " << source << " wants chunk " << chunkPosition << endl;

        // This is the chunk that source wants
        TorrentNode * desiredChunk = static_cast<TorrentNode*>(&list->operator[](chunkPosition));

        // Will add source to the waitlist (Note: Will not add if already in list)
        desiredChunk->add(source);

        // Manager raises priority to acknowledge source's message.
        list -> set(desiredChunk, desiredChunk->getPriority() + 1);

        TorrentNode * node = static_cast<TorrentNode*>(list->lead);

        while (node != nullptr && node->getWaitlist().size() > 0) {

            if (node->getPosition() < chunkPosition){

                // Tell the worker to do some work.
                sendWorkOrder(source, node->getPosition(), node->getWaitlist());

                // Reward the worker for helping out.
                list -> set(desiredChunk, desiredChunk->getPriority() + node->getWaitlist().size());

                // Reset the node's waitlist and priority.
                node->reset();

                break;
            } else {
                node = static_cast<TorrentNode*>(node->front);
            }
        }
    }

};
