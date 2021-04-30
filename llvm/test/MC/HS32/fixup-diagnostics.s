# RUN: not llvm-mc -filetype=obj -triple hs32 --fatal-warnings %s 2>&1 \
# RUN:         | FileCheck -check-prefixes=CHECK-ERROR %s

# Check prefixes
# CHECK-ERROR   - check error/warnings

# CHECK-ERROR: error: symbol 'bar' can not be undefined in a subtraction expression
add r1, r2, %lo(foo-bar)
