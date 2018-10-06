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

#MPI version to come

clean:
	rm *.o
	rm Simulate
#	rm *.exe
