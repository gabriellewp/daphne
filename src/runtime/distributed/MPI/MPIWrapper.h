/*
 * Copyright 2021 The DAPHNE Consortium
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_RUNTIME_DISTRIBUTED_MPIWRAPPER_H
#define SRC_RUNTIME_DISTRIBUTED_MPIWRAPPER_H

#include <ir/daphneir/Daphne.h>

#include "mpi.h"
#include <iostream>
#include <stdlib.h>

using mlir::daphne::VectorSplit;
using mlir::daphne::VectorCombine;

#define N 8
using namespace std;

template <class DT>
class MPIWrapper{
    private:
        int n_procs;
        int whoami;
        char * hostname; 
        MPI_Comm current_comm;
        DCTX(_ctx);
    protected:  
        int getRank(const MPI_Comm & communicator) const;
        int getNumberOfProcesses(const MPI_Comm & communicator) const;
        char * getHostName() const;
        void execute(const char *mlirCode,
                 DT ***res,
                 const Structure **inputs,
                 size_t numInputs, size_t numOutputs, 
                 int64_t *outRows, int64_t *outCols,  
                 VectorSplit *splits, VectorCombine *combines);
        void doComputation(DTRes **&res,
                      size_t numOutputs,
                      const Structure **args,
                      size_t numInputs,
                      const char *mlirCode,
                      VectorCombine *combineVector);
    public:
        MPIWrapper(int & argc, char** & argv){
            //init the workers here
            int rank, size;
            char* hostname;
            MPI_Init(&argc, &argv);
            MPI_Comm_size(MPI_COMM_WORLD, &size);
            MPI_Comm_rank(MPI_COMM_WORLD, &rank);
            current_comm = MPI_COMM_WORLD;
            n_procs = size;
            whoami = rank;
        }
        ~MPIWrapper() = default;
        
        //void freeMatrix(int ** mat);
        //void fill(int n, int m, double ** matrix, int nr);
        //like in DistributedWrapper or MTWrapper
        // void execute(const char *mlirCode,
        //          DT ***res,
        //          const Structure **inputs,
        //          size_t numInputs,
        //          size_t numOutputs,
        //          int64_t *outRows,
        //          int64_t *outCols,
        //          VectorSplit *splits,
        //          VectorCombine *combines)
                //MPI_functionality
        //will need to remove runMPI later
        //int runMPI(int option, int senderRank, int recvRank, int** arr, int row, int col);
        
};

const int HOSTNAME_LENGTH = 64;  //will need to remove this later

// inline void MPIWrapper::fill(int n, int m, double ** matrix, int nr){
//     double * data = (double *) malloc(sizeof(double) * n * m);
//     for (int i = 0; i < n; i++) {
//         for (int j = 0; j < m; j++) {
//             data[i*N + j] = i*N + j;
//         }
//     }
//     *matrix = data;
// }

// inline int MPIWrapper::getRank(const MPI_Comm & communicator) const {
//     int commRank;
//     MPI_Comm_rank(communicator, &commRank);
//     return commRank;
// }

// inline int MPIWrapper::getNumberOfProcesses(const MPI_Comm & communicator) const {
//     int commProcesses;
//     MPI_Comm_size(communicator, &commProcesses);
//     return commProcesses;
// }

inline int MPIWrapper::execute(const char *mlirCode,
                 DT ***res,
                 const Structure **inputs,
                 size_t numInputs, 
                 size_t numOutputs, 
                 int64_t *outRows, 
                 int64_t *outCols,  
                 VectorSplit *splits, 
                 VectorCombine *combines) const {
    //set number of workers
    //do something
    //get env of distributed workers, the env of distributed workers=IP:PORT,IP:PORT,IP:PORT, then we parse this so
    //the each of the worker in the workers vector will have their own IP:PORT


    //create a buffer to store the output, alloct the output for row-wise combine
    //iterate for numberof Inputs, we shhould broadcast or distribute the input here
    //if the onput splits says broadcast then we broadcast, otherwise distribute the inputs. We do not need to implement broadcast or distribute classes like in distributed/coordinator/kernels broadcast and distributed (distributed-ICCS branch)
    //lastly, collect the output
    //to partition the data
    //k = mat->getNumCols() / workersSize;m = mat->getNumCols() % workersSize;

    //in the MPI workers is the rank created by MPI Init, so we dont need to initialize the workers like MTWrapper or DistributedWrapper
    //allocate output based on the numOutputs, outRows, outCols
    for(size_t i = 0; i < numOutputs; ++i) {
            if(*(res[i]) == nullptr && outRows[i] != -1 && outCols[i] != -1) {
                auto zeroOut = combines[i] == mlir::daphne::VectorCombine::ADD;
                // TODO we know result is only DenseMatrix<double> for now,
                // but in the future this will change to support other DataTypes
                *(res[i]) = DataObjectFactory::create<DT>(outRows[i], outCols[i], zeroOut);
            }
    }

    for (auto i = 0u; i < numInputs; ++i) {
        // if already placed on workers, skip
        // TODO maybe this is not enough. We might also need to check if data resides in the specific way we need to.
        // (i.e. rows/cols splitted accordingly). If it does then we can skip.
        // checking whether data is placed among many workers based on their dataPlacement attribute on Structure class (Distributed-ICCS)
        if (inputs[i]->dataPlacement.isPlacedOnWorkers == true)
            continue;

        if (isBroadcast(splits[i], inputs[i])){
            if(whoami == 0){
                MPI_Bcast(&(inputs[i][0][0]), row*col, MPI_INT, 0, current_comm);
                 
            }
        }
        else {
            // we need to something like MPI_Scatter, but how do we partition the matrix? 
            //distribute(inputs[i], _ctx);
            //convert 2d into 1d array
            if(whoami == 0){
                int * data = (int *) malloc(sizeof(int) * row * col);
                for (int q = 0; q < row; q++)
                {
                    for (int t = 0; t < col; t++)
                    {
                        data[q * col + t] = inputs[i][q][t];
                    }
                }
                MPI_Scatter(data, row*col/numberOfProcesses, MPI_INT, temp, row*col/numberOfProcesses, MPI_INT, 0, current_comm);
            }
        }
        // do we need to tag whether the inputs[i] dataPlacement is true?
        // DataPlacement::DistributedMap dataMap;
        // while (!caller.isQueueEmpty()){
        //     auto response = caller.getNextResult();
        //     auto ix = response.storedInfo.ix;
        //     auto workerAddr = response.storedInfo.workerAddr;

        //     auto storedData = response.result;
            
        //     storedData.set_type(response.storedInfo.dataType);
        //     DistributedData data(*ix, storedData);
        //     dataMap[workerAddr] = data;
        // }
        // DataPlacement dataPlacement(dataMap);
        // dataPlacement.isPlacedOnWorkers = true;
        // mat->dataPlacement = dataPlacement;     

    }

    //do the computation, this function can only start if we know the mlir code... 
    doComputation();

    //do results collection
    MPI_Datatype subRows;
    int ** arrlocal;
    arrlocal = new int * [rowChunkSize]; //allocate rows
    MPI_Type_vector(rowChunkSize, col, col, MPI_INT, &subRows);
    MPI_Type_commit(&subRows);
    MPI_Gather(&(arrlocal[0][0]),  rowChunkSize*col, MPI_INT, &(arrglobal[0][0]), 1, subRows, recvRank, MPI_COMM_WORLD);


    
}

inline int MPIWrapper::doComputation(const ::distributed::Task *request,
                                 ::distributed::ComputeResult *response){
    distributed::Task task;
    for (size_t i = 0; i < numInputs; i++) {
        auto map =  args[i]->dataPlacement.getMap();
        *task.add_inputs()->mutable_stored() = map[addr].getData();
    }
    task.set_mlir_code(mlirCode);
    //task actually the request here
    //StoredInfo storedInfo ({addr, nullptr});
    //look into compute in WorkerImpl.cpp (DistributedICCS)
    DaphneUserConfig cfg;
    cfg.use_vectorized_exec = true;
    // TODO Decide if vectorized pipelines should be used on this worker.
    // TODO Decide if selectMatrixReprs should be used on this worker.
    // TODO Once we hand over longer pipelines to the workers, we might not
    // want to hardcode insertFreeOp to false anymore. But maybe we will insert
    // the FreeOps at the coordinator already.
    DaphneIrExecutor executor(false, false, cfg);

    mlir::OwningModuleRef module(mlir::parseSourceString<mlir::ModuleOp>(request->mlir_code(), executor.getContext()));
    if (!module) {
        auto message = "Failed to parse source string.\n";
        llvm::errs() << message;
        return -1;
        //return ::grpc::Status(::grpc::StatusCode::ABORTED, message);
    }

    auto *distOp = module->lookupSymbol(DISTRIBUTED_FUNCTION_NAME);
    mlir::FuncOp distFunc;
    if (!(distFunc = llvm::dyn_cast_or_null<mlir::FuncOp>(distOp))) {
        auto message = "MLIR fragment has to contain `dist` FuncOp\n";
        llvm::errs() << message;
        return -1;
        //return ::grpc::Status(::grpc::StatusCode::ABORTED, message);
    }
    auto distFuncTy = distFunc.getType();

    std::vector<void *> inputs;
    std::vector<void *> outputs;
    auto packedInputsOutputs = createPackedCInterfaceInputsOutputs(distFuncTy,
        request->inputs(),
        outputs,
        inputs);
    
    // Increase the reference counters of all inputs to the `dist` function.
    // (But only consider data objects, not scalars.)
    // This is necessary to avoid them from being destroyed within the
    // function. Note that this increasing is in-line with the treatment of
    // local function calls, where we also increase the inputs' reference
    // counters before the call, for the same reason. See ManageObjsRefsPass
    // for details.
    for(size_t i = 0; i < inputs.size(); i++)
        // TODO Use CompilerUtils::isObjType() once this branch has been rebased.
        // if(CompilerUtils::isObjType(distFuncTy.getInput(i)))
        if(distFuncTy.getInput(i).isa<mlir::daphne::MatrixType, mlir::daphne::FrameType>())
            reinterpret_cast<Structure*>(inputs[i])->increaseRefCounter();

    // Execution
    // TODO Before we run the passes, we should insert information on shape
    // (and potentially other properties) into the types of the arguments of
    // the DISTRIBUTED_FUNCTION_NAME function. At least the shape can be
    // obtained from the cached data partitions in localData_. Then, shape
    // inference etc. should work within this function.
    if (!executor.runPasses(module.get())) {
        std::stringstream ss;
        ss << "Module Pass Error.\n";
        // module->print(ss, llvm::None);
        llvm::errs() << ss.str();
        return -1;
        //return ::grpc::Status(::grpc::StatusCode::ABORTED, ss.str());
    }

    mlir::registerLLVMDialectTranslation(*module->getContext());

    auto engine = executor.createExecutionEngine(module.get());
    if (!engine) {
        return ::grpc::Status(::grpc::StatusCode::ABORTED, "Failed to create JIT-Execution engine");
    }
    auto error = engine->invokePacked(DISTRIBUTED_FUNCTION_NAME,
        llvm::MutableArrayRef<void *>{&packedInputsOutputs[0], (size_t)0});

    if (error) {
        std::stringstream ss("JIT-Engine invocation failed.");
        llvm::errs() << "JIT-Engine invocation failed: " << error << '\n';
        //return ::grpc::Status(::grpc::StatusCode::ABORTED, ss.str());
        return -1;
    }

    for (auto zipped : llvm::zip(outputs, distFuncTy.getResults())) {
        auto output = std::get<0>(zipped);
        auto type = std::get<1>(zipped);

        auto identification = "tmp_" + std::to_string(tmp_file_counter_++);
        localData_[identification] = output;

        distributed::WorkData::DataCase dataCase = dataCaseForType(type);

        distributed::WorkData workData;
        switch (dataCase) {
        case distributed::WorkData::kStored: {
            auto matTy = type.dyn_cast<mlir::daphne::MatrixType>();
            if(matTy.getElementType().isa<mlir::Float64Type>()){
                auto mat = static_cast<Matrix<double> *>(output);
                workData.mutable_stored()->set_num_rows(mat->getNumRows());
                workData.mutable_stored()->set_num_cols(mat->getNumCols());
                if (matTy.getRepresentation() == mlir::daphne::MatrixRepresentation::Sparse)
                    workData.mutable_stored()->set_type(distributed::StoredData::Type::StoredData_Type_CSRMatrix_f64);
                else
                    workData.mutable_stored()->set_type(distributed::StoredData::Type::StoredData_Type_DenseMatrix_f64);
            } else {
                auto mat = static_cast<Matrix<int64_t> *>(output);
                workData.mutable_stored()->set_num_rows(mat->getNumRows());
                workData.mutable_stored()->set_num_cols(mat->getNumCols());
                if (matTy.getRepresentation() == mlir::daphne::MatrixRepresentation::Sparse)
                    workData.mutable_stored()->set_type(distributed::StoredData::Type::StoredData_Type_CSRMatrix_i64);
                else
                    workData.mutable_stored()->set_type(distributed::StoredData::Type::StoredData_Type_DenseMatrix_i64);
            }
            workData.mutable_stored()->set_filename(identification);
            break;
        }
        default: assert(false);
        }
        *response->add_outputs() = workData;
    }
    
}
//we will have our definition of broadcast, collect, send and receive here. so replace runMPI with individual functionalities


// inline int MPIWrapper::runMPI(int option, int senderRank, int recvRank, int** arr, int row, int col){
//     int rowChunkSize;

//     if(option != 3 && option !=4 && option !=5){
//         if(myRank != senderRank && myRank != recvRank) {
//             option = 999;
//         }
//     }

//     cout<<"running option "<<option<<endl;
//     switch(option){
//         case 1:
//         {
//             cout << "do MPI send rank "<<myRank<<"\n";
//             arr[0][0] = 3;
//             if(myRank == senderRank){
//                 MPI_Send(&(arr[0][0]), row*col, MPI_INT, recvRank, 0, MPI_COMM_WORLD);
//             }
//             break;
//         }
//         case 2:
//         {
//             cout << "do MPI recv "<<myRank<<"\n";
//             MPI_Status status;
//             if(myRank == recvRank){
//                 MPI_Recv(&(arr[0][0]), row*col, MPI_INT, senderRank, 0, MPI_COMM_WORLD, &status);
//                 cout<<"received array content"<<endl;
//                 for(int i=0; i<row; i++){
//                     for(int j=0; j<col; j++){
//                         cout<<arr[i][j]<<" ";
//                     }
//                     cout<<endl;
//                 }
//             }
//             break;
//         }
//         case 3:
//         {
            
//             cout << "do MPI broadcast \n";
//             if(myRank==senderRank){
//                 arr[0][0] = 4;
                
//             }
//             MPI_Bcast(&(arr[0][0]), row*col, MPI_INT, senderRank, MPI_COMM_WORLD);
//             cout<<"received array content from broadcast at rank"<<myRank<<endl;
//             for(int i=0; i<row; i++){
//                 for(int j=0; j<col; j++){
//                     cout<<arr[i][j]<<" ";
//                 }
//                 cout<<endl;
//             }
            
//             break;
//         }
//         case 4:
//         {
//             // cout<<"do MPI Scatter at "<<myRank<<endl;
//             rowChunkSize = row / numberOfProcesses;
//             int temp[rowChunkSize][col];
//             //e.g split by row
            
//             cout<<"bleh"<<endl;
//             for(int i=0; i<rowChunkSize; i++){
//                 for(int j=0; j<col; j++){
//                     cout<<"before received data at rank "<<myRank<<":"<<temp[i][j]<<" ";
//                 }
//                 cout<<endl;
//             }

//             //convert 2d into 1d array
//             int * data = (int *) malloc(sizeof(int) * row * col);
//             for (int q = 0; q < row; q++)
//             {
//                 for (int t = 0; t < col; t++)
//                 {
//                     data[q * col + t] = arr[q][t];
//                 }
//             }

//             MPI_Scatter(data, row*col/numberOfProcesses, MPI_INT, temp, row*col/numberOfProcesses, MPI_INT, senderRank, MPI_COMM_WORLD);
            
        
            
//             for(int i=0; i<rowChunkSize; i++){
//                 for(int j=0; j<col; j++){
//                     cout<<"received data at rank "<<myRank<<":"<<temp[i][j]<<" ";
//                 }
//                 cout<<endl;
//             }
//             // int rows;  
//             // double *matrix_A = NULL;
//             // rows = N / numberOfProcesses;

//             // if(myRank == 0){                          
//             //     fill(N, N, &matrix_A, 10);   
//             // }

//             // double cc[rows][N];

//             // MPI_Scatter(matrix_A, N*N/numberOfProcesses, MPI_DOUBLE, cc, N*N/numberOfProcesses, MPI_DOUBLE, 0, MPI_COMM_WORLD);        

//             // for (int i = 0; i < rows; i++) {
//             //     for (int j = 0; j < N; j++) {
//             //         cout<<myRank<<": "<<cc[i][j]<<"  ";
//             //     }
//             //     cout<<endl;
//             // }
//             delete[] data;
//             break;
//         }
//         case 5:
//         {
            
//             //do MPI gather on row basis
//             rowChunkSize = row / numberOfProcesses;
//             // int ** arrglobal; int ** arrlocal;
//             //arrglobal = new int * [row]; //allocate rows
//             // for(int i=0; i< row; i++){
//             //     arrglobal[i] = new int[col]; //allocate cols
//             // }
            
//             int arrglobal[row][col];
//             // const int c_col = col;
//             // int arrglobal  = new int[row][c_col];
            

//             //arrlocal = new int * [rowChunkSize]; //allocate rows
//             int arrlocal[rowChunkSize][col];
//             //int arrlocal = new int[row][c_col];
//             // for(int i=0; i< rowChunkSize; i++){
//             //     arrlocal[i] = new int[col]; //allocate cols
//             // }

//             for(int i=0; i < rowChunkSize; i++){
//                 for(int j=0; j < col; j++){
//                     arrlocal[i][j] = 5+myRank;
//                 }
//             }
            
            

//             MPI_Datatype subRows;
//             MPI_Type_vector(rowChunkSize, col, col, MPI_INT, &subRows);
//             MPI_Type_commit(&subRows);

//             cout<<"do MPI Gather"<<endl;
//             MPI_Gather(&(arrlocal[0][0]),  rowChunkSize*col, MPI_INT, &(arrglobal[0][0]), 1, subRows, recvRank, MPI_COMM_WORLD);
//             if(myRank == recvRank){
//                 for (int i = 0; i < row; i++) {
//                     for (int j = 0; j < col; j++) {
//                         cout<<myRank<<":"<<arrglobal[i][j]<<"  ";
//                     }
//                     cout<<endl;
//                 }
//             }
            

//             //freeMatrix(arrlocal);
//             //freeMatrix(arrglobal);
//             // for (int i = 0; i < row; i++) {
//             //     delete [] arrglobal[i];
//             // }
//             // delete [] arrglobal;
//             // arrglobal = 0;
//             // for (int i = 0; i < rowChunkSize; i++) {
//             //     delete [] arrlocal[i];
//             // }
//             // delete [] arrlocal;
//             //arrlocal = 0;
//             MPI_Type_free(&subRows);


//             //need to handle rows partition if the module with nProc is not equal to 0
//             //lets try without mpi_type_vector, such as converting the matrix into 1d array shape
//             break;
//         }
//         default:
//             cout<<"rank "<<myRank<<" is doing nothing"<<endl;
//             return 0;
//     }
//     return 1;
// }

// inline void MPIWrapper::freeMatrix(int** matrix){
//     free(matrix[0]);

//     free(matrix);
// }



#endif //SRC_RUNTIME_DISTRIBUTED_MPIWRAPPER_H