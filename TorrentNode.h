#include <vector>
#include <iostream>
#include "PriorityNode.h"
#include "CheckSum.h"

using namespace std;

#pragma once
class TorrentNode : public PriorityNode {

private:
    
    short checksum;
    int position;
    char * data;
    unsigned int dataLength;

    vector<int> waitlist;

public:

    TorrentNode(char * data, unsigned int dataLength, int position): checksum(0), waitlist() {
        this->data = data;
        this->position = position;
        this->dataLength = dataLength;
        this->checksum = generateChecksum(data, dataLength);
    };

    ~TorrentNode() {}

    void setChecksum(short checksum) {
        this->checksum = checksum;
    }

    short getChecksum() {
        return checksum;
    }

    int getPosition() {
        return position;
    }

    void printData() {
        for (unsigned int i = 0; i < dataLength; i++) {
            std::cout << data[i] << std::endl;
        }
    }

    void add(int address) {
        for (unsigned int i = 0; i < waitlist.size(); i++) {
            //return early if already on the waitlist
            if(waitlist[i] == address){
                return;
            }
        }
        waitlist.push_back(address);
    }

    const std::vector<int> & getWaitlist() {
        return waitlist;
    }

    void clear() {
        setPriority(0);
        waitlist.clear();
    }
};

