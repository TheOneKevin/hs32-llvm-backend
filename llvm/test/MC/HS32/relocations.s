# RUN: llvm-mc -triple hs32 < %s -show-encoding \
# RUN:     | FileCheck -check-prefix=CHECK-INSTR -check-prefix=CHECK-FIXUP %s
# RUN: llvm-mc -filetype=obj -triple hs32 < %s \
# RUN:     | llvm-readobj -r - | FileCheck -check-prefix=CHECK-REL %s

# Check prefixes
# CHECK-INSTR   - check instruction printout
# CHECK-FIXUP   - check fixup
# CHECK-REL     - check relocations

# CHECK-INSTR: mov r1, %hi(foo)
# CHECK-FIXUP: fixup A - offset: 0, value: %hi(foo), kind: fixup_hs32_hi
# CHECK-REL: 0x0 R_HS32_HI foo
mov r1, %hi(foo)
# CHECK-INSTR: mov r1, %lo(foo-8)
# CHECK-FIXUP: fixup A - offset: 0, value: %lo(foo-8), kind: fixup_hs32_lo
# CHECK-REL: 0x4 R_HS32_LO foo
mov r1, %lo(foo)-8
# CHECK-INSTR: mov r1, %hi(foo+(2*8))
# CHECK-FIXUP: fixup A - offset: 0, value: %hi(foo+(2*8)), kind: fixup_hs32_hi
# CHECK-REL: 0x8 R_HS32_HI foo
mov r1, %hi(foo)+2*8
# CHECK-INSTR: jmp foo
# CHECK-FIXUP: fixup A - offset: 0, value: foo, kind: fixup_hs32_branch
# CHECK-REL: 0xC R_HS32_BRANCH foo
jmp foo
# CHECK-INSTR: jal foo
# CHECK-FIXUP: fixup A - offset: 0, value: foo, kind: fixup_hs32_branch
# CHECK-REL: 0x10 R_HS32_BRANCH foo
jal foo
