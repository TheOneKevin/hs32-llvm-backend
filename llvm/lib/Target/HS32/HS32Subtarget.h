#ifndef LLVM_HS32SUBTARGET_H
#define LLVM_HS32SUBTARGET_H

#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/MC/MCInstrItineraries.h"
#include "llvm/Support/Alignment.h"

#define GET_SUBTARGETINFO_HEADER
#include "HS32GenSubtargetInfo.inc"


namespace llvm {
class StringRef;

class HS32Subtarget : public HS32GenSubtargetInfo {
  virtual void anchor();

public:
  HS32Subtarget(const Triple &TT, const std::string &CPU,
                const std::string &FS, const TargetMachine &TM);

  // Defined in "HS32GenSubtargetInfo.inc"
  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);
};

} // namespace llvm

#endif // LLVM_HS32SUBTARGET_H
