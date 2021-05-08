include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Register file and instruction descriptions
//===----------------------------------------------------------------------===//

include "HS32RegisterInfo.td"
include "HS32CallingConv.td"
include "HS32InstrInfo.td"

def HS32InstrInfo : InstrInfo {
    let guessInstructionProperties = 0;
}

//===----------------------------------------------------------------------===//
// HS32 processors supported
//===----------------------------------------------------------------------===//

def : ProcessorModel<"generic-hs32", NoSchedModel, []>;

//===----------------------------------------------------------------------===//
// Define HS32 target
//===----------------------------------------------------------------------===//

def HS32AsmParser : AsmParser {
    let ShouldEmitMatchRegisterAltName = 1;
}

def HS32 : Target {
    let InstructionSet = HS32InstrInfo;
    let AssemblyParsers = [HS32AsmParser];
}

//===----------------------------------------------------------------------===//
// To see the expanded form of ADDI, simply execute
// > cd /llvm/lib/Target/HS32/
// > ../../../build/bin/llvm-tblgen HS32.td -I../../../include | grep "def ADDI" -A 20
//===----------------------------------------------------------------------===//
