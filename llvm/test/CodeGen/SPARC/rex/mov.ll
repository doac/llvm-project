; RUN: llc < %s -march=sparc -mattr=+rex | FileCheck %s

; CHECK:	rmov	%i0, %o0
; CHECK:	rmov	%i1, %o1
; CHECK:	rmov	%i2, %o2
; CHECK:	rmov	%i3, %o3
; CHECK:	rmov	%i4, %o4
; CHECK:	rmov	%i5, %o5

define void @copyphysreg(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e, i32 %f) #0 {
entry:
  call void @func(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e, i32 %f)
  ret void
}

declare void @func(i32, i32, i32, i32, i32, i32) #0

attributes #0 = { nounwind }
