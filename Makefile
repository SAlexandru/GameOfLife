serial:
	g++ -std=c++11 -Wall -Wextra -O2 src/main.cpp -o bin/main

openmp:
	g++ -std=c++11 -Wall -Wextra -O2 src/main_openmp.cpp -o bin/main_openmp -fopenmp

openmpi:
	mpic++ -std=c++11 -Wall -Wextra -O2 -I/usr/local/Cellar/boost/1.63.0/include/boost  -L/usr/local/Cellar/boost/1.63.0/lib/ -lboost_mpi -lboost_serialization src/main_openmpi.cpp -o bin/main_openmpi

generator:
	g++ -std=c++11 -Wall -Wextra -O2 src/generator.cpp -o bin/generator

all: serial openmp openmpi generator

clean:
	rm -rf bin/*
	rm -rf src/*.o
	rm -rf output/*


