#include "HS32MCTargetDesc.h"
#include "HS32FixupKinds.h"

#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
class HS32ELFObjectWriter : public MCELFObjectTargetWriter {
public:
  HS32ELFObjectWriter(uint8_t OSABI);
  ~HS32ELFObjectWriter() override;

protected:
  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;
};
} // namespace

HS32ELFObjectWriter::HS32ELFObjectWriter(uint8_t OSABI)
: MCELFObjectTargetWriter(false, OSABI, ELF::EM_HS32, false) {}

HS32ELFObjectWriter::~HS32ELFObjectWriter() {}

unsigned HS32ELFObjectWriter::getRelocType(MCContext &Ctx,
                                           const MCValue &Target,
                                           const MCFixup &Fixup,
                                           bool IsPCRel) const {
  // Determine the type of the relocation
  switch ((unsigned)Fixup.getKind()) {
  case HS32::fixup_hs32_hi:       return ELF::R_HS32_HI;
  case HS32::fixup_hs32_lo:       return ELF::R_HS32_LO;
  case HS32::fixup_hs32_pcrel_lo: return ELF::R_HS32_PCREL_LO;
  case HS32::fixup_hs32_pcrel_hi: return ELF::R_HS32_PCREL_HI;
  case HS32::fixup_hs32_branch:   return ELF::R_HS32_BRANCH;
  default:
    llvm_unreachable("Unimplemented");
  }
}

std::unique_ptr<MCObjectTargetWriter>
    llvm::createHS32ELFObjectWriter(uint8_t OSABI)
{
  return std::make_unique<HS32ELFObjectWriter>(OSABI);
}
