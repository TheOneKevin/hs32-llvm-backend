include "HS32InstrFormats.td"

//===----------------------------------------------------------------------===//
// Operand definitions
//===----------------------------------------------------------------------===//

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

// This is same as simm16 with custom PrintMethod
def simm16offset : Operand<i32> {
    let ParserMatchClass = SImmAsmOperand<16>;
    let PrintMethod = "printSimmOffsetOperand";
    let DecoderMethod = "decodeSImmOperand<16>";
}

// Operand that is 7 bits and encodes the shift type and amount
def ShiftRegAsmOperand : AsmOperandClass {
    let Name = "ShiftReg";
    let RenderMethod = "addShiftOperands";
    let DiagnosticType = !strconcat("Invalid", Name);
}

def shift_rn : Operand<i32> {
    let ParserMatchClass = ShiftRegAsmOperand;
    let PrintMethod = "printShiftOperand";
    let EncoderMethod = "getShiftOpValue";
    let DecoderMethod = "decodeShiftOperand";
    let MIOperandInfo = (ops GPR, i32imm);
}

//===----------------------------------------------------------------------===//
// LDR and STR memory instructions
//===----------------------------------------------------------------------===//

let mayLoad = true in {
multiclass LDR_op<string opcodestr> {
    // Imm
    def ri : HSInstTypeI<{0b000, 0b10, 0b100},
        (outs GPR:$rd), (ins GPR:$rm, simm16offset:$imm16),
        opcodestr#"\t$rd, [${rm}${imm16}]", []>;

    // Reg no shift
    def rr : HSInstTypeR<{0b000, 0b10, 0b001},
        (outs GPR:$rd), (ins GPR:$rm, GPR:$rn),
        opcodestr#"\t$rd, [${rm}+${rn}]", []>;

    // Reg shifted
    def rr_sh : HSInstTypeRs<{0b000, 0b10, 0b001},
        (outs GPR:$rd), (ins GPR:$rm, shift_rn:$rnsh),
        opcodestr#"\t$rd, [${rm}+${rnsh}]", []>;
}}

let mayStore = true in {
multiclass STR_op<string opcodestr> {
    // Imm
    def ri : HSInstTypeI<{0b001, 0b10, 0b100},
        (outs GPR:$rd), (ins GPR:$rm, simm16offset:$imm16),
        opcodestr#"\t[${rm}${imm16}], $rd", []>;

    // Reg no shift
    def rr : HSInstTypeR<{0b001, 0b10, 0b001},
        (outs GPR:$rd), (ins GPR:$rm, GPR:$rn),
        opcodestr#"\t[${rm}+${rn}], $rd", []>;

    // Reg shifted
    def rr_sh : HSInstTypeRs<{0b001, 0b10, 0b001},
        (outs GPR:$rd), (ins GPR:$rm, shift_rn:$rnsh),
        opcodestr#"\t[${rm}+${rnsh}], $rd", []>;
}}

defm LDR : LDR_op<"ldr">;
defm STR : STR_op<"str">;

//===----------------------------------------------------------------------===//
// MOV instructions
//===----------------------------------------------------------------------===//

// MOV_op - same as ALU_op with rm zeroed
let rm = 0 in {
multiclass MOV_op<bits<3> family, bits<2> var, string opcodestr> {
    // Imm
    def ri : HSInstTypeI<{family, 0b00, 0b1,var},
        (outs GPR:$rd), (ins uimm16:$imm16),
        opcodestr#"\t$rd, $imm16", []>;

    // Reg no shift
    def rr : HSInstTypeR<{family, 0b00, 0b0,var},
        (outs GPR:$rd), (ins GPR:$rn),
        opcodestr#"\t$rd, $rn", []>;

    // Reg shifted
    def rr_sh : HSInstTypeRs<{family, 0b00, 0b0,var},
        (outs GPR:$rd), (ins shift_rn:$rnsh),
        opcodestr#"\t$rd, $rnsh", []>;
}}

defm MOV : MOV_op<0b001, 0b00, "mov">;

//===----------------------------------------------------------------------===//
// ALU instructions
//===----------------------------------------------------------------------===//

// Describes binary operations, with reg, imm and reg/sh variants
// defined as ri, rr and rr_sh.
multiclass ALU_op<bits<3> family, bits<2> var, string opcodestr> {
    // Reg, Imm
    def ri : HSInstTypeI<{family, 0b00, 0b1,var},
        (outs GPR:$rd), (ins GPR:$rm, uimm16:$imm16),
        opcodestr#"\t$rd, $rm, $imm16", []>;

    // Reg, Reg no shift
    def rr : HSInstTypeR<{family, 0b00, 0b0,var},
        (outs GPR:$rd), (ins GPR:$rm, GPR:$rn),
        opcodestr#"\t$rd, $rm, $rn", []>;

    // Reg, Reg shifted
    def rr_sh : HSInstTypeRs<{family, 0b00, 0b0,var},
        (outs GPR:$rd), (ins GPR:$rm, shift_rn:$rnsh),
        opcodestr#"\t$rd, $rm, $rnsh", []>;
}

defm ADD    : ALU_op<0b010, 0b00, "add">;
defm ADDC   : ALU_op<0b010, 0b01, "addc">;
defm SUB    : ALU_op<0b011, 0b00, "sub">;
defm SUBC   : ALU_op<0b011, 0b10, "subc">;
defm RSUB   : ALU_op<0b011, 0b01, "rsub">;
defm RSUBC  : ALU_op<0b011, 0b11, "rsubc">;
defm AND    : ALU_op<0b100, 0b00, "and">;
defm BIC    : ALU_op<0b100, 0b01, "bic">;
defm OR     : ALU_op<0b101, 0b00, "or">;
defm XOR    : ALU_op<0b110, 0b00, "xor">;

//===----------------------------------------------------------------------===//
// CMP and TST instructions
//===----------------------------------------------------------------------===//

// CMP_op - same as ALU_op except with different operands
let rd = 0 in {
multiclass CMP_op<bits<3> family, bits<2> var, string opcodestr> {
    // Reg, Imm
    def ri : HSInstTypeI<{family, 0b01, 0b1,var},
        (outs ), (ins GPR:$rm, uimm16:$imm16),
        opcodestr#"\t$rm, $imm16", []>;

    // Reg, Reg no shift
    def rr : HSInstTypeR<{family, 0b01, 0b0,var},
        (outs ), (ins GPR:$rm, GPR:$rn),
        opcodestr#"\t$rm, $rn", []>;

    // Reg, Reg shifted
    def rr_sh : HSInstTypeRs<{family, 0b00, 0b0,var},
        (outs ), (ins GPR:$rm, shift_rn:$rnsh),
        opcodestr#"\t$rm, $rnsh", []>;
}}

defm CMP    : CMP_op<0b011, 0b00, "cmp">;
defm TST    : CMP_op<0b100, 0b00, "tst">;

//===----------------------------------------------------------------------===//
// Branch instructions
//===----------------------------------------------------------------------===//

// BRANCH_op - defines the "branch" and "branch and link" variants
// described by b and bl
let rd = 0, rm = 0 in {
multiclass BRANCH_op<bits<4> cond, string opcodestr_b, string opcodestr_bl> {
    // branch
    def b : HSInstTypeI<{0b0101, cond}, (outs ), (ins simm16:$imm16),
            opcodestr_b#"\t$imm16", []> { let TSFlags{0} = 1; }
    // branch and link
    def bl : HSInstTypeI<{0b0111, cond}, (outs ), (ins simm16:$imm16),
             opcodestr_bl#"\t$imm16", []> { let TSFlags{0} = 1; }
}}

defm JMP    : BRANCH_op<0b0000, "jmp", "jal">;
defm BEQ    : BRANCH_op<0b0001, "beq", "bleq">;
defm BNE    : BRANCH_op<0b0010, "bne", "blne">;
defm BCS    : BRANCH_op<0b0011, "bcs", "blcs">;
defm BNC    : BRANCH_op<0b0100, "bnc", "blnc">;
defm BSS    : BRANCH_op<0b0101, "bss", "blss">;
defm BNS    : BRANCH_op<0b0110, "bns", "blns">;
defm BOV    : BRANCH_op<0b0111, "bov", "blov">;
defm BNV    : BRANCH_op<0b1000, "bnv", "blnv">;
defm BAB    : BRANCH_op<0b1001, "bab", "blab">;
defm BBE    : BRANCH_op<0b1010, "bbe", "blbe">;
defm BGE    : BRANCH_op<0b1011, "bge", "blge">;
defm BLT    : BRANCH_op<0b1100, "blt", "bllt">;
defm BGT    : BRANCH_op<0b1101, "bgt", "blgt">;
defm BLE    : BRANCH_op<0b1110, "ble", "blle">;
