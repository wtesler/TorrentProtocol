#include <mpi.h>
#include "Manager.h"
#include "Worker.h"
#include <time.h>

using namespace std;

// Creates and starts a new Manager process.
void init_manager_process(Manager * manager, int size);

// Creates and starts a new Worker process.
void init_worker_process(Worker * worker, int rank);

int main(int argc, char* argv[]) {

    // Initialize MPI
    MPI_Init(&argc, &argv);

	// Get size and rank
	int size, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Each process will be either a Manager or a Worker.
    Manager * manager = nullptr;
    Worker * worker = nullptr;

    // Initialize them.
    if (rank == 0) manager = new Manager(size);
    else worker = new Worker(rank);

    // Wait.
    MPI_Barrier(MPI_COMM_WORLD);

    timespec time1, time2;
    clock_gettime(CLOCK_MONOTONIC, &time1);

    // Start either one.
    if (rank == 0) manager -> start();
    else worker -> start();

    clock_gettime(CLOCK_MONOTONIC, &time2);
    long completionTime = time2.tv_nsec - time1.tv_nsec;

    if (rank == 0) {
        cout << "Completion Time: " << completionTime << " nanoseconds" << endl;
    }

    // Delete whichever one was used.
    if (rank == 0) delete manager;
    else delete worker;

    // Cleanup MPI
    MPI_Finalize();  

    return 0;
}

void init_manager_process(Manager * manager, int size){
    manager = new Manager(size);
    manager->start();
}

void init_worker_process(Worker * worker, int rank){
    worker = new Worker(rank);
    timespec time;
    time.tv_nsec = rank * 100000;
    nanosleep(&time, NULL);
    worker->start();
}




