/*
 *  Copyright 2021 The DAPHNE Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "MPIProcess.h"
void compute(int n_row, int n_col, double ** matrix){
    //double * data = (double *) malloc(sizeof(double) * n * m);
    for (int i = 0; i < n_row; i++) {
        for (int j = 0; j < n_col; j++) {
            cout << matrix[i*N + j]; << ", ";
        }
        cout << endl;
    }
 
}
int main (int argc, char ** argv) {
    //init the workers here
    //     //init 2d array
    //     int ** arr;
    //     int row = 4; int col = 3;
    arr = new int * [row]; //allocate rows
    for(int i=0; i< row; i++){
        arr[i] = new int[col]; //allocate cols
    }

    for(int i=0; i<row; i++){
        for(int j=0; j<col; j++){
            arr[i][j] = 5;
        }
    }

//    // mpiProc.runMPI(1, 1, 2, arr, row, col);
//    // mpiProc.runMPI(2, 1, 2, arr, row, col);
//    // mpiProc.runMPI(3, 0, 2, arr, row, col);
//    // mpiProc.runMPI(4, 0, 2, arr, row, col);
//     mpiProc.runMPI(5, NULL, 0, NULL, row, col);
    
//     mpiProc.freeMatrix(arr);
    int rank, size;
    char* hostname;
    //building the server like grpc main.cpp
    MPI_Request request;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    hostname = new char[HOSTNAME_LENGTH];
    int nameLength;
    MPI_Get_processor_name(hostname, &nameLength);
    cout << rank << " on " <<  hostname << " is initialized!" <<endl;
    int rowChunkSize = row / size;

    // if(rank != 0){
    //     float message;
    //     int index;
    //     MPI_Status status;
    //     while(1){
    //         message = (random()/(double)1147483648)+rank;
    //         //MPI_Send(&(arr[0][0]), row*col, MPI_INT, recvRank, 0, MPI_COMM_WORLD);
    //         //we need to partition the matrix,  split by??
    //         //rather than using MPI_Isend maybe we can use MPI_Iscatter? but will it limit the partitioning only to by row (not sure if the partition by column is allowed)? and whether the partition chunk can be dynamic when using MPI_Iscatter
    //         MPI_Isend(&message, 1, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, &request);

    //         if(message > 1.5) //keep looping until the message generate 1.5
    //         cout<<"Sending data from the process: "<< rank +0.1 << ", message:" <<  message<<endl;
    //         break;
    //     }
    // }
    // else{ //rank masters receiving the data
    //     int dataOut = 13, pr;
    //     float dataIn = -1;
    //     //requestList = (MPI_Request*)malloc((size-1)*sizeof(MPI_Request));
    //     while(1){
    //         dataIn = -1; //allocated buffer to receive the message, can be 2d matrix as well
    //         for(int proc=1;proc<size;proc++){
    //             MPI_Recv(&dataIn, 1, MPI_FLOAT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD,&status);
    //             cout<<"Receiving data from the process: " <<proc <<", message:"<< dataIn<<endl;
                
    //         }
    //         //MPI_Wait(&request,&status);
    //         if(dataIn > 1.5){
    //             //cout<<"Receiving data from the process:"<< dataIn<<endl;
    //             break;
    //         }
    //     }
    // }
    MPI_Finalize();
    delete [] hostname;
    return 0;
}