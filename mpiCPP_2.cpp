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
#include <cstring>
#include <vector>
#include <algorithm>
#include <iterator>
#include <chrono>

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

int main(int argc, char* argv[]) {
	std::cout << "MPI 2!" << std::endl;
	int nodenum;
	int myid;
	MPI_Init(&argc, &argv);                 //Start MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);           //get rank of node's process
	MPI_Comm_size(MPI_COMM_WORLD, &nodenum);
	// std::cout << "nodenum: " << nodenum << std::endl;

	// printf("Hello from MPI %d\n", myid);

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
		std::ofstream outFile(outputFile);
		outFile << "";
		outFile.close();

		//read lines from input file one at a time
		std::ifstream myFile(inputFile);
		//line 1: input DCD file
		std::string dcdFile;
		std::getline(myFile, dcdFile);

		dcdFile.erase(std::remove(dcdFile.begin(), dcdFile.end(), '\r'), dcdFile.end());
		dcdFile.erase(std::remove(dcdFile.begin(), dcdFile.end(), '\n'), dcdFile.end());
		std::cout << ".dcd file: " << dcdFile << std::endl;
		//broadcast dcd file name
		int nameLen = dcdFile.size();
		MPI_Bcast(&nameLen, 1, MPI_INT, 0, MPI_COMM_WORLD);

		char * cstr = new char [dcdFile.length() + 1];
		std::strcpy (cstr, dcdFile.c_str());
		MPI_Bcast(&cstr, nameLen + 1, MPI_CHAR, 0, MPI_COMM_WORLD);

		//line 2: k
		std::string temp;
		int k = 0;
		std::getline(myFile, temp);
		k = stoi(temp);
		std::cout << "k: " << k << std::endl;
		//broadcast k to other processes
		std::cout << "broadcast k..." << std::endl;
		MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD); // bcast k

		//line 3: atoms in A
		getline(myFile, temp);
		std::cout << "Atoms in A: " << temp << std::endl;
		std::vector<int> aIndx = parseNumbers(temp);

		std::cout << "broadcast aIndx" << std::endl;
		int aCount = aIndx.size();
		MPI_Bcast(&aCount, 1, MPI_INT, 0, MPI_COMM_WORLD);//bcast aCount
		MPI_Bcast(&aIndx[0], aCount, MPI_INT, 0, MPI_COMM_WORLD);//bcast a indexes

		//line 4: atoms in B
		getline(myFile, temp);
		std::cout << "Atoms in B: " << temp << std::endl;
		std::vector<int> bIndx = parseNumbers(temp);

		std::cout << "broadcast bIndx" << std::endl;
		int bCount = bIndx.size();
		MPI_Bcast(&bCount, 1, MPI_INT, 0, MPI_COMM_WORLD); //bcast bCount
		MPI_Bcast(&bIndx[0], bCount, MPI_INT, 0, MPI_COMM_WORLD); //bcast b indexes

		myFile.close();
		std::cout << "The file '" << inputFile << "' as been closed.\n" << std::endl;

		//open dcd file
		//get fileName as C-style string because we need to pass it as a char**

		// delete[]c_dcdFile;
		delete[]cstr;
	} else {
		//other processes

		//receive dcd file name
		int nameLen;
		MPI_Bcast(&nameLen, 1, MPI_INT, 0, MPI_COMM_WORLD);
		char* cstr[nameLen + 1];
		MPI_Bcast(&cstr, nameLen + 1, MPI_CHAR, 0, MPI_COMM_WORLD);

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

		// delete[]c_dcdFile;
	}


	MPI_Finalize();
	std::cout << "Done" << std::endl;
	return 0;
}