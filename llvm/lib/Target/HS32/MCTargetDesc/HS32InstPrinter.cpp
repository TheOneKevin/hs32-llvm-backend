#include "HS32InstPrinter.h"

#define DEBUG_TYPE "asm-printer"

namespace llvm {

#include "HS32GenAsmWriter.inc"

void HS32InstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                StringRef Annot, const MCSubtargetInfo &STI,
                                raw_ostream &O) {
  printInstruction(MI, Address, O);
  printAnnotation(O, Annot);
}

void HS32InstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                   raw_ostream &O)  {
  const MCOperand &Op = MI->getOperand(OpNo);
  if(Op.isReg()) {
    O << getRegisterName(Op.getReg());
  } else if(Op.isImm()) {
    O << formatImm(Op.getImm());
  } else {
    assert(Op.isExpr() && "Unknown operand kind in printOperand");
    O << *Op.getExpr();
  }
}

void HS32InstPrinter::printSimmOffsetArg(const MCInst *MI, unsigned OpNo,
                                         raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  // Check if is constant, otherwise probably a modifier expression
  if(Op.isImm()) {
    signed Val = static_cast<signed>(Op.getImm());
    if (Val == 0) {
      return;
    }
    if (Val > 0) {
      O << "+";
    }
    O << Val;
  } else {
    // We only support positive offset for now
    O << "+";
    O << *Op.getExpr();
  }
}

} // end namespace llvm
