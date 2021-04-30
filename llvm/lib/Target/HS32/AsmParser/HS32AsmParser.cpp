#include "MCTargetDesc/HS32MCTargetDesc.h"
#include "MCTargetDesc/HS32MCExpr.h"
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
  bool invalidOperand(SMLoc const &Loc, OperandVector const &Operands,
                      uint64_t const &ErrorInfo);
  bool invalidImmediate(OperandVector const &Operands, uint64_t ErrorInfo,
                        bool Signed, unsigned Bits);

protected:
  bool ParseOperand(OperandVector &Operands);
  bool ParseExpression(OperandVector &Operands);
  bool ParseModifier(OperandVector &Operands);
  bool ParseMemory(OperandVector &Operands);
  bool tryParseRegisterName(unsigned &RegNo);

public:
  enum HS32MatchResultTy {
    Match_Dummy = FIRST_TARGET_MATCH_RESULT_TY,
#define GET_OPERAND_DIAGNOSTIC_TYPES
    #include "HS32GenAsmMatcher.inc"
#undef GET_OPERAND_DIAGNOSTIC_TYPES
  };

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

private:
  bool isValidModifierExpr(const MCExpr *Expr) const;

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
    if(!isImm()) return false;
    return isValidModifierExpr(getImm()) ||
           (isConstantImm() && isUInt<16>(getConstantImm()));
  }

  bool isSImm16() const {
    if(!isImm()) return false;
    return isValidModifierExpr(getImm()) ||
           (isConstantImm() && isInt<16>(getConstantImm()));
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

bool HS32AsmParser::invalidImmediate(OperandVector const &Operands,
                                     uint64_t ErrorInfo,
                                     bool Signed, unsigned Bits) {
  auto Op = (HS32Operand const &) *Operands[ErrorInfo];
  SMLoc ErrorLoc = Op.Start;
  // Different error messages for constants vs. modifiers
  if(Op.isConstantImm()) {
    int64_t Upper = 0, Lower = 0;
    if(Signed)
      Upper = maxIntN(Bits), Lower = minIntN(Bits);
    else
      Upper = maxUIntN(Bits), Lower = 0;
    return Error(ErrorLoc, "immediate must be an integer within ["
                           + Twine(Lower) + ", " + Twine(Upper) + "]");
  }
  return Error(ErrorLoc, "malformed immediate expression");
}

bool HS32Operand::isValidModifierExpr(const MCExpr *Expr) const {
  // TODO: More robust features. For now, we restrict this to
  //       single fixups only - no symbolrefs.
  return isa<HS32MCExpr>(Expr) || Expr->getKind() == MCExpr::SymbolRef;
}

// </editor-fold>

//===----------------------------------------------------------------------===//
// Main parser code
//===----------------------------------------------------------------------===//
// <editor-fold desc="">

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "HS32GenAsmMatcher.inc"

bool HS32AsmParser::tryParseRegisterName(unsigned &RegNo) {
  const AsmToken &Tok = Parser.getTok();
  StringRef Name = Tok.getString();
  RegNo = MatchRegisterName(Name);
  if (RegNo == HS32::NoRegister) {
    RegNo = MatchRegisterAltName(Name);
    if(RegNo == HS32::NoRegAltName) {
      return true;
    }
  }
  getLexer().Lex(); // consume name on success
  return false;
}

bool HS32AsmParser::ParseExpression(OperandVector &Operands) {
  const MCExpr* EVal;
  const SMLoc StartLoc = Parser.getTok().getLoc();
  const SMLoc EndLoc = Parser.getTok().getEndLoc();
  // Assume head of expression is '%', i.e., no +%hi(..) or -%lo(..)
  // we will deal with the other cases during error checking
  if(getLexer().is(AsmToken::Percent)) {
    return ParseModifier(Operands);
  }
  // SymbolRefs and constants are dropped down to here
  if(!Parser.parseExpression(EVal)) {
    Operands.push_back(HS32Operand::CreateImm(EVal, StartLoc, EndLoc));
    return false;
  }
  return true;
}

bool HS32AsmParser::ParseModifier(OperandVector &Operands) {
  const SMLoc StartLoc = Parser.getTok().getLoc();
  SMLoc EndLoc = Parser.getTok().getEndLoc();
  // Consume '%'
  Parser.Lex();
  // Parse modifier and expression
  if(Parser.getTok().is(AsmToken::Identifier) &&
     getLexer().peekTok().is(AsmToken::LParen)) {
    StringRef ModifierName = Parser.getTok().getString();
    HS32MCExpr::VariantKind Kind = HS32MCExpr::getKindByName(ModifierName);
    if(Kind == HS32MCExpr::VK_HS32_None) {
      return Error(EndLoc, "unknown modifier name");
    }
    // Consume modifier and '('
    Parser.Lex();
    Parser.Lex();
    // Parse expression within ()
    const MCExpr *Expr;
    if(Parser.parseParenExpression(Expr, EndLoc)) {
      return Error(EndLoc, "bad modifier subexpression");
    }
    Operands.push_back(HS32Operand::CreateImm(
        HS32MCExpr::create(Kind, Expr, false, getContext()), StartLoc, EndLoc));
    return false;
  }
  return Error(EndLoc, "expected modifier and '('");
}

bool HS32AsmParser::ParseDirective(AsmToken DirectiveID) {
  // TODO: Implement
  return true;
}

bool HS32AsmParser::ParseMemory(OperandVector &Operands) {
  unsigned Rm, Rn;
  SMLoc StartLoc = Parser.getTok().getLoc();
  SMLoc EndLoc = Parser.getTok().getEndLoc();
  SMLoc Arg1Loc, OpLoc, Arg2Loc;

  // Consume '['
  getLexer().Lex();

  // Parse register
  Arg1Loc = getLexer().getLoc();
  ParseRegister(Rm, StartLoc, EndLoc);

  // If this is [reg], inject +0]
  if(getLexer().is(AsmToken::RBrac)) {
    // Consume ']'
    getLexer().Lex();
    // Inject '+0]'
    getLexer().UnLex(AsmToken(AsmToken::RBrac, "]"));
    getLexer().UnLex(AsmToken(AsmToken::Integer, "0", 0));
    getLexer().UnLex(AsmToken(AsmToken::Plus, "+"));
  }

  // Parse operator
  const auto Op = getLexer().getTok();
  if(Op.isNot(AsmToken::Plus) && Op.isNot(AsmToken::Minus))
    return Error(EndLoc, "malformed memory reference, expecting '+' or '-'");
  OpLoc = getLexer().getLoc();
  Arg2Loc = SMLoc::getFromPointer(OpLoc.getPointer() + 1);

  // Consume operator
  getLexer().Lex();

  // Push beginning of the operand
  Operands.push_back(HS32Operand::CreateToken("[", StartLoc));
  Operands.push_back(HS32Operand::CreateReg(Rm, Arg1Loc, EndLoc));

  // Parse the rest of the operand
  if(!tryParseRegisterName(Rn) && Op.is(AsmToken::Plus)) {
    if(getLexer().isNot(AsmToken::RBrac))
      return Error(EndLoc, "malformed memory reference, expecting ']'");
    // Consume ']'
    getLexer().Lex();
    // Push operands
    Operands.push_back(HS32Operand::CreateToken("+", OpLoc));
    Operands.push_back(HS32Operand::CreateReg(Rn, Arg2Loc, EndLoc));
    Operands.push_back(HS32Operand::CreateToken("]", EndLoc));
    return false;
  }

  // Add back the operator (for signed imm)
  if(getLexer().isNot(AsmToken::Percent)) {
    getLexer().UnLex(Op);
  } else if(Op.isNot(AsmToken::Plus)) {
    return Error(OpLoc, "bad modifier expression, only '+' supported");
  }

  if(!ParseExpression(Operands)) {
    if(getLexer().isNot(AsmToken::RBrac))
      return Error(EndLoc, "malformed memory reference, expecting ']'");
    // Consume ']'
    getLexer().Lex();
    // Push operands
    Operands.push_back(HS32Operand::CreateToken("]", EndLoc));
    return false;
  }

  return Error(EndLoc, "malformed memory reference");
}

bool HS32AsmParser::ParseOperand(OperandVector &Operands) {
  SMLoc StartLoc = Parser.getTok().getLoc();
  SMLoc EndLoc = Parser.getTok().getEndLoc();

  // First, parse register
  unsigned RegNo;
  if (!tryParseRegisterName(RegNo)) {
    Operands.push_back(HS32Operand::CreateReg(RegNo, StartLoc, EndLoc));
    return false;
  }

  // Second, parse expression
  switch (getLexer().getKind()) {
    case AsmToken::LBrac:
      return ParseMemory(Operands);

    case AsmToken::Percent:
    case AsmToken::Identifier:
    case AsmToken::LParen:
    case AsmToken::Plus:
    case AsmToken::Minus:
    case AsmToken::Integer:
    case AsmToken::Dot:
    case AsmToken::Dollar:
    case AsmToken::Exclaim:
    case AsmToken::Tilde:
      if (!ParseExpression(Operands)) {
        return false;
      }
      break;
    default:
      break;
  }
  return Error(StartLoc, "unknown operand");
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
// </editor-fold>

//===----------------------------------------------------------------------===//
// HS32AsmParser methods implementation
//===----------------------------------------------------------------------===//
// <editor-fold desc="">

OperandMatchResultTy HS32AsmParser::tryParseRegister(unsigned &RegNo,
                                                     SMLoc &StartLoc,
                                                     SMLoc &EndLoc) {
  const AsmToken &Tok = Parser.getTok();
  StartLoc = Tok.getLoc();
  EndLoc = Tok.getEndLoc();
  if (Tok.isNot(AsmToken::Identifier))
    return MatchOperand_NoMatch;
  if(!tryParseRegisterName(RegNo))
    return MatchOperand_Success;
  return MatchOperand_NoMatch;
}

bool HS32AsmParser::ParseRegister(unsigned &RegNo,
                                  SMLoc &StartLoc, SMLoc &EndLoc) {
  if(tryParseRegister(RegNo, StartLoc, EndLoc))
    return TokError("invalid register name");
  return false;
}

bool HS32AsmParser::MatchAndEmitInstruction(SMLoc Loc, unsigned int &Opcode,
                                            OperandVector &Operands,
                                            MCStreamer &Out, uint64_t &ErrorInfo,
                                            bool MatchingInlineAsm) {
  MCInst Inst;
  switch(MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm)) {
    case Match_Success:         return emit(Inst, Loc, Out);
    case Match_MissingFeature:  return missingFeature(Loc, ErrorInfo);
    case Match_InvalidUImm16:   return invalidImmediate(Operands, ErrorInfo, false, 16);
    case Match_InvalidSImm16:   return invalidImmediate(Operands, ErrorInfo, true, 16);
    case Match_InvalidOperand:  return invalidOperand(Loc, Operands, ErrorInfo);
    case Match_MnemonicFail:    return Error(Loc, "invalid instruction");
    default:                    return true;
  }
}

// </editor-fold>

extern "C" void LLVMInitializeHS32AsmParser() {
  RegisterMCAsmParser<HS32AsmParser> X(getTheHS32Target());
}
