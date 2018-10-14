/*A c++ file to read the input file and parse the numbers
Then it sends the dcd file name and arrays of aIndices and bIndices
to a c file which does the MPI thing
Ross van der Heyde VHYROS001
14 October 2018*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

extern "C" {
#include "my_mpi.h"
}

int* parseNumbers(std::string& numbers) {
	// std::cout << "parseNumbers: " << numbers << std::endl;
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

	return &vec[0];
}

int main(int argc, char const *argv[]) {
	std::cout << "MPI Prelims" << std::endl;

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

	//read input file
	std::ifstream myFile(inputFile);

	//read lines from file one at a time
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
	int* aIndx = parseNumbers(temp);

	//line 4: atoms in B
	getline(myFile, temp);
	std::cout << "Atoms in B: " << temp << std::endl;
	int* bIndx = parseNumbers(temp);

	myFile.close();
	std::cout << "The file '" << inputFile << "' as been closed.\n" << std::endl;

	//now call the actual mpi function from mpi.c
	std::cout << "call function from mpi.c" << std::endl;

	/*now we have the atom ID from set A in aIndx,
	the atoms IDs from set B in bIndx,
	and the dcd file name in dcdFile, and
	k in k*/
	calculate();
	std::cout << "Done" << std::endl;
}