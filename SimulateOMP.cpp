/*CSC4000W HPC assignment. The Open MP version
Ross van der Heyde VHYROS001
19 September 2019*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include "dcdplugin.c"
#include <omp.h>

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

double getTime() {
	// std::chrono::milliseconds prevTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	// double test = prevTime.count();
}

//represents the distance between 2 points (aIndx and bIndx)
//at a specific timestep
struct shortPairData {
	int timestep;
	// int aIndx;
	// int bIndx;
	std::string key;
	double distance;

	std::string toString() {
		return std::to_string(timestep) + "," + key + "," + std::to_string(distance);
	}
};

int main(int argc, char const *argv[]) {
	std::cout << "Simulate with OpenMP" << std::endl;

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

	double totalTime = 0;
	double initTime = getTime();

	//dcd file has been opened
	//can we read timesteps in parallel? (lets assume not for now)
	//read timestep data into vector
	//in parallel:
	//calculate distances
	//find shortest
	//combine shorted into a vector
	//write vector to file



	//read timesteps. this seems very inefficient
	/*std::vector<molfile_timestep_t> timesteps;
	for (int timestepCounter = 0; timestepCounter < handle->nsets; ++timestepCounter) {
		// std::cout << "timestep: " << timestepCounter << std::endl;
		molfile_timestep_t timestep; // data for the current timestep
		timestep.coords = (double *)malloc(3 * sizeof(double) * natoms); //change to normal array?
		int rc = read_next_timestep(v, natoms, &timestep);

		timesteps.push_back(timestep);
	}*/

	std::vector<shortPairData> writeToFile;// e.g. "0,304,168043,14.23986456"
	// std::cout << "timesteps.size: " << timesteps.size() << std::endl;
	// std::cout << "About to enter the parallel section...!" << std::endl;
	// #pragma omp parallel for //firstprivate(writeToFile)
	for (int t = 0; t < 10; ++t) {
		// the timesteps are independent of each other, so this loop can be parallelized

		//---------------
		std::cout << "timestep: " << t << std::endl;
		molfile_timestep_t timestep; // data for the current timestep
		timestep.coords = (float *)malloc(3 * sizeof(float) * natoms); //change to normal array?

		//vector of distances
		// std::vector<std::pair<std::string, double> > distances;
		int rc = read_next_timestep(handle, natoms, &timestep);

		//check for error
		if (rc) {
			std::cout << "error in read_next_timestep on frame " << t << std::endl;
			// return 1;
		}
		//----------------------

		// const molfile_timestep_t timestep = timesteps[t];
		std::vector<std::pair<std::string, double> > distances;
		// for (std::vector<int>::iterator aIt = aIndx.begin(); aIt != aIndx.end(); ++aIt) {
		std::cout << "About to enter the parallel section...!" << std::endl;
		#pragma omp parallel for
		for (int aIt = 0; aIt < aIndx.size(); aIt++) {
			//the calculations for distances of the a atoms are independent of each other,
			//so this loop can be parallelized.

			//co-ords of atom
			double ax, ay, az;
			// int a = *aIt;
			int a = aIndx[aIt];
			// std::cout << "a atom: " << a << std::endl;
			ax = timestep.coords[3 * a];
			ay = timestep.coords[(3 * a) + 1];
			az = timestep.coords[(3 * a) + 2];

			for (std::vector<int>::iterator bIt = bIndx.begin(); bIt != bIndx.end(); ++bIt) {
				//coords of atom
				double bx, by, bz;
				int b = *bIt;
				// std::cout << "b atom: " << b << std::endl;
				bx = timestep.coords[3 * b];
				by = timestep.coords[(3 * b) + 1];
				bz = timestep.coords[(3 * b) + 2];

				double dist = sqrt(pow((ax - bx), 2) + pow((ay - by), 2) + pow((az - bz), 2));

				// key is "aAtomNo,bAtomNo". not using a hashtable because we want to sort
				std::string key = std::to_string(a) + "," + std::to_string(b);

				std::pair<std::string, double> entry(key, dist);
				#pragma omp critical
				{
					distances.push_back(entry);
				}
				// std::cout << entry.first << ": " << entry.second << std::endl;
			}
		}
		//back to serial
		std::cout << "back to serial" << std::endl;
		//sort first k based on distances
		std::partial_sort(distances.begin(), distances.begin() + k, distances.end(),
		                  [](const std::pair<std::string, double>& lhs, const std::pair<std::string, double>& rhs)
		                  ->bool{return lhs.second <= rhs.second;});

		//---------------------------
		//open output file for writing
		std::ofstream outFile(outputFile, std::ios_base::app);
		outFile.precision(15);

		//write k shorted distances to file
		for (int i = 0; i < k; ++i) {
			outFile << t << "," << distances[i].first << ","
			        << distances[i].second << "\n";
		}
		outFile.close();

		delete[] timestep.coords;//not sure if this is necessary
		//-----------------------


		// add k distances to writeToFile
		/*for (int j = 0; j < k; j++) {
			shortPairData spd;
			spd.timestep = t;
			spd.key = distances[j].first;
			spd.distance = distances[j].second;
			#pragma omp critical
			{
				writeToFile.push_back(spd);
			}
		}*/
	}
	/*std::cout << "Back to serial" << std::endl;
	// delete[] c_dcdFile;
	//we are now back to serial
	//at some point we have to write to a file, which must be done in serial.
	//the k smallest distances must be accessible at this time
	// probably in a vector, which must be written to by many threads.
	//open output file for appending

	//TODO fix this sorting...in process
	std::sort(writeToFile.begin(), writeToFile.end(),
	[](const shortPairData p1, const shortPairData p2)->bool{
		if (p1.timestep == p2.timestep) {
			return p1.distance <= p2.distance;
		} else{
			return p1.timestep < p2.timestep;
		}
	});


	// the results seem to be out of order somehow,
	//with calculations correct but timesteps not matching michelle's example output

	std::cout << "writing to output file '" << outputFile << "' " << std::endl;
	std::ofstream outFile(outputFile);
	outFile.precision(15);

	std::cout << "writeToFile.size: " << writeToFile.size() << std::endl;
	for (std::vector<shortPairData>::iterator i = writeToFile.begin(); i != writeToFile.end(); ++i) {
		outFile << i->toString() << std::endl;
	}

	outFile.close();
	std::cout << "writing output complete" << std::endl;*/

	std::cout << "Closing file '" << dcdFile << "'" << std::endl;
	close_file_read(v);
	std::cout << ".dcd file closed" << std::endl;

	delete[] c_dcdFile;

	std::cout << "Complete" << std::endl;
	return 0;
}