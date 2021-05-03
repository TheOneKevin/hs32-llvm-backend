#include "HS32InstPrinter.h"
#include "HS32BaseInfo.h"

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

void HS32InstPrinter::printSimmOffsetOperand(const MCInst *MI, unsigned OpNo,
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

void HS32InstPrinter::printShiftOperand(const MCInst *MI, unsigned OpNo,
                                        raw_ostream &O) {
  // register shift operand is [reg, imm]
  // where imm is { value (5 bits), type (2) }
  const MCOperand &Rn = MI->getOperand(OpNo);
  unsigned Sh = MI->getOperand(OpNo+1).getImm();
  HS32II::ShiftTypes Type = static_cast<HS32II::ShiftTypes>(Sh & 0b11);
  O << getRegisterName(Rn.getReg()) << " "
    << HS32II::getStringFromShiftType(Type) << " " << (Sh >> 2);
}

} // end namespace llvm
