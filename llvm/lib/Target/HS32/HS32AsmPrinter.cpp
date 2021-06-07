#include "HS32.h"
#include "HS32MCInstLower.h"
#include "MCTargetDesc/HS32InstPrinter.h"
#include "HS32TargetMachine.h"
#include "TargetInfo/HS32TargetInfo.h"

#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "asmprinter"

namespace {

class HS32AsmPrinter : public AsmPrinter {
public:
  explicit HS32AsmPrinter(TargetMachine &TM,
                           std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) { }

  StringRef getPassName() const override { return "HS32 Assembly Printer"; }
  void emitInstruction(const MachineInstr *MI) override;

  // Defined by tablegen
  bool emitPseudoExpansionLowering(MCStreamer &OutStreamer,
                                   const MachineInstr *MI);
};

} // end anonymous namespace

#include "HS32GenMCPseudoLowering.inc"

void HS32AsmPrinter::emitInstruction(const MachineInstr *MI) {
  if(emitPseudoExpansionLowering(*OutStreamer, MI)) {
    return;
  }

  MCInst I;
  HS32MCInstLower MCInstLowering(OutContext, *this);

  MCInstLowering.lowerInstruction(*MI, I);
  EmitToStreamer(*OutStreamer, I);
}

static AsmPrinter *createHS32AsmPrinterPass(TargetMachine &tm,
                                            std::unique_ptr<MCStreamer> &&Streamer) {
  return new HS32AsmPrinter(tm, std::move(Streamer));
}

extern "C" void LLVMInitializeHS32AsmPrinter() {
  TargetRegistry::RegisterAsmPrinter(getTheHS32Target(),
                                     createHS32AsmPrinterPass);
}
