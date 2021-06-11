# RUN: llvm-mc -filetype=obj -triple hs32 < %s \
# RUN:     | llvm-objdump -d --print-imm-hex - | FileCheck -check-prefix=CHECK-HEX %s
# RUN: llvm-mc -filetype=obj -triple hs32 < %s \
# RUN:     | llvm-objdump -d - | FileCheck -check-prefix=CHECK %s
# RUN: llvm-mc -filetype=obj -triple hs32 < %s \
# RUN:     | llvm-readobj -r - | FileCheck -check-prefix=CHECK-REL %s

# Check prefixes
# CHECK     - instruction with decimal imm
# CHECK-HEX - instruction with hex imm
# CHECK-REL - relocations emitted

mov r2, %lo(0xffff)+1+0xcafe
mov r2, %hi(0xffff1234)
# CHECK-HEX: mov r2, -0x3502
# CHECK-HEX: mov r2, -0x1

.equ addr, 0xdeadbeef
jal %hi(addr)
bne %lo(addr)
# CHECK: jal -8531
# CHECK: bne -16657

tmp1: .fill 1234
tmp2:
    mov r3, %hi(tmp2-tmp1)
    mov r3, %lo(tmp2-tmp1)
# CHECK: mov r3, 0
# CHECK: mov r3, 1234

tmp3:
    mov r5, %lo(tmp3-tmp1)
    mov r5, %lo(tmp3-tmp2)
# CHECK: mov r5, 1242
# CHECK: mov r5, 8

# Ensure all symbols are resolved
# CHECK-REL-NOT: R_HS32
