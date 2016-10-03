; RUN: llc < %s -march=sparc -mattr=+rex | FileCheck %s

; CHECK:        radd %o1, %o0
define i32 @fradd(i32 %a, i32 %b) #0 {
  %val = add i32 %a, %b
  ret i32 %val
}

; CHECK:        rsub %o1, %o0
define i32 @frsub(i32 %a, i32 %b) #0 {
  %val = sub i32 %a, %b
  ret i32 %val
}

; CHECK:        rand %o1, %o0
define i32 @frand(i32 %a, i32 %b) #0 {
  %val = and i32 %a, %b
  ret i32 %val
}

; CHECK:        randn %o1, %o0
define i32 @frandn(i32 %a, i32 %b) #0 {
  %notv = xor i32 %b, -1
  %val = and i32 %a, %notv
  ret i32 %val
}

; CHECK:        ror %o1, %o0
define i32 @fror(i32 %a, i32 %b) #0 {
  %val = or i32 %a, %b
  ret i32 %val
}

; CHECK:        rorn %o1, %o0
define i32 @frorn(i32 %a, i32 %b) #0 {
  %notv = xor i32 %b, -1
  %val = or i32 %a, %notv
  ret i32 %val
}

; CHECK:        rxor %o1, %o0
define i32 @frxor(i32 %a, i32 %b) #0 {
  %val = xor i32 %a, %b
  ret i32 %val
}

; CHECK:        rsll %o1, %o0
define i32 @frsll(i32 %a, i32 %b) #0 {
  %shl = shl i32 %a, %b
  ret i32 %shl
}

; CHECK:        rsrl %o1, %o0
define i32 @frsrl(i32 %a, i32 %b) #0 {
  %lshr = lshr i32 %a, %b
  ret i32 %lshr
}

; Modifies ICC

; CHECK:        raddcc %o3, %o1
; CHECK:        addxcc %o2, %o0, %o0
define i64 @fraddcc(i64 %a, i64 %b) #0 {
  %val = add i64 %b, %a
  ret i64 %val
}

; CHECK:        rsubcc %o3, %o1
; CHECK:        subxcc %o0, %o2, %o0
define i64 @frsubcc(i64 %a, i64 %b) #0 {
  %val = sub i64 %a, %b
  ret i64 %val
}

; With immediate

; CHECK:        raddcc 15, %o1
; CHECK:        addxcc %o0, 0, %o0
define i64 @fraddccimm(i64 %a) #0 {
  %val = add i64 15, %a
  ret i64 %val
}

; CHECK:        raddcc 15, %o0
define i32 @fraddimm(i32 %a) #0 {
  %val = add i32 15, %a
  ret i32 %val
}

; CHECK:        rsll 4, %o0
define i32 @frsllimm(i32 %a) #0 {
  %shl = shl i32 %a, 4
  ret i32 %shl
}

; CHECK:        rsrl 4, %o0
define i32 @frsrlimm(i32 %a) #0 {
  %lshr = lshr i32 %a, 4
  ret i32 %lshr
}

attributes #0 = { nounwind }
