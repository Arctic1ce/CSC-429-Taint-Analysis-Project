#include "llvm/Transforms/Instrumentation/TaintAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

PreservedAnalyses TaintAnalysis::run(Module &M, ModuleAnalysisManager &AM) {
    errs() << "Analyzing module: " << M.getName() << "\n";
    
    LLVMContext &context = M.getContext();
    Type *Int32Type = Type::getInt32Ty(context);
    Type *VoidPtrType = PointerType::get(Type::getInt8Ty(context), 0);

    Module *module = &M;
    FunctionCallee InsertFunc = module->getOrInsertFunction(
        "__shadowlib_insert",
        FunctionType::get(Type::getVoidTy(context),
            {VoidPtrType, Int32Type},
            false)
    );

    FunctionCallee GetFunc = module->getOrInsertFunction(
        "__shadowlib_get",
        FunctionType::get(Type::getInt32Ty(context),
            {VoidPtrType},
            false)
    );

    FunctionCallee AssertFunc = module->getOrInsertFunction(
        "__shadowlib_assert",
        FunctionType::get(Type::getVoidTy(context),
            {Int32Type, Int32Type, VoidPtrType},
            false)
    );

    FunctionCallee PrintFunc = module->getOrInsertFunction(
        "__shadowlib_print",
        FunctionType::get(Type::getVoidTy(context), {}, false)
    );

    for (Function &F : M) {
        if (F.empty()) continue;

        for (auto &BB : F) {
            for (auto &I : BB) {

                if (CallInst *CI = dyn_cast<CallInst>(&I)) {
                    Function *CalledFunc = CI->getCalledFunction();
                    if (CalledFunc) {
                        StringRef FuncName = CalledFunc->getName();

                        if (FuncName == "scanf" || FuncName == "gets") {
                            errs() << "Found user input function: " << FuncName << " at ";
                            I.getDebugLoc().print(errs());
                            errs() << "\n";

                            Value *DestAddr = CI->getArgOperand(FuncName == "scanf" ? 1 : 0);

                            IRBuilder<> Builder(CI->getNextNode());

                            Value *voidPtr = Builder.CreateBitCast(DestAddr, VoidPtrType);
                            Value *type = ConstantInt::get(Int32Type, 1);
                            Builder.CreateCall(InsertFunc, {voidPtr, type});

                            outs() << "User input! Stored address: " << DestAddr << "\ttainted: " << 1 << "\n";
                        }
                    }
                }

                if (StoreInst *SI = dyn_cast<StoreInst>(&I)) {
                    Value *StoreAddr = SI->getPointerOperand();
                    Value *StoredValue = SI->getValueOperand();
                    
                    IRBuilder<> Builder(&I);
                    Value *voidPtr = Builder.CreateBitCast(StoreAddr, VoidPtrType);
                    
                    // Check if the stored value is a load instruction
                    if (LoadInst *LI = dyn_cast<LoadInst>(StoredValue)) {
                        Value *LoadAddr = LI->getPointerOperand();
                        Value *loadVoidPtr = Builder.CreateBitCast(LoadAddr, VoidPtrType);
                        
                        // Get taint status of loaded value
                        Value *taintStatus = Builder.CreateCall(GetFunc, {loadVoidPtr});
                        
                        // Propagate taint status to stored address
                        Builder.CreateCall(InsertFunc, {voidPtr, taintStatus});
                        
                        outs() << "Propagated taint - Stored address: " << StoreAddr 
                              << " from loaded address: " << LoadAddr << "\n";
                    } else {
                        // Default case (untainted)
                        Value *type = ConstantInt::get(Int32Type, 0);
                        Builder.CreateCall(InsertFunc, {voidPtr, type});
                        
                        outs() << "Stored address: " << StoreAddr << "\ttainted: " << 0 << "\n";
                    }
                }

                if (BinaryOperator *BO = dyn_cast<BinaryOperator>(&I)) {
                    Value *Operand1 = BO->getOperand(0);
                    Value *Operand2 = BO->getOperand(1);

                    IRBuilder<> Builder(&I);

                    // Check if Operand1 is a constant
                    Value *taintStatus1;
                    if (isa<Constant>(Operand1)) {
                        taintStatus1 = ConstantInt::get(Int32Type, 0); // Un-tainted for constants
                    } else {
                        Value *voidPtr1 = Builder.CreateBitCast(Operand1, VoidPtrType);
                        taintStatus1 = Builder.CreateCall(GetFunc, {voidPtr1});
                    }

                    // Check if Operand2 is a constant
                    Value *taintStatus2;
                    if (isa<Constant>(Operand2)) {
                        taintStatus2 = ConstantInt::get(Int32Type, 0); // Un-tainted for constants
                    } else {
                        Value *voidPtr2 = Builder.CreateBitCast(Operand2, VoidPtrType);
                        taintStatus2 = Builder.CreateCall(GetFunc, {voidPtr2});
                    }

                    // Combine taint statuses (taint if either operand is tainted)
                    Value *combinedTaint = Builder.CreateOr(taintStatus1, taintStatus2);

                    // Mark the result of the binary operation as tainted if applicable
                    Value *resultVoidPtr = Builder.CreateBitCast(&I, VoidPtrType);
                    Builder.CreateCall(InsertFunc, {resultVoidPtr, combinedTaint});

                    outs() << "Binary operation result tainted based on operands. Instruction: " << I << "\n";
                }


            }
        }

        if (!F.getEntryBlock().empty()) {
            Instruction *Terminator = F.getEntryBlock().getTerminator();
            if (Terminator) {
                IRBuilder<> Builder(Terminator);
                Builder.CreateCall(PrintFunc);
            }
        }
    }
    
    return PreservedAnalyses::all();
}