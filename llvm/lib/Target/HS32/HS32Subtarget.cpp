#include "HS32Subtarget.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "hs32-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "HS32GenSubtargetInfo.inc"

void HS32Subtarget::anchor() { }

HS32Subtarget &
HS32Subtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS,
                                               const TargetMachine &TM) {
  StringRef CPUName = CPU;
  if (CPUName.empty())
    CPUName = "hs32";
  ParseSubtargetFeatures(CPUName, CPUName, FS);
  return *this;
}

HS32Subtarget::HS32Subtarget(const Triple &TT, const StringRef CPU,
                             const StringRef FS, const TargetMachine &TM)
    : HS32GenSubtargetInfo(TT, CPU, CPU, FS),
      FrameLowering(initializeSubtargetDependencies(CPU, FS, TM)),
      InstrInfo(), RegInfo(getHwMode()), TLInfo(TM, *this) { }
