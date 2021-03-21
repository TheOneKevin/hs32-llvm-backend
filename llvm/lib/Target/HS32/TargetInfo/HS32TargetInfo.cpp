#include "HS32TargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

namespace llvm {
Target &getTheHS32Target() {
  static Target TheHS32Target;
  return TheHS32Target;
}
} // namespace llvm

extern "C" void LLVMInitializeHS32TargetInfo() {
  RegisterTarget<Triple::hs32, false> X(
      getTheHS32Target(), "hs32", "HS32 Processor Family", "HS32");
}
