; RUN: llc < %s -march=sparc -mattr=+rex | FileCheck %s

; CHECK: rld32    [-16777152], %o1
; CHECK: rset32   -252645136, %o0
; CHECK: rand     %o1, %o0

define i32 @test() #0 {
entry:
  %0 = load i32, i32* inttoptr (i32 -16777152 to i32*), align 64
  %and = and i32 %0, -252645136
  ret i32 %and
}
      
attributes #0 = { nounwind }
