#ifndef LLVM_HS32TARGETMACHINE_H
#define LLVM_HS32TARGETMACHINE_H

#include "llvm/Target/TargetMachine.h"

namespace llvm {

class HS32TargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;

public:
  HS32TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                    CodeGenOpt::Level OL, bool JIT);

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};

} // namespace llvm

#endif // LLVM_HS32TARGETMACHINE_H
