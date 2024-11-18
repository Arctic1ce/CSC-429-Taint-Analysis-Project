#include "llvm/Transforms/Instrumentation/TaintAnalysis.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

PreservedAnalyses TaintAnalysis::run(Function &F, FunctionAnalysisManager &AM) {
    errs() << "Analyzing function: " << F.getName() << "\n";
    
    for (auto &BB : F) {
        for (auto &I : BB) {
            if (CallInst *CI = dyn_cast<CallInst>(&I)) {
                Function *CalledFunc = CI->getCalledFunction();
                if (CalledFunc) {
                    StringRef FuncName = CalledFunc->getName();

                    // check for scaf and gets function calls
                    if (FuncName == "scanf" || 
                        FuncName == "gets") {
                        errs() << "Found user input function: " << FuncName << " at ";
                        I.getDebugLoc().print(errs());
                        errs() << "\n";

                        // for scanf, the first argument is the format string, subsequent arguments are pointers
                        // for gets, the first and only argument is the destination buffer
                        Value *DestAddr = CI->getArgOperand(FuncName == "scanf" ? 1 : 0);
                        errs() << "Destination address: ";
                        DestAddr->print(errs());
                        errs() << "\n";
                    }
                }
            }
        }
    }
    
    return PreservedAnalyses::all();
}
