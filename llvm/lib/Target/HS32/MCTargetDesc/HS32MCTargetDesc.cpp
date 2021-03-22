#include "HS32MCTargetDesc.h"
#include "HS32MCAsmInfo.h"
#include "TargetInfo/HS32TargetInfo.h"

#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MachineLocation.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_REGINFO_MC_DESC
#include "HS32GenRegisterInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "HS32GenSubtargetInfo.inc"

#define GET_INSTRINFO_MC_DESC
#include "HS32GenInstrInfo.inc"

static MCInstrInfo *createHS32MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitHS32MCInstrInfo(X); // in HS32GenInstrInfo.inc
  return X;
}

static MCRegisterInfo *createHS32MCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitHS32MCRegisterInfo(
      X, HS32::R0, 0, 0, HS32::PC
      ); // in HS32GenRegInfo.inc
  return X;
}

static MCAsmInfo *createHS32MCAsmInfo(const MCRegisterInfo &MRI,
                                      const Triple &TT,
                                      const MCTargetOptions &TO) {
  MCAsmInfo *MAI = new HS32ELFMCAsmInfo(TT);
  return MAI;
}

static MCSubtargetInfo *createHS32SubtargetInfo(const Triple &TT,
                                                StringRef CPU, StringRef FS) {
  return createHS32MCSubtargetInfoImpl(TT, CPU, CPU, FS);
}

extern "C" void LLVMInitializeHS32TargetMC() {
  Target &T = getTheHS32Target();

  // Register the MC asm info.
  RegisterMCAsmInfoFn X(T, createHS32MCAsmInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(T, createHS32MCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(T, createHS32MCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(T, createHS32SubtargetInfo);

  // Register the code emitter.
  TargetRegistry::RegisterMCCodeEmitter(T, createHS32MCCodeEmitter);

  // Register the asm backend.
  TargetRegistry::RegisterMCAsmBackend(T, createHS32AsmBackend);
}
