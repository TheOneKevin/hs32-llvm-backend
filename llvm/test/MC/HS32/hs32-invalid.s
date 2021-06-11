# RUN: not llvm-mc --triple hs32 %s 2>&1 | FileCheck %s
# RUN: not llvm-mc --triple hs32 %s --show-encoding --fatal-warnings 2>&1 \
# RUN:         | FileCheck -check-prefixes=CHECK-ENC %s

# Check prefixes
# CHECK     - regular check error
# CHECK-ENC - check error + warnings during encoding

# Out of bounds check

# CHECK: error: immediate must be an integer within [-32768, 32767]
ldr r6, [r3+98743]
# CHECK: error: immediate must be an integer within [-32768, 32767]
str [r4-123456], r3
# CHECK: error: shift amount must be an integer in [0, 31]
mov r2, r3 srx 32
# CHECK: error: immediate must be an integer within [-32768, 32767]
mov r2, 32768
# CHECK: error: immediate must be an integer within [-32768, 32767]
mov r2, -32769
# CHECK: error: shift amount must be an integer in [0, 31]
add r2, r4, r3 ror 32
# CHECK: error: immediate must be an integer within [-32768, 32767]
add r5, r2, 32768
# CHECK: error: invalid instruction
rsubc r1, r3, -32769
# CHECK: error: immediate must be an integer within [-32768, 32767]
cmp r1, 32768
# CHECK: error: immediate must be an integer within [-32768, 32767]
tst r6, -32769
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
# CHECK: error: invalid register name
ldr r1, [r21+r2]
# CHECK: error: invalid register name
ldr r1, [r18]
# CHECK: error: invalid register name
str [r24+3], r1
# CHECK: error: invalid register name
str [%hi(foo)+3], r1
# CHECK: error: bad modifier expression, only '+' supported
str [r0-%hi(foo)+3], r1
# CHECK: error: bad modifier subexpression
# CHECK: error: malformed memory reference
ldr r1, [r1+%hi(foo)+%lo(bar)]

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
bne

# Malformed operand

# CHECK: error: invalid operand for instruction
ldr r6, [r3], r2
# CHECK: error: invalid operand for instruction
ldr r10, r4
# CHECK: error: invalid operand for instruction
ldr r10 shr 3, [r4]
# CHECK: error: invalid operand for instruction
str r7, [r4]
# CHECK: error: invalid operand for instruction
str r1, r3
# CHECK: error: invalid operand for instruction
str [r1], r3 ror 3
# CHECK: error: invalid operand for instruction
mov r0 shl 23, r2
# CHECK: error: invalid operand for instruction
mov r0, 123, r3
# CHECK: error: invalid operand for instruction
mov r1, r2, r3
# CHECK: error: invalid operand for instruction
mov r21, r2, r3
# CHECK: error: invalid operand for instruction
add r1, r0 shl 23, r2
# CHECK: error: invalid operand for instruction
or  123, r5, r4
# CHECK: error: invalid operand for instruction
add r4, 456, r4
# CHECK: error: invalid operand for instruction
add abcde, r2, r3
# CHECK: error: invalid operand for instruction
addc r4, [r2], r3
# CHECK: error: invalid operand for instruction
and r4, %hi(foo),r4
# CHECK: error: invalid operand for instruction
xor r4, [r1], %hi(foo)
# CHECK: error: invalid operand for instruction
bic r1, r3, 13, r9
# CHECK: error: invalid operand for instruction
cmp r1, r2, r3
# CHECK: error: invalid operand for instruction
cmp 101, r2, r3
# CHECK: error: invalid operand for instruction
jmp 132, r2

# CHECK: error: malformed memory reference, expecting '+' or '-'
ldr r1, [r2 shl 2]
# CHECK: error: malformed memory reference
ldr r1, [r2 + shl 2]
# CHECK: error: malformed memory reference
ldr r1, [r2 + foo shl 2]
# CHECK: error: invalid shift type
ldr r1, [r2 + r3 foo 2]
# CHECK: error: invalid shift type
mov r1, r2 foo bar
# CHECK: error: expected integer here
mov r1, r3 shl -1
# CHECK: error: expected integer here
mov r1, r2 ror foo
# CHECK: error: unexpected token
mov r1, 2 shl bar
# CHECK: error: unexpected token
mov r1, foo shl bar

# Malformed expressions

# CHECK-ENC: error: symbolref fixups outside branch is unsupported
mov r12, r19
# CHECK-ENC: error: symbolref fixups outside branch is unsupported
add r1, r3, adwf
# CHECK-ENC: error: using pcrel in branch is dangerous
jmp %pcrel(foo)
# CHECK: error: malformed immediate expression
ldr r1, [r2+r22]
# CHECK: error: unknown token in expression
# CHECK: error: bad modifier subexpression
# CHECK: error: unknown operand
mov r5, %hi(foo)+%lo(bar)
# CHECK: error: unknown token in expression
# CHECK: error: unknown operand
mov r1, 2+%hi(foo)
# CHECK: error: unknown token in expression
# CHECK: error: unknown operand
mov r1, -%hi(foo)
# CHECK: error: unknown token in expression
# CHECK: error: unknown operand
mov r1, +%hi(foo)

# Unknown operand/register

# CHECK: error: unknown operand
bss ,,,,
# CHECK: error: unknown operand
xor 1+4*2&5,*(&*&)
# CHECK: error: unknown operand
rsubc r1, r2, %%%foo(bar)
# CHECK: error: unknown operand
rsubc r1, %%%foo(bar), r2
# CHECK: error: unknown modifier name
str [r1+%bar(foo)], r2
# CHECK: error: unknown modifier name
mov r1, %bar(foo)
# CHECK: error: malformed memory reference
ldr r0, [r1+%%%hi(foo)]
# CHECK: error: malformed memory reference
ldr r0, [r1+%%%hi(foo)]
