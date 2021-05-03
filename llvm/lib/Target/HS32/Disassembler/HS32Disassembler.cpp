#include "MCTargetDesc/HS32MCTargetDesc.h"
#include "TargetInfo/HS32TargetInfo.h"

#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCFixedLenDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "hs32-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {

class HS32Disassembler : public MCDisassembler {
public:
  HS32Disassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) { }
  virtual ~HS32Disassembler() { }

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
};

} // end anonymous namespace

static MCDisassembler *createHS32Disassembler(const Target &T,
                                              const MCSubtargetInfo &STI,
                                              MCContext &Ctx) {
  return new HS32Disassembler(STI, Ctx);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeHS32Disassembler() {
  TargetRegistry::RegisterMCDisassembler(getTheHS32Target(),
                                         createHS32Disassembler);
}

//===----------------------------------------------------------------------===//
// Functions used in HS32GenDisassemblerTables.inc
//===----------------------------------------------------------------------===//

static const uint16_t GPRDecoderTable[] = {
    HS32::R0, HS32::R1, HS32::R2, HS32::R3,
    HS32::R4, HS32::R5, HS32::R6, HS32::R7,
    HS32::R8, HS32::R9, HS32::R10, HS32::R11,
    HS32::R12, HS32::SP, HS32::LR, HS32::PC
};

static DecodeStatus DecodeGPRRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t Address, const void *Decoder) {
  if(RegNo > sizeof(GPRDecoderTable)) {
    return MCDisassembler::Fail;
  }
  unsigned Reg = GPRDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

template<int N>
static DecodeStatus decodeSImmOperand(MCInst &Inst, uint64_t Imm,
                                      uint64_t Address, const void *Decoder) {
  assert(isUInt<N>(Imm) && "Invalid immediate");
  Inst.addOperand(MCOperand::createImm(SignExtend64<N>(Imm)));
  return MCDisassembler::Success;
}

static DecodeStatus decodeShiftOperand(MCInst &Inst, uint64_t Rnsh,
                                       uint64_t Address, const void *Decoder) {
  unsigned RegNo = (Rnsh & 0b1111'00'00000) >> 7;
  unsigned Shift = (Rnsh & 0b0000'11'11111);
  if(DecodeGPRRegisterClass(Inst, RegNo, Address, Decoder)
      != MCDisassembler::Success) {
    return MCDisassembler::Fail;
  }
  Inst.addOperand(MCOperand::createImm(Shift));
  return MCDisassembler::Success;
}

#include "HS32GenDisassemblerTables.inc"

//===----------------------------------------------------------------------===//
// HS32Disassembler class methods
//===----------------------------------------------------------------------===//

#define OP_MASK 0xFF00'0000
#define RD_MASK 0x00F0'0000
#define RM_MASK 0x000F'0000

DecodeStatus HS32Disassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                              ArrayRef<uint8_t> Bytes,
                                              uint64_t Address,
                                              raw_ostream &CStream) const {
  Size = 4;
  if(Bytes.size() < 4) {
    Size = 0;
    return MCDisassembler::Fail;
  }
  uint32_t Insn = support::endian::read32be(Bytes.data());

  // Get opcode and family, group, var
  uint8_t Op = (Insn & OP_MASK) >> 24;

  // Normalize unused fields
  // BCC_pi and LCC_pi
  if((Op & 0xF0) == 0b0101'0000 || (Op & 0xF0) == 0b0111'0000) {
    Insn = Insn & ~RD_MASK;
    Insn = Insn & ~RM_MASK;
  }
  // All other instructions
  else {
    switch (Op) {
    // RMOV and IMOV
    case 0b001'00'000:
    case 0b001'00'100:
      Insn = Insn & ~RM_MASK;
      break;

    // RCMP and RTST
    case 0b011'01'000:
    case 0b100'01'000:
    // ICMP and ITST
    case 0b011'01'100:
    case 0b100'01'100:
      Insn = Insn & ~RD_MASK;
      break;
    }
  }
  // Finally parse
  return decodeInstruction(DecoderTable32, Instr, Insn,
                           Address, this, STI);
}
