; RUN: llc < %s -march=sparc -mattr=+rex | FileCheck %s

; CHECK:       rcmp	%o0, %o1
; CHECK:       rbe
define i8 @be(i32 %a, i32 %b) #0 {
  %val = icmp eq i32 %a, %b
  %r = select i1 %val, i8 10, i8 20
  ret i8 %r
}

; CHECK:       rcmp	%o0, %o1
; CHECK:       rbne
define i8 @bne(i32 %a, i32 %b) #0 {
  %val = icmp ne i32 %a, %b
  %r = select i1 %val, i8 10, i8 20
  ret i8 %r
}

; CHECK:       rcmp	%o0, %o1
; CHECK:       rbgu
define i1 @bgu(i32 %a, i32 %b) #0 {
  %val = icmp ugt i32 %a, %b
  ret i1 %val
}

; CHECK:       rcmp	%o0, %o1
; CHECK:       rbcc
define i1 @bcc(i32 %a, i32 %b) #0 {
  %val = icmp uge i32 %a, %b
  ret i1 %val
}

; CHECK:       rcmp	%o0, %o1
; CHECK:       rbcs
define i1 @bcs(i32 %a, i32 %b) #0 {
  %val = icmp ult i32 %a, %b
  ret i1 %val
}

; CHECK:       rcmp	%o0, %o1
; CHECK:       rbleu
define i1 @bleu(i32 %a, i32 %b) #0 {
  %val = icmp ule i32 %a, %b
  ret i1 %val
}

; CHECK:       rcmp	%o0, %o1
; CHECK:       rbg
define i1 @bg(i32 %a, i32 %b) #0 {
  %val = icmp sgt i32 %a, %b
  ret i1 %val
}

; CHECK:       rcmp	%o0, %o1
; CHECK:       rbge
define i1 @bge2(i32 %a, i32 %b) #0 {
  %val = icmp sge i32 %a, %b
  ret i1 %val
}

; CHECK:       rcmp	%o0, %o1
; CHECK:       rbl
define i1 @blwhat(i32 %a, i32 %b) #0 {
  %val = icmp slt i32 %a, %b
  ret i1 %val
}

; CHECK:       rcmp	%o0, %o1
; CHECK:       rble
define i1 @ble(i32 %a, i32 %b) #0 {
  %val = icmp sle i32 %a, %b
  ret i1 %val
}

; CHECK:       rcmp	%o0, %o1
; CHECK:       rbge
define i1 @bge(i32 %a, i32 %b) #0 {
  %val = icmp sge i32 %a, %b
  ret i1 %val
}

; Floating point comparison

; CHECK:       rfbe
define i1 @fbe(float %a, float %b) #0 {
  %val = fcmp oeq float %a, %b
  ret i1 %val
}

; CHECK:       rfbg
define i1 @fbg(float %a, float %b) #0 {
  %val = fcmp ogt float %a, %b
  ret i1 %val
}

; CHECK:       rfbge
define i1 @fbge(float %a, float %b) #0 {
  %val = fcmp oge float %a, %b
  ret i1 %val
}

; CHECK:       rfbl
define i1 @fbl(float %a, float %b) #0 {
  %val = fcmp olt float %a, %b
  ret i1 %val
}

; CHECK:       rfble
define i1 @fble(float %a, float %b) #0 {
  %val = fcmp ole float %a, %b
  ret i1 %val
}

; CHECK:       rfbne
define i1 @rfbne(float %a, float %b) #0 {
  %val = fcmp one float %a, %b
  ret i1 %val
}

; CHECK:       rfbo
define i1 @fbo(float %a, float %b) #0 {
  %val = fcmp ord float %a, %b
  ret i1 %val
}

; CHECK:       rfbu
define i1 @fbu(float %a, float %b) #0 {
  %val = fcmp uno float %a, %b
  ret i1 %val
}

; CHECK:       rfbue
define i1 @fbue(float %a, float %b) #0 {
  %val = fcmp ueq float %a, %b
  ret i1 %val
}

; CHECK:       rfbug
define i1 @fbug(float %a, float %b) #0 {
  %val = fcmp ugt float %a, %b
  ret i1 %val
}

; CHECK:       rfbuge
define i1 @fbuge(float %a, float %b) #0 {
  %val = fcmp uge float %a, %b
  ret i1 %val
}

; CHECK:       rfbul
define i1 @fbul(float %a, float %b) #0 {
  %val = fcmp ult float %a, %b
  ret i1 %val
}

; CHECK:       rfbule
define i1 @fbule(float %a, float %b) #0 {
  %val = fcmp ule float %a, %b
  ret i1 %val
}

; CHECK:       rfbne
define i1 @fbne(float %a, float %b) #0 {
  %val = fcmp une float %a, %b
  ret i1 %val
}

attributes #0 = { nounwind }
