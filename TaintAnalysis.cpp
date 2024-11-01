#include "llvm/Transforms/Instrumentation/TaintAnalysis.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

PreservedAnalyses TaintAnalysis::run(Function &F, FunctionAnalysisManager &AM) {
    return PreservedAnalyses::all();
}
