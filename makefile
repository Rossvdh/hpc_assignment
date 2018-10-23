# makefile HPC assignment
# Ross van der Heyde VHYROS001
# 17 September 2018

# serial version that reads all timestep data at once--------------
serial: Simulate_v2.o
	g++ -g -o Simulate_v2 Simulate_v2.o -std=c++11

Simulate_v2.o: Simulate_v2.cpp dcdplugin.c
	g++ -c -g -o Simulate_v2.o Simulate_v2.cpp -std=c++11

runserial: serial
	 ./Simulate_v2 -i example_input_file1.txt -o serial_Output.txt

cleanserial:
	rm Simulate_v2.o
	rm Simulate_v2

#openp version parallelized at timestep------------------------
openmp: omp_v2.o
	g++ -g -fopenmp -o omp_v2 omp_v2.o -std=c++11

omp_v2.o: omp_v2.cpp dcdplugin.c
	g++ -c -g -fopenmp -o omp_v2.o omp_v2.cpp -std=c++11

runopenmp: openmp
	./omp_v2 -i example_input_file1.txt -o openMPOutput.txt

cleanopenmp:
	rm omp_v2.o
	rm omp_v2

#MPI -------------------------------------------------------------------
mpi: mpiCPP.o
	mpic++ -lm -g -o my_mpiCPP my_mpiCPP.o

mpiCPP.o: mpiCPP.cpp
	mpic++ -lm -g -c -o my_mpiCPP.o mpiCPP.cpp -std=c++11

runmpi: mpi
	mpiexec -n 4 my_mpiCPP -i example_input_file1.txt -o mpiOutput.txt

debugmpi: mpiCPP
	gdb --args my_mpiCPP -i example_input_file1.txt -o mpiOutput.txt

cleanmpi:
	rm my_mpiCPP.o
	rm my_mpiCPP
