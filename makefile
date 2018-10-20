# makefile HPC assignment
# Ross van der Heyde VHYROS001
# 17 September 2018

# serial version -------------------------------------------------
serial: Simulate.o
	g++ -g -o Simulate Simulate.o -std=c++11

Simulate.o: Simulate.cpp dcdplugin.c
	g++ -c -g -o Simulate.o Simulate.cpp -std=c++11

debug: Simulate
	gdb Simulate.exe

runserial: Simulate
	 ./Simulate -i example_input_file1.txt -o serialOutput1.txt
# ./Simulate -i testInput.txt -o testOutput.txt

# serial version that reads all timestep data once--------------
serialv2: Simulate_v2.o
	g++ -g -o Simulate_v2 Simulate_v2.o -std=c++11

Simulate_v2.o: Simulate_v2.cpp dcdplugin.c
	g++ -c -g -o Simulate_v2.o Simulate_v2.cpp -std=c++11

runserialv2: serialv2
	 ./Simulate_v2 -i example_input_file1.txt -o serial_v2_Output1.txt

# openmp version parallelized at a ---------------
openmp: SimulateOMP.o
	g++ -g -fopenmp -o SimulateOMP SimulateOMP.o -std=c++11

SimulateOMP.o: SimulateOMP.cpp dcdplugin.c
	g++ -c -g -fopenmp -o SimulateOMP.o SimulateOMP.cpp -std=c++11

runopenmp: openmp
	./SimulateOMP -i example_input_file1.txt -o openMPoutput.txt 

#openp version parallelized at timestep------------------------
openmptimestep: omp_v2.o
	g++ -g -fopenmp -o omp_v2 omp_v2.o -std=c++11

omp_v2.o: omp_v2.cpp dcdplugin.c
	g++ -c -g -fopenmp -o omp_v2.o omp_v2.cpp -std=c++11

runopenmptimestep: openmptimestep
	./omp_v2 -i example_input_file1.txt -o openMPTimestepOutput.txt

#MPI -------------------------------------------------------------------
mpi: my_mpi.o library
	mpicc -g -o my_mpi my_mpi.o -I ./library/ -L ./library/ -llibrary

library: library.o
	g++ -g -o ./library/liblibrary.so ./library/library.o -fPIC -shared -std=c++11

library.o: ./library/library.cpp
	g++ -g -c -o ./library/library.o ./library/library.cpp -fPIC -shared -std=c++11


my_mpi.o: my_mpi.c
	mpicc -g -c -o my_mpi.o my_mpi.c -I ./library/ -L ./library/ -llibrary

runmpi: mpi
	export LD_LIBRARY_PATH=library/ && mpiexec -n 4 my_mpi -i example_input_file1.txt -o mpiOutput.txt

debugmpi:
	export LD_LIBRARY_PATH=library/ && gdb --args my_mpi -i example_input_file1.txt -o mpiOutput.txt
# ---------------------------------------------------------------------
test: fileTest.c
	gcc -g -o fileTest fileTest.c

runtest: test
	./fileTest -i example_input_file1.txt


# ---------------------------------------------------------------------
clean:
	rm *.o
	rm Simulate
	rm my_mpi
#	rm *.exe
