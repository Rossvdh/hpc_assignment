#include "my_mpi.h"
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern void func(void);

int main(int argc, char* argv[]) {
	printf("calculate called!\n");
	int nodenum;
	int myid;
	MPI_Init(&argc, &argv);                 //Start MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);           //get rank of node's process
	MPI_Comm_size(MPI_COMM_WORLD, &nodenum);

	printf("Hello from MPI %d\n", myid);
	func();

	MPI_Finalize();
	return 0;
}