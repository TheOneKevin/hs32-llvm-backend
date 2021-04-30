#include "HS32MCTargetDesc.h"
#include "HS32FixupKinds.h"
#include "HS32MCExpr.h"

#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCMachObjectWriter.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

class HS32AsmBackend : public MCAsmBackend {
public:
  HS32AsmBackend() : MCAsmBackend(support::big) { }

  unsigned getNumFixupKinds() const override {
    return HS32::NumTargetFixupKinds;
  }

  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override {
    const static MCFixupKindInfo Infos[HS32::NumTargetFixupKinds] = {
        // order must be as-declared in HS322FixupKinds.h
        // name, offset, size, flags
        { "fixup_hs32_hi",        0, 16, 0 },
        { "fixup_hs32_lo",        0, 16, 0 },
        { "fixup_hs32_pcrel_hi",  0, 16, MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_hs32_pcrel_lo",  0, 16, MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_hs32_branch",    0, 16, MCFixupKindInfo::FKF_IsPCRel }
    };

    if(Kind < FirstTargetFixupKind)
      return MCAsmBackend::getFixupKindInfo(Kind);

    assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
           "Invalid kind!");
    return Infos[Kind - FirstTargetFixupKind];
  }

  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override;
  bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                            const MCRelaxableFragment *DF,
                            const MCAsmLayout &Layout) const override;
  bool mayNeedRelaxation(const MCInst &Inst,
                         const MCSubtargetInfo &STI) const override;
  void relaxInstruction(MCInst &Inst,
                        const MCSubtargetInfo &STI) const override;

  // The size of NOP in bytes
  unsigned getMinimumNopSize() const override { return 4; }
  bool writeNopData(raw_ostream &OS, uint64_t Count) const override {
    if(Count % 4 != 0)
      return false;
    // addi r0, r0, 0
    for(uint64_t i = 0; i < Count; i += 4) {
      OS << "\x44\x00\x00\x00";
    }
    return true;
  }
};

} // end anonymous namespace

static uint64_t adjustFixupValue(const MCFixup &Fixup, uint64_t Value,
                                 MCContext &Ctx) {
  switch(unsigned(Fixup.getKind())) {
  case FK_Data_1:
  case FK_Data_2:
  case FK_Data_4:
  case FK_Data_8:
    return Value;
  case HS32::fixup_hs32_hi:
    return (Value >> 16) & 0xFFFF;
  case HS32::fixup_hs32_lo:
    return Value & 0xFFFF;
  case HS32::fixup_hs32_pcrel_lo: {
    if (!isInt<16>(Value+4))
      Ctx.reportWarning(Fixup.getLoc(), "pcrel may overflow");
    return (Value+4) & 0xFFFF;
  }
  case HS32::fixup_hs32_pcrel_hi: {
    return ((Value+4) >> 16) & 0xFFFF;
  }
  case HS32::fixup_hs32_branch: {
    if (!isInt<16>(Value))
      Ctx.reportError(Fixup.getLoc(), "fixup value out of range");
    if (Value & 0x3)
      Ctx.reportError(Fixup.getLoc(), "fixup must be 4-byte aligned");
    return Value & 0xFFFF;
  }
  default:
    llvm_unreachable("Unhandled fixup kind");
  }
}

void HS32AsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                                const MCValue &Target, MutableArrayRef<char> Data,
                                uint64_t Value, bool IsResolved,
                                const MCSubtargetInfo *STI) const {
  // Fixup stays 0
  if(!Value) return;

  // Adjust fixup value
  MCContext &Ctx = Asm.getContext();
  Value = adjustFixupValue(Fixup, Value, Ctx);

  MCFixupKind Kind = Fixup.getKind();
  MCFixupKindInfo Info = getFixupKindInfo(Kind);

  // Shift value into place
  Value <<= Info.TargetOffset;

  // Get fixup bytes and offset within fragment
  unsigned Offset = Fixup.getOffset();
#ifndef NDEBUG
  unsigned NumBytes = (Info.TargetSize + 7) / 8;
  assert(Offset + NumBytes <= Data.size() && "Invalid fixup offset!");
#endif

  // Apply fixup within the fragment, masking the values in.
  static const unsigned InstrSize = 4;
  for(unsigned i = 0; i != InstrSize; ++i) {
    // We're in big-endian.
    Data[Offset + i] |= uint8_t((Value >> ((InstrSize-i-1) * 8)) & 0xff);
  }
}

bool HS32AsmBackend::fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                                          const MCRelaxableFragment *DF,
                                          const MCAsmLayout &Layout) const {
  return false;
}

bool HS32AsmBackend::mayNeedRelaxation(const MCInst &Inst,
                                       const MCSubtargetInfo &STI) const {
  return false;
}

void HS32AsmBackend::relaxInstruction(MCInst &Inst,
                                      const MCSubtargetInfo &STI) const {
  llvm_unreachable("relaxInstruction() unimplemented");
}

namespace {

class HS32ELFAsmBackend : public HS32AsmBackend {
public:
  uint8_t OSABI;
  HS32ELFAsmBackend(uint8_t OSABI)
    : HS32AsmBackend(), OSABI(OSABI) {}

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    return createHS32ELFObjectWriter(OSABI);
  }
};

} // end anonymous namespace

MCAsmBackend *llvm::createHS32AsmBackend(const Target &T,
                                         const MCSubtargetInfo &STI,
                                         const MCRegisterInfo &MRI,
                                         const MCTargetOptions &Options) {
  const Triple &TheTriple = STI.getTargetTriple();
  uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(TheTriple.getOS());
  return new HS32ELFAsmBackend(OSABI);
}
