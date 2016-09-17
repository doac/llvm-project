; RUN: llc < %s -march=sparc -mattr=+rex | FileCheck %s

; Returning struct

%struct.a = type { i32 }

; CHECK-LABEL: retstruct
; CHECK:       saverex %sp, -96, %sp
; CHECK:       ld [%fp+64], %i0
; CHECK:       raddcc	4, %i7
; CHECK-NEXT:  rretrest
define void @retstruct(%struct.a* noalias nocapture sret %agg.result) #0 {
 call void @func()
 ret void
}

; CHECK-LABEL: retstructleaf
; CHECK:       enterrex
; CHECK:       ld [%sp+64], %o0
; CHECK:       add %o7, 4, %o7
; CHECK-NEXT:  rretl
define void @retstructleaf(%struct.a* noalias nocapture sret %agg.result) #0 {
  ret void
}

; Calling function returning struct

; CHECK-LABEL: callretstruct
; CHECK:       saverex %sp, -104, %sp
; CHECK:       call getstruct
; CHECK-NEXT:  unimp 4
define void @callretstruct() #0 {
 %b = alloca %struct.a, align 4
 call void @getstruct(%struct.a* nonnull sret %b)
 ret void
}

declare void @func() #1
declare void @getstruct(%struct.a* sret) #2

attributes #0 = { nounwind }
