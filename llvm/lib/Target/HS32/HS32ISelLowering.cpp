#include "HS32.h"
#include "HS32RegisterInfo.h"
#include "HS32Subtarget.h"
#include "HS32TargetMachine.h"
#include "HS32ISelLowering.h"
#include "MCTargetDesc/HS32BaseInfo.h"
#include "MCTargetDesc/HS32MCTargetDesc.h"

#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "hs32-lower"

HS32TargetLowering::HS32TargetLowering(const TargetMachine &TM,
                                       const HS32Subtarget &STI)
    : TargetLowering(TM), Subtarget(STI) {
  // Set up the register classes.
  addRegisterClass(MVT::i32, &HS32::GPRRegClass);

  // Compute derived properties from the register classes.
  computeRegisterProperties(Subtarget.getRegisterInfo());
  setStackPointerRegisterToSaveRestore(HS32::SP);

  // Support for all load/store types
  for (auto N : {ISD::EXTLOAD, ISD::SEXTLOAD, ISD::ZEXTLOAD}) {
    setLoadExtAction(N, MVT::i32, MVT::i1, Promote);
    setLoadExtAction(N, MVT::i32, MVT::i8, Promote);
    setLoadExtAction(N, MVT::i32, MVT::i16, Promote);
    setLoadExtAction(N, MVT::i32, MVT::i32, Legal);
    setLoadExtAction(N, MVT::i32, MVT::i64, Expand);
  }

  // TODO: Add all setOperationAction
  setBooleanContents(ZeroOrOneBooleanContent);

  setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);

  setMinFunctionAlignment(Align(2));
  setMinimumJumpTableEntries(UINT_MAX);
}

const char *HS32TargetLowering::getTargetNodeName(unsigned int Opcode) const {
#define NODE(name) case HS32ISD::name: return #name
    switch(Opcode) {
    default: return nullptr;
    NODE(RET_FLAG);
    }
#undef NODE
}

SDValue HS32TargetLowering::LowerOperation(SDValue Op,
                                           SelectionDAG &DAG) const {
  switch(Op.getOpcode()) {
  default: report_fatal_error("unimplemented operand");
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  }
}

//===----------------------------------------------------------------------===//
// DAG lowering implementation
//===----------------------------------------------------------------------===//

SDValue HS32TargetLowering::LowerGlobalAddress(SDValue Op,
                                               SelectionDAG &DAG) const {
  auto DL = DAG.getDataLayout();

  const GlobalAddressSDNode *Node = cast<GlobalAddressSDNode>(Op);
  const const GlobalValue *GV = Node->getGlobal();
  int64_t Offset = Node->getOffset();

  // We don't support PIE yet
  if(!isPositionIndependent()) {
    SDValue AdHi = DAG.getTargetGlobalAddress(
        GV, SDLoc(Op), getPointerTy(DL), Offset, HS32II::MO_Hi);
    SDValue AdLo = DAG.getTargetGlobalAddress(
        GV, SDLoc(Op), getPointerTy(DL), Offset, HS32II::MO_Lo);
    // (movt (mov adlo) adhi)
    SDValue MnLo =
        SDValue(DAG.getMachineNode(HS32::MOVri, SDLoc(Op), getPointerTy(DL), AdLo), 0);
    SDValue MnHi =
        SDValue(DAG.getMachineNode(HS32::MOVTri, SDLoc(Op), getPointerTy(DL), MnLo, AdHi), 0);
    return MnHi;
  }
  report_fatal_error("unable to lower global address");
}

//===----------------------------------------------------------------------===//
// Calling convention implementation
//===----------------------------------------------------------------------===//

#include "HS32GenCallingConv.inc"

SDValue HS32TargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  switch(CallConv) {
  case CallingConv::C:
    break;
  default:
    report_fatal_error("unsupported calling convention");
  }

  if(IsVarArg) {
    report_fatal_error("VarArg not supported");
  }

  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();

  // Assign locations to arguments
  SmallVector<CCValAssign, 9> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_HS32);

  for(auto &VA : ArgLocs) {
    if(!VA.isRegLoc()) {
      report_fatal_error("too many arguments");
    }
    EVT RegVT = VA.getLocVT();
    if(RegVT != MVT::i32) {
      LLVM_DEBUG(
          dbgs() << "LowerFormalArguments: Unhandled argument type"
                 << RegVT.getEVTString() << "\n");
      report_fatal_error("unhandled argument type");
    }
    const Register VReg = RegInfo.createVirtualRegister(&HS32::GPRRegClass);
    RegInfo.addLiveIn(VA.getLocReg(), VReg);
    SDValue ArgIn = DAG.getCopyFromReg(Chain, DL, VReg, RegVT);
    InVals.push_back(ArgIn);
  }
  return Chain;
}

SDValue HS32TargetLowering::LowerReturn(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs,
    const SmallVectorImpl<SDValue> &OutVals,
    const SDLoc &DL, SelectionDAG &DAG) const {
  if(IsVarArg) {
    report_fatal_error("VarArg not supported");
  }
  // Assignment of return value to a location
  SmallVector<CCValAssign, 9> RVLocs;

  // Info about registers and stack
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(),
                 RVLocs, *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, RetCC_HS32);
  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result to output registers
  for(unsigned i = 0, e = RVLocs.size(); i < e; ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "can only return in registers!");
    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[i], Flag);

    // Guarantee all emitted copies are stuck together with flags
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  // Update chain
  RetOps[0] = Chain;
  if(Flag.getNode()) {
    RetOps.push_back(Flag);
  }
  return DAG.getNode(HS32ISD::RET_FLAG, DL, MVT::Other, RetOps);
}
