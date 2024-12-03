#include "llvm/Transforms/Instrumentation/TaintAnalysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/AbstractCallSite.h"
#include "../../libc/examples/lab2/dynamic_type_checking/shadowlib.h"

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
            {VoidPtrType, Int32Type, PointerType::get(Type::getInt8Ty(context), 0)},
            false)
    );

    FunctionCallee GetFunc = module->getOrInsertFunction(
        "__shadowlib_get",
        FunctionType::get(Type::getInt32Ty(context),
            {VoidPtrType},
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
                            StringRef VarName;
                            if (auto *AllocInst = dyn_cast<AllocaInst>(DestAddr)) {
                                outs() << "Variable name: " << AllocInst->getName() << "\n";
                                VarName = AllocInst->getName();
                            } else if (LoadInst *LI = dyn_cast<LoadInst>(DestAddr)) {
                                outs() << "Variable name 2: " << LI->getPointerOperand()->getName() << "\n";
                                VarName = LI->getPointerOperand()->getName();
                            }

                            IRBuilder<> Builder(CI->getNextNode());

                            Value *voidPtr = Builder.CreateBitCast(DestAddr, VoidPtrType);
                            Value *type = ConstantInt::get(Int32Type, 1);
                            //Builder.CreateCall(InsertFunc, {voidPtr, type});
                            //Value *varName = ConstantDataArray::getString(context, "user_input");
                            Value *varName = ConstantDataArray::getString(context, VarName);
                            Value *varNamePtr = Builder.CreateGlobalStringPtr(VarName);
                            Builder.CreateCall(InsertFunc, {voidPtr, type, varNamePtr});

                            outs() << "User input! Stored address: " << DestAddr << "\ttainted: " << 1 << "\n";
                        } else {
                            if (FuncName != "printf") {
                                outs() << "Function: " << FuncName << "\n";

                                IRBuilder<> Builder(&I);
                                
                                for (unsigned i = 0; i < CI->arg_size(); ++i) {
                                    Value *ArgAddr = CI->getArgOperand(i);

                                    if (LoadInst *LI = dyn_cast<LoadInst>(ArgAddr)) {
                                        Value *LoadAddr = LI->getPointerOperand();
                                        StringRef VarName = LoadAddr->getName();

                                        outs() << "VarName: " << VarName << "\n";

                                        outs() << *LoadAddr << "\n";

                                        outs() << "ITS A LOAD!\n";
                                        Value *argVoidPtr = Builder.CreateBitCast(LoadAddr, VoidPtrType);
                                        outs() << "Address CASTED\n";

                                        Value *type = ConstantInt::get(Int32Type, 1);
                                        Value *varName = ConstantDataArray::getString(context, VarName);
                                        Value *varNamePtr = Builder.CreateGlobalStringPtr(VarName);
                                        Builder.CreateCall(InsertFunc, {argVoidPtr, type, varNamePtr});
                                        outs() << "Tainted argument of function: " << FuncName << "\n";
                                    }
                                }
                            }
                        }
                    }
                } else if (StoreInst *SI = dyn_cast<StoreInst>(&I)) {
                    Value *StoreAddr = SI->getPointerOperand();
                    Value *StoredValue = SI->getValueOperand();
                    StringRef VarName = StoreAddr->getName();

                    outs() << "=====Stored Variable Name: " << VarName << "======\n";
                    
                    IRBuilder<> Builder(&I);
                    Value *voidPtr = Builder.CreateBitCast(StoreAddr, VoidPtrType);
                    
                    //LoadInst *LI = dyn_cast<LoadInst>(StoredValue)
                    if (BinaryOperator *BO = dyn_cast<BinaryOperator>(StoredValue)) {
                        outs() << "checking binary operator\n";
                        Value *Operand1 = BO->getOperand(0);
                        Value *Operand2 = BO->getOperand(1);

                        IRBuilder<> Builder(&I);

                        Value *taintStatus1;
                        if (isa<Constant>(Operand1)) {
                            taintStatus1 = ConstantInt::get(Int32Type, 0); // Un-tainted for constants
                        } else if (LoadInst *LI = dyn_cast<LoadInst>(Operand1)) {
                            Value *LoadAddr = LI->getPointerOperand();

                            outs() << *LoadAddr << "\n";

                            outs() << "ITS A LOAD!\n";
                            Value *loadVoidPtr = Builder.CreateBitCast(LoadAddr, VoidPtrType);
                            outs() << "Address CASTED\n";
                            
                            // Get taint status of loaded value
                            taintStatus1 = Builder.CreateCall(GetFunc, {loadVoidPtr});
                        }

                        Value *taintStatus2;
                        if (isa<Constant>(Operand2)) {
                            taintStatus2 = ConstantInt::get(Int32Type, 0); // Un-tainted for constants
                        } else if (LoadInst *LI = dyn_cast<LoadInst>(Operand2)) {
                            Value *LoadAddr = LI->getPointerOperand();

                            outs() << *LoadAddr << "\n";

                            outs() << "ITS A LOAD!\n";
                            Value *loadVoidPtr = Builder.CreateBitCast(LoadAddr, VoidPtrType);
                            outs() << "Address CASTED\n";
                            
                            // Get taint status of loaded value
                            taintStatus2 = Builder.CreateCall(GetFunc, {loadVoidPtr});
                        }
                        
                        Value *combinedTaint = Builder.CreateOr(taintStatus1, taintStatus2);

                        // Propagate taint status to stored address
                        Value *varName = ConstantDataArray::getString(context, VarName);
                        Value *varNamePtr = Builder.CreateGlobalStringPtr(VarName);
                        Builder.CreateCall(InsertFunc, {voidPtr, combinedTaint, varNamePtr});
                        
                        outs() << "Propagated taint BinaryOperator - Stored address: " << StoreAddr << "\n";
                    } else if (LoadInst *LI = dyn_cast<LoadInst>(StoredValue)) { // Check if the stored value is a load instruction
                        Value *LoadAddr = LI->getPointerOperand();
                        Value *loadVoidPtr = Builder.CreateBitCast(LoadAddr, VoidPtrType);
                        
                        // Get taint status of loaded value
                        Value *taintStatus = Builder.CreateCall(GetFunc, {loadVoidPtr});
                        
                        // Propagate taint status to stored address
                        Value *varName = ConstantDataArray::getString(context, VarName);
                        Value *varNamePtr = Builder.CreateGlobalStringPtr(VarName);
                        Builder.CreateCall(InsertFunc, {voidPtr, taintStatus, varNamePtr});
                        
                        outs() << "Propagated taint - Stored address: " << StoreAddr 
                              << " from loaded address: " << LoadAddr << "\n";
                    } else if (CallInst *CI = dyn_cast<CallInst>(StoredValue)) {
                        // Mark the stored result as tainted
                        Value *type = ConstantInt::get(Int32Type, 1);
                        Value *varName = ConstantDataArray::getString(context, VarName);
                        Value *varNamePtr = Builder.CreateGlobalStringPtr(VarName);
                        Builder.CreateCall(InsertFunc, {voidPtr, type, varNamePtr});
                        outs() << "Function call result stored at address: " << StoreAddr << "\ttainted: " << 1 << "\n";
                    } else {
                        // Default case (untainted)
                        Value *type = ConstantInt::get(Int32Type, 0);
                        Value *varName = ConstantDataArray::getString(context, VarName);
                        Value *varNamePtr = Builder.CreateGlobalStringPtr(VarName);
                        Builder.CreateCall(InsertFunc, {voidPtr, type, varNamePtr});
                        
                        outs() << "Default Stored address: " << StoreAddr << "\ttainted: " << 0 << "\n";
                    }
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