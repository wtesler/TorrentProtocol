#include <mpi.h>
#include "Manager.h"
#include "Worker.h"

using namespace std;

// Creates and starts a new Manager process.
void init_manager_process(Manager * manager);

// Creates and starts a new Worker process.
void init_worker_process(Worker * worker, int rank);

int main(int argc, char* argv[]) {

    // Initialize MPI
    MPI_Init(&argc, &argv);

	// Get size and rank
	int size, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	cout << "Rank: " << rank << endl;

    // Each process will be either a Manager or a Worker.
    Manager * manager = nullptr;
    Worker * worker = nullptr;
    if (rank == 0) init_manager_process(manager);
    else init_worker_process(worker, rank);

    // Delete whichever one was used.
    if (manager != nullptr) delete manager;
    if (worker != nullptr) delete worker;

    // Cleanup MPI
    MPI_Finalize();  

    return 0;
}

void init_manager_process(Manager * manager){
    manager = new Manager();
    manager->start();
}

void init_worker_process(Worker * worker, int rank){
    worker = new Worker(rank);
    worker->start();
}




