#include "HS32.h"
#include "HS32Subtarget.h"
#include "HS32RegisterInfo.h"
#include "HS32FrameLowering.h"
#include "HS32TargetMachine.h"

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_REGINFO_TARGET_DESC
#include "HS32GenRegisterInfo.inc"

#define DEBUG_TYPE "hs32registerinfo"

using namespace llvm;

HS32RegisterInfo::HS32RegisterInfo(unsigned int HwMode)
    : HS32GenRegisterInfo(HS32::LR, 0, 0, HS32::PC, HwMode) { }

const MCPhysReg *HS32RegisterInfo::getCalleeSavedRegs(
    const MachineFunction *MF) const {
  return CSR_SaveList;
}

BitVector HS32RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  // Ensure register aliases are reserved
  markSuperRegs(Reserved, HS32::FP);
  markSuperRegs(Reserved, HS32::SP);
  markSuperRegs(Reserved, HS32::LR);
  markSuperRegs(Reserved, HS32::PC);

  return Reserved;
}

void HS32RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                           int SPAdj, unsigned int FIOperandNum,
                                           RegScavenger *RS) const {
  // LLVM_DEBUG(errs() << "subroutines not supported");
  assert(SPAdj == 0 && "Unexpected non-zero SPAdj value");

  MachineInstr &MI = *II;
  DebugLoc DL = MI.getDebugLoc();
  MachineBasicBlock &MBB = *MI.getParent();
  const MachineFunction &MF = *MBB.getParent();
  const HS32TargetMachine &TM = (const HS32TargetMachine &) MF.getTarget();
  const TargetInstrInfo &TII = *TM.getSubtargetImpl()->getInstrInfo();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetFrameLowering *TFI = TM.getSubtargetImpl()->getFrameLowering();
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  int Offset = MFI.getObjectOffset(FrameIndex);

  // Add one to the offset because SP points to an empty slot.
  Offset += MFI.getStackSize() - TFI->getOffsetOfLocalArea() + 1;
  // Fold incoming offset.
  Offset += MI.getOperand(FIOperandNum + 1).getImm();

  if (!isUInt<16>(Offset)) {
    report_fatal_error(
        "Frame offsets outside of the unsigned 16-bit range not supported");
  }

  MI.getOperand(FIOperandNum).ChangeToRegister(getFrameRegister(MF), false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
}

Register HS32RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return HS32::FP;
}
