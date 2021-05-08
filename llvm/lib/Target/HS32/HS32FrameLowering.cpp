#include "HS32FrameLowering.h"

using namespace llvm;

bool HS32FrameLowering::hasFP(const MachineFunction &MF) const {
  return true;
}

void HS32FrameLowering::emitPrologue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {

}

void HS32FrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {

}
