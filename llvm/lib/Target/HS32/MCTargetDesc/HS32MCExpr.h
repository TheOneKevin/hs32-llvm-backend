#ifndef LLVM_HS32MCEXPR_H
#define LLVM_HS32MCEXPR_H

#include "llvm/MC/MCExpr.h"

namespace llvm {

class HS32MCExpr : public MCTargetExpr {
public:
  enum VariantKind {
    VK_HS32_None = 0,
    VK_HS32_HI,
    VK_HS32_LO,
    VK_HS32_PCREL
  };

private:
  const VariantKind Kind;
  const MCExpr *SubExpr;
  bool Negated;
  int64_t evaluateAsInt64(int64_t Value) const;

private:
  explicit HS32MCExpr(VariantKind Kind, const MCExpr *Expr, bool Negated)
      : Kind(Kind), SubExpr(Expr), Negated(Negated) { }
  ~HS32MCExpr() { }

public:
  // Gets the type of the expression
  VariantKind getKind() const { return Kind; }
  // Gets the name of the expression
  const char *getName() const;
  const MCExpr *getSubExpr() const { return SubExpr; }
  // Evaluates the fixup as a constant
  bool evaluateAsConstant(int64_t &Result) const;

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Ressult, const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;
  MCFragment *findAssociatedFragment() const override {
    return getSubExpr()->findAssociatedFragment();
  }
  void fixELFSymbolsInTLSFixups(MCAssembler &Asm) const override { }

public:
  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }
  static bool classof(const HS32MCExpr* E) { return true; }
  // Creates HS32 machine code expression
  static const HS32MCExpr *create(VariantKind Kind, const MCExpr *Expr,
                                  bool Negated, MCContext &Ctx);
  // Gets the kind of expression from its name
  static VariantKind getKindByName(StringRef Name);
};

} // end namespace llvm

#endif // LLVM_HS32MCEXPR_H
