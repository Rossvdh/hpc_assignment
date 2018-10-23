/*MPI in C++
Ross van der Heyde VHYROS001
20 October 2019. Big shout out to Kushual for telling me about mpic++*/

// #include "my_mpi.h"
#include "mpi.h"
#include "dcdplugin.c"
#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <stddef.h>

std::vector<int> parseNumbers(std::string& numbers) {
	std::vector<int> vec;
	std::istringstream iss(numbers);
	std::string token;

	while (std::getline(iss, token, ',')) {
		int dash = token.find("-");
		if (dash > 0) {
			int start = std::stoi(token.substr(0, dash));
			int end = std::stoi(token.substr(dash + 1));

			for (int i = start; i <= end; ++i) {
				vec.push_back(i);
			}

		} else {
			vec.push_back(std::stoi(token));
		}
	}

	return vec;
}

struct shortPairData {
	int timestep;
	int aIndx;
	int bIndx;
	// std::string key;
	double distance;
};

std::string spdToString(shortPairData& spd) {
	return std::to_string(spd.timestep) + "," + std::to_string(spd.aIndx) + ","
	       + std::to_string(spd.bIndx) + "," + std::to_string(spd.distance);
}

int main(int argc, char* argv[]) {
	std::cout << "MPI!" << std::endl;
	int nodenum;
	int myid;
	MPI_Init(&argc, &argv);                 //Start MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);           //get rank of node's process
	MPI_Comm_size(MPI_COMM_WORLD, &nodenum);
	// std::cout << "nodenum: " << nodenum << std::endl;

	// printf("Hello from MPI %d\n", myid);

	//define new mpi type for sending/receiving shortPairData
	MPI_Datatype mpi_spd;
	int blocklengths [] = {1, 1, 1, 1};
	// MPI_Aint displaces [] = {0, 8, 16, 24};
	MPI_Aint displaces [] = {offsetof(shortPairData, timestep), offsetof(shortPairData, aIndx),
	                         offsetof(shortPairData, bIndx), offsetof(shortPairData, distance)
	                        };
	MPI_Datatype types [] = {MPI_INT, MPI_INT, MPI_INT, MPI_DOUBLE};
	MPI_Type_create_struct(4, blocklengths, displaces, types, &mpi_spd);
	MPI_Type_commit(&mpi_spd);

	if (myid == 0) {
		std::cout << "Main mpi process\n" << std::endl;
		//read cmd line params
		std::string inputFile = "";
		std::string outputFile = "";

		for (int i = 0; i < argc; ++i) {
			if (std::string(argv[i]) == "-i") {
				// input file
				inputFile = std::string(argv[i + 1]);
			} else if (std::string(argv[i]) == "-o") {
				outputFile = std::string(argv[i + 1]);
			}
		}

		std::cout << "input file: " << inputFile << std::endl;
		std::cout << "output file: " << outputFile << std::endl;

		//clear text from output file
		// std::ofstream outFile(outputFile);
		// outFile << "";
		// outFile.close();

		//read lines from input file one at a time
		std::ifstream myFile(inputFile);
		//line 1: input DCD file
		std::string dcdFile;
		std::getline(myFile, dcdFile);

		dcdFile.erase(std::remove(dcdFile.begin(), dcdFile.end(), '\r'), dcdFile.end());
		dcdFile.erase(std::remove(dcdFile.begin(), dcdFile.end(), '\n'), dcdFile.end());
		std::cout << ".dcd file: " << dcdFile << std::endl;

		//line 2: k
		std::string temp;
		int k = 0;
		std::getline(myFile, temp);
		k = stoi(temp);
		std::cout << "k: " << k << std::endl;

		//line 3: atoms in A
		getline(myFile, temp);
		std::cout << "Atoms in A: " << temp << std::endl;

		//todo: parse line into numbers
		std::vector<int> aIndx = parseNumbers(temp);

		/*std::cout << "aIndx:" << std::endl;
		for (std::vector<int>::iterator i = aIndx.begin(); i != aIndx.end(); ++i) {
			std::cout << *i << std::endl;
		}*/

		//line 4: atoms in B
		getline(myFile, temp);
		std::cout << "Atoms in B: " << temp << std::endl;
		std::vector<int> bIndx = parseNumbers(temp);

		myFile.close();
		std::cout << "The file '" << inputFile << "' as been closed.\n" << std::endl;

		//open dcd file
		//get fileName as C-style string because we need to pass it as a char**
		char* c_dcdFile = new char [dcdFile.size() + 1];
		std::copy(dcdFile.begin(), dcdFile.end(), c_dcdFile);
		c_dcdFile[dcdFile.size()] = '\0';
		char* fileName [] = {c_dcdFile};

		int natoms;
		double initTime = MPI_Wtime();
		void* v = open_dcd_read(*fileName, "dcd", &natoms);
		std::cout << "'" << dcdFile << "' opened" << std::endl;
		std::cout << "natoms: " << natoms << std::endl;
		//cast to dcdhandle
		dcdhandle* handle = (dcdhandle*) v;
		std::cout << "frames: " << handle->nsets << std::endl;

		//broadcast natoms
		MPI_Bcast(&natoms, 1, MPI_INT, 0, MPI_COMM_WORLD);//bcast natoms

		//read dcd file into vector of timesteps
		std::vector<molfile_timestep_t> timesteps;
		int numTimesteps = handle->nsets;

		std::cout << "0 numTimesteps: " << numTimesteps << std::endl;
		std::cout << "0 natoms: " << natoms << std::endl;
		float* timestepData [numTimesteps];

		std::cout << "about to start reading" << std::endl;
		for (int timestepCounter = 0; timestepCounter < numTimesteps; ++timestepCounter) {
			// std::cout << "timestep: " << timestepCounter << std::endl;
			molfile_timestep_t timestep; // data for the current timestep
			timestep.coords = (float *)malloc(3 * sizeof(float) * natoms);
			int rc = read_next_timestep(v, natoms, &timestep);

			//copy from timestep.coords to coords;
			// float coords [3 * natoms];

			timestepData[timestepCounter] = new float [3 * natoms];
			for (int i = 0; i < (3 * natoms); i++) {
				timestepData[timestepCounter][i] = timestep.coords[i];
			}
		}

		std::cout << "main: timestepData.size(): " << numTimesteps << std::endl;

		std::cout << "Closing file '" << dcdFile << "'" << std::endl;
		close_file_read(v);
		std::cout << ".dcd file closed" << std::endl;
		delete[] c_dcdFile;
		double time = MPI_Wtime() - initTime;
		std::cout << "Time to read file: " << time << '\n';

		//broadcast k to other processes
		std::cout << "broadcast k..." << std::endl;
		MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD); // bcast k

		// broadcast aIndxes and bIndxes to other processes
		std::cout << "broadcast aIndx" << std::endl;
		int aCount = aIndx.size();
		MPI_Bcast(&aCount, 1, MPI_INT, 0, MPI_COMM_WORLD);//bcast aCount
		MPI_Bcast(&aIndx[0], aCount, MPI_INT, 0, MPI_COMM_WORLD);//bcast a indexes

		std::cout << "broadcast bIndx" << std::endl;
		int bCount = bIndx.size();
		MPI_Bcast(&bCount, 1, MPI_INT, 0, MPI_COMM_WORLD); //bcast bCount
		MPI_Bcast(&bIndx[0], bCount, MPI_INT, 0, MPI_COMM_WORLD); //bcast b indexes

		//send appropriate timesteps to processes
		std::cout << "timesteps.size(): " << numTimesteps << std::endl;
		std::cout << "intv 0: 0 to " << std::floor(numTimesteps / nodenum) << std::endl;
		initTime = MPI_Wtime();
		int counts[nodenum];
		for (int i = 1; i < nodenum; ++i) {
			int start = i * std::floor(numTimesteps / nodenum) + 1;
			int end;
			if (i == nodenum - 1) {
				end = numTimesteps - 1;
			} else {
				end = (i + 1) * std::floor(numTimesteps / nodenum);
			}

			int count = end - start + 1;
			counts[i] = count;
			MPI_Send(&count, 1, MPI_INT, i, 3, MPI_COMM_WORLD);
			MPI_Send(&start, 1, MPI_INT, i, 4, MPI_COMM_WORLD);


			//send timestep data
			for (int j = start; j <= end; j++) {
				MPI_Send(&timestepData[j][0], 3 * natoms, MPI_FLOAT, i, 5, MPI_COMM_WORLD);
			}
			std::cout << "intv " << i << ": " << start << " to " << end << std::endl;
		}
		time = MPI_Wtime();
		std::cout << "Time to broadcast: " << time << '\n';

		//main does first quarter/eighth whatever
		initTime = MPI_Wtime();
		int start = 0;
		int end = std::floor(numTimesteps / nodenum);

		//print number of timesteps
		int timestepCount = std::floor(numTimesteps / nodenum) + 1;
		std::cout << "process " << myid << "\t timestepCount:" << timestepCount << std::endl;

		//calculate (distances, k shortest)
		shortPairData kShortest [k * timestepCount];

		std::cout << "process " << myid << " calculating" << std::endl;
		for (int t = start; t <= end; t++) {
			//vector of distances
			std::vector<shortPairData> distances;

			for (int ai = 0; ai < aCount; ai++) {
				int a = aIndx[ai];
				float ax, ay, az;
				// std::cout << "a atom: " << a << std::endl;
				ax = timestepData[t][3 * a];
				ay = timestepData[t][(3 * a) + 1];
				az = timestepData[t][(3 * a) + 2];

				for (int bi = 0; bi < bCount; bi++) {
					int b = bIndx[bi];
					float bx, by, bz;

					bx = timestepData[t][3 * b];
					by = timestepData[t][(3 * b) + 1];
					bz = timestepData[t][(3 * b) + 2];

					double dist = sqrt(pow((ax - bx), 2) + pow((ay - by), 2) + pow((az - bz), 2));

					shortPairData spd;
					spd.timestep = t;
					spd.aIndx = a;
					spd.bIndx = b;
					spd.distance = dist;
					distances.push_back(spd);
				}
			}

			std::partial_sort(distances.begin(), distances.begin() + k, distances.end(),
			                  [](const shortPairData & lhs, const shortPairData & rhs)
			                  ->bool{return lhs.distance <= rhs.distance;});

			for (int i = 0; i < k; ++i) {
				// std::cout << "kShortest[(t  * k ) + i]" << ((t  * k ) + i) << std::endl;
				kShortest[(t  * k ) + i] = distances[i];
			}
		}
		std::cout << "process " << myid << " calculations complete" << std::endl;

		//receive k shortest from other processes
		shortPairData* writeToFile[nodenum];
		// int counts[nodenum];
		writeToFile[0] = kShortest;
		counts[0] = timestepCount;
		std::cout << "main is receiving shortest pair data..." << std::endl;
		for (int i = 1; i < nodenum; i++) {
			// MPI_Recv(&timestepCount, 1, MPI_INT, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			// MPI_Recv(&counts[i], 1, MPI_INT, i, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			writeToFile[i] = new shortPairData[k * counts[i]];
			std::cout << "process 0 receiving " << (k * counts[i]) << " points from process " << i << '\n';
			MPI_Recv(writeToFile[i], k * counts[i], mpi_spd, i, 7, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		time = MPI_Wtime();
		std::cout << "Time to calculate and receive data: " << time << '\n';
		//sort. Shouldn't be necessary
		//write to output file (use c++ library function?)
		std::cout << "main is writing to file..." << std::endl;
		initTime = MPI_Wtime();
		std::ofstream outFile(outputFile, std::ofstream::out | std::ofstream::ate);
		outFile.precision(15);

		//write k shortest distances to file
		for (int i = 0; i < nodenum; ++i) {
			// std::cout << "i: " << i << std::endl;
			int count = counts[i];
			std::cout << "process 0 about to write " << (k * count) << " points from process " << i << " to file" << '\n';
			for (int j = 0; j < (k * count); j++) {
				// std::cout << "\tj: " << j << std::endl;
				outFile << spdToString(writeToFile[i][j]) << "\n";
			}
		}
		std::cout << "file writing complete, about to close output file" << std::endl;
		outFile.close();
		std::cout << "output file closed" << '\n';

		time = MPI_Wtime() - initTime;
		std::cout << "Time to write to file: " << time << '\n';

		for (int i = 1; i < nodenum; i++) {
			delete[]writeToFile[i];
		}
		for (int i = 0; i < numTimesteps; ++i) {
			delete[]timestepData[i];
		}

	} else {
		//other 3 processes

		//receive natoms via bcast
		int natoms;
		MPI_Bcast(&natoms, 1, MPI_INT, 0, MPI_COMM_WORLD);//bcast natoms
		std::cout << "process " << myid << "\t natoms:" << natoms << std::endl;

		//receive k
		int k;
		MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);//bcast k
		std::cout << "process " << myid << " \t k:" << k << std::endl;

		//receive aIndxes
		int aCount;
		MPI_Bcast(&aCount, 1, MPI_INT, 0, MPI_COMM_WORLD); //bcast aCount
		int aIndx [aCount];
		MPI_Bcast(&aIndx, aCount, MPI_INT, 0, MPI_COMM_WORLD);//bcast a indexes
		std::cout << "process " << myid << "\t aCount:" << aCount << std::endl;

		//receive bIndxes
		int bCount;
		MPI_Bcast(&bCount, 1, MPI_INT, 0, MPI_COMM_WORLD);//bcast bCount
		int bIndx [bCount];
		MPI_Bcast(&bIndx, bCount, MPI_INT, 0, MPI_COMM_WORLD);//bcast b indexes
		std::cout << "process " << myid << "\t bCount:" << bCount << std::endl;

		//receive number of timesteps to be received
		int timestepCount;
		MPI_Recv(&timestepCount, 1, MPI_INT, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		int start;
		MPI_Recv(&start, 1, MPI_INT, 0, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		//print number of timesteps
		std::cout << "process " << myid << "\t timestepCount:" << timestepCount << std::endl;
		std::cout << "process " << myid << "\t start index:" << start << std::endl;

		//receive timesteps data
		float* timestepData [timestepCount];

		for (int i = 0; i < timestepCount; i++) {
			timestepData[i] = new float[3 * natoms];

			MPI_Recv(&timestepData[i][0], 3 * natoms, MPI_FLOAT, 0, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		std::cout << "process " << myid << "\t timestepData received" << std::endl;

		//calculate (distances, k shortest)
		shortPairData kShortest[k * timestepCount];

		std::cout << "process " << myid << " calculating" << std::endl;
		for (int t = 0; t < timestepCount; t++) {
			//vector of distances
			std::vector<shortPairData> distances;

			for (int ai = 0; ai < aCount; ai++) {
				int a = aIndx[ai];
				float ax, ay, az;
				// std::cout << "a atom: " << a << std::endl;
				ax = timestepData[t][3 * a];
				ay = timestepData[t][(3 * a) + 1];
				az = timestepData[t][(3 * a) + 2];

				for (int bi = 0; bi < bCount; bi++) {
					int b = bIndx[bi];
					float bx, by, bz;

					bx = timestepData[t][3 * b];
					by = timestepData[t][(3 * b) + 1];
					bz = timestepData[t][(3 * b) + 2];

					double dist = sqrt(pow((ax - bx), 2) + pow((ay - by), 2) + pow((az - bz), 2));

					shortPairData spd;
					spd.timestep = (t + start);
					spd.aIndx = a;
					spd.bIndx = b;
					spd.distance = dist;
					distances.push_back(spd);
				}
			}

			std::partial_sort(distances.begin(), distances.begin() + k, distances.end(),
			                  [](const shortPairData & lhs, const shortPairData & rhs)
			                  ->bool{return lhs.distance <= rhs.distance;});

			for (int i = 0; i < k; ++i) {
				kShortest[(t * k) + i] = distances[i];
			}
		}
		std::cout << "process " << myid << " calculations complete" << std::endl;


		//TODO: send kShortest back to main
		// MPI_Send(&count, 1, MPI_INT, i, 3, MPI_COMM_WORLD);
		std::cout << "process " << myid << " is sending shortPairData to main..." << std::endl;
		// MPI_Send(&timestepCount, 1, MPI_INT, 0, 6, MPI_COMM_WORLD);
		// std::cout << "timestepCount sent from process " << myid << " to main" << std::endl;
		std::cout <<  "process " << myid << " timeStepCount * k: " << (timestepCount * k) << std::endl;

		MPI_Send(kShortest, k * timestepCount, mpi_spd, 0, 7, MPI_COMM_WORLD);
		std::cout << "kShortest sent from process " << myid << " to main" << std::endl;


		/*for each timstep:
			distances[]
			for a in aIndx
				for b in bIndx
					calculate distance
					add distances[]
			kShort[] = k shortest in distance[]
		  send kShort to main*/
		//send k_shortest back to main
		for (int i = 0; i < timestepCount; i++) {
			delete[] timestepData[i];
		}
	}


	MPI_Finalize();
	std::cout << "process " << myid << " Done" << std::endl;
	return 0;
}
