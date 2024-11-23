#ifndef LLVM_TRANSFORMS_INSTRUMENTATION_TAINTANALYSIS_H
#define LLVM_TRANSFORMS_INSTRUMENTATION_TAINTANALYSIS_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class TaintAnalysis : public PassInfoMixin<TaintAnalysis> {
public:
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // namespace llvm

#endif