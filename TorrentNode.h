#include <vector>
#include <iostream>
#include "PriorityNode.h"
#include "CheckSum.h"

#pragma once
class TorrentNode : public PriorityNode {

private:
    
    short checksum;

    char * data;
    unsigned int dataLength;

    std::vector<int> waitlist;


public:

    TorrentNode(char * data, unsigned int dataLength): checksum(0) {
        this->data = data;
        this->dataLength = dataLength;
        this->checksum = generateChecksum(data, dataLength);

    };

    void setChecksum(short checksum) {
        this->checksum = checksum;
    }

    short getChecksum() {
        return checksum;
    }

    void printData() {
        for (unsigned int i = 0; i < dataLength; i++) {
            std::cout << data[i] << std::endl;
        }
    }
};

