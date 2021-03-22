include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Register file and instruction descriptions
//===----------------------------------------------------------------------===//

include "HS32RegisterInfo.td"
include "HS32InstrInfo.td"

def HS32InstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// HS32 processors supported
//===----------------------------------------------------------------------===//

def : ProcessorModel<"generic-hs32", NoSchedModel, []>;

//===----------------------------------------------------------------------===//
// Define HS32 target
//===----------------------------------------------------------------------===//

def HS32 : Target {
    let InstructionSet = HS32InstrInfo;
}

//===----------------------------------------------------------------------===//
// To see the expanded form of ADDI, simply execute
// > cd /llvm/lib/Target/HS32/
// > ../../../build/bin/llvm-tblgen HS32.td -I../../../include | grep "def ADDI" -A 20
//===----------------------------------------------------------------------===//
