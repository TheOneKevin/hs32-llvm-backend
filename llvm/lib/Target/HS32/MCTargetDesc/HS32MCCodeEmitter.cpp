#include "HS32MCTargetDesc.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/Support/EndianStream.h"

using namespace llvm;

#define DEBUG_TYPE "hs32-mccodeemitter"

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");

namespace {
class HS32MCCodeEmitter : public MCCodeEmitter {
  HS32MCCodeEmitter(const HS32MCCodeEmitter &) = delete;
  void operator=(const HS32MCCodeEmitter &) = delete;
  MCContext &Ctx;

public:
  HS32MCCodeEmitter(MCContext &ctx) : Ctx(ctx) {}

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
  return new HS32MCCodeEmitter(Ctx);
}

unsigned HS32MCCodeEmitter::getMachineOpValue(const MCInst &MI,const MCOperand &MO,
                                              SmallVectorImpl<MCFixup> &Fixups,
                                              const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  llvm_unreachable("Unhandled expression!");
}

void HS32MCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const {
  uint32_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
  support::endian::write<uint32_t>(OS, Bits, support::big);
  ++MCNumEmitted;
}
