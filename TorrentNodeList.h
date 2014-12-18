/* Copyright (c) 2014 Will Tesler */

#include "TorrentNode.h"
#include "PriorityList.h"

using namespace std;

#pragma once
class TorrentNodeList : public PriorityList
{

private:

public:

    // The other constructor should be used.
    TorrentNodeList(void){};

    // Constructor.
    /* This method takes a pointer to an array of byte data, the size of that array, and the desired chunk size.
    1. Partitions the data into chunks. Each chunk has the characteristics of a node in a List.
    2. weave an initial linked list out of the chunks.
    */
    TorrentNodeList(char* data, const unsigned int DATA_SIZE_IN_BYTES, const unsigned int CHUNK_SIZE_IN_BYTES)
        : PriorityList((DATA_SIZE_IN_BYTES / CHUNK_SIZE_IN_BYTES) + 1) {

            for (unsigned int i = 0; i < nodes.size()-1; i++) {
                nodes[i] = new TorrentNode(data, CHUNK_SIZE_IN_BYTES, i);
                data+=CHUNK_SIZE_IN_BYTES;
            }
            // Last node gets the remainder amount of data.
            nodes[nodes.size()-1] = new TorrentNode(data, DATA_SIZE_IN_BYTES % CHUNK_SIZE_IN_BYTES, nodes.size()-1);

            // Weave the initial linked list.
            for (unsigned int i = 1; i < nodes.size(); i++) {
                nodes[i-1]->back = nodes[i];
                nodes[i]->front = nodes[i-1];
            }

            // Initialize the lead and tail of the list
            lead = nodes[0];
            tail = nodes[nodes.size() - 1];
    }
};

