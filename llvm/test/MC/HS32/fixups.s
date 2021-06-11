# RUN: llvm-mc -triple hs32 < %s -show-encoding \
# RUN:     | FileCheck -check-prefix=CHECK-FIXUP %s
# RUN: llvm-mc -filetype=obj -triple hs32 < %s \
# RUN:     | llvm-objdump -d - | FileCheck -check-prefix=CHECK-INSTR %s
# RUN: llvm-mc -filetype=obj -triple hs32 < %s \
# RUN:     | llvm-readobj -r - | FileCheck -check-prefix=CHECK-REL %s

.LBB0:
# CHECK-FIXUP: fixup A - offset: 0, value: %hi(val), kind: fixup_hs32_hi
# CHECK-INSTR: mov r1, -13570
mov r1, %hi(val)
# CHECK-FIXUP: fixup A - offset: 0, value: %lo(val), kind: fixup_hs32_lo
# CHECK-INSTR: mov r1, -17730
mov r1, %lo(val)
# CHECK-FIXUP: fixup A - offset: 0, value: %lo(val), kind: fixup_hs32_lo
# CHECK-INSTR: ldr r1, [r2-17730]
ldr r1, [r2+%lo(val)]
# CHECK-FIXUP: fixup A - offset: 0, value: %lo(.LBB2-.LBB1), kind: fixup_hs32_lo
# CHECK-INSTR: str [r1+1240], r2
str [r1+%lo(.LBB2-.LBB1)], r2
# CHECK-FIXUP: fixup A - offset: 0, value: .LBB0, kind: fixup_hs32_branch
# CHECK-INSTR: jmp -16
jmp .LBB0
# CHECK-FIXUP: fixup A - offset: 0, value: .LBB0, kind: fixup_hs32_branch
# CHECK-INSTR: bleq -20
bleq .LBB0
# CHECK-FIXUP: fixup A - offset: 0, value: %pcrel(.LBB2), kind: fixup_hs32_pcrel_lo
# CHECK-INSTR: str [pc+1248], r2
str [pc+%pcrel(.LBB2)], r2

.LBB1:
.fill 1236
add r0, r0, 0

.LBB2:
.set val, 0xcafebabe

# Ensure all symbols are resolved
# CHECK-REL-NOT: R_HS32
