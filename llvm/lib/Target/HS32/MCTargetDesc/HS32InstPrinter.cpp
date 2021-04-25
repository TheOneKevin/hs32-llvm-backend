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

} // end namespace llvm
