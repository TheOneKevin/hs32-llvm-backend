#ifndef LLVM_HS32INSTRINFO_H
#define LLVM_HS32INSTRINFO_H

#include "HS32RegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "HS32GenInstrInfo.inc"
#undef GET_INSTRINFO_HEADER

namespace llvm {

namespace HS32CC {

// HS32 specific condition codes, see HS32InstrInfo.td
enum CondCode {
  COND_EQ,
  COND_NE,
  COND_CS, // Unused
  COND_NC, // Unused
  COND_SS, // Unused
  COND_NS, // Unused
  COND_OV, // Unused
  COND_NV, // Unused
  COND_AB,
  COND_BE,
  COND_GE,
  COND_LT,
  COND_GT,
  COND_LE
};

} // end namespace HS32CC

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
