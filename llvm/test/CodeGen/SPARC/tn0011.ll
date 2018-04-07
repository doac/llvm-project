; RUN: llc < %s -mcpu=gr712rc -mtriple=sparc -mattr=fix-tn0011 | FileCheck %s

; CHECK: .p2align 4
; CHECK-NEXT: casa
define i32 @test_atomic(i32* %p, i32 %v) {
entry:
  %0 = atomicrmw nand i32* %p, i32 %v seq_cst
  ret i32 %0
}
