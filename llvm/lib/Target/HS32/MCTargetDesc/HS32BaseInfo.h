#ifndef LLVM_HS32BASEINFO_H
#define LLVM_HS32BASEINFO_H

#include "HS32MCTargetDesc.h"

namespace llvm {

// All target-specific flags used in HS32InstrFormats.td
namespace HS32II {

const uint64_t InstBranchMask = 1 << 0;

} // end namespace HS32II
} // end namespace llvm

#endif // LLVM_HS32BASEINFO_H
