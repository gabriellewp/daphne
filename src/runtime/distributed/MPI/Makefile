#this Makefile is only for convenient MPI module testing
#need to be integrated with DAPHNE CMakefiles

CFLAGS = -Wall -ansi -pedantic 
LFLAGS = -lstdc++

mainMPI: mainMPI.cpp MPIWrapper.h
	mpicc $(CFLAGS) mainMPI.cpp -o mainMPI $(LFLAGS)
# MPIProcess: MPIScatter.cpp
# 	mpicc $(CFLAGS) MPIScatter.cpp -o MPIScatter $(LFLAGS)
clean:
	rm -f mainMPI *# *~ *.o PI*