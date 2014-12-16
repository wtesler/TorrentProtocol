#pragma once
class PriorityNode {

private:
    
    int priority;

public:

    PriorityNode * front;
    PriorityNode * back;

    //structors
    PriorityNode(void) : front(nullptr), back(nullptr), priority(0) {};
    ~PriorityNode(void){};

    int getPriority() {
        return priority;
    }

    int setPriority(int priority) {
        this->priority = priority;
    }
};

