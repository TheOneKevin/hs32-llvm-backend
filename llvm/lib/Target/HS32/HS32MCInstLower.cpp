#include "HS32.h"
#include "HS32MCInstLower.h"
#include "MCTargetDesc/HS32BaseInfo.h"
#include "MCTargetDesc/HS32MCExpr.h"

#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

MCOperand HS32MCInstLower::lowerSymbolOperand(const MachineOperand &MO,
                                              MCSymbol *Sym) const {
  HS32MCExpr::VariantKind Kind;
  switch(MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case HS32II::MO_None:
    Kind = HS32MCExpr::VK_HS32_None;
    break;
  case HS32II::MO_Hi:
    Kind = HS32MCExpr::VK_HS32_HI;
    break;
  case HS32II::MO_Lo:
    Kind = HS32MCExpr::VK_HS32_LO;
    break;
  case HS32II::MO_PcrelHi:
    Kind = HS32MCExpr::VK_HS32_PCREL;
  }

  const MCExpr *ME =
      MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_None, Ctx);

  if (!MO.isJTI() && MO.getOffset()) {
    // ME + offset
    ME = MCBinaryExpr::createAdd(
        ME, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);
  }

  ME = HS32MCExpr::create(Kind, ME, false, Ctx);
  return MCOperand::createExpr(ME);
}

void HS32MCInstLower::lowerInstruction(const MachineInstr &MI,
                                       MCInst &OutMI) const {
  OutMI.setOpcode(MI.getOpcode());
  for(const MachineOperand &MO : MI.operands()) {
    MCOperand MCOp;
    switch(MO.getType()) {
    default:
      MI.print(errs());
      llvm_unreachable("unknown operand type");
    case MachineOperand::MO_Register:
      // Ignore all implicit register operands
      if(MO.isImplicit())
        continue;
      MCOp = MCOperand::createReg(MO.getReg());
      break;
    case MachineOperand::MO_Immediate:
      MCOp = MCOperand::createImm(MO.getImm());
      break;
    case MachineOperand::MO_GlobalAddress:
      MCOp = lowerSymbolOperand(MO, Printer.getSymbol(MO.getGlobal()));
      break;
    case MachineOperand::MO_MachineBasicBlock:
      MCOp = MCOperand::createExpr(
        MCSymbolRefExpr::create(MO.getMBB()->getSymbol(), Ctx));
      break;
    }
    OutMI.addOperand(MCOp);
  }
}
