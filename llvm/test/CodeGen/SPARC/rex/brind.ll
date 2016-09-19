; RUN: llc < %s -verify-machineinstrs -march=sparc -mattr=+rex | FileCheck %s

; CHECK: call %i0
; CHECK-NEXT: rjmp        %i1

define void @foo(i32 ()* %func, i32* %label) #0 {
entry:
  %call = tail call i32 %func() #0
  indirectbr i32* %label, []
  ret void
}

attributes #0 = { nounwind }
