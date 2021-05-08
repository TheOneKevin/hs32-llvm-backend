#ifndef LLVM_HS32FRAMELOWERING_H
#define LLVM_HS32FRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class HS32Subtarget;

class HS32FrameLowering : public TargetFrameLowering {
public:
  explicit HS32FrameLowering(const HS32Subtarget &STI)
      : TargetFrameLowering(StackGrowsDown, Align(32), 0) { };
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  bool hasFP(const MachineFunction &MF) const override;
};

} // end namespace llvm

#endif // LLVM_HS32FRAMELOWERING_H
