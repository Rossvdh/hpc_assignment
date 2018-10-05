# makefile HPC assignment
# Ross van der Heyde VHYROS001
# 17 September 2018

Simulate: Simulate.o
	g++ -g -o Simulate Simulate.o -std=c++11

Simulate.o: Simulate.cpp dcdplugin.c
	g++ -c -g -o Simulate.o Simulate.cpp -std=c++11

debug: Simulate
	gdb Simulate.exe

run: Simulate
	 ./Simulate -i example_input_file1.txt -o outPutExample1.txt      
# ./Simulate -i testInput.txt -o testOutput.txt

openmp: SimulateOMP.o
	g++ -g -fopenmp -o SimulateOMP SimulateOMP.o -std=c++11

SimulateOMP.o: SimulateOMP.cpp dcdplugin.c
	g++ -c -g -fopenmp -o SimulateOMP.o SimulateOMP.cpp -std=c++11

runopenmp: openmp
	./SimulateOMP -i example_input_file1.txt -o openMPoutput.txt 

test: Test.c
	g++ -g -o Test Test.c

runTest: test
	./test

clean:
	rm *.o
	rm Simulate
#	rm *.exe
