#ifndef LLVM_HS32INSTRINFO_H
#define LLVM_HS32INSTRINFO_H

#include "HS32RegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "HS32GenInstrInfo.inc"
#undef GET_INSTRINFO_HEADER

namespace llvm {

class HS32InstrInfo : public HS32GenInstrInfo {
public:
  explicit HS32InstrInfo();

  void copyPhysReg(MachineBasicBlock &MBB,
                   MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, MCRegister DestReg,
                   MCRegister SrcReg, bool KillSrc) const override;

  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MI, Register SrcReg,
                           bool isKill, int FrameIndex,
                           const TargetRegisterClass *RC,
                           const TargetRegisterInfo *TRI) const override;
  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MI, Register DestReg,
                            int FrameIndex, const TargetRegisterClass *RC,
                            const TargetRegisterInfo *TRI) const override;
};

} // end namespace llvm

#endif // LLVM_HS32INSTRINFO_H
