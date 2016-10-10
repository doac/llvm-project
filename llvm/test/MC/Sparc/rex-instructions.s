        ! RUN: llvm-mc %s -arch=sparc -mattr=+rex -show-encoding | FileCheck %s
        ! RUN: llvm-mc %s -arch=sparc -mattr=+rex -filetype=obj

// New SPARC instructions

// Section 6.2.1 - SAVEREX and ADDREX

        ! CHECK: saverex %sp, -20, %sp   ! encoding: [0x9d,0xe3,0x9f,0xec]
        saverex %sp, -20, %sp

        ! CHECK: saverex %sp, -4096, %sp   ! encoding: [0x9d,0xe3,0x90,0x00]
        saverex %sp, -4096, %sp

        ! CHECK: addrex %sp, -20, %sp   ! encoding: [0x9c,0x03,0x9f,0xec]
        addrex %sp, -20, %sp

// REX instructions

// Section 6.3.1 - Branch on Integer Condition Codes - Short

.BB0:

        ! CHECK: rbleu 10  ! encoding: [0x10,0x05]
        rbleu 10

        ! CHECK: rbleu -10  ! encoding: [0x10,0xfb]
        rbleu -10

        ! CHECK: rbleu 254  ! encoding: [0x10,0x7f]
        rbleu 254

        ! CHECK: rbleu -256  ! encoding: [0x10,0x80]
        rbleu -256

        ! CHECK: rbleu .BB0  ! encoding: [0x10,A]
        ! CHECK-NEXT !   fixup A - offset: 0, value: .BB0, kind: fixup_sparc_br8
        rbleu .BB0

        ! CHECK: rba   10  ! encoding: [0x20,0x05]
        rba   10
        ! CHECK: rbn   10  ! encoding: [0x00,0x05]
        rbn   10
        ! CHECK: rbne  10  ! encoding: [0x24,0x05]
        rbne  10
        ! CHECK: rbe   10  ! encoding: [0x04,0x05]
        rbe   10
        ! CHECK: rbg   10  ! encoding: [0x28,0x05]
        rbg   10
        ! CHECK: rble  10  ! encoding: [0x08,0x05]
        rble  10
        ! CHECK: rbge  10  ! encoding: [0x2c,0x05]
        rbge  10
        ! CHECK: rbl   10  ! encoding: [0x0c,0x05]
        rbl   10
        ! CHECK: rbgu  10  ! encoding: [0x30,0x05]
        rbgu  10
        ! CHECK: rbleu 10  ! encoding: [0x10,0x05]
        rbleu 10
        ! CHECK: rbcc  10  ! encoding: [0x34,0x05]
        rbcc  10
        ! CHECK: rbcs  10  ! encoding: [0x14,0x05]
        rbcs  10
        ! CHECK: rbpos 10  ! encoding: [0x38,0x05]
        rbpos 10
        ! CHECK: rbneg 10  ! encoding: [0x18,0x05]
        rbneg 10
        ! CHECK: rbvc  10  ! encoding: [0x3c,0x05]
        rbvc  10
        ! CHECK: rbvs  10  ! encoding: [0x1c,0x05]
        rbvs  10

// Section 6.3.2 - Branch on Floating-Point Condition Codes - Short

        ! CHECK: rfbule 10  ! encoding: [0x39,0x05]
        rfbule 10

        ! CHECK: rfbule -10  ! encoding: [0x39,0xfb]
        rfbule -10

        ! CHECK: rfbule 254  ! encoding: [0x39,0x7f]
        rfbule 254

        ! CHECK: rfbule -256  ! encoding: [0x39,0x80]
        rfbule -256

        ! CHECK: rfbule .BB0  ! encoding: [0x39,A]
        ! CHECK-NEXT !   fixup A - offset: 0, value: .BB0, kind: fixup_sparc_br8
        rfbule .BB0

        ! CHECK: rfba 10  ! encoding: [0x21,0x05]
        rfba 10
        ! CHECK: rfbn 10  ! encoding: [0x01,0x05]
        rfbn 10
        ! CHECK: rfbu 10  ! encoding: [0x1d,0x05]
        rfbu 10
        ! CHECK: rfbg 10  ! encoding: [0x19,0x05]
        rfbg 10
        ! CHECK: rfbug 10  ! encoding: [0x15,0x05]
        rfbug 10
        ! CHECK: rfbl 10  ! encoding: [0x11,0x05]
        rfbl 10
        ! CHECK: rfbul 10  ! encoding: [0x0d,0x05]
        rfbul 10
        ! CHECK: rfblg 10  ! encoding: [0x09,0x05]
        rfblg 10
        ! CHECK: rfbne 10  ! encoding: [0x05,0x05]
        rfbne 10
        ! CHECK: rfbe 10  ! encoding: [0x25,0x05]
        rfbe 10
        ! CHECK: rfbue 10  ! encoding: [0x29,0x05]
        rfbue 10
        ! CHECK: rfbge 10  ! encoding: [0x2d,0x05]
        rfbge 10
        ! CHECK: rfbuge 10  ! encoding: [0x31,0x05]
        rfbuge 10
        ! CHECK: rfble 10  ! encoding: [0x35,0x05]
        rfble 10
        ! CHECK: rfbo 10  ! encoding: [0x3d,0x05]
        rfbo 10

// Section 6.3.3 - Arithmetic operations - Accumulator with register

        ! CHECK: radd    %o0, %i7    ! encoding: [0xbc,0x00]
        radd %o0, %i7
        ! CHECK: raddcc  %o1, %fp    ! encoding: [0xb8,0x11]
        raddcc %o1, %i6
        ! CHECK: rsub    %o2, %i5    ! encoding: [0xb4,0x22]
        rsub %o2, %i5
        ! CHECK: rsubcc  %o3, %i4    ! encoding: [0xb0,0x33]
        rsubcc %o3, %i4
        ! CHECK: rand    %l4, %i3    ! encoding: [0xac,0x44]
        rand %l4, %i3
        ! CHECK: randcc  %l5, %i2    ! encoding: [0xa8,0x55]
        randcc %l5, %i2
        ! CHECK: randn   %l6, %i1    ! encoding: [0xa5,0x46]
        randn %l6, %i1
        ! CHECK: randncc %l7, %i0    ! encoding: [0xa1,0x57]
        randncc %l7, %i0
        ! CHECK: ror     %i0, %l7    ! encoding: [0x9c,0x88]
        ror %i0, %l7
        ! CHECK: rorcc   %i1, %l6    ! encoding: [0x98,0x99]
        rorcc %i1, %l6
        ! CHECK: rorn    %i2, %l5    ! encoding: [0x95,0x8a]
        rorn %i2, %l5
        ! CHECK: rorncc  %i3, %l4    ! encoding: [0x91,0x9b]
        rorncc %i3, %l4
        ! CHECK: rxor    %i4, %o3    ! encoding: [0x8c,0xcc]
        rxor %i4, %o3
        ! CHECK: rxorcc  %i5, %o2    ! encoding: [0x88,0xdd]
        rxorcc %i5, %o2
        ! CHECK: rsll    %fp, %o1    ! encoding: [0x85,0x6e]
        rsll %i6, %o1
        ! CHECK: rsrl    %i7, %o0    ! encoding: [0x81,0xaf]
        rsrl %i7, %o0

// Section 6.3.4 - Arithmetic operations - Accumulator with immediate

        ! CHECK: raddcc 10, %l4   ! encoding: [0x92,0x0a]
        raddcc 10, %l4

        ! CHECK: raddcc 15, %l4   ! encoding: [0x92,0x0f]
        raddcc 15, %l4

        ! CHECK: raddcc -16, %l4   ! encoding: [0x92,0x10]
        raddcc -16, %l4

        ! CHECK: rsll 4, %l5  ! encoding: [0x97,0x64]
        rsll 4, %l5

        ! CHECK: rsll 31, %l5  ! encoding: [0x97,0x7f]
        rsll 31, %l5

        ! CHECK: rsrl 2, %l6   ! encoding: [0x9b,0xa2]
        rsrl 2, %l6

// Section 6.3.5 - Comparison with register

        ! CHECK: rcmp %l6, %o1  ! encoding: [0x99,0x71]
        rcmp %l6, %o1

// Section 6.3.6 - Comparison with immediate

        ! CHECK: rcmp %l4, 10  ! encoding: [0x93,0x0a]
        rcmp %l4, 10

// Section 6.3.7 - Constant assignment

        ! CHECK: rset5 10, %o1  ! encoding: [0x86,0x2a]
        rset5 10, %o1

        ! CHECK: rone 4, %o1  ! encoding: [0x86,0xa4]
        rone 4, %o1

// Section 6.3.8 - Bit-mask operations

        ! CHECK: rsetbit 13, %o3  ! encoding: [0x8e,0x8d]
        rsetbit 13, %o3

        ! CHECK: rclrbit 13, %o3  ! encoding: [0x8f,0x4d]
        rclrbit 13, %o3

        ! CHECK: rinvbit 13, %o3  ! encoding: [0x8e,0xcd]
        rinvbit 13, %o3

        ! CHECK: rmasklo 13, %o3  ! encoding: [0x8e,0x4d]
        rmasklo 13, %o3

        ! CHECK: rtstbit 13, %o3  ! encoding: [0x8e,0x6d]
        rtstbit 13, %o3

// Section 6.3.9 - Register to register copy

        ! CHECK: rmov %o2, %g3  ! encoding: [0xad,0x22]
        rmov %o2, %g3

// Section 6.3.10 - Negation

        ! CHECK: rneg %o3  ! encoding: [0x8f,0xc4]
        rneg %o3

        ! CHECK: rnot %o3  ! encoding: [0x8f,0xc5]
        rnot %o3

// Section 6.3.11 - Return instructions

        ! CHECK: rretrest  ! encoding: [0x83,0xc0]
        rretrest

        ! CHECK: rretl  ! encoding: [0x83,0xc1]
        rretl

// Section 6.3.12 - Load/Store - 8/16/32/64 - one register

        ! CHECK: rld [%o3], %l4  ! encoding: [0xd0,0x03]
        rld [%o3], %l4
        ! CHECK: rldub [%o3], %l4  ! encoding: [0xd0,0x43]
        rldub [%o3], %l4
        ! CHECK: rlduh [%o3], %l4  ! encoding: [0xd0,0x83]
        rlduh [%o3], %l4
        ! CHECK: rldd [%o3], %l4  ! encoding: [0xd0,0xc3]
        rldd [%o3], %l4
        ! CHECK: rldf [%o3], %f1  ! encoding: [0xe4,0x23]
        rldf [%o3], %f1
        ! CHECK: rlddf [%o3], %f2  ! encoding: [0xe8,0xa3]
        rlddf [%o3], %f2

        ! CHECK: rst %l4, [%o3] ! encoding: [0xd1,0x03]
        rst %l4, [%o3]
        ! CHECK: rstb %l4, [%o3] ! encoding: [0xd1,0x43]
        rstb %l4, [%o3]
        ! CHECK: rsth %l4, [%o3] ! encoding: [0xd1,0x83]
        rsth %l4, [%o3]
        ! CHECK: rstd %l4, [%o3] ! encoding: [0xd1,0xc3]
        rstd %l4, [%o3]
        ! CHECK: rstf %f1, [%o3] ! encoding: [0xe5,0x23]
        rstf %f1, [%o3]
        ! CHECK: rstdf %f4, [%l4]  ! encoding: [0xf1,0xa4]
        rstdf %f4, [%l4]

// Section 6.3.13 - Load/Store - 8/16/32/64 - fixed register plus immediate

        ! CHECK: rld [%i0+124], %l4 ! encoding: [0xd2,0x9f]
        rld [%i0+124], %l4
        ! CHECK: rld [%i0+40], %l4 ! encoding: [0xd2,0x8a]
        rld [%i0+40], %l4
        ! CHECK: rld [%o0+40], %l4 ! encoding: [0xd2,0xca]
        rld [%o0+40], %l4
        ! CHECK: rld [%fp+40], %l4 ! encoding: [0xd2,0x0a]
        rld [%fp+40], %l4
        ! CHECK: rld [%sp+40], %l4  ! encoding: [0xd2,0x4a]
        rld [%sp+40], %l4
        ! CHECK: rldf [%i0+40], %f1  ! encoding: [0xe6,0xaa]
        rldf [%i0+40], %f1
        ! CHECK: rldf [%fp+40], %f1  ! encoding: [0xe6,0x2a]
        rldf [%fp+40], %f1
        ! CHECK: rldf [%sp+40], %f1  ! encoding: [0xe6,0x6a]
        rldf [%sp+40], %f1

        ! CHECK: rst %l4, [%i0+40] ! encoding: [0xd3,0x8a]
        rst %l4, [%i0+40]
        ! CHECK: rst %l4, [%o0+40] ! encoding: [0xd3,0xca]
        rst %l4, [%o0+40]
        ! CHECK: rst %l4, [%fp+40] ! encoding: [0xd3,0x0a]
        rst %l4, [%fp+40]
        ! CHECK: rst %l4, [%sp+40] ! encoding: [0xd3,0x4a]
        rst %l4, [%sp+40]
        ! CHECK: rstf %f1, [%i0+40]  ! encoding: [0xe7,0xaa]
        rstf %f1, [%i0+40]
        ! CHECK: rstf %f1, [%fp+40]  ! encoding: [0xe7,0x2a]
        rstf %f1, [%fp+40]
        ! CHECK: rstf %f1, [%sp+40]  ! encoding: [0xe7,0x6a]
        rstf %f1, [%sp+40]

// Section 6.3.14 - Load/Store - 8/16/32/64 - one register - auto-incrementing

        ! CHECK: rldinc [%o3], %l4  ! encoding: [0xd0,0x13]
        rldinc [%o3], %l4
        ! CHECK: rldubinc [%o3], %l4  ! encoding: [0xd0,0x53]
        rldubinc [%o3], %l4
        ! CHECK: rlduhinc [%o3], %l4  ! encoding: [0xd0,0x93]
        rlduhinc [%o3], %l4
        ! CHECK: rlddinc [%o3], %l4  ! encoding: [0xd0,0xd3]
        rlddinc [%o3], %l4
        ! CHECK: rldfinc [%o3], %f1  ! encoding: [0xe4,0x33]
        rldfinc [%o3], %f1
        ! CHECK: rlddfinc [%o3], %f2  ! encoding: [0xe8,0xb3]
        rlddfinc [%o3], %f2

        ! CHECK: rstinc %l4, [%o3]  ! encoding: [0xd1,0x13]
        rstinc %l4, [%o3]
        ! CHECK: rstbinc %l4, [%o3]  ! encoding: [0xd1,0x53]
        rstbinc %l4, [%o3]
        ! CHECK: rsthinc %l4, [%o3]  ! encoding: [0xd1,0x93]
        rsthinc %l4, [%o3]
        ! CHECK: rstdinc %l4, [%o3]  ! encoding: [0xd1,0xd3]
        rstdinc %l4, [%o3]
        ! CHECK: rstfinc %f1, [%o3]  ! encoding: [0xe5,0x33]
        rstfinc %f1, [%o3]
        ! CHECK: rstdfinc %f4, [%l4]  ! encoding: [0xf1,0xb4]
        rstdfinc %f4, [%l4]

// Section 6.3.15 - Miscellaneous operations - no source operands

        ! CHECK: rpush %l4  ! encoding: [0x93,0xc2]
        rpush %l4

        ! CHECK: rpop %l4  ! encoding: [0x93,0xc3]
        rpop %l4

        ! CHECK: rta 5 ! encoding: [0x97,0xc6]
        rta 5

        ! CHECK: rleave  ! encoding: [0x83,0xc7]
        rleave

        ! CHECK: rgetpc %l4  ! encoding: [0x93,0xc9]
        rgetpc %l4

// Section 6.4.1 - Branch on integer condition codes - long

        ! CHECK: rbleu,l 50000   ! encoding: [0x12,0xa8,0x00,0x61]
        rbleu,l 50000

        ! CHECK: rbleu,l -50000  ! encoding: [0x12,0x58,0xff,0x9e]
        rbleu,l -50000

        ! CHECK: rbleu,l .BB0    ! encoding: [0x12,A,A,A]
        ! CHECK-NEXT: ! fixup A - offset: 0, value: .BB0, kind: fixup_sparc_br24
        rbleu,l .BB0

        ! CHECK: rba,l   50000  ! encoding: [0x22,0xa8,0x00,0x61]
        rba,l   50000
        ! CHECK: rbn,l   50000  ! encoding: [0x02,0xa8,0x00,0x61]
        rbn,l   50000
        ! CHECK: rbne,l  50000  ! encoding: [0x26,0xa8,0x00,0x61]
        rbne,l  50000
        ! CHECK: rbe,l   50000  ! encoding: [0x06,0xa8,0x00,0x61]
        rbe,l   50000
        ! CHECK: rbg,l   50000  ! encoding: [0x2a,0xa8,0x00,0x61]
        rbg,l   50000
        ! CHECK: rble,l  50000  ! encoding: [0x0a,0xa8,0x00,0x61]
        rble,l  50000
        ! CHECK: rbge,l  50000  ! encoding: [0x2e,0xa8,0x00,0x61]
        rbge,l  50000
        ! CHECK: rbl,l   50000  ! encoding: [0x0e,0xa8,0x00,0x61]
        rbl,l   50000
        ! CHECK: rbgu,l  50000  ! encoding: [0x32,0xa8,0x00,0x61]
        rbgu,l  50000
        ! CHECK: rbleu,l 50000  ! encoding: [0x12,0xa8,0x00,0x61]
        rbleu,l 50000
        ! CHECK: rbcc,l  50000  ! encoding: [0x36,0xa8,0x00,0x61]
        rbcc,l  50000
        ! CHECK: rbcs,l  50000  ! encoding: [0x16,0xa8,0x00,0x61]
        rbcs,l  50000
        ! CHECK: rbpos,l 50000  ! encoding: [0x3a,0xa8,0x00,0x61]
        rbpos,l 50000
        ! CHECK: rbneg,l 50000  ! encoding: [0x1a,0xa8,0x00,0x61]
        rbneg,l 50000
        ! CHECK: rbvc,l  50000  ! encoding: [0x3e,0xa8,0x00,0x61]
        rbvc,l  50000
        ! CHECK: rbvs,l  50000  ! encoding: [0x1e,0xa8,0x00,0x61]
        rbvs,l  50000

// Section 6.4.2 - Branch on floating-point condition codes - long

        ! CHECK: rfbule,l 50000  ! encoding: [0x3b,0xa8,0x00,0x61]
        rfbule,l 50000

        ! CHECK: rfbule,l -50000  ! encoding: [0x3b,0x58,0xff,0x9e]
        rfbule,l -50000

        ! CHECK: rfbule,l .BB0    ! encoding: [0x3b,A,A,A]
        ! CHECK-NEXT: ! fixup A - offset: 0, value: .BB0, kind: fixup_sparc_br24
        rfbule,l .BB0

        ! CHECK: rfba,l 50000    ! encoding: [0x23,0xa8,0x00,0x61]
        rfba,l 50000
        ! CHECK: rfbn,l 50000    ! encoding: [0x03,0xa8,0x00,0x61]
        rfbn,l 50000
        ! CHECK: rfbu,l 50000    ! encoding: [0x1f,0xa8,0x00,0x61]
        rfbu,l 50000
        ! CHECK: rfbg,l 50000    ! encoding: [0x1b,0xa8,0x00,0x61]
        rfbg,l 50000
        ! CHECK: rfbug,l 50000   ! encoding: [0x17,0xa8,0x00,0x61]
        rfbug,l 50000
        ! CHECK: rfbl,l 50000    ! encoding: [0x13,0xa8,0x00,0x61]
        rfbl,l 50000
        ! CHECK: rfbul,l 50000   ! encoding: [0x0f,0xa8,0x00,0x61]
        rfbul,l 50000
        ! CHECK: rfblg,l 50000   ! encoding: [0x0b,0xa8,0x00,0x61]
        rfblg,l 50000
        ! CHECK: rfbne,l 50000   ! encoding: [0x07,0xa8,0x00,0x61]
        rfbne,l 50000
        ! CHECK: rfbe,l 50000    ! encoding: [0x27,0xa8,0x00,0x61]
        rfbe,l 50000
        ! CHECK: rfbue,l 50000   ! encoding: [0x2b,0xa8,0x00,0x61]
        rfbue,l 50000
        ! CHECK: rfbge,l 50000   ! encoding: [0x2f,0xa8,0x00,0x61]
        rfbge,l 50000
        ! CHECK: rfbuge,l 50000  ! encoding: [0x33,0xa8,0x00,0x61]
        rfbuge,l 50000
        ! CHECK: rfble,l 50000   ! encoding: [0x37,0xa8,0x00,0x61]
        rfble,l 50000
        ! CHECK: rfbo,l 50000    ! encoding: [0x3f,0xa8,0x00,0x61]
        rfbo,l 50000

// Section 6.4.3 - Call and link

        ! CHECK: call 50000  ! encoding: [0x40,0x00,0x30,0xd4]
        call 50000

        ! CHECK: call foo ! encoding: [0b01AAAAAA,A,A,A]
   ! CHECK-NEXT: ! fixup A - offset: 0, value: (foo)+2, kind: fixup_sparc_call30
        call foo

// Section 6.4.4 - Constant assignment

        ! CHECK: rset21 50000, %l4  ! encoding: [0x92,0xf0,0x06,0x1a]
        rset21 50000, %l4

// Section 6.4.5 - Generic format 3 SPARC operation

// Section 6.4.6 - Floating-point operations

// Section 6.5.1 - Set 32-bit constant

        ! CHECK: rset32 50000, %l4  ! encoding: [0x93,0xe8,0x00,0x00,0xc3,0x50]
        rset32 50000, %l4

        ! CHECK: rset32 foo, %l4     ! encoding: [0x93,0xe8,A,A,A,A]
        ! CHECK-NEXT:               !   fixup A - offset: 2, value: foo, kind: fixup_sparc_32
        rset32 foo, %l4

       ! CHECK: rset32pc 50000, %l4  ! encoding: [0x93,0xe9,0x00,0x00,0xc3,0x50]
        rset32pc 50000, %l4

        ! CHECK: rset32pc foo, %l4     ! encoding: [0x93,0xe9,A,A,A,A]
                   !   fixup A - offset: 2, value: foo, kind: fixup_sparc_disp32
        rset32pc foo, %l4

// Section 6.5.2 - Load from 32-bit address

        ! CHECK: rld32 [50000], %l4  ! encoding: [0x93,0xea,0x00,0x00,0xc3,0x50]
        rld32 [50000], %l4

        ! CHECK: rld32 [foo], %l4  ! encoding: [0x93,0xea,A,A,A,A]
                       !   fixup A - offset: 2, value: foo, kind: fixup_sparc_32
        rld32 [foo], %l4

       ! CHECK: rld32pc [50000], %l4 ! encoding: [0x93,0xeb,0x00,0x00,0xc3,0x50]
        rld32pc [50000], %l4

        ! CHECK: rld32pc [foo], %l4  ! encoding: [0x93,0xeb,A,A,A,A]
                   !   fixup A - offset: 2, value: foo, kind: fixup_sparc_disp32
        rld32pc [foo], %l4
