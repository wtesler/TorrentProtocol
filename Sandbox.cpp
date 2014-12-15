#include <mpi.h>
#include "Manager.h"
#include "Worker.h"
#include <chrono>
#include <thread>

using namespace std;

void init_manager_process(Manager * manager);
void init_worker_process(Worker * worker);

int main(int argc, char* argv[]) {

    MPI_Init(&argc, &argv);

	// Get Size and Rank
	int size, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	cout << "Rank: " << rank << endl;

    Manager * manager = nullptr;
    Worker * worker = nullptr;
    if (rank == 0) {
        init_manager_process(manager);

    } else {
        init_worker_process(worker);
    }

    if (manager != nullptr) delete manager;
    if (worker != nullptr) delete worker;

    MPI_Finalize();  
    return 0;
}

void init_manager_process(Manager * manager){
    manager = new Manager();
    manager->something();
}

void init_worker_process(Worker * worker){
    this_thread::sleep_for(chrono::seconds(1));
    worker = new Worker();
    worker->something();
}




