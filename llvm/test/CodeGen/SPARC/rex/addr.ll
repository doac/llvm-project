; RUN: llc < %s -verify-machineinstrs -march=sparc -mattr=+rex | FileCheck %s

; CHECK: rstb %l4, [%i0]
; CHECK: rsth %l4, [%i1]
; CHECK: rst  %l4, [%i2]
; CHECK: rstd %i4, [%i3]
define void @storevalues(i8* %a, i16* %b, i32* %c, i64* %d, i64 %val) #0 {
  store i8 2, i8* %a
  store i16 2, i16* %b
  store i32 2, i32* %c
  store i64 %val, i64* %d
  ret void
}

; CHECK: rldub [%o0], %o0
; CHECK: rlduh [%o1], %o0
; CHECK: rld  [%o2], %o0
; CHECK: rldd [%o3], %o0
define void @loadvalues(i8* %a, i16* %b, i32* %c, i64* %d) #0 {
  %1 = load volatile i8, i8* %a
  %2 = load volatile i16, i16* %b
  %3 = load volatile i32, i32* %c
  %4 = load volatile i64, i64* %d
  ret void
}

; CHECK: rldf	[%o0], %f0
; CHECK: stf	%f0, [%o0]
; CHECK: rlddf	[%o1], %f0
; CHECK: stdf	%f0, [%o1]
define void @storeloadfloatvalues(float* %a, double* %b) #0 {
  %1 = load float, float* %a
  %mulf = fmul float %1, %1
  store float %mulf, float* %a
  %2 = load double, double* %b
  %muld = fmul double %2, %2
  store double %muld, double* %b
  ret void
}

attributes #0 = { nounwind }



