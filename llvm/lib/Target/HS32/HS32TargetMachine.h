#ifndef LLVM_HS32TARGETMACHINE_H
#define LLVM_HS32TARGETMACHINE_H

#include "HS32Subtarget.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class HS32TargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  HS32Subtarget Subtarget;

public:
  HS32TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                    CodeGenOpt::Level OL, bool JIT);

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  const HS32Subtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }

  const HS32Subtarget *getSubtargetImpl() const {
    return &Subtarget;
  }

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};

} // namespace llvm

#endif // LLVM_HS32TARGETMACHINE_H
