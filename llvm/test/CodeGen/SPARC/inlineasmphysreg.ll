; RUN: llc -verify-machineinstrs < %s -march=sparc | FileCheck %s

@a = global i32 0

; CHECK: test_i0
; CHECK: save
; CHECK: mov 4, %i1
; CHECK: mov %i0, %i2
; CHECK: mov %i1, %i0
; CHECK: add %i0, %i0, %i0
define void @test_i0(i32 %arg) #0 {
entry:
  %0 = tail call i32 asm sideeffect "add $0, $0, $0", "={r24},0"(i32 4)
  store volatile i32 %arg, i32* @a
  ret void
}

; CHECK: test_o0
; CHECK: save
; CHECK: mov 4, %o0
; CHECK: add %o0, %o0, %o0
define void @test_o0(i32 %arg) #0 {
entry:
  %0 = tail call i32 asm sideeffect "add $0, $0, $0", "={r8},0"(i32 4)
  store volatile i32 %arg, i32* @a
  ret void
}

attributes #0 = { nounwind }
