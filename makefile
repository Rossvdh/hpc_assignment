# makefile HPC assignment
# Ross van der Heyde VHYROS001
# 17 September 2018

Simulate: Simulate.o
	g++ -g -o Simulate Simulate.o -std=c++11

Simulate.o: Simulate.cpp
	g++ -c -g -o Simulate.o Simulate.cpp -std=c++11

debug: Simulate
	gdb Simulate.exe

run: Simulate
	./Simulate -i testInput.txt -o testOutput.txt

clean:
	rm *.o
	rm *.exe