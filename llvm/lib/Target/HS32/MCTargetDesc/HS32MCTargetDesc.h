#ifndef LLVM_HS32MCTARGETDESC_H
#define LLVM_HS32MCTARGETDESC_H

#include "llvm/MC/MCTargetOptions.h"
#include "llvm/MC/MCELFObjectWriter.h"

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class StringRef;
class Target;
class Triple;
class raw_ostream;
class raw_pwrite_stream;

MCAsmBackend *createHS32AsmBackend(const Target &T,
                                   const MCSubtargetInfo &STI,
                                   const MCRegisterInfo &MRI,
                                   const MCTargetOptions &Options);

MCCodeEmitter *createHS32MCCodeEmitter(const MCInstrInfo &MCII,
                                       const MCRegisterInfo &MRI,
                                       MCContext &Ctx);

std::unique_ptr<MCObjectTargetWriter> createHS32ELFObjectWriter(uint8_t OSABI);

} // namespace llvm

#define GET_REGINFO_ENUM
#include "HS32GenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "HS32GenInstrInfo.inc"

#endif // LLVM_HS32MCTARGETDESC_H
