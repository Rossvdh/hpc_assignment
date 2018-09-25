/*CSC4000W HPC assignment
Ross van der Heyde VHYROS001
19 September 2019*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "dcdplugin.c"

std::vector<int> parseNumbers(std::string& numbers) {
	std::cout << "parseNumbers: " << numbers << std::endl;
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

int main(int argc, char const *argv[]) {
	std::cout << "Simulate" << std::endl;

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

	//read input file
	std::ifstream myFile(inputFile);

	//read lines from file one at a time
	//line 1: input DCD file
	std::string dcdFile;
	std::getline(myFile, dcdFile);

	dcdFile.erase(std::remove(dcdFile.begin(), dcdFile.end(), '\r'), dcdFile.end());
	dcdFile.erase(std::remove(dcdFile.begin(), dcdFile.end(), '\n'), dcdFile.end());
	std::cout << "DCD file: " << dcdFile << std::endl;

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

	// std::cout << "aIndx:" << std::endl;
	/*for (std::vector<int>::iterator i = aIndx.begin(); i != aIndx.end(); ++i) {
		std::cout << *i << std::endl;
	}*/

	//line 4: atoms in B
	getline(myFile, temp);
	std::cout << "Atoms in B: " << temp << std::endl;
	std::vector<int> bIndx = parseNumbers(temp);

	// std::cout << "bIndx:" << std::endl;
	/*for (std::vector<int>::iterator i = bIndx.begin(); i != bIndx.end(); ++i) {
		std::cout << *i << std::endl;
	}*/

	myFile.close();
	std::cout << "The file '" << inputFile << "' as been closed.\n" << std::endl;

	//now the fun starts

	//first, read DCD file, which is in dcdFile.
	std::cout << "Reading .dcd file '" << dcdFile << "'...\n" << std::endl;

	//get fileName as C-style string because we need to pass it as a char**
	char* c_dcdFile = new char [dcdFile.size() + 1];
	std::copy(dcdFile.begin(), dcdFile.end(), c_dcdFile);
	c_dcdFile[dcdFile.size()] = '\0';

	char* fileName [] = {c_dcdFile};
	int num = testFunction(2, fileName);

	std::cout << "returned val: " << num << std::endl;
	delete[] c_dcdFile;

	return 0;
}