include "HS32InstrFormats.td"

class UImmAsmOperand<int width>
    : AsmOperandClass {
  let Name = "UImm"#width;
  let RenderMethod = "addImmOperands";
}

def uimm16 : Operand<i16> {
  let ParserMatchClass = UImmAsmOperand<16>;
}

class ALU_ri<bits<3> family, bits<3> var, string opcodestr>
    : HSInstTypeI<
        {family, 0b00, var},
        (outs GPR:$rd),
        (ins GPR:$rm, simm16:$imm16),
        opcodestr#"\t$rd, $rm, $imm16", []
    > { }


def IADD    : ALU_ri<0b010, 0b100, "ADD">;
def IADDC   : ALU_ri<0b010, 0b101, "ADDC">;

def ISUB    : ALU_ri<0b011, 0b100, "SUB">;
def IRSUB   : ALU_ri<0b011, 0b101, "RSUB">;
def ISUBC   : ALU_ri<0b011, 0b110, "SUBC">;
def IRSUBC  : ALU_ri<0b011, 0b111, "RSUBC">;

def IAND    : ALU_ri<0b100, 0b100, "AND">;
def IBIC    : ALU_ri<0b100, 0b101, "BIC">;
def IOR     : ALU_ri<0b101, 0b100, "OR">;
def IXOR    : ALU_ri<0b110, 0b100, "XOR">;


