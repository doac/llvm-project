; RUN: llc < %s -O1 -mtriple=sparc | FileCheck %s -check-prefix=NO_ITIN
; RUN: llc < %s -O1 -mtriple=sparc -mcpu=leon2 | FileCheck %s -check-prefix=LEON2_ITIN
; RUN: llc < %s -O1 -mtriple=sparc -mcpu=leon3 | FileCheck %s -check-prefix=LEON3_4_ITIN
; RUN: llc < %s -O1 -mtriple=sparc -mcpu=leon4 | FileCheck %s -check-prefix=LEON3_4_ITIN
; RUN: llc < %s -O1 -mtriple=sparc -mcpu=leon5 | FileCheck %s -check-prefix=LEON5

; NO_ITIN-LABEL: f32_ops:
; NO_ITIN:       ld
; NO_ITIN-NEXT:  ld
; NO_ITIN-NEXT:  ld
; NO_ITIN-NEXT:  ld
; NO_ITIN-NEXT:  fadds
; NO_ITIN-NEXT:  fsubs
; NO_ITIN-NEXT:  fmuls
; NO_ITIN-NEXT:  retl
; NO_ITIN-NEXT:  fdivs

; LEON2_ITIN-LABEL: f32_ops:
; LEON2_ITIN:       ld
; LEON2_ITIN-NEXT:  ld
; LEON2_ITIN-NEXT:  fadds
; LEON2_ITIN-NEXT:  ld
; LEON2_ITIN-NEXT:  fsubs
; LEON2_ITIN-NEXT:  ld
; LEON2_ITIN-NEXT:  fmuls
; LEON2_ITIN-NEXT:  retl
; LEON2_ITIN-NEXT:  fdivs

; LEON3_4_ITIN-LABEL: f32_ops:
; LEON3_4_ITIN:       ld
; LEON3_4_ITIN-NEXT:  ld
; LEON3_4_ITIN-NEXT:  ld
; LEON3_4_ITIN-NEXT:  fadds
; LEON3_4_ITIN-NEXT:  ld
; LEON3_4_ITIN-NEXT:  fsubs
; LEON3_4_ITIN-NEXT:  fmuls
; LEON3_4_ITIN-NEXT:  retl
; LEON3_4_ITIN-NEXT:  fdivs

; LEON5-LABEL: f32_ops:
; LEON5:       ld
; LEON5-NEXT:  ld
; LEON5-NEXT:  ld
; LEON5-NEXT:  fadds
; LEON5-NEXT:  ld
; LEON5-NEXT:  fsubs
; LEON5-NEXT:  fmuls
; LEON5-NEXT:  retl
; LEON5-NEXT:  fdivs

define float @f32_ops(ptr byval(float) %a, ptr byval(float) %b, ptr byval(float) %c, ptr byval(float) %d) {
entry:
  %0 = load float, ptr %a, align 8
  %1 = load float, ptr %b, align 8
  %2 = load float, ptr %c, align 8
  %3 = load float, ptr %d, align 8
  %4 = fadd float %0, %1
  %5 = fsub float %4, %2
  %6 = fmul float %5, %3
  %7 = fdiv float %6, %4
  ret float %7
}

;; On LEON5 a load result reaches the next instruction two cycles after the
;; load, so the independent instructions are scheduled between the load and
;; its use.

; NO_ITIN-LABEL: load_use:
; NO_ITIN:       ld [%o0], [[L:%o[0-9]]]
; NO_ITIN-NEXT:  add [[L]], 1
; NO_ITIN-NEXT:  add %o1, %o2
; NO_ITIN-NEXT:  xor %o1, %o2

; LEON3_4_ITIN-LABEL: load_use:
; LEON3_4_ITIN:       ld [%o0], [[L:%o[0-9]]]
; LEON3_4_ITIN-NEXT:  add [[L]], 1
; LEON3_4_ITIN-NEXT:  add %o1, %o2
; LEON3_4_ITIN-NEXT:  xor %o1, %o2

; LEON5-LABEL: load_use:
; LEON5:       ld [%o0], [[L:%o[0-9]]]
; LEON5-NEXT:  add %o1, %o2
; LEON5-NEXT:  xor %o1, %o2
; LEON5-NEXT:  add [[L]], 1

define i32 @load_use(ptr %p, i32 %x, i32 %y) {
entry:
  %0 = load i32, ptr %p
  %1 = add i32 %0, 1
  %2 = add i32 %x, %y
  %3 = xor i32 %x, %y
  %4 = or i32 %2, %3
  %5 = and i32 %1, %4
  ret i32 %5
}

;; The scheduling model comes from the tune CPU. This function has the same
;; CPU and features as load_use, but must not share its subtarget.

; LEON3_4_ITIN-LABEL: load_use_tune_leon5:
; LEON3_4_ITIN:       ld [%o0], [[L:%o[0-9]]]
; LEON3_4_ITIN-NEXT:  add %o1, %o2
; LEON3_4_ITIN-NEXT:  xor %o1, %o2
; LEON3_4_ITIN-NEXT:  add [[L]], 1

define i32 @load_use_tune_leon5(ptr %p, i32 %x, i32 %y) #0 {
entry:
  %0 = load i32, ptr %p
  %1 = add i32 %0, 1
  %2 = add i32 %x, %y
  %3 = xor i32 %x, %y
  %4 = or i32 %2, %3
  %5 = and i32 %1, %4
  ret i32 %5
}

attributes #0 = { "tune-cpu"="leon5" }
