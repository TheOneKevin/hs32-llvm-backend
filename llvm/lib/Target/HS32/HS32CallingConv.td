//===----------------------------------------------------------------------===//
// HS32 return value calling convention
//===----------------------------------------------------------------------===//

def RetCC_HS32 : CallingConv
<[
    CCIfType<[ i8, i16 ], CCPromoteToType<i32>>,
    CCIfType<[i32], CCAssignToReg<[ R10, R11 ]>>
]>;

def CC_HS32 : CallingConv
<[
    CCIfType<[ i8, i16 ], CCPromoteToType<i32>>,
    CCIfType<[i32], CCAssignToReg<[ R1, R2, R3, R4, R5, R6, R7, R8, R9 ]>>,
    CCAssignToStack<4, 4>
]>;

def CSR : CalleeSavedRegs<(add (sequence "R%u", 1, 9), FP, SP, LR)>;
