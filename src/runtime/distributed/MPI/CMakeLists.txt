# Copyright 2021 The DAPHNE Consortium
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# *****************************************************************************
# MPI compilation
# *****************************************************************************
set(GCC_COVERAGE_COMPILE_FLAGS "-Wall -ansi -pedantic")
set(GCC_COVERAGE_COMPILE_FLAGS "-lstdc++")
find_package(MPI REQUIRED)
# *****************************************************************************
# MPI app
# *****************************************************************************
set(SOURCES MPIWorkerImpl.h MPIWrapper.h ../../../compiler/execution/DaphneIrExecutor.cpp)
get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(conversion_libs GLOBAL PROPERTY MLIR_CONVERSION_LIBS)
set(LIBS
        ${dialect_libs}
        ${conversion_libs}
        MLIRDaphne
        MLIRDaphneTransforms
        MLIRExecutionEngine
        #DaphneIrExecutor
        DataStructures
        IO
        #CallData
        #Proto
        )

add_library(MPIImpl ${SOURCES})
#llvm_update_compile_flags(WorkerImpl)
target_link_libraries(MPIImpl PRIVATE ${LIBS})

# *****************************************************************************
# DistributedWorker executable
# *****************************************************************************

add_executable(DistributedWorkerMPI main.cpp)
target_link_libraries(DistributedWorkerMPI
        PRIVATE
        #Proto
        MPIImpl
        MLIRDaphne)



# add this line only when you are using openmpi which has a different c++ bindings
#add_definitions(-DOMPI_SKIP_MPICXX)

set(LIBS DataStructures MPI_C MLIRDaphne)

add_executable(mainMPI mainMPI.cpp)
target_link_libraries(mainMPI PRIVATE ${LIBS})

