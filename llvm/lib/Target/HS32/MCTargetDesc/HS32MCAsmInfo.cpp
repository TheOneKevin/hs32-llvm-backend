#include "HS32MCAsmInfo.h"
#include "llvm/ADT/Triple.h"
using namespace llvm;

void HS32ELFMCAsmInfo::anchor() {}

HS32ELFMCAsmInfo::HS32ELFMCAsmInfo(const Triple &TT) {
  CodePointerSize = 4;
  CalleeSaveStackSlotSize = 4;
  IsLittleEndian = false;
  SupportsDebugInformation = true;
  CommentString = ";";
}
