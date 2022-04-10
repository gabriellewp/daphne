#include <mpi.h>
#include <stdio.h>
#include <iostream>

const std::string dirPath = "test/runtime/distributed/worker/";

//this maybe very similar to the one in grpc
TEMPLATE_PRODUCT_TEST_CASE("Simple distributed worker functionality test", TAG_DISTRIBUTED, (DenseMatrix), (double))
{
    using DT = TestType;
    using VT = typename DT::VT;
    MPIWrapper mpiWrapper;
    WHEN ("Sending a task where no outputs are expected")
    {
        distributed::Task task;
        task.set_mlir_code("func @" + WorkerImpl::DISTRIBUTED_FUNCTION_NAME +
            "() -> () {\n"
            "  \"daphne.return\"() : () -> ()\n"
            "}\n");
        //where is the indication that the data should be distributed/broadcasted    
        int status = mpiWorker.compute(task, result); 
        
        cout << "status: " << status << endl;
    }

    WHEN("Sending simple random generation task")
    {
        distributed::Task task;
        task.set_mlir_code("func @" + WorkerImpl::DISTRIBUTED_FUNCTION_NAME +
            "() -> !daphne.Matrix<?x?xf64> {\n"
            "    %3 = \"daphne.constant\"() {value = 2 : si64} : () -> si64\n"
            "    %4 = \"daphne.cast\"(%3) : (si64) -> index\n"
            "    %5 = \"daphne.constant\"() {value = 3 : si64} : () -> si64\n"
            "    %6 = \"daphne.cast\"(%5) : (si64) -> index\n"
            "    %7 = \"daphne.constant\"() {value = 1.000000e+02 : f64} : () -> f64\n"
            "    %8 = \"daphne.constant\"() {value = 2.000000e+02 : f64} : () -> f64\n"
            "    %9 = \"daphne.constant\"() {value = 1.000000e+00 : f64} : () -> f64\n"
            "    %10 = \"daphne.constant\"() {value = -1 : si64} : () -> si64\n"
            "    %11 = \"daphne.randMatrix\"(%4, %6, %7, %8, %9, %10) : (index, index, f64, f64, f64, si64) -> !daphne.Matrix<?x?xf64>"
            "    \"daphne.return\"(%11) : (!daphne.Matrix<?x?xf64>) -> ()\n"
            "  }");

        int status = mpiWrapper.execute(task, result);
        cout << "status: " << status << endl;
    }
}