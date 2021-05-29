# RUN: llvm-mc %s -triple=hs32 -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK,CHECK-INST %s
# RUN: llvm-mc -filetype=obj -triple hs32 < %s \
# RUN:     | llvm-objdump -d - | FileCheck -check-prefix=CHECK-INST %s

# Check prefixes
# CHECK      - check encoding
# CHECK-INST - check instruction printout

# Test ldr and str operations

# CHECK-INST: ldr r1, [r2]
# CHECK: encoding: [0x14,0x12,0x00,0x00]
ldr r1, [r2]
# CHECK-INST: ldr r1, [r2]
# CHECK: encoding: [0x14,0x12,0x00,0x00]
ldr r1, [r2+10-10]
# CHECK-INST: ldr r3, [r2-1]
# CHECK: encoding: [0x14,0x32,0xff,0xff]
ldr r3, [r2+1-2]
# CHECK-INST: ldr r5, [r3+r4]
# CHECK: encoding: [0x11,0x53,0x40,0x00]
ldr r5, [r3+r4]
# CHECK-INST: ldr r7, [r1+r6 shl 6]
# CHECK: encoding: [0x11,0x71,0x63,0x00]
ldr r7, [r1+r6 shl 6]
# CHECK-INST: ldr r7, [r1+r6 shr 6]
# CHECK: encoding: [0x11,0x71,0x63,0x20]
ldr r7, [r1+r6 shr 6]
# CHECK-INST: ldr r7, [r1+r6 srx 6]
# CHECK: encoding: [0x11,0x71,0x63,0x40]
ldr r7, [r1+r6 srx 6]
# CHECK-INST: ldr r7, [r1+r6 ror 6]
# CHECK: encoding: [0x11,0x71,0x63,0x60]
ldr r7, [r1+r6 ror 6]
# CHECK-INST: str [r2], r1
# CHECK: encoding: [0x34,0x12,0x00,0x00]
str [r2], r1
# CHECK-INST: str [r2], r1
# CHECK: encoding: [0x34,0x12,0x00,0x00]
str [r2+8-8], r1
# CHECK-INST: str [r3+5], r5
# CHECK: encoding: [0x34,0x53,0x00,0x05]
str [r3+1+4], r5
# CHECK-INST: str [r2+r3], r4
# CHECK: encoding: [0x31,0x42,0x30,0x00]
str [r2+r3], r4
# CHECK-INST: str [r2+r5 shl 31], r4
# CHECK: encoding: [0x31,0x42,0x5f,0x80]
str [r2+r5 shl 31], r4
# CHECK-INST: str [r2+r5 shr 31], r4
# CHECK: encoding: [0x31,0x42,0x5f,0xa0]
str [r2+r5 shr 31], r4
# CHECK-INST: str [r2+r5 srx 31], r4
# CHECK: encoding: [0x31,0x42,0x5f,0xc0]
str [r2+r5 srx 31], r4
# CHECK-INST: str [r2+r5 ror 31], r4
# CHECK: encoding: [0x31,0x42,0x5f,0xe0]
str [r2+r5 ror 31], r4


# Test MOV and ALU

# CHECK-INST: mov r6, 32256
# CHECK: encoding: [0x24,0x60,0x7e,0x00]
mov r6, 0x7EFE - 0xFE
# CHECK-INST: mov r3, r4
# CHECK: encoding: [0x20,0x30,0x40,0x00]
mov r3, r4
# CHECK-INST: add r1, r2, 4936
# CHECK: encoding: [0x44,0x12,0x13,0x48]
add r1, r2, 1234<<(6*2-10)
# CHECK-INST: add r2, r3, r4
# CHECK: encoding: [0x40,0x23,0x40,0x00]
add r2, r3, r4
# CHECK-INST: addc lr, r2, 1194
# CHECK: encoding: [0x45,0xe2,0x04,0xaa]
addc lr, r2, 1234-40
# CHECK-INST: addc r2, sp, r4
# CHECK: encoding: [0x41,0x2d,0x40,0x00]
addc r2, sp, r4
# CHECK-INST: sub r5, r4, 587
# CHECK: encoding: [0x64,0x54,0x02,0x4b]
sub r5, r4, 294*2-1
# CHECK-INST: sub r5, r4, sp
# CHECK: encoding: [0x60,0x54,0xd0,0x00]
sub r5, r4, sp
# TODO: add all ALU instructions here

# Test CMP instructions

# CHECK-INST: cmp r1, 4096
# CHECK: encoding: [0x6c,0x01,0x10,0x00]
cmp r1, 4096
# CHECK-INST: cmp r1, lr
# CHECK: encoding: [0x68,0x01,0xe0,0x00]
cmp r1, lr
# CHECK-INST: tst lr, 512
# CHECK: encoding: [0x8c,0x0e,0x02,0x00]
tst lr, 1<<9
# CHECK-INST: tst sp, r2
# CHECK: encoding: [0x88,0x0d,0x20,0x00]
tst sp, r2
# CHECK-INST: jmp 0
# CHECK: encoding: [0x50,0x00,0x00,0x00]
jmp 0
# CHECK-INST: jmp -1000
# CHECK: encoding: [0x50,0x00,0xfc,0x18]
jmp -1000
# CHECK-INST: jal -4682
# CHECK: encoding: [0x70,0x00,0xed,0xb6]
jal -4938 + 0x100
