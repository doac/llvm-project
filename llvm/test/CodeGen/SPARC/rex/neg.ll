; RUN: llc < %s -march=sparc -mattr=+rex | FileCheck %s

; CHECK: rneg %o0
define i32 @fneg(i32 %a) #0 {
entry:
  %sub = sub nsw i32 0, %a
  ret i32 %sub
}

; CHECK: rnot %o0
define i32 @fnot(i32 %a) #0 {
entry:
  %neg = xor i32 %a, -1
  ret i32 %neg
}

attributes #0 = { nounwind }
