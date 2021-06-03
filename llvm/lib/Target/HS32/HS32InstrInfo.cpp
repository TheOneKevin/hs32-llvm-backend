#include "HS32InstrInfo.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/MCContext.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

#include "HS32.h"
#include "HS32RegisterInfo.h"
#include "HS32TargetMachine.h"
#include "MCTargetDesc/HS32MCTargetDesc.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "HS32GenInstrInfo.inc"

using namespace llvm;

HS32InstrInfo::HS32InstrInfo() : HS32GenInstrInfo() { }

void HS32InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI,
                                const DebugLoc &DL, MCRegister DestReg,
                                MCRegister SrcReg, bool KillSrc) const {
  assert(HS32::GPRRegClass.contains(DestReg, SrcReg) && "Impossible reg-to-reg copy");
  BuildMI(MBB, MI, DL, get(HS32::MOVrr), DestReg)
    .addReg(SrcReg, getKillRegState(KillSrc));
}

void HS32InstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MI,
                                        Register SrcReg, bool IsKill,
                                        int FrameIndex,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if(MI != MBB.end()) {
    MI->getDebugLoc();
  }
  // TODO: Implement checks in store/loadRegToStackSlot
  BuildMI(MBB, MI, DL, get(HS32::STRri))
      .addReg(SrcReg, getKillRegState(IsKill))
      .addFrameIndex(FrameIndex)
      .addImm(0);
}

void HS32InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator MI,
                                         Register DestReg, int FrameIndex,
                                         const TargetRegisterClass *RC,
                                         const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if(MI != MBB.end()) {
    MI->getDebugLoc();
  }
  BuildMI(MBB, MI, DL, get(HS32::LDRri), DestReg)
      .addFrameIndex(FrameIndex)
      .addImm(0);
}
