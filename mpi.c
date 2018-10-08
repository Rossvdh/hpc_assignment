/*CSC4000W HPC assignment
MPI version
Ross van der Heyde VHYROS001
7 October 2018*/

#include <stdio.h>
#include <stdlib.h>
#include "dcdplugin.c"
#include "mpi.h"

int main(int argc, char *argv[]) {
	int myid;
	int nodenum;

	printf("Hello\n");

	MPI_Init(&argc, &argv);                 //Start MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);           //get rank of node's process
	MPI_Comm_size(MPI_COMM_WORLD, &nodenum);

	if (myid == 0) {
		//read timestep data from dcdfile into vector
		//here in main thread.
		//can all the threads access the vector/How to make a variable shared? NO
		//or do I have to send the data to each thread? Yes, I think so
		printf("Simulate MPI");

		//read cmd line params
		char inputFile[] = "";
		char outputFile[] = "";

		int i;
		for (i = 0; i < argc; ++i) {
			if (argv[i] == "-i") {
				// input file
				inputFile = argv[i + 1];
			} else if (argv[i] == "-o") {
				outputFile = argv[i + 1];
			}
		}

		printf("input file: %s\n", inputFile);
		printf("output file: %s\n", outputFile);

		//clear text from output file
		std::ofstream outFile(outputFile);
		outFile << "";
		outFile.close();

		//read input file
		std::ifstream myFile(inputFile);

		//read lines from file one at a time
		//line 1: input DCD file
		char dcdFile[];
		std::getline(myFile, dcdFile);

		dcdFile.erase(std::remove(dcdFile.begin(), dcdFile.end(), '\r'), dcdFile.end());
		dcdFile.erase(std::remove(dcdFile.begin(), dcdFile.end(), '\n'), dcdFile.end());
		printf(".dcd file: %s\n", dcdFile);

		//line 2: k
		char temp[];
		int k = 0;
		std::getline(myFile, temp);
		k = stoi(temp);
		printf("k: %d\n", k);

		//line 3: atoms in A
		getline(myFile, temp);
		printf("Atoms in A: %s\n", temp);

		//todo: parse line into numbers
		std::vector<int> aIndx = parseNumbers(temp);

		//line 4: atoms in B
		getline(myFile, temp);
		printf("Atoms in B: %s\n", temp);
		std::vector<int> bIndx = parseNumbers(temp);

		myFile.close();
		printf("The file '%s' as been closed.\n", inputFile);

		//read DCD file
		printf("Reading .dcd file '%s""'...\n", dcdFile);

		//get fileName as C-style string because we need to pass it as a char**
		char* c_dcdFile = new char [dcdFile.size() + 1];
		std::copy(dcdFile.begin(), dcdFile.end(), c_dcdFile);
		c_dcdFile[dcdFile.size()] = '\0';

		char* fileName [] = {c_dcdFile};

		int natoms = 0;
		void* v = open_dcd_read(*fileName, "dcd", &natoms);
		printf("'%s' opened\n", dcdFile);
		printf("natoms: %d\n", natoms);

		//cast to dcdhandle
		dcdhandle* handle = (dcdhandle*) v;
		printf("frames: %d\n", handle->nsets);

		//read timesteps. this seems very inefficient. uses a lot of memory. maybe its just my weak laptop
		timestepCounter
		std::vector<molfile_timestep_t> timesteps;
		int timestepCounter;
		for (timestepCounter = 0; timestepCounter < handle->nsets; ++timestepCounter) {
			// std::cout << "timestep: " << timestepCounter << std::endl;
			molfile_timestep_t timestep; // data for the current timestep
			timestep.coords = (float *)malloc(3 * sizeof(float) * natoms); //change to normal array?
			int rc = read_next_timestep(v, natoms, &timestep);

			timesteps.push_back(timestep);
		}
	}

	//now on other threads:
	//cacluate on a disjoint section of the timestep data vector
	//[0, timesteps/3)
	//[timesteps/3, 2*(timesteps/3))
	//[2*(timesteps/3), timesteps)where timesteps is the number of timesteps.
	//determine k nearest pairs
	//send to main thread.
	//main thread waits for k shortest for each timestep.
	//main collates into writeToFiel vector
	//main writes to output file

	// std::cout << "Hello from thread " << myid << std::endl;
	printf("Hello from thread %d\n", myid);

	MPI_Finalize();

	if (myid == 0) {
		printf("Finalized\n");
	}

	return 0;
}