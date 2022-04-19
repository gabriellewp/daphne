//this should be the worker implementation like grpc 
#ifndef SRC_RUNTIME_DISTRIBUTED_MPIWORKER_H
#define SRC_RUNTIME_DISTRIBUTED_MPIWORKER_H
#include <ir/daphneir/Daphne.h>
class MPIWorker{
    private:
        int rankId;
    public:
    void setWorkerRank(int rank);
    int compute(const ::distributed::Task *request,
                                 ::distributed::ComputeResult *response);
}

inline int MPIWorker::compute(const ::distributed::Task *request,
                                 ::distributed::ComputeResult *response){

    //the content of this MPIWorker compute is almost similar to the distributedcompute kernel. But since distributecompute kernel only has computation by grpc, we put this class on this one
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
#endif //SRC_RUNTIME_DISTRIBUTED_MPIWORKER_H