#ifndef LLVM_HS32MCASMINFO_H
#define LLVM_HS32MCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
class Triple;

class HS32ELFMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit HS32ELFMCAsmInfo(const Triple &Triple);
};

} // namespace llvm

#endif // LLVM_HS32MCASMINFO_H
