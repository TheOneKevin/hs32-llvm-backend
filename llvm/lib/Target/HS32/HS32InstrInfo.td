include "HS32InstrFormats.td"

class ImmOperand<string prefix, int width> : AsmOperandClass {
    let Name = prefix#"Imm"#width;
    let RenderMethod = "addImmOperands";
    let DiagnosticType = !strconcat("Invalid", Name);
}
class UImmAsmOperand<int width> : ImmOperand<"U", width> { }
class SImmAsmOperand<int width> : ImmOperand<"S", width> { }

def uimm16 : Operand<i32> {
    let ParserMatchClass = UImmAsmOperand<16>;
}

def simm16 : Operand<i32> {
    let ParserMatchClass = SImmAsmOperand<16>;
    let DecoderMethod = "decodeSImmOperand<16>";
}

def simm16offset : Operand<i32> {
    let ParserMatchClass = SImmAsmOperand<16>;
    let PrintMethod = "printSimmOffsetArg";
    let DecoderMethod = "decodeSImmOperand<16>";
}

//===----------------------------------------------------------------------===//
// LDR and STR memory instructions
//===----------------------------------------------------------------------===//

def ILDR : HSInstTypeI<{0b000, 0b10, 0b100},
                       (outs GPR:$rd), (ins GPR:$rm, simm16offset:$imm16),
                       "ldr\t$rd, [${rm}${imm16}]", []> {
    let mayLoad = true;
}

def RLDR : HSInstTypeR<{0b000, 0b10, 0b001},
                       (outs GPR:$rd), (ins GPR:$rm, GPR:$rn),
                       "ldr\t$rd, [${rm}+${rn}]", []> {
    let mayLoad = true;
    let sh = 0;
}

def ISTR : HSInstTypeI<{0b001, 0b10, 0b100},
                       (outs ), (ins GPR:$rm, simm16offset:$imm16, GPR:$rd),
                       "str\t[${rm}${imm16}], $rd", []> {
    let mayStore = true;
}

def RSTR : HSInstTypeR<{0b001, 0b10, 0b001},
                       (outs ), (ins GPR:$rm, GPR:$rn, GPR:$rd),
                       "str\t[${rm}+${rn}], $rd", []> {
    let mayStore = true;
    let sh = 0;
}

//===----------------------------------------------------------------------===//
// MOV instructions
//===----------------------------------------------------------------------===//

def RMOV : HSInstTypeR<{0b001, 0b00, 0b000},
                       (outs GPR:$rd), (ins GPR:$rn),
                       "mov \t$rd, $rn", []> { let sh = 0; let rm = 0; }
def IMOV : HSInstTypeI<{0b001, 0b00, 0b100},
                       (outs GPR:$rd), (ins uimm16:$imm16),
                       "mov \t$rd, $imm16", []> { let rm = 0; }

//===----------------------------------------------------------------------===//
// ALU instructions
//===----------------------------------------------------------------------===//

class ALU_ri<bits<3> family, bits<3> var, string opcodestr>
    : HSInstTypeI<
        {family, 0b00, var},
        (outs GPR:$rd), (ins GPR:$rm, uimm16:$imm16),
        opcodestr#"\t$rd, $rm, $imm16", []
    > { }

class ALU_rr<bits<3> family, bits<3> var, string opcodestr>
    : HSInstTypeR<
        {family, 0b00, var},
        (outs GPR:$rd), (ins GPR:$rm, GPR:$rn),
        opcodestr#"\t$rd, $rm, $rn", []
    > { let sh = 0; }

def RADD    : ALU_rr<0b010, 0b000, "add">;
def IADD    : ALU_ri<0b010, 0b100, "add">;
def RADDC   : ALU_rr<0b010, 0b001, "addc">;
def IADDC   : ALU_ri<0b010, 0b101, "addc">;

def RSUB    : ALU_rr<0b011, 0b000, "sub">;
def ISUB    : ALU_ri<0b011, 0b100, "sub">;
def RRSUB   : ALU_rr<0b011, 0b001, "rsub">;
def IRSUB   : ALU_ri<0b011, 0b101, "rsub">;
def RSUBC   : ALU_rr<0b011, 0b010, "subc">;
def ISUBC   : ALU_ri<0b011, 0b110, "subc">;
def RRSUBC  : ALU_rr<0b011, 0b011, "rsubc">;
def IRSUBC  : ALU_ri<0b011, 0b111, "rsubc">;

def RAND    : ALU_rr<0b100, 0b000, "and">;
def IAND    : ALU_ri<0b100, 0b100, "and">;
def RBIC    : ALU_rr<0b100, 0b001, "bic">;
def IBIC    : ALU_ri<0b100, 0b101, "bic">;
def ROR     : ALU_ri<0b101, 0b000, "or">;
def IOR     : ALU_ri<0b101, 0b100, "or">;
def RXOR    : ALU_rr<0b110, 0b000, "xor">;
def IXOR    : ALU_ri<0b110, 0b100, "xor">;

//===----------------------------------------------------------------------===//
// CMP and TST instructions
//===----------------------------------------------------------------------===//

class CMP_ri<bits<3> family, bits<3> var, string opcodestr>
    : HSInstTypeI<
        {family, 0b01, var},
        (outs ), (ins GPR:$rm, uimm16:$imm16),
        opcodestr#"\t$rm, $imm16", []
    > { let rd = 0; }

class CMP_rr<bits<3> family, bits<3> var, string opcodestr>
    : HSInstTypeR<
        {family, 0b01, var},
        (outs ), (ins GPR:$rm, GPR:$rn),
        opcodestr#"\t$rm, $rn", []
    > { let sh = 0; let rd = 0; }

def RCMP    : CMP_rr<0b011, 0b000, "cmp">;
def ICMP    : CMP_ri<0b011, 0b100, "cmp">;
def RTST    : CMP_rr<0b100, 0b000, "tst">;
def ITST    : CMP_ri<0b100, 0b100, "tst">;

//===----------------------------------------------------------------------===//
// Branch instructions
//===----------------------------------------------------------------------===//

class BCC_pi<bits<4> cond, string opcodestr>
    : HSInstTypeI<
        {0b0101, cond}, (outs ), (ins simm16:$imm16),
        opcodestr#"\t$imm16", []
    > { let rd = 0; let rm = 0; }
class LCC_pi<bits<4> cond, string opcodestr>
    : HSInstTypeI<
        {0b0111, cond}, (outs ), (ins simm16:$imm16),
        opcodestr#"\t$imm16", []
    > { let rd = 0; let rm = 0; }

def JMP     : BCC_pi<0b0000, "jmp">;
def BEQ     : BCC_pi<0b0001, "beq">;
def BNE     : BCC_pi<0b0010, "bne">;
def BCS     : BCC_pi<0b0011, "bcs">;
def BNC     : BCC_pi<0b0100, "bnc">;
def BSS     : BCC_pi<0b0101, "bss">;
def BNS     : BCC_pi<0b0110, "bns">;
def BOV     : BCC_pi<0b0111, "bov">;
def BNV     : BCC_pi<0b1000, "bnv">;
def BAB     : BCC_pi<0b1001, "bab">;
def BBE     : BCC_pi<0b1010, "bbe">;
def BGE     : BCC_pi<0b1011, "bge">;
def BLT     : BCC_pi<0b1100, "blt">;
def BGT     : BCC_pi<0b1101, "bgt">;
def BLE     : BCC_pi<0b1110, "ble">;

def JAL     : LCC_pi<0b0000, "jal">;
def BALEQ   : LCC_pi<0b0001, "bleq">;
def BALNE   : LCC_pi<0b0010, "blne">;
def BALCS   : LCC_pi<0b0011, "blcs">;
def BALNC   : LCC_pi<0b0100, "blnc">;
def BALSS   : LCC_pi<0b0101, "blss">;
def BALNS   : LCC_pi<0b0110, "blns">;
def BALOV   : LCC_pi<0b0111, "blov">;
def BALNV   : LCC_pi<0b1000, "blnv">;
def BALAB   : LCC_pi<0b1001, "blab">;
def BALBE   : LCC_pi<0b1010, "blbe">;
def BALGE   : LCC_pi<0b1011, "blge">;
def BALLT   : LCC_pi<0b1100, "bllt">;
def BALGT   : LCC_pi<0b1101, "blgt">;
def BALLE   : LCC_pi<0b1110, "blle">;
