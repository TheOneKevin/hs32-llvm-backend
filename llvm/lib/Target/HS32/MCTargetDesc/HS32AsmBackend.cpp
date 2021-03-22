#include "HS32MCTargetDesc.h"

#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmBackend.h"
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
  HS32AsmBackend() : MCAsmBackend(support::big) {}

  unsigned getNumFixupKinds() const override { return 0; }

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

void HS32AsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                const MCValue &Target, MutableArrayRef<char> Data,
                uint64_t Value, bool IsResolved,
                const MCSubtargetInfo *STI) const {
  return;
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
  llvm_unreachable("HS32AsmBackend::relaxInstruction() unimplemented");
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

