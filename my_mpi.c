#include "my_mpi.h"
#include "mpi.h"
#include "dcdplugin.c"
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

		/*printf("a atoms:\n");
			printf("a size: %d\n", aCount);
			int i;
			for (i = 0; i < aCount; ++i) {
				printf("%d\n", aIndx[i]);
			}

			printf("b atoms:\n");
			printf("b size: %d\n", bCount);
			for (i = 0; i < bCount; ++i) {
				printf("%d\n", bIndx[i]);
			}*/

		//read dcd file into vector of timesteps
		int natoms;
		void* v = open_dcd_read(dcdFileName, "dcd", &natoms);
		printf("'%s' opened\n", dcdFileName);
		printf("natoms: %d\n", natoms);

		//cast to dcdhandle
		dcdhandle* handle = (dcdhandle*) v;
		// std::cout << "cast" << std::endl;
		printf("frames: %d\n", handle->nsets);

		printf("Closing file '%s'\n", dcdFileName);
		close_file_read(v);
		printf(".dcd file closed\n");
		//send aIndxes and bIndxes to other processes
		//send appropriate timesteps to processes

		//in all (4) processes:
		//receive aIndxes and bIndxes
		//receive timesteps data
		// calculate (distances, k shortest)
		//send back to main

		//in main:
		//receive k shortest from other processes
		//sort
		//write to output file (use c++ library function?)

		free(aIndx);
		free(bIndx);

	}


	MPI_Finalize();
	printf("Done\n");
	return 0;
}