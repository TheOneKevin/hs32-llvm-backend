#include "HS32Subtarget.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "hs32-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "HS32GenSubtargetInfo.inc"

void HS32Subtarget::anchor() { }

HS32Subtarget::HS32Subtarget(const Triple &TT, const std::string &CPU,
                             const std::string &FS, const TargetMachine &TM)
    : HS32GenSubtargetInfo(TT, CPU,CPU, FS) { }

