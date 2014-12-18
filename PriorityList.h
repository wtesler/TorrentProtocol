/* Copyright (c) 2014 Will Tesler */

#include "PriorityNode.h"
#include <vector>

using namespace std;

#pragma once
class PriorityList {

protected:

    // Underlying container.
    // Conditions: This container is immutable once filled.
    vector< PriorityNode* > nodes;

    // Ensures strict ordering of the linked list.
    void prioritize(PriorityNode * node, bool forward){

        PriorityNode * scanResult = scan(node, forward);
        if (scanResult == node) {
            // No need to alter placement.
            return;
        };

        PriorityNode * frontNeighbor = node -> front;
        PriorityNode * backNeighbor = node -> back;

        // Stitch the two cuts strands together.
        if (frontNeighbor) { frontNeighbor -> back = backNeighbor; }
        if (backNeighbor) { backNeighbor -> front = frontNeighbor; }

        // These are your new neighbors
        PriorityNode * newFrontNeighbor = !forward ? scanResult : scanResult -> front;
        PriorityNode * newBackNeighbor =  forward ? scanResult : scanResult -> back;

        // Make sure the lead and tail still point to the correct place.
        if (node == lead) {
            lead = backNeighbor;
        }
        if (node == tail) {
            tail = frontNeighbor;
        }

        if (scanResult == lead) {
            // Node is in the lead
            node -> back = lead;
            node -> front = nullptr;
            lead -> front = node;
            lead = node;
        } else if (scanResult == tail) {
            // Node is the tail
            node -> front = tail;
            node -> back = nullptr;
            tail -> back = node;
            tail = node;
        } else {
            // Your neighbors need to meet you.
            if (newFrontNeighbor) { newFrontNeighbor->back = node;}
            if (newBackNeighbor){ newBackNeighbor->front = node;}
            // Get to know your new neighbors.
            node->front = newFrontNeighbor;
            node->back = newBackNeighbor;
        }
    };

    /*
    1. Scan forward or backward starting from node.
    2. When a node of critical priority is found, return it.
    */
    PriorityNode * scan(PriorityNode * node, bool forward) {
        PriorityNode * iter = nullptr;
        if (forward) {
            iter = node -> front;
            while (iter != nullptr && iter->getPriority() <= node->getPriority()) {
                iter = iter->front;
            }

            if (iter){ iter = iter->back;} 
            else {iter = lead;}

        } else {
            iter = node -> back;
            while (iter != nullptr && iter->getPriority() >= node->getPriority()) {
                iter = iter->back;
            }

            if (iter){iter = iter->front;}
            else {iter = tail;}

        }
        return iter;
    };

public:

    //Extrema of the list
    PriorityNode * lead;
    PriorityNode * tail;

    // Constructor
    PriorityList(unsigned int size) {
            nodes = vector<PriorityNode*>(size);
    };

    PriorityList(void){};
    ~PriorityList(void){};

    // Overrides the [] operator to provide direct read access to the underlying vector
    // Note, this does not take priority into consideration, it simply returns nodes from the immutable vector.
    PriorityNode& operator [](const int i) {
        return *nodes[i];
    };

    PriorityNode& nodeAt(const int i) {
        return *nodes[i];
    };

    // Changes the priority of a node.
    // This will cause prioritize to be called.
    void set(PriorityNode * node, int priority){
        int oldPriority = node->getPriority();
        node->setPriority(priority);
        prioritize(node, priority > oldPriority);
    };


    // Returns the size of the underlying container;
    unsigned int size() const {
        return nodes.size();
    };
};

