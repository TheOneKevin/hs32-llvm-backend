#include "HS32MCExpr.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Object/ELF.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {

const struct ModifierEntry {
  const char *const Name;
  HS32MCExpr::VariantKind VariantKind;
} ModifierNames[] = {
    { "hi", HS32MCExpr::VK_HS32_HI },
    { "lo", HS32MCExpr::VK_HS32_LO },
    { "pcrel", HS32MCExpr::VK_HS32_PCREL }
};

} // end of anonymous namespace

const HS32MCExpr *HS32MCExpr::create(VariantKind Kind, const MCExpr *Expr,
                                     bool Negated, MCContext &Ctx) {
  return new (Ctx) HS32MCExpr(Kind, Expr, Negated);
}

void HS32MCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  assert(Kind != VK_HS32_None);
  OS << '%' << getName() << '(';
  getSubExpr()->print(OS, MAI);
  OS << ')';
}

bool HS32MCExpr::evaluateAsRelocatableImpl(MCValue &Result,
                                           const MCAsmLayout *Layout,
                                           const MCFixup *Fixup) const {
  return getSubExpr()->evaluateAsRelocatable(Result, Layout, Fixup);
}

void HS32MCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*getSubExpr());
}

const char *HS32MCExpr::getName() const {
  const auto &Modifier =
      llvm::find_if(ModifierNames, [this](ModifierEntry const &Mod) {
        return Mod.VariantKind == Kind;
      });

  if(Modifier != std::end(ModifierNames)) {
    return Modifier->Name;
  }
  return nullptr;
}

HS32MCExpr::VariantKind HS32MCExpr::getKindByName(StringRef Name) {
  const auto &Modifier =
      llvm::find_if(ModifierNames, [&Name](ModifierEntry const &Mod) {
        return Mod.Name == Name;
      });

  if(Modifier != std::end(ModifierNames)) {
    return Modifier->VariantKind;
  }
  return VK_HS32_None;
}

bool HS32MCExpr::evaluateAsConstant(int64_t &Result) const {
  MCValue Value;
  if(Kind == VK_HS32_PCREL)
    return false;
  if(!getSubExpr()->evaluateAsRelocatable(Value, nullptr, nullptr))
    return false;
  if(!Value.isAbsolute())
    return false;
  Result = evaluateAsInt64(Value.getConstant());
  return true;
}

int64_t HS32MCExpr::evaluateAsInt64(int64_t Value) const {
  switch(Kind) {
  case VK_HS32_LO:
    return Value & 0xffff;
  case VK_HS32_HI:
    return (Value >> 16) & 0xffff;
  default:
    llvm_unreachable("Invalid kind");
  }
}
