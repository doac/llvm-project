; RUN: llc < %s -march=sparc -mattr=+rex | FileCheck %s

; CHECK:        rsetbit 4, %o0
define i32 @setbit_4(i32 %a) #0 {
entry:
  %or = or i32 %a, 16
  ret i32 %or
}

; CHECK-NOT:    rsetbit 0, %o0
define i32 @setbit_none(i32 %a) #0 {
entry:
  %or = or i32 %a, 0
  ret i32 %or
}

; CHECK:        rsetbit 31, %o0
define i32 @setbit_31(i32 %a) #0 {
entry:
  %or = or i32 %a, 2147483648
  ret i32 %or
}

; CHECK:        rclrbit 4, %o0
define i32 @clrbit_4(i32 %a) #0 {
entry:
  %and = and i32 %a, -17
  ret i32 %and
}

; CHECK:        rclrbit 31, %o0
define i32 @clrbit_31(i32 %a) #0 {
entry:
  %and = and i32 %a, 2147483647
  ret i32 %and
}

; CHECK:        rinvbit 4, %o0
define i32 @invbit(i32 %a) #0 {
entry:
  %xor = xor i32 %a, 16
  ret i32 %xor
}

; CHECK:        rmasklo 4, %o0
define i32 @masklo(i32 %a) #0 {
entry:
  %and = and i32 %a, 31
  ret i32 %and
}

attributes #0 = { nounwind }
