; RUN: llc < %s -march=sparc -mattr=+rex | FileCheck %s

; CHECK-LABEL: small
; CHECK:        rset5 1, %o0
define i32 @small(i32 %a) {
 ret i32 1
}

; CHECK-LABEL: medium
; CHECK:        rset21 511, %o0
define i32 @medium(i32 %a) {
  ret i32 511
}

; CHECK-LABEL: large
; CHECK:        rset32 42223123, %o0
define i32 @large(i32 %a) {
  ret i32 42223123
}

; CHECK-LABEL: neg-small
; CHECK:        rset5 -4, %o0
define i32 @neg-small(i32 %a) {
 ret i32 -4
}

; CHECK-LABEL: neg-medium
; CHECK:        rset21 -511, %o0
define i32 @neg-medium(i32 %a) {
  ret i32 -511
}

; CHECK-LABEL: neg-large
; CHECK:        rset32 -42223123, %o0
define i32 @neg-large(i32 %a) {
  ret i32 -42223123
}

; CHECK:        rone 14, %o0
define i32 @pow2(i32 %a) {
  ret i32 16384
}
