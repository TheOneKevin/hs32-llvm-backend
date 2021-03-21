#include "HS32TargetMachine.h"
#include "TargetInfo/HS32TargetInfo.h"

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

extern "C" void LLVMInitializeHS32Target() {
  RegisterTargetMachine<HS32TargetMachine> X(getTheHS32Target());
}

namespace {

std::string computeDataLayout(const Triple &TT, StringRef CPU,
                              const TargetOptions &Options) {
  std::string Ret = "";

  // Big endian
  Ret += "E";

  // ELF name mangling
  Ret += "-m:e";

  // Pointer alignments
  Ret += "-p:32:32";

  // Integer alignments
  Ret += "-i8:8:8-i16:8:32-i32:8:32";

  // Only 32 bit registers
  Ret += "-n32";

  // Aggregate types and alignment
  Ret += "-a:0:32";

  // Stack alignment
  Ret += "-S32";

  return Ret;
}

Reloc::Model getEffectiveRelocModel(const Triple &TT,
                                    Optional<Reloc::Model> RM) {
  // If not defined we default to static
  if (!RM.hasValue()) {
    return Reloc::Static;
  }
  return *RM;
}

CodeModel::Model getEffectiveCodeModel(Optional<CodeModel::Model> CM,
                                       bool JIT) {
  if (!CM) {
    return CodeModel::Small;
  } else if (CM == CodeModel::Large) {
    llvm_unreachable("Large code model is not supported");
  } else if (CM == CodeModel::Kernel) {
    llvm_unreachable("Kernel code model is not implemented yet");
  }
  return CM.getValue();
}

} // end anonymous namespace

HS32TargetMachine::HS32TargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     Optional<Reloc::Model> RM,
                                     Optional<CodeModel::Model> CM,
                                     CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, computeDataLayout(TT, CPU, Options), TT, CPU, FS,
                        Options, getEffectiveRelocModel(TT, RM),
                        ::getEffectiveCodeModel(CM, false), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()) {
  initAsmInfo();
}

TargetPassConfig *HS32TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new TargetPassConfig(*this, PM);
}
