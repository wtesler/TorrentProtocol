
#pragma once
class Worker {

private:
    
    MPI_Request * sendRequest;
    MPI_Status * sendStatus;


public:

    Worker(void) {

    }

    ~Worker(void) {

    }

    void request(int chunkNum) {
        MPI_Isend(&chunkNum, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, sendRequest);
    }

    bool isSendComplete(){
        if (sendRequest != nullptr) {
            int * flag;
            MPI_Test(sendRequest, flag, sendStatus);
            return (*flag != 0);
        } else {
            return true;
        }
    }


};

