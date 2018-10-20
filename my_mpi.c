#include "my_mpi.h"
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern void func(void);

int main(int argc, char* argv[]) {
	printf("MPI!\n");
	int nodenum;
	int myid;
	MPI_Init(&argc, &argv);                 //Start MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);           //get rank of node's process
	MPI_Comm_size(MPI_COMM_WORLD, &nodenum);

	// printf("Hello from MPI %d\n", myid);

	if (myid == 0) {
		printf("Main mpi process\n");
		// readInputFile(char* argv[], int argc, int* k, int* aIndx, int* bIndx, char* dcdFileName, char* outputFileName)
		int k;
		int* aIndx;
		int* bIndx;
		int aCount, bCount;
		char* dcdFileName;
		char* outputFileName;
		readInputFile(argv, argc, &k, &aIndx, &aCount, &bIndx, &bCount, &dcdFileName, &outputFileName);
		printf("k: %d\n", k);
		printf("dcdFileName: %s\n", dcdFileName);
		printf("outputFileName: %s\n", outputFileName);

		printf("a atoms:\n");
		printf("a size: %d\n", aCount);
		int i;
		for (i = 0; i < aCount; ++i) {
			printf("%d\n", aIndx[i]);
		}

		printf("b atoms:\n");
		printf("b size: %d\n", bCount);
		for (i = 0; i < bCount; ++i) {
			printf("%d\n", bIndx[i]);
		}

		free(aIndx);
		free(bIndx);
	}


	MPI_Finalize();
	printf("Done\n");
	return 0;
}