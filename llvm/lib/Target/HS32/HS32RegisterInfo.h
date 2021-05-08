#ifndef LLVM_HS32REGISTERINFO_H
#define LLVM_HS32REGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "HS32GenRegisterInfo.inc"

namespace llvm {

struct HS32RegisterInfo : public HS32GenRegisterInfo {
  HS32RegisterInfo(unsigned HwMode);
  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  BitVector getReservedRegs(const MachineFunction &MF) const override;

  // Stack frame processing methods
  void eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = NULL) const override;
  Register getFrameRegister(const MachineFunction &MF) const override;
};

} // end namespace llvm

#endif // LLVM_HS32REGISTERINFO_H
