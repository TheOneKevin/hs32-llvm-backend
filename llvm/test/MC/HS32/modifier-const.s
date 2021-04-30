# RUN: llvm-mc -filetype=obj -triple hs32 < %s \
# RUN:     | llvm-objdump -d --print-imm-hex - | FileCheck -check-prefix=CHECK-HEX %s

# Check prefixes
# CHECK-HEX - instruction with hex imm

# CHECK-HEX: mov r2, 0xcafe
mov r2, %lo(0xFFFF)+1+0xCAFE
# CHECK-HEX: mov r2, 0xffff
mov r2, %hi(0xFFFF0000)
