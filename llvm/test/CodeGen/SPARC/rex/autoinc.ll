; RUN: llc < %s -march=sparc -mattr=+rex | FileCheck %s

; SIGNED UNSIGNED?

; Auto-incrementing store

; CHECK: rstbinc %o1, [%o0]
define i8* @rstbincfunc(i8* %v, i8 %q) #0 {
entry:
  store i8 %q, i8* %v
  %incdec.ptr = getelementptr inbounds i8, i8* %v, i32 1
  ret i8* %incdec.ptr
}

; CHECK: rsthinc   %o1, [%o0]
define i16* @rsthincfunc(i16* %v, i16 %q) #0 {
entry:
  store i16 %q, i16* %v
  %incdec.ptr = getelementptr inbounds i16, i16* %v, i32 1
  ret i16* %incdec.ptr
}

; CHECK: rstinc   %o1, [%o0]
define i32* @rstincfunc(i32* %v, i32 %q) #0 {
entry:
  store i32 %q, i32* %v
  %incdec.ptr = getelementptr inbounds i32, i32* %v, i32 1
  ret i32* %incdec.ptr
}

; CHECK: rstdinc  %o2, [%o0]
define i64* @rstdincfunc(i64* %v, i32 %a, i64 %q) #0 {
entry:
  store i64 %q, i64* %v
  %incdec.ptr = getelementptr inbounds i64, i64* %v, i32 1
  ret i64* %incdec.ptr
}

; CHECK: rstfinc   %f0, [%o0]
define float* @rstfincfunc(float* %v, float %q) #0 {
entry:
  %mul = fmul float %q, %q
  store float %mul, float* %v
  %incdec.ptr = getelementptr inbounds float, float* %v, i32 1
  ret float* %incdec.ptr
}

; CHECK: rstdfinc   %f0, [%o0]
define double* @rstdfincfunc(double* %v, float %a, double %q) #0 {
entry:
  %mul = fmul double %q, %q
  store double %mul, double* %v
  %incdec.ptr = getelementptr inbounds double, double* %v, i32 1
  ret double* %incdec.ptr
}

; Auto-incrementing load

; CHECK: rldubinc   [%o0], %o2
define i8* @rldubincfunc(i8* %v, i8* %q) #0 {
entry:
  %0 = load i8, i8* %v
  store i8 %0, i8* %q
  %incdec.ptr = getelementptr inbounds i8, i8* %v, i32 1
  ret i8* %incdec.ptr
}

; REX does not have a rldsbinc instruction
; CHECK: ldsb [%o0], %o2
define i32* @rldsbincfunc(i8* %v, i32* %q) #0 {
entry:
  %0 = load i8, i8* %v
  %1 = sext i8 %0 to i32
  store i32 %1, i32* %q
  %incdec.ptr = getelementptr inbounds i8, i8* %v, i32 1
  %2 = bitcast i8* %incdec.ptr to i32*
  ret i32* %2
}

; CHECK: rlduhinc   [%o0], %o2
define i16* @rlduhincfunc(i16* %v, i16* %q) #0 {
entry:
  %0 = load i16, i16* %v
  store i16 %0, i16* %q
  %incdec.ptr = getelementptr inbounds i16, i16* %v, i32 1
  ret i16* %incdec.ptr
}

; REX does not have a rldshinc instruction
; CHECK: ldsh [%o0], %o2
define i32* @rldshincfunc(i16* %v, i32* %q) #0 {
entry:
  %0 = load i16, i16* %v
  %1 = sext i16 %0 to i32
  store i32 %1, i32* %q
  %incdec.ptr = getelementptr inbounds i16, i16* %v, i32 1
  %2 = bitcast i16* %incdec.ptr to i32*
  ret i32* %2
}

; CHECK: rldinc   [%o0], %o2
define i32* @rldincfunc(i32* %v, i32* %q) #0 {
entry:
  %0 = load i32, i32* %v
  store i32 %0, i32* %q
  %incdec.ptr = getelementptr inbounds i32, i32* %v, i32 1
  ret i32* %incdec.ptr
}

; CHECK: rldfinc   [%o0], %f0
define float* @rldfincfunc(float* %v, float* %q) #0 {
entry:
  %0 = load float, float* %v
  store float %0, float* %q
  %incdec.ptr = getelementptr inbounds float, float* %v, i32 1
  ret float* %incdec.ptr
}

; CHECK: rlddfinc   [%o0], %f0
define double* @rlddfincfunc(double* %v, double* %q) #0 {
entry:
  %0 = load double, double* %v
  store double %0, double* %q
  %incdec.ptr = getelementptr inbounds double, double* %v, i32 1
  ret double* %incdec.ptr
}

attributes #0 = { nounwind }
