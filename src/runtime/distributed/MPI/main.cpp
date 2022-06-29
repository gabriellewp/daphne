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
#include "MPIWrapper.h"
#include <string>
using namespace std;
string_code hashit (std::string const& inString) {
    if (inString == "compute") return compute;
    if (inString == "broadcast") return broadcast;
    if (inString == "distribute") return distribute;
    
}
int main (int argc, char ** argv) { //should be initialized by 2 workers
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    string command = "broadcast"; int command_length = 0;
    bool waiting = true;
    MPI_Comm child_comm;

    while (waiting){
        if(myrank==0){
            //receive the mlir code or whatever from daphne
            MPI_Send(message, message.size(), MPI_CHAR, 1, 0, MPI_COMM_WORLD);  
        }else
            MPI_Status status;
            //probe for an incoming message from process zero
            MPI_PROBE(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            //count the buffer size of incoming message
            MPI_Get_count(&status, MPI_INT, &command_length);
            char* command_buf = new char[command_length];
            MPI_Recv(&command_buf, command_length, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            cout<<"rank "<<rank<<"receive message from rank"<<0<<"commanding for "<<command_buf;
            switch (hashit(command_buf)):
                case compute:
                    cout << "do compute" <<endl;
                    //MPIWorker.compute();
                case broadcast: 
                    //spawn processes at another comm, and broadcast to them
                    MPI_Comm_spawn( "./worker", MPI_ARGV_NULL, num_workers, MPI_INFO_NULL,
                    0, MPI_COMM_WORLD,
                    &child_comm, MPI_ERRCODES_IGNORE );
                    MPI_Bcast(&command_buf, command_length, MPI_CHAR, MPI_ROOT, child_comm);

                case distribute:
                    //spawn processes at another comm, and broadcast to them
                    MPI_Comm_spawn( "./worker", MPI_ARGV_NULL, num_workers, MPI_INFO_NULL,
                    0, MPI_COMM_WORLD,
                    &child_comm, MPI_ERRCODES_IGNORE );
                    for(int i= 0; i <num_workers; i++){
                        MPI_Send(&command_buf, command_length, MPI_CHAR, num_workers, 0, child_comm);
                    } 
                default:
                    cout << "do nothing" << endl;

            //MPI Worker do something
            delete[] command_buf;
        }
        
    }
    MPI_Finalize();
    return 0;
}