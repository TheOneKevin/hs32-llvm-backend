# RUN: not llvm-mc --triple hs32 %s 2>&1 | FileCheck %s

# Out of bounds check

# CHECK: error: immediate must be an integer within [-32768, 32767]
ldr r6, [r3+98743]
# CHECK: error: immediate must be an integer within [-32768, 32767]
str [r4-123456], r3
# CHECK: error: immediate must be an integer within [0, 65535]
mov r2, 65536
# CHECK: error: immediate must be an integer within [0, 65535]
mov r2, -1
# CHECK: error: immediate must be an integer within [0, 65535]
add r5, r2, 65536
# CHECK: error: immediate must be an integer within [0, 65535]
rsubc r1, r3, -1
# CHECK: error: immediate must be an integer within [0, 65535]
cmp r1, -1
# CHECK: error: immediate must be an integer within [0, 65535]
tst r6, -100
# CHECK: error: immediate must be an integer within [-32768, 32767]
jmp 32768
# CHECK: error: immediate must be an integer within [-32768, 32767]
blgt -32769

# Malformed memory reference

# CHECK: error: malformed memory reference
str [1234+r4], r2
# CHECK: error: malformed memory reference
str [1234+3245], r1
# CHECK: error: invalid operand for instruction
str r3, [r1+r2]
# CHECK: error: unknown token in expression
# CHECK: error: malformed memory reference
str [r1-r2], r3
# CHECK: error: malformed memory reference
ldr r6, [1234+r7]
# CHECK: error: malformed memory reference
ldr r3, [1234+3245]
# CHECK: error: invalid operand for instruction
ldr [r0+r9], r1
# CHECK: error: unknown token in expression
# CHECK: error: malformed memory reference
ldr r3, [r1-r0]

# Arity error

# CHECK: error: too few operands for instruction
ldr r6
# CHECK: error: too few operands for instruction
str [r4-4367]
# CHECK: error: too few operands for instruction
mov r1
# CHECK: error: too few operands for instruction
xor r5, r2
# CHECK: error: too few operands for instruction
bic r1
# CHECK: error: too few operands for instruction
cmp r1
# CHECK: error: too few operands for instruction
tst r6
# CHECK: error: too few operands for instruction
jal
# CHECK: error: too few operands for instruction
bnc

# Malformed operand

# CHECK: error: invalid operand for instruction
ldr r6, [r3], r2
# CHECK: error: invalid operand for instruction
ldr r10, r4
# CHECK: error: invalid operand for instruction
str r7, [r4]
# CHECK: error: invalid operand for instruction
str r1, r3
# CHECK: error: invalid operand for instruction
mov r0, 123, r3
# CHECK: error: invalid operand for instruction
mov r1, r2, r3
# CHECK: error: invalid operand for instruction
or  123, r5, r4
# CHECK: error: invalid operand for instruction
add r4, 456, r4
# CHECK: error: invalid operand for instruction
bic r1, r3, 13, r9
# CHECK: error: invalid operand for instruction
cmp r1, r2, r3
# CHECK: error: invalid operand for instruction
cmp 101, r2, r3
# CHECK: error: invalid operand for instruction
jmp 132, r2

# Unknown operand

# CHECK: error: unknown operand
bss ,,,,
# CHECK: error: unknown operand
xor 1+4*2&5,*(&*&)
