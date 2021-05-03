//===----------------------------------------------------------------------===//
// Encoding patterns for instruction types
//===----------------------------------------------------------------------===//

class HSInst<dag outs, dag ins, string asmstr, list<dag> pattern>
    : Instruction
{
    field bits<32> Inst;
    let Size = 4;

    bits<8> Opcode = 0;
    let Inst{31-24} = Opcode;

    let Namespace = "HS32";
    let AsmString = asmstr;
    let Pattern = pattern;
    field bits<32> SoftFail = 0;
    dag OutOperandList = outs;
    dag InOperandList = ins;
}

// I-type instruction encoding
class HSInstTypeI<bits<8> opcode, dag outs, dag ins, string asmstr, list<dag> pattern>
    : HSInst<outs, ins, asmstr, pattern>
{
    bits<4> rd;
    bits<4> rm;
    bits<16> imm16;

    let Opcode = opcode;
    let Inst{23-20} = rd;
    let Inst{19-16} = rm;
    let Inst{15-0} = imm16;
}

// R-type instruction encoding with 0 shift and 0 banks
class HSInstTypeR<bits<8> opcode, dag outs, dag ins, string asmstr, list<dag> pattern>
    : HSInst<outs, ins, asmstr, pattern>
{
    bits<4> rd;
    bits<4> rm;
    bits<4> rn;

    let Opcode = opcode;
    let Inst{23-20} = rd;
    let Inst{19-16} = rm;
    let Inst{15-12} = rn;
    let Inst{11-0} = 0;
}

// Same as HSInstTypeR with shifted rn
// where rnsh = { rn (4 bits), shtype (2), shval (5) }
class HSInstTypeRs<bits<8> opcode, dag outs, dag ins, string asmstr, list<dag> pattern>
    : HSInst<outs, ins, asmstr, pattern>
{
    bits<4> rd;
    bits<4> rm;
    bits<11> rnsh;

    let Opcode = opcode;
    let Inst{23-20} = rd;
    let Inst{19-16} = rm;
    let Inst{15-5} = rnsh;
    let Inst{4-0} = 0;
}
