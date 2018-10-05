/*CSC4000W HPC assignment
Ross van der Heyde VHYROS001
19 September 2019*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
// #include <stdlib.h>
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

double getTime() {
	// std::chrono::milliseconds prevTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	// double test = prevTime.count();
}

int main(int argc, char const *argv[]) {
	std::cout << "Simulate serial" << std::endl;

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

	//read timesteps
	for (int timestepCounter = 0; timestepCounter < 10; timestepCounter++) {//REMOVE THIS WHEN NOT TESTING
		// for (int timestepCounter = 0; timestepCounter < handle->nsets; timestepCounter++) {
		double prevTime = getTime();

		std::cout << "timestep: " << timestepCounter << std::endl;
		molfile_timestep_t timestep; // data for the current timestep
		timestep.coords = (float *)malloc(3 * sizeof(float) * natoms); //change to normal array?

		//vector of distances
		std::vector<std::pair<std::string, double> > distances;


		if (timestepCounter == 0) {
			//read first time step
			float unitcell[6];
			unitcell[0] = unitcell[2] = unitcell[5] = 0.0f;
			unitcell[1] = unitcell[3] = unitcell[4] = 90.0f;
			//read first timestep
			int rc = read_dcdstep(handle->fd, handle->natoms, handle->x, handle->y, handle->z,
			                      unitcell, handle->nfixed, handle->first, handle->freeind, handle->fixedcoords,
			                      handle->reverse, handle->charmm);
			handle->first = 0;
			handle->setsread++;
			std::cout << "read_dcdstep called" << std::endl;
			if (rc < 0) {
				print_dcderror("read_dcdstep", rc);
				delete [] timestep.coords;
				return MOLFILE_ERROR;
			}
			// int temp = skip_dcdstep(handle->fd, handle->natoms, handle->nfixed, handle->charmm);
			// std::cout << "temp: " << temp << std::endl;
			// std::cout << "skip_dcdstep called" << std::endl;

			//copy handle->x, handle->y and handle->z into timestep.coords
			// timestep.coords is xyz, xyz, xyz ....
			//but you've copied from handle->xyz so that is it xxxxxx..xxx,yyyyyyyy...yyy,zzzzzzz....zzz

			for (int i = 0; i < handle->natoms; ++i) {
				timestep.coords[(3 * i)] = handle->x[i];
				timestep.coords[(3 * i) + 1] = handle->y[i];
				timestep.coords[(3 * i) + 2] = handle->z[i];
			}

			// memcpy(timestep.coords, handle->x, handle->natoms * sizeof(float));
			// memcpy(timestep.coords + handle->natoms, handle->y, handle->natoms * sizeof(float));
			// memcpy(timestep.coords + (2 * handle->natoms), handle->z, handle->natoms * sizeof(float));
		} else {

			int rc = read_next_timestep(handle, natoms, &timestep);

			//check for error
			if (rc) {
				std::cout << "error in read_next_timestep on frame " << timestepCounter << std::endl;
				return 1;
			}
		}

		//calculate distances between those in aIndx and bIndx, put in distances
		for (std::vector<int>::iterator aIt = aIndx.begin(); aIt != aIndx.end(); ++aIt) {
			//co-ords of atom
			float ax, ay, az;
			int a = *aIt;
			// std::cout << "a atom: " << a << std::endl;
			ax = timestep.coords[3 * a];
			ay = timestep.coords[(3 * a) + 1];
			az = timestep.coords[(3 * a) + 2];

			for (std::vector<int>::iterator bIt = bIndx.begin(); bIt != bIndx.end(); ++bIt) {
				//coords of atom
				float bx, by, bz;
				int b = *bIt;
				// std::cout << "b atom: " << b << std::endl;
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
		//You don't need to sort. Just go through array and keep k shortest distances
		std::partial_sort(distances.begin(), distances.begin() + k, distances.end(),
		                  [](const std::pair<std::string, double>& lhs, const std::pair<std::string, double>& rhs)
		                  ->bool{return lhs.second <= rhs.second;});
		//maybe use dv&c algorithm k times, each time discarding the closest pair?

		totalTime += (getTime() - prevTime);

		//open output file for appending
		std::ofstream outFile(outputFile, std::ios_base::app);
		outFile.precision(15);

		//write k shorted distances to file
		for (int i = 0; i < k; ++i) {
			outFile << timestepCounter << "," << distances[i].first << ","
			        << distances[i].second << "\n";
		}
		outFile.close();

		delete[] timestep.coords;//not sure if this is necessary
	}

	std::cout << "atoms Time: " << (totalTime / 1000) << " s" << std::endl;
	double temp2 = getTime() - initTime;
	std::cout << "total time: " << (temp2 / 1000) << " s" << std::endl;

	std::cout << "Output written to " << outputFile << std::endl;

	//close file
	std::cout << "Closing file '" << dcdFile << "'" << std::endl;
	close_file_read(v);
	std::cout << ".dcd file closed" << std::endl;

	delete[] c_dcdFile;

	std::cout << "Complete" << std::endl;
	return 0;
}