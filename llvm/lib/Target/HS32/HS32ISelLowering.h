#ifndef LLVM_HS32ISELLOWERING_H
#define LLVM_HS32ISELLOWERING_H

#include "HS32.h"
#include "HS32InstrInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/CodeGen/ISDOpcodes.h"

namespace llvm {

class HS32Subtarget;
class HS32TargetMachine;

namespace HS32ISD {
enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  RET_FLAG,
  CMP, TST,
  BRCOND, BLCOND
};
} // namespace HS32ISD

class HS32TargetLowering : public TargetLowering {
  const HS32Subtarget &Subtarget;

public:
  explicit HS32TargetLowering(const TargetMachine &TM,
                              const HS32Subtarget &STI);
public:
  // Provide custom hooks for lowering operations
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  const char *getTargetNodeName(unsigned Opcode) const override;

private:
  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &dl,
                      SelectionDAG &DAG) const override;
  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;
  bool shouldConvertConstantLoadToIntImm(const APInt &Imm,
                                         Type *Ty) const override {
    return true;
  }

private:
  SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;

private:
  SDValue getCmpNode(SDValue LHS, SDValue RHS, ISD::CondCode CC, SDValue &CCout,
                     SelectionDAG &DAG, SDLoc LD) const;
};

} // namespace llvm

#endif // LLVM_HS32ISELLOWERING_H
