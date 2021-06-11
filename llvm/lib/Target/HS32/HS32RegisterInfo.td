//===----------------------------------------------------------------------===//
// Description of HS32 register file
//===----------------------------------------------------------------------===//

let Namespace = "HS32" in {
    class HS32Reg<bits<4> enc, string n, list<string> alt = []> : Register<n, alt> {
        let HWEncoding{3-0} = enc;
    }
    def RegNamesRaw : RegAltNameIndex;
}

// General purpose registers
foreach Index = 0-12 in {
    def "R"#Index : HS32Reg<Index, "r"#Index>, DwarfRegNum<[Index]>;
}

// Special registers
let RegAltNameIndices = [RegNamesRaw] in {
    def FP  : HS32Reg<12, "fp", ["r12"]>, DwarfRegNum<[12]>;
    def SP  : HS32Reg<13, "sp", ["r13"]>,  DwarfRegNum<[13]>;
    def LR  : HS32Reg<14, "lr", ["r14"]>,  DwarfRegNum<[14]>;
    def PC  : HS32Reg<15, "pc", ["r15"]>,  DwarfRegNum<[15]>;
}

// The order of registers is the preferred allocation sequence
def GPR : RegisterClass<"HS32", [i32], 32, (add (sequence "R%u", 0, 12), FP, SP, LR, PC)>;

// Arithmetic flags
def FLAGS : HS32Reg<15, "flags">, DwarfRegNum<[69]>;
def PSR : RegisterClass<"HS32", [i32], 32, (add FLAGS)>;
