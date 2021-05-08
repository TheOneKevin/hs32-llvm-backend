#ifndef LLVM_HS32SUBTARGET_H
#define LLVM_HS32SUBTARGET_H

#include "HS32FrameLowering.h"
#include "HS32ISelLowering.h"
#include "HS32InstrInfo.h"

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
  HS32FrameLowering FrameLowering;
  HS32InstrInfo InstrInfo;
  HS32RegisterInfo RegInfo;
  HS32TargetLowering TLInfo;
  SelectionDAGTargetInfo TSInfo;

  HS32Subtarget &initializeSubtargetDependencies(StringRef CPU, StringRef FS,
                                                 const TargetMachine &TM);

public:
  HS32Subtarget(const Triple &TT, const StringRef CPU,
                const StringRef FS, const TargetMachine &TM);

  // Defined in "HS32GenSubtargetInfo.inc"
  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);

  const HS32FrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }

  const HS32InstrInfo *getInstrInfo() const override {
    return &InstrInfo;
  }

  const HS32TargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }

  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }

  const HS32RegisterInfo *getRegisterInfo() const override {
    return &RegInfo;
  }
};

} // namespace llvm

#endif // LLVM_HS32SUBTARGET_H
