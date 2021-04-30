#include "HS32MCTargetDesc.h"
#include "HS32FixupKinds.h"
#include "HS32MCExpr.h"
#include "HS32BaseInfo.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/Support/EndianStream.h"

using namespace llvm;

#define DEBUG_TYPE "hs32-mccodeemitter"

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");
STATISTIC(MCNumFixups, "Number of fixups emitted");

namespace {
class HS32MCCodeEmitter : public MCCodeEmitter {
  HS32MCCodeEmitter(const HS32MCCodeEmitter &) = delete;
  void operator=(const HS32MCCodeEmitter &) = delete;

  MCContext &Ctx;
  MCInstrInfo const &MCII;

public:
  HS32MCCodeEmitter(MCContext &Ctx, MCInstrInfo const &MCII)
      : Ctx(Ctx), MCII(MCII) {}

  ~HS32MCCodeEmitter() override {}

  // Defined in "HS32GenMCCodeEmitter.inc"
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  // getMachineOpValue - Return binary encoding of operand. If the machine
  // operand requires relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI,const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;
};
} // namespace

#include "HS32GenMCCodeEmitter.inc"

MCCodeEmitter *llvm::createHS32MCCodeEmitter(const MCInstrInfo &MCII,
                                             const MCRegisterInfo &MRI,
                                             MCContext &Ctx) {
  return new HS32MCCodeEmitter(Ctx, MCII);
}

unsigned HS32MCCodeEmitter::getMachineOpValue(const MCInst &MI,const MCOperand &MO,
                                              SmallVectorImpl<MCFixup> &Fixups,
                                              const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  MCInstrDesc const &Desc = MCII.get(MI.getOpcode());

  if(MO.isExpr()) {
    const MCExpr *Expr = MO.getExpr();
    HS32::Fixups FixupKind = HS32::fixup_hs32_invalid;
    switch (Expr->getKind()) {
    case MCExpr::Target: {
      const HS32MCExpr *HSExpr = cast<HS32MCExpr>(Expr);
      switch(HSExpr->getKind()) {
      case HS32MCExpr::VK_HS32_None:
        llvm_unreachable("Unhandled variant kind");
      case HS32MCExpr::VK_HS32_HI:
        FixupKind = HS32::fixup_hs32_hi;
        break;
      case HS32MCExpr::VK_HS32_LO:
        FixupKind = HS32::fixup_hs32_lo;
        break;
      case HS32MCExpr::VK_HS32_PCREL:
        if(Desc.TSFlags & HS32II::InstBranchMask) {
          Ctx.reportWarning(MO.getExpr()->getLoc(),
                            "using pcrel in branch is dangerous");
        }
        FixupKind = HS32::fixup_hs32_pcrel_lo;
        break;
      }
      break;
    }
    case MCExpr::SymbolRef: {
      if(Desc.TSFlags & HS32II::InstBranchMask) {
        FixupKind = HS32::fixup_hs32_branch;
        break;
      }
      Ctx.reportError(MO.getExpr()->getLoc(),
                      "symbolref fixups outside branch is unsupported");
      break;
    }
    default:
      llvm_unreachable("Invalid expression kind");
    }

    // assert(FixupKind != HS32::fixup_hs32_invalid && "Unhandled expression!");
    Fixups.push_back(MCFixup::create(0, Expr,
                                     MCFixupKind(FixupKind), MI.getLoc()));
    ++MCNumFixups;
    return 0;
  }

  llvm_unreachable("Unhandled expression!");
}

void HS32MCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const {
  uint32_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
  support::endian::write<uint32_t>(OS, Bits, support::big);
  ++MCNumEmitted;
}
