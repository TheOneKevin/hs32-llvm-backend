#ifndef LLVM_HS32FIXUPKINDS_H
#define LLVM_HS32FIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

#undef HS32

namespace llvm {
namespace HS32 {

enum Fixups {
  fixup_hs32_hi = FirstTargetFixupKind,
  fixup_hs32_lo,
  fixup_hs32_pcrel_hi,
  fixup_hs32_pcrel_lo,
  fixup_hs32_branch,
  fixup_hs32_invalid,

  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};

} // end namespace HS32
} // end namespace llvm

#endif // LLVM_HS32FIXUPKINDS_H
