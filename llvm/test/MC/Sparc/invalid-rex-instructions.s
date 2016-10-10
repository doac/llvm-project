        ! RUN: not llvm-mc %s -arch=sparc -mattr=+rex -show-encoding 2>&1 | FileCheck %s

// New SPARC instructions

// Section 6.2.1 - SAVEREX and ADDREX

        ! CHECK: error: invalid operand for instruction
        saverex %sp, 0, %sp

        ! CHECK: error: invalid operand for instruction
        saverex %sp, -4097, %sp

        ! CHECK: error: invalid operand for instruction
        addrex %sp, 0, %sp

        ! CHECK: error: invalid operand for instruction
        addrex %sp, -4097, %sp

// REX instructions

// Section 6.3.1 - Branch on Integer Condition Codes - Short

        ! CHECK: error: invalid operand for instruction
        rbleu 256

        ! CHECK: error: invalid operand for instruction
        rbleu -257

        ! CHECK: error: invalid operand for instruction
        rbleu 3

// Section 6.3.2 - Branch on Floating-Point Condition Codes - Short

        ! CHECK: error: invalid operand for instruction
        rfbule 256

        ! CHECK: error: invalid operand for instruction
        rfbule -257

        ! CHECK: error: invalid operand for instruction
        rfbule 3

// Section 6.3.3 - Arithmetic operations - Accumulator with register

        ! CHECK: error: invalid operand for instruction
        rand %sp, %o0

// Section 6.3.4 - Arithmetic operations - Accumulator with immediate

        ! CHECK: error: invalid operand for instruction
        raddcc 16, %l4

        ! CHECK: error: invalid operand for instruction
        raddcc -17, %l4

        ! CHECK: error: invalid operand for instruction
        rsll 32, %l5

        ! CHECK: error: invalid operand for instruction
        rsll -1, %l5

// Section 6.3.5 - Comparison with register

// Section 6.3.6 - Comparison with immediate

// Section 6.3.7 - Constant assignment

// Section 6.3.8 - Bit-mask operations

// Section 6.3.9 - Register to register copy

// Section 6.3.10 - Negation

// Section 6.3.11 - Return instructions

// Section 6.3.12 - Load/Store - 8/16/32/64 - one register

        ! CHECK: error: invalid operand for instruction
        rldub [%g1], %o1

        ! CHECK: error: invalid operand for instruction
        rldd [%i0], %i1

        ! CHECK: error: invalid operand for instruction
        rlddf [%i0], %f1

// Section 6.3.13 - Load/Store - 8/16/32/64 - fixed register plus immediate

        ! CHECK: error: invalid operand for instruction
        rld [%i1 + 4], %i0
        ! CHECK: error: invalid operand for instruction
        rldf [%o0 + 4], %f0
        ! CHECK: error: invalid operand for instruction
        rldf [%sp + 1], %f0
        ! CHECK: error: invalid operand for instruction
        rldf [%sp - 4], %f0
        ! CHECK: error: invalid operand for instruction
        rldf [%sp + 128], %f0

// Section 6.3.14 - Load/Store - 8/16/32/64 - one register - auto-incrementing

// Section 6.3.15 - Miscellaneous operations - no source operands

        ! CHECK: error: invalid operand for instruction
        rta -1
        ! CHECK: error: invalid operand for instruction
        rta 8

// Section 6.4.1 - Branch on integer condition codes - long

        ! CHECK: error: invalid operand for instruction
        rbleu,l 16777216

        ! CHECK: error: invalid operand for instruction
        rbleu,l -16777218

        ! CHECK: error: invalid operand for instruction
        rbleu,l 3

// Section 6.4.2 - Branch on floating-point condition codes - long

// Section 6.4.3 - Call and link

// Section 6.4.4 - Constant assignment

        ! CHECK: error: invalid operand for instruction
        rset21 1048576, %l4
        ! CHECK: error: invalid operand for instruction
        rset21 -1048577, %l4

// Section 6.4.5 - Generic format 3 SPARC operation

// Section 6.4.6 - Floating-point operations

// Section 6.5.1 - Set 32-bit constant

// Section 6.5.2 - Load from 32-bit address
