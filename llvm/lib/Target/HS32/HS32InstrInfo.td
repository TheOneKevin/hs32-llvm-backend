include "HS32InstrFormats.td"

class UImmAsmOperand<int width>
    : AsmOperandClass {
  let Name = "UImm"#width;
  let RenderMethod = "addImmOperands";
  let DiagnosticType = !strconcat("Invalid", Name);
}

def uimm16 : Operand<i16> {
  let ParserMatchClass = UImmAsmOperand<16>;
}

//===----------------------------------------------------------------------===//
// MOV encodings
//===----------------------------------------------------------------------===//

def RMOV    : HSInstTypeR<{0b001, 0b00, 0b000},
                          (outs GPR:$rd), (ins GPR:$rm, GPR:$rn),
                          "MOV \t$rd, $rm, $rn", []>;
def IMOV    : HSInstTypeI<{0b001, 0b00, 0b100},
                          (outs GPR:$rd), (ins GPR:$rm, uimm16:$imm16),
                          "MOV \t$rd, $rm, $imm16", []>;

//===----------------------------------------------------------------------===//
// ALU encodings
//===----------------------------------------------------------------------===//

class ALU_ri<bits<3> family, bits<3> var, string opcodestr>
    : HSInstTypeI<
        {family, 0b00, var},
        (outs GPR:$rd),
        (ins GPR:$rm, uimm16:$imm16),
        opcodestr#"\t$rd, $rm, $imm16", []
    > { }

class ALU_rr<bits<3> family, bits<3> var, string opcodestr>
    : HSInstTypeR<
        {family, 0b00, var},
        (outs GPR:$rd),
        (ins GPR:$rm, GPR:$rn),
        opcodestr#"\t$rd, $rm, $rn", []
    > { }

def RADD    : ALU_rr<0b010, 0b000, "ADD">;
def IADD    : ALU_ri<0b010, 0b100, "ADD">;
def RADDC   : ALU_rr<0b010, 0b001, "ADDC">;
def IADDC   : ALU_ri<0b010, 0b101, "ADDC">;

def RSUB    : ALU_rr<0b011, 0b000, "SUB">;
def ISUB    : ALU_ri<0b011, 0b100, "SUB">;
def RRSUB   : ALU_rr<0b011, 0b001, "RSUB">;
def IRSUB   : ALU_ri<0b011, 0b101, "RSUB">;
def RSUBC   : ALU_rr<0b011, 0b010, "SUBC">;
def ISUBC   : ALU_ri<0b011, 0b110, "SUBC">;
def RRSUBC  : ALU_rr<0b011, 0b011, "RSUBC">;
def IRSUBC  : ALU_ri<0b011, 0b111, "RSUBC">;

def RAND    : ALU_rr<0b100, 0b000, "AND">;
def IAND    : ALU_ri<0b100, 0b100, "AND">;
def RBIC    : ALU_rr<0b100, 0b001, "BIC">;
def IBIC    : ALU_ri<0b100, 0b101, "BIC">;
def ROR     : ALU_ri<0b101, 0b000, "OR">;
def IOR     : ALU_ri<0b101, 0b100, "OR">;
def RXOR    : ALU_rr<0b110, 0b000, "XOR">;
def IXOR    : ALU_ri<0b110, 0b100, "XOR">;

//===----------------------------------------------------------------------===//
// CMP and TST encodings
//===----------------------------------------------------------------------===//

class CMP_ri<bits<3> family, bits<3> var, string opcodestr>
    : HSInstTypeI<
        {family, 0b01, var},
        (outs ),
        (ins GPR:$rm, uimm16:$imm16),
        opcodestr#"\t$rm, $imm16", []
    > { }

class CMP_rr<bits<3> family, bits<3> var, string opcodestr>
    : HSInstTypeR<
        {family, 0b01, var},
        (outs ),
        (ins GPR:$rm, GPR:$rn),
        opcodestr#"\t$rm, $rn", []
    > { }

def RCMP    : CMP_rr<0b011, 0b000, "CMP">;
def ICMP    : CMP_ri<0b011, 0b100, "CMP">;
def RTST    : CMP_rr<0b100, 0b000, "TST">;
def ITST    : CMP_ri<0b100, 0b100, "TST">;
