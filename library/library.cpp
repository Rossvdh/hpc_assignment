/*Create a library to call C++ functions in C file
Ross van der Heyde VHYROS001
14 October 2018*/

#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>

extern "C" void func(void) {
	std::cout << "\n This is a C++ code\n";
}

/*extern "C" int* parseNumbers(std::string& numbers, int* count) {
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

	*count = vec.size();

	if (*count == 10) {
		//must be b
		for (int i = 0; i < *count; ++i) {
			std::cout << "vec[" << i << "]: " << vec[i] << std::endl;
		}

		int* t = &vec[0];
		for (int i = 0; i < *count; ++i) {
			std::cout << "t[" << i << "]: " << t[i] << std::endl;
		}
	}

	return &vec[0];
}*/


extern "C" std::vector<int> parseNumbers(std::string& numbers, int* count) {
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

	*count = vec.size();

	/*if (*count == 10) {
		//must be b
		for (int i = 0; i < *count; ++i) {
			std::cout << "vec[" << i << "]: " << vec[i] << std::endl;
		}

		int* t = &vec[0];
		for (int i = 0; i < *count; ++i) {
			std::cout << "t[" << i << "]: " << t[i] << std::endl;
		}
	}*/

	return vec;
}

//call the function from the C file.
extern "C" void readInputFile(char* argv[], int argc, int* k, int** aIndx, int* aCount, int** bIndx, int* bCount, char** dcdFileName, char** outputFileName) {
	std::cout << "\nin the c++" << std::endl;
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

	*outputFileName = (char*) outputFile.c_str();
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
	*dcdFileName = (char*) dcdFile.c_str();

	//line 2: k
	std::string temp;
	std::getline(myFile, temp);
	*k = stoi(temp);
	std::cout << "k: " << *k << std::endl;

	//line 3: atoms in A
	getline(myFile, temp);
	std::cout << "Atoms in A: " << temp << std::endl;
	// *aIndx = parseNumbers(temp, aCount);
	std::vector<int> a = parseNumbers(temp, aCount);

	*aIndx = new int [a.size()];
	std::cout << "*aIndx allocated: " << a.size() << std::endl;
	for (int i = 0; i < a.size(); i++) {
		std::cout << "a[" << i << "]: " << a[i] << std::endl;
		(*aIndx)[i] = a[i];
	}
	std::cout << "copy complete" << std::endl;

	//line 4: atoms in B
	getline(myFile, temp);
	std::cout << "Atoms in B: " << temp << std::endl;
	// *bIndx = parseNumbers(temp, bCount);
	std::vector<int> b = parseNumbers(temp, bCount);
	*bIndx = new int[b.size()];

	std::cout << "*bIndx allocated: " << b.size() << std::endl;
	for (int i = 0; i < b.size(); i++) {
		std::cout << "b[" << i << "]: " << b[i] << std::endl;
		(*bIndx)[i] = b[i];
	}
	std::cout << "copy complete" << std::endl;


	myFile.close();
	std::cout << "The file '" << inputFile << "' as been closed.\n" << std::endl;
	std::cout << "End of the c++\n" << std::endl;
}