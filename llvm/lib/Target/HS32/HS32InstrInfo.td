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

def uimm16 : Operand<i32>, ImmLeaf<i32, [{ return isUInt<16>(Imm); }]> {
    let ParserMatchClass = UImmAsmOperand<16>;
}

def simm16 : Operand<i32>, ImmLeaf<i32, [{ return isInt<16>(Imm); }]> {
    let ParserMatchClass = SImmAsmOperand<16>;
    let DecoderMethod = "decodeSImmOperand<16>";
}

// This is same as simm16 with custom PrintMethod
def simm16offset : Operand<i32>, ImmLeaf<i32, [{ return isInt<16>(Imm); }]> {
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

def shift_rn : Operand<i32>,
               ComplexPattern<i32, 2, "SelectShiftOperand", [shl,srl,sra,rotr]> {
    let ParserMatchClass = ShiftRegAsmOperand;
    let PrintMethod = "printShiftOperand";
    let EncoderMethod = "getShiftOpValue";
    let DecoderMethod = "decodeShiftOperand";
    let MIOperandInfo = (ops GPR, i32imm);
}

//===----------------------------------------------------------------------===//
// Custom DAG node types and PatLeafs
// - See TargetSelectionDAG.td for DAG type descriptions (used by HS32ISD::*)
//===----------------------------------------------------------------------===//

// Signed 32 bit immediate
def simm32 : ImmLeaf<i32, [{ return isInt<32>(Imm); }]>;

// Immediate where bits [15:0] are 0
def lo16zeros : PatLeaf<(i32 imm), [{
    return ((uint32_t) N->getZExtValue() & 0xffff) == 0;
    }]>;

def HS32Retflag : SDNode<"HS32ISD::RET_FLAG", SDTNone,
                         [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def HI16 : SDNodeXForm<imm, [{
    return CurDAG->getTargetConstant(
     ((uint32_t) N->getZExtValue() >> 16) & 0xffff, SDLoc(N), N->getValueType(0));
    }]>;

def LO16 : SDNodeXForm<imm, [{
    return CurDAG->getTargetConstant(
     (uint32_t) N->getZExtValue() & 0xffff, SDLoc(N), N->getValueType(0));
    }]>;

//===----------------------------------------------------------------------===//
// LDR and STR memory instructions
//===----------------------------------------------------------------------===//

let mayLoad = 1 in {
multiclass LdrOp<string opcodestr> {
    // Imm
    def ri : HSInstTypeI<{0b000, 0b10, 0b100},
        (outs GPR:$rd), (ins GPR:$rm, simm16offset:$imm16),
        opcodestr#"\t$rd, [${rm}${imm16}]", []>;

    // Reg no shift
    def rr : HSInstTypeR<{0b000, 0b10, 0b001},
        (outs GPR:$rd), (ins GPR:$rm, GPR:$rn),
        opcodestr#"\t$rd, [${rm}+${rn}]", []>;

    // Reg shifted
    def rsh : HSInstTypeRs<{0b000, 0b10, 0b001},
        (outs GPR:$rd), (ins GPR:$rm, shift_rn:$rnsh),
        opcodestr#"\t$rd, [${rm}+${rnsh}]", []>;
}}

let mayStore = 1 in {
multiclass StrOp<string opcodestr> {
    // Imm
    def ri : HSInstTypeI<{0b001, 0b10, 0b100},
        (outs ), (ins GPR:$rd, GPR:$rm, simm16offset:$imm16),
        opcodestr#"\t[${rm}${imm16}], $rd", []>;

    // Reg no shift
    def rr : HSInstTypeR<{0b001, 0b10, 0b001},
        (outs ), (ins GPR:$rd, GPR:$rm, GPR:$rn),
        opcodestr#"\t[${rm}+${rn}], $rd", []>;

    // Reg shifted
    def rsh : HSInstTypeRs<{0b001, 0b10, 0b001},
        (outs ), (ins GPR:$rd, GPR:$rm, shift_rn:$rnsh),
        opcodestr#"\t[${rm}+${rnsh}], $rd", []>;
}}

defm LDR : LdrOp<"ldr">;
defm STR : StrOp<"str">;

//===----------------------------------------------------------------------===//
// MOV instructions
//===----------------------------------------------------------------------===//

// MovOp - same as ALUOp with rm zeroed
let rm = 0, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {
    multiclass MovOp<bits<3> family, bits<2> var, string opcodestr> {
        // Imm
        def ri : HSInstTypeI<{family, 0b00, 0b1,var},
            (outs GPR:$rd), (ins simm16:$imm16),
            opcodestr#"\t$rd, $imm16",
            [(set GPR:$rd, simm16:$imm16)]>;

        // Reg no shift
        def rr : HSInstTypeR<{family, 0b00, 0b0,var},
            (outs GPR:$rd), (ins GPR:$rn),
            opcodestr#"\t$rd, $rn", []>;

        // Reg shifted
        def rsh : HSInstTypeRs<{family, 0b00, 0b0,var},
            (outs GPR:$rd), (ins shift_rn:$rnsh),
            opcodestr#"\t$rd, $rnsh", []>;
    }

    // Special move upper imm instruction
    let Constraints = "$src = $rd" in {
    def MOVTri : HSInstTypeI<{0b001, 0b00, 0b110},
            (outs GPR:$rd), (ins GPR:$src, simm16:$imm16),
            "movt\t$rd, $imm16", [
                (set GPR:$rd, (or (and GPR:$src, 0xffff), lo16zeros:$imm16))
            ]>;
    }
}

defm MOV : MovOp<0b001, 0b00, "mov">;

//===----------------------------------------------------------------------===//
// ALU instructions
//===----------------------------------------------------------------------===//

// Describes binary operations, with reg, imm and reg/sh variants
// defined as ri, rr and rr_sh.
let Defs = [FLAGS] in {
multiclass ALUOp<bits<3> family, bits<2> var, string opcodestr,
                 SDPatternOperator opnode> {
    // Reg, Imm
    def ri : HSInstTypeI<{family, 0b00, 0b1,var},
        (outs GPR:$rd), (ins GPR:$rm, simm16:$imm16),
        opcodestr#"\t$rd, $rm, $imm16",
        [(set GPR:$rd, (opnode GPR:$rm, simm16:$imm16))]>;

    // Reg, Reg no shift
    def rr : HSInstTypeR<{family, 0b00, 0b0,var},
        (outs GPR:$rd), (ins GPR:$rm, GPR:$rn),
        opcodestr#"\t$rd, $rm, $rn",
        [(set GPR:$rd, (opnode GPR:$rm, GPR:$rn))]>;

    // Reg, Reg shifted
    def rsh : HSInstTypeRs<{family, 0b00, 0b0,var},
        (outs GPR:$rd), (ins GPR:$rm, shift_rn:$rnsh),
        opcodestr#"\t$rd, $rm, $rnsh",
        [(set GPR:$rd, (opnode GPR:$rm, shift_rn:$rnsh))]>;
}}

// Define the custom operators
def  bic   : PatFrag<(ops node:$a, node:$b), (and node:$a, (not node:$b))>;
def  rsub  : PatFrag<(ops node:$a, node:$b), (sub node:$b, node:$a)>;
def  rsubc : PatFrag<(ops node:$a, node:$b), (subc node:$b, node:$a)>;

defm ADD    : ALUOp<0b010, 0b00, "add",   add>;
defm ADDC   : ALUOp<0b010, 0b01, "addc",  addc>;
defm SUB    : ALUOp<0b011, 0b00, "sub",   sub>;
defm SUBC   : ALUOp<0b011, 0b10, "subc",  subc>;
defm AND    : ALUOp<0b100, 0b00, "and",   and>;
defm BIC    : ALUOp<0b100, 0b01, "bic",   bic>;
defm OR     : ALUOp<0b101, 0b00, "or",    or>;
defm XOR    : ALUOp<0b110, 0b00, "xor",   xor>;

//===----------------------------------------------------------------------===//
// CMP and TST instructions
//===----------------------------------------------------------------------===//

// CmpOp - same as ALUOp except with different operands
let rd = 0, Defs = [FLAGS] in {
multiclass CmpOp<bits<3> family, bits<2> var, string opcodestr> {
    // Reg, Imm
    def ri : HSInstTypeI<{family, 0b01, 0b1,var},
        (outs ), (ins GPR:$rm, simm16:$imm16),
        opcodestr#"\t$rm, $imm16", []>;

    // Reg, Reg no shift
    def rr : HSInstTypeR<{family, 0b01, 0b0,var},
        (outs ), (ins GPR:$rm, GPR:$rn),
        opcodestr#"\t$rm, $rn", []>;

    // Reg, Reg shifted
    def rsh : HSInstTypeRs<{family, 0b00, 0b0,var},
        (outs ), (ins GPR:$rm, shift_rn:$rnsh),
        opcodestr#"\t$rm, $rnsh", []>;
}}

defm CMP    : CmpOp<0b011, 0b00, "cmp">;
defm TST    : CmpOp<0b100, 0b00, "tst">;

//===----------------------------------------------------------------------===//
// Branch and return instructions
//===----------------------------------------------------------------------===//

// BranchOp - defines the "branch" and "branch and link" variants
// described by b and bl
let rd = 0, rm = 0, isBranch = 1 in {
multiclass BranchOp<bits<4> cond, string opcodestr_b, string opcodestr_bl> {
    // branch
    def b : HSInstTypeI<{0b0101, cond},
        (outs ), (ins simm16:$imm16), opcodestr_b#"\t$imm16", []
    > {
        let TSFlags{0} = 1;
    }
    // branch and link
    def bl : HSInstTypeI<{0b0111, cond},
        (outs ), (ins simm16:$imm16), opcodestr_bl#"\t$imm16", []
    > {
        let TSFlags{0} = 1;
        let isCall = 1;
        let Defs = [LR];
    }
}}

defm JMP    : BranchOp<0b0000, "jmp", "jal">;
defm BEQ    : BranchOp<0b0001, "beq", "bleq">;
defm BNE    : BranchOp<0b0010, "bne", "blne">;
defm BCS    : BranchOp<0b0011, "bcs", "blcs">;
defm BNC    : BranchOp<0b0100, "bnc", "blnc">;
defm BSS    : BranchOp<0b0101, "bss", "blss">;
defm BNS    : BranchOp<0b0110, "bns", "blns">;
defm BOV    : BranchOp<0b0111, "bov", "blov">;
defm BNV    : BranchOp<0b1000, "bnv", "blnv">;
defm BAB    : BranchOp<0b1001, "bab", "blab">;
defm BBE    : BranchOp<0b1010, "bbe", "blbe">;
defm BGE    : BranchOp<0b1011, "bge", "blge">;
defm BLT    : BranchOp<0b1100, "blt", "bllt">;
defm BGT    : BranchOp<0b1101, "bgt", "blgt">;
defm BLE    : BranchOp<0b1110, "ble", "blle">;

let isBarrier = 1, isReturn = 1, isTerminator = 1 in
def RET : Pseudo<(outs ), (ins ), [(HS32Retflag)]>,
          PseudoInstExpansion<(MOVrr PC, LR)>;

//===----------------------------------------------------------------------===//
// Codegen specific patterns
//===----------------------------------------------------------------------===//

// Materialize 32-bit signed immediates
def : Pat<(simm32:$imm), (MOVTri (MOVri (LO16 imm:$imm)), (HI16 imm:$imm))>;

// Match load patterns
// TODO: proper s/zext load support (see HS32ISelLowering.cpp)
multiclass LdrPat<PatFrag LoadOp, HSInst Inst> {
    def : Pat<(LoadOp GPR:$rm), (Inst GPR:$rm, 0)>;
    def : Pat<(LoadOp (add GPR:$rm, simm16:$imm)), (Inst GPR:$rm, simm16:$imm)>;
}
defm : LdrPat<load, LDRri>;

// Match store patterns
// TODO: proper trunc support (see HS32ISelLowering.cpp)
multiclass StrPat<PatFrag StoreOp, HSInst Inst> {
    def : Pat<(StoreOp GPR:$rd, GPR:$rm), (Inst GPR:$rd, GPR:$rm, 0)>;
    def : Pat<(StoreOp GPR:$rd, (add GPR:$rm, simm16:$imm)),
              (Inst GPR:$rd, GPR:$rm, simm16:$imm)>;
}
defm : StrPat<store, STRri>;
