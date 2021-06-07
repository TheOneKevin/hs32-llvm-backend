#ifndef LLVM_HS32BASEINFO_H
#define LLVM_HS32BASEINFO_H

#include "HS32MCTargetDesc.h"
#include "llvm/ADT/StringSwitch.h"

namespace llvm {

// All target-specific flags used in HS32InstrFormats.td
namespace HS32II {

const uint64_t InstBranchMask = 1 << 0;

// Types for Rn register shifting
enum ShiftTypes {
  shl = 0, shr = 1, srx = 2, ror = 3, Invalid
};

// Machine operands (for global lowering)
enum {
  MO_None, MO_Hi, MO_Lo, MO_PcrelHi
};

static inline ShiftTypes getShiftTypeFromString(StringRef Name) {
  return StringSwitch<ShiftTypes>(Name)
      .Case("shl", ShiftTypes::shl)
      .Case("shr", ShiftTypes::shr)
      .Case("ror", ShiftTypes::ror)
      .Case("srx", ShiftTypes::srx)
      .Default(ShiftTypes::Invalid);
}

static inline const char *getStringFromShiftType(ShiftTypes Type) {
  switch(Type) {
  case shl: return "shl";
  case shr: return "shr";
  case ror: return "ror";
  case srx: return "srx";
  default: return "";
  }
}

} // end namespace HS32II
} // end namespace llvm

#endif // LLVM_HS32BASEINFO_H
