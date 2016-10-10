! RUN: llvm-mc %s -arch=sparc -mattr=rex -filetype=obj | llvm-readobj -s -sd | FileCheck %s

! CHECK: 0000: B4CD0000 00000000 00000000 00000000
! CHECK: 0010: B4CD

foo:
        rxor %i5, %i5
        .align 16
        rxor %i5, %i5
