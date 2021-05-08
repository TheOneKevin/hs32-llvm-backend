#include "HS32.h"
#include "HS32TargetMachine.h"
#include "HS32ISelLowering.h"
#include "MCTargetDesc/HS32MCTargetDesc.h"

#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
#define DEBUG_TYPE "hs32-isel"

namespace llvm {

// Lowers LLVM IR to HS32 MC
class HS32DAGToDAGISel : public SelectionDAGISel {
public:
  HS32DAGToDAGISel(HS32TargetMachine &TM, CodeGenOpt::Level OptLevel)
      : SelectionDAGISel(TM, OptLevel) { }

  StringRef getPassName() const override {
    return "HS32 DAG->DAG Instruction Selection";
  }

  bool SelectShiftOperand(SDValue N, SDValue Reg, SDValue Imm);

  // Include tablegen header file
  #include "HS32GenDAGISel.inc"

private:
  void Select(SDNode *N) override;
};

} // end namespace llvm

void HS32DAGToDAGISel::Select(SDNode *N) {
  LLVM_DEBUG(dbgs() << "Selecting: "; N->dump(CurDAG); dbgs() << "\n");
  // If we have a custom node, we already have selected
  if(N->isMachineOpcode()) {
    LLVM_DEBUG(errs() << "== "; N->dump(CurDAG); errs() << "\n");
    N->setNodeId(-1);
    return;
  }

  // Select the default instruction
  SelectCode(N);
}

bool HS32DAGToDAGISel::SelectShiftOperand(SDValue N, SDValue Reg,
                                          SDValue Imm) {
  // TODO: Support shift operands
  return false;
}

// This pass converts a legalized DAG into a target-specific DAG
FunctionPass *llvm::createHS32ISelDag(HS32TargetMachine &TM,
                                      CodeGenOpt::Level OptLevel) {
  return new HS32DAGToDAGISel(TM, OptLevel);
}
