#ifndef LLVM_HS32_H
#define LLVM_HS32_H

#include "MCTargetDesc/HS32MCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class HS32TargetMachine;
class MachineInstr;
class AsmPrinter;
class MCInst;

FunctionPass *createHS32ISelDag(HS32TargetMachine &TM, CodeGenOpt::Level OL);

} // end namespace llvm

#endif // LLVM_HS32_H
