#ifndef LLVM_HS32MCINSTLOWER_H
#define LLVM_HS32MCINSTLOWER_H

#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/Support/Compiler.h"

namespace llvm {

class AsmPrinter;
class MachineInstr;
class MachineOperand;
class MCContext;
class MCInst;
class MCOperand;
class MCSymbol;

// Lowers MachineInstr to MCInst
class HS32MCInstLower {
private:
  MCContext &Ctx;
  AsmPrinter &Printer;

public:
  HS32MCInstLower(MCContext &Ctx, AsmPrinter &Printer)
    : Ctx(Ctx), Printer(Printer) { }

  MCOperand lowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym) const;
  void lowerInstruction(const MachineInstr &MI, MCInst &OutMI) const;
};

} // end namespace llvm

#endif // LLVM_HS32MCINSTLOWER_H
