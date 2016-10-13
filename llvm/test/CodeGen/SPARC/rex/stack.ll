; RUN: llc < %s -verify-machineinstrs -march=sparc -mattr=+rex | FileCheck %s

; CHECK:   st %i0, [%fp+-4]
; CHECK:   st %i0, [%fp+-8]
; CHECK:   st %i0, [%fp+-12]
; CHECK:   st %i0, [%fp+-16]
; CHECK:   st %i0, [%fp+-20]
; CHECK:   st %i0, [%fp+-24]
; CHECK:   st %i0, [%fp+-28]
; CHECK:   st %i0, [%fp+-32]
; CHECK:   st %i0, [%fp+-36]
; CHECK:   st %i0, [%fp+-40]
; CHECK:   st %i0, [%fp+-44]
; CHECK:   st %i0, [%fp+-48]
define void @stacki32() #0 {
  %a = alloca i32;
  %b = alloca i32;
  %c = alloca i32;
  %d = alloca i32;
  %e = alloca i32;
  %f = alloca i32;
  %g = alloca i32;
  %h = alloca i32;
  %i = alloca i32;
  %j = alloca i32;
  %k = alloca i32;
  %l = alloca i32;

  store i32 1, i32* %a
  store i32 1, i32* %b
  store i32 1, i32* %c
  store i32 1, i32* %d
  store i32 1, i32* %e
  store i32 1, i32* %f
  store i32 1, i32* %g
  store i32 1, i32* %h
  store i32 1, i32* %i
  store i32 1, i32* %j
  store i32 1, i32* %k
  store i32 1, i32* %l

  call void @func()

  ret void
}

; CHECK:        st %o0, [%sp+%o1]
; CHECK:        rset21  136, %o1
; CHECK:        st %o0, [%sp+%o1]
; CHECK:        rset21  132, %o1
; CHECK:        st %o0, [%sp+%o1]
; CHECK:        rset21  128, %o1
; CHECK:        st %o0, [%sp+%o1]
; CHECK:        rst     %o0, [%sp+124]
; CHECK:        rst     %o0, [%sp+120]
; CHECK:        rst     %o0, [%sp+116]
; CHECK:        rst     %o0, [%sp+112]
; CHECK:        rst     %o0, [%sp+108]
; CHECK:        rst     %o0, [%sp+104]
; CHECK:        rst     %o0, [%sp+100]
; CHECK:        rst     %o0, [%sp+96]
define void @stacki32leaf() #0 {
  %a = alloca i32;
  %b = alloca i32;
  %c = alloca i32;
  %d = alloca i32;
  %e = alloca i32;
  %f = alloca i32;
  %g = alloca i32;
  %h = alloca i32;
  %i = alloca i32;
  %j = alloca i32;
  %k = alloca i32;
  %l = alloca i32;

  store i32 1, i32* %a
  store i32 1, i32* %b
  store i32 1, i32* %c
  store i32 1, i32* %d
  store i32 1, i32* %e
  store i32 1, i32* %f
  store i32 1, i32* %g
  store i32 1, i32* %h
  store i32 1, i32* %i
  store i32 1, i32* %j
  store i32 1, i32* %k
  store i32 1, i32* %l

  ret void
}

; CHECK:        rldub   [%i0], %i0
; CHECK:        stb %i0, [%fp+%i1]
; CHECK:        rlduh   [%i0], %i0
; CHECK:        sth %i0, [%fp+%i1]
; CHECK:        ld [%fp+%i0], %i0
; CHECK:        st %i0, [%fp+%i1]
; CHECK:        ldd [%fp+%i0], %i0
; CHECK:        std %i0, [%fp+%i2]
; CHECK:        ld [%fp+%i0], %f0
; CHECK:        st %f0, [%fp+%i0]
; CHECK:        ldd [%fp+%i0], %f0
; CHECK:        std %f0, [%fp+%i0]
define void @stackother() #0 {

  %x = alloca [64 x i32];

  %a = alloca i8;
  %b = alloca i16;
  %c = alloca i32;
  %d = alloca i64;
  %e = alloca float;
  %f = alloca double;

  %1 = load volatile i8,     i8* %a
  store volatile i8  %1,    i8* %a
  %2 = load volatile i16,    i16* %b
  store volatile i16 %2,    i16* %b
  %3 = load volatile i32,    i32* %c
  store volatile i32 %3,    i32* %c
  %4 = load volatile i64,    i64* %d
  store volatile i64 %4,    i64* %d
  %5 = load volatile float,  float* %e
  store volatile float  %5, float*  %e
  %6 = load volatile double, double* %f
  store volatile double %6, double* %f

  call void @func()

  ret void

}

define void @hugestack() #0 {
  %x = alloca [1048576 x i32]
  %idx = getelementptr inbounds [1048576 x i32], [1048576 x i32]* %x,
                       i32 0, i32 1023
  store i32 123, i32* %idx

  call void @func()
  ret void
}

declare void @func() #1

attributes #0 = { nounwind }
