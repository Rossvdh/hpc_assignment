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
	std::vector<int> aIndx = parseNumbers(temp);

	//line 4: atoms in B
	getline(myFile, temp);
	std::cout << "Atoms in B: " << temp << std::endl;
	std::vector<int> bIndx = parseNumbers(temp);

	myFile.close();
	std::cout << "The file '" << inputFile << "' as been closed.\n" << std::endl;

	//now the fun starts

	//read DCD file
	std::cout << "Reading .dcd file '" << dcdFile << "'...\n" << std::endl;

	//get fileName as C-style string because we need to pass it as a char**
	char* c_dcdFile = new char [dcdFile.size() + 1];
	std::copy(dcdFile.begin(), dcdFile.end(), c_dcdFile);
	c_dcdFile[dcdFile.size()] = '\0';

	char* fileName [] = {c_dcdFile};

	int natoms = 0;
	void* v = open_dcd_read(*fileName, "dcd", &natoms);
	std::cout << "'" << dcdFile << "' opened" << std::endl;
	std::cout << "natoms: " << natoms << std::endl;

	//cast to dcdhandle
	dcdhandle* handle = (dcdhandle*) v;
	// std::cout << "cast" << std::endl;
	std::cout << "frames: " << handle->nsets << std::endl;

	//read timesteps
	for (int timestepCounter = 0; timestepCounter <= 15; timestepCounter++) {
		std::cout << "timestep: " << timestepCounter << std::endl;
		molfile_timestep_t timestep; // data for the current timestep
		timestep.coords = (double *)malloc(3 * sizeof(double) * natoms); //change to normal array?
		int rc = read_next_timestep(v, natoms, &timestep);

		//check for error
		if (rc) {
			std::cout << "error in read_next_timestep on frame " << timestepCounter << std::endl;
			return 1;
		}

		//vector of distances
		std::vector<std::pair<std::string, double> > distances;

		//calculate distances between those in aIndx and bIndx, put in distances
		for (std::vector<int>::iterator aIt = aIndx.begin(); aIt != aIndx.end(); ++aIt) {
			//co-ords of atom
			double ax, ay, az;
			int a = *aIt;
			std::cout << "a atom: " << a << std::endl;
			ax = timestep.coords[3 * a];
			ay = timestep.coords[(3 * a) + 1];
			az = timestep.coords[(3 * a) + 2];

			for (std::vector<int>::iterator bIt = bIndx.begin(); bIt != bIndx.end(); ++bIt) {
				//coords of atom
				double bx, by, bz;
				int b = *bIt;
				std::cout << "b atom: " << b << std::endl;
				bx = timestep.coords[3 * b];
				by = timestep.coords[(3 * b) + 1];
				bz = timestep.coords[(3 * b) + 2];

				double dist = sqrt(pow((ax - bx), 2) + pow((ay - by), 2) + pow((az - bz), 2));

				// key is "aAtomNo,bAtomNo". not using a hashtable because we want to sort
				std::string key = std::to_string(a) + "," + std::to_string(b);

				std::pair<std::string, double> entry(key, dist);
				distances.push_back(entry);
				// std::cout << entry.first << ": " << entry.second << std::endl;
			}
		}

		//sort based on distance
		std::sort(distances.begin(), distances.end(),
		          [](const std::pair<std::string, double>& lhs, const std::pair<std::string, double>& rhs)
		          ->bool{return lhs.second <= rhs.second;});

		//open output file for appending
		std::ofstream outFile(outputFile, std::ios_base::app);

		//write k shorted distances to file
		for (int i = 0; i < k; ++i) {
			outFile << timestepCounter << "," << distances[i].first << ","
			        << distances[i].second << "\n";
		}
		outFile.close();
	}

	//close file
	std::cout << "Closing file '" << dcdFile << "'" << std::endl;
	close_file_read(v);
	std::cout << ".dcd file closed" << std::endl;

	delete[] c_dcdFile;

	std::cout << "Complete" << std::endl;
	return 0;
}