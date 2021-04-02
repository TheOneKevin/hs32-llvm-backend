#include "MCTargetDesc/HS32MCTargetDesc.h"
#include "TargetInfo/HS32TargetInfo.h"

#include "llvm/ADT/APInt.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/TargetRegistry.h"

#define DEBUG_TYPE "hs32-asm-parser"

using namespace llvm;

namespace {

//===----------------------------------------------------------------------===//
// HS32AsmParser definition
//===----------------------------------------------------------------------===//

class HS32AsmParser : public MCTargetAsmParser {
  const MCSubtargetInfo &STI;
  MCAsmParser &Parser;

#define GET_ASSEMBLER_HEADER
#include "HS32GenAsmMatcher.inc"

  bool MatchAndEmitInstruction(SMLoc Loc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;

  bool ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc) override;

  OperandMatchResultTy tryParseRegister(unsigned &RegNo, SMLoc &StartLoc,
                                        SMLoc &EndLoc) override;

  bool ParseDirective(AsmToken DirectiveID) override;

  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;

private:
  bool emit(MCInst &Inst, SMLoc const &Loc, MCStreamer &Out) const;
  bool missingFeature(SMLoc const &Loc, uint64_t const &ErrorInfo);
  bool invalidOperand(SMLoc const &Loc, OperandVector const &Operands, uint64_t const &ErrorInfo);

protected:
  bool ParseOperand(OperandVector &Operands);
  unsigned ParseRegisterName();
  bool ParseExpression(const MCExpr *&Eval);

public:
  HS32AsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
               const MCInstrInfo &MII, const MCTargetOptions &Options)
      : MCTargetAsmParser(Options, STI, MII), STI(STI), Parser(Parser) {
    MCAsmParserExtension::Initialize(Parser);
    setAvailableFeatures(ComputeAvailableFeatures(STI.getFeatureBits()));
  }
};

//===----------------------------------------------------------------------===//
// HS32Operand definition
//===----------------------------------------------------------------------===//

class HS32Operand : public MCParsedAsmOperand {
  typedef MCParsedAsmOperand Base;
  enum KindTy { k_Token, k_Register, k_Immediate } Kind;

public:
  HS32Operand(StringRef Tok, SMLoc const &S)
      : Base(), Kind(k_Token), Tok(Tok), Start(S), End(S){};
  HS32Operand(unsigned Reg, SMLoc const &S, SMLoc const &E)
      : Base(), Kind(k_Register), RegImm({Reg, nullptr}), Start(S), End(E){};
  HS32Operand(MCExpr const *Imm, SMLoc const &S, SMLoc const &E)
      : Base(), Kind(k_Immediate), RegImm({0, Imm}), Start(S), End(E){};

  struct RegisterImmediate {
    unsigned Reg;
    MCExpr const *Imm;
  };
  union {
    StringRef Tok;
    RegisterImmediate RegImm;
  };

  SMLoc Start, End;

public:
  bool isToken() const override { return Kind == k_Token; }
  bool isImm() const override { return Kind == k_Immediate; }
  bool isReg() const override { return Kind == k_Register; }
  bool isMem() const override { return false; }

  bool isConstantImm() const {
    return isImm() && dyn_cast<MCConstantExpr>(getImm());
  }

  int64_t getConstantImm() const {
    const MCExpr *Val = getImm();
    return static_cast<const MCConstantExpr *>(Val)->getValue();
  }

  bool isUImm16() const {
    return isConstantImm() && isUInt<16>(getConstantImm());
  }

  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(getReg()));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    const MCExpr* Expr = getImm();
    assert(Expr && "Expr shouldn't be null!");
    if (auto *CE = dyn_cast<MCConstantExpr>(Expr))
      Inst.addOperand(MCOperand::createImm(CE->getValue()));
    else
      Inst.addOperand(MCOperand::createExpr(Expr));
  }

  StringRef getToken() const {
    assert(Kind == k_Token && "Invalid access!");
    return Tok;
  }

  unsigned getReg() const override {
    assert(Kind == k_Register && "Invalid type access!");
    return RegImm.Reg;
  }

  const MCExpr *getImm() const {
    assert(Kind == k_Immediate && "Invalid type access!");
    return RegImm.Imm;
  }

  SMLoc getStartLoc() const override { return Start; }
  SMLoc getEndLoc() const override { return End; }

  void print(raw_ostream &O) const override {}

  static std::unique_ptr<HS32Operand> CreateToken(StringRef Str, SMLoc S) {
    return std::make_unique<HS32Operand>(Str, S);
  }

  static std::unique_ptr<HS32Operand> CreateReg(unsigned RegNum, SMLoc S,
                                                SMLoc E) {
    return std::make_unique<HS32Operand>(RegNum, S, E);
  }

  static std::unique_ptr<HS32Operand> CreateImm(const MCExpr *Val, SMLoc S,
                                                SMLoc E) {
    return std::make_unique<HS32Operand>(Val, S, E);
  }
};


} // namespace

//===----------------------------------------------------------------------===//
// Helper functions
//===----------------------------------------------------------------------===//
// <editor-fold desc="">

bool HS32AsmParser::emit(MCInst &Inst, SMLoc const &Loc, MCStreamer &Out) const {
  Inst.setLoc(Loc);
  Out.emitInstruction(Inst, STI);
  return false;
}

bool HS32AsmParser::missingFeature(const SMLoc &Loc, const uint64_t &ErrorInfo) {
  return Error(Loc, "instruction requires a CPU feature not currently enabled");
}

bool HS32AsmParser::invalidOperand(SMLoc const &Loc,
                                  OperandVector const &Operands,
                                  uint64_t const &ErrorInfo) {
  SMLoc ErrorLoc = Loc;
  if (ErrorInfo != ~0U) {
    if (ErrorInfo >= Operands.size())
      return Error(ErrorLoc, "too few operands for instruction");
    ErrorLoc = ((HS32Operand const &) *Operands[ErrorInfo]).getStartLoc();
    if (ErrorLoc == SMLoc())
      ErrorLoc = Loc;
  }
  return Error(ErrorLoc, "invalid operand for instruction");
}

// </editor-fold>

//===----------------------------------------------------------------------===//
// HS32AsmParser method implementation
//===----------------------------------------------------------------------===//
// <editor-fold desc="">

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "HS32GenAsmMatcher.inc"

OperandMatchResultTy HS32AsmParser::tryParseRegister(unsigned &RegNo,
                                                     SMLoc &StartLoc,
                                                     SMLoc &EndLoc) {
  const AsmToken &Tok = Parser.getTok();
  StartLoc = Tok.getLoc();
  EndLoc = Tok.getEndLoc();

  if (Tok.isNot(AsmToken::Identifier))
    return MatchOperand_NoMatch;

  unsigned RegNum = ParseRegisterName();
  if (RegNum == HS32::NoRegAltName)
    return MatchOperand_NoMatch;
  getLexer().Lex();
  RegNo = RegNum;
  return MatchOperand_Success;
}

unsigned HS32AsmParser::ParseRegisterName() {
  const AsmToken &Tok = Parser.getTok();
  StringRef Name = Tok.getString();
  unsigned RegNum = MatchRegisterName(Name);
  if (RegNum == HS32::NoRegister) {
    return MatchRegisterAltName(Name);
  }
  return RegNum;
}

bool HS32AsmParser::ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc) {
  if(tryParseRegister(RegNo, StartLoc, EndLoc))
    return TokError("invalid register name");
  return false;
}

bool HS32AsmParser::ParseExpression(const MCExpr *&EVal) {
  // TODO: Handle more complex cases
  return Parser.parseExpression(EVal);
}

bool HS32AsmParser::ParseDirective(AsmToken DirectiveID) {
  // TODO: Implement
  return true;
}

bool HS32AsmParser::ParseOperand(OperandVector &Operands) {
  SMLoc S = Parser.getTok().getLoc();
  SMLoc E = SMLoc::getFromPointer(S.getPointer() - 1);

  // First, parse register
  unsigned RegNo;
  if (tryParseRegister(RegNo, S, E) == MatchOperand_Success) {
    Operands.push_back(HS32Operand::CreateReg(RegNo, S, E));
    return false;
  }

  // Second, parse expression
  const MCExpr *EVal;

  switch (getLexer().getKind()) {
  case AsmToken::Identifier:
  case AsmToken::LParen:
  case AsmToken::Plus:
  case AsmToken::Minus:
  case AsmToken::Integer:
  case AsmToken::Dot:
  case AsmToken::Dollar:
  case AsmToken::Exclaim:
  case AsmToken::Tilde:
    if (!ParseExpression(EVal)) {
      break;
    }
    LLVM_FALLTHROUGH;
  default:
    return Error(S, "unknown operand");
  }

  // This is an expression
  Operands.push_back(HS32Operand::CreateImm(EVal, S, E));

  // TODO: Memory operands
  return false;
}

bool HS32AsmParser::ParseInstruction(ParseInstructionInfo &Info,
                      StringRef Name, SMLoc NameLoc,
                      OperandVector &Operands) {
  Operands.push_back(HS32Operand::CreateToken(Name, NameLoc));

  if(getLexer().is(AsmToken::EndOfStatement))
    return false;

  // Parse first operand
  if (ParseOperand(Operands))
    return true;

  // Parse each operand after comma
  while(getLexer().is(AsmToken::Comma)) {
    // Consume comma token
    getLexer().Lex();

    // Parse next operand
    if (ParseOperand(Operands))
      return true;
  }

  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    SMLoc Loc = getLexer().getLoc();
    getParser().eatToEndOfStatement();
    return Error(Loc, "unexpected token");
  }

  getParser().Lex(); // Consume the EndOfStatement.
  return false;
}

bool HS32AsmParser::MatchAndEmitInstruction(SMLoc Loc, unsigned int &Opcode,
                                            OperandVector &Operands,
                                            MCStreamer &Out, uint64_t &ErrorInfo,
                                            bool MatchingInlineAsm) {
  MCInst Inst;
  switch(MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm)) {
  case Match_Success:        return emit(Inst, Loc, Out);
  case Match_MissingFeature: return missingFeature(Loc, ErrorInfo);
  case Match_InvalidOperand: return invalidOperand(Loc, Operands, ErrorInfo);
  case Match_MnemonicFail:   return Error(Loc, "invalid instruction");
  default:                   return true;
  }
}

// </editor-fold>

extern "C" void LLVMInitializeHS32AsmParser() {
  RegisterMCAsmParser<HS32AsmParser> X(getTheHS32Target());
}
