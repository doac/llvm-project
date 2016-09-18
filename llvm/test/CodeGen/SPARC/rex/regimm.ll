; RUN: llc < %s -verify-machineinstrs -march=sparc -mattr=+rex | FileCheck %s

; CHECK: rst	%i1, [%i0]
; CHECK: rst	%i1, [%i0+4]
; CHECK: rst	%i1, [%i0+8]
; CHECK: rst	%i1, [%i0+12]
; CHECK: rst	%i1, [%i0+60]
; CHECK: rst    %i1, [%i2]
; CHECK: rld	[%i0], %i1
; CHECK: rld	[%i0+4], %i1
; CHECK: rld	[%i0+8], %i1
; CHECK: rld	[%i0+12], %i1
; CHECK: rld	[%i0+60], %i0
; CHECK: rld    [%i2], %i0
define void @regi0imm(i32* %a, i32 %b) #0 {
  %arrayidx1 = getelementptr inbounds i32, i32* %a, i32 1
  %arrayidx2 = getelementptr inbounds i32, i32* %a, i32 2
  %arrayidx3 = getelementptr inbounds i32, i32* %a, i32 3
  %arrayidx4 = getelementptr inbounds i32, i32* %a, i32 15
  %arrayidx5 = getelementptr inbounds i32, i32* %a, i32 16

  store i32 %b, i32* %a
  store i32 %b, i32* %arrayidx1
  store i32 %b, i32* %arrayidx2
  store i32 %b, i32* %arrayidx3
  store i32 %b, i32* %arrayidx4
  store i32 %b, i32* %arrayidx5

  %1 = load volatile i32, i32* %a
  %2 = load volatile i32, i32* %arrayidx1
  %3 = load volatile i32, i32* %arrayidx2
  %4 = load volatile i32, i32* %arrayidx3
  %5 = load volatile i32, i32* %arrayidx4
  %6 = load volatile i32, i32* %arrayidx5

  call void @noleafopt()

  ret void
}

; CHECK: rst	%o1, [%o0]
; CHECK: rst	%o1, [%o0+4]
; CHECK: rst	%o1, [%o0+8]
; CHECK: rst	%o1, [%o0+12]
; CHECK: rst	%o1, [%o0+60]
; CHECK: rst	%o1, [%o2]
; CHECK: rld	[%o0], %o1
; CHECK: rld	[%o0+4], %o1
; CHECK: rld	[%o0+8], %o1
; CHECK: rld	[%o0+12], %o1
; CHECK: rld	[%o0+60], %o0
; CHECK: rld	[%o2], %o0
define void @rego0imm(i32* %a, i32 %b) #0 {
  %arrayidx1 = getelementptr inbounds i32, i32* %a, i32 1
  %arrayidx2 = getelementptr inbounds i32, i32* %a, i32 2
  %arrayidx3 = getelementptr inbounds i32, i32* %a, i32 3
  %arrayidx4 = getelementptr inbounds i32, i32* %a, i32 15
  %arrayidx5 = getelementptr inbounds i32, i32* %a, i32 16

  store i32 %b, i32* %a
  store i32 %b, i32* %arrayidx1
  store i32 %b, i32* %arrayidx2
  store i32 %b, i32* %arrayidx3
  store i32 %b, i32* %arrayidx4
  store i32 %b, i32* %arrayidx5

  %1 = load volatile i32, i32* %a
  %2 = load volatile i32, i32* %arrayidx1
  %3 = load volatile i32, i32* %arrayidx2
  %4 = load volatile i32, i32* %arrayidx3
  %5 = load volatile i32, i32* %arrayidx4
  %6 = load volatile i32, i32* %arrayidx5

  ret void
}

; CHECK: rldf	[%i0], %f0
; CHECK: rldf	[%i0+4], %f0
; CHECK: rldf	[%i0+8], %f0
; CHECK: rldf	[%i0+12], %f0
; CHECK: rldf	[%i0+60], %f0
; CHECK: rldf	[%i1], %f0
; CHECK: rstf	%f0, [%i0]
; CHECK: rstf	%f0, [%i0+4]
; CHECK: rstf	%f0, [%i0+8]
; CHECK: rstf	%f0, [%i0+12]
; CHECK: rstf	%f0, [%i0+60]
; CHECK: rstf	%f0, [%i1]
define void @floatregi0imm(float* %a) #0 {
  %arrayidx1 = getelementptr inbounds float, float* %a, i32 1
  %arrayidx2 = getelementptr inbounds float, float* %a, i32 2
  %arrayidx3 = getelementptr inbounds float, float* %a, i32 3
  %arrayidx4 = getelementptr inbounds float, float* %a, i32 15
  %arrayidx5 = getelementptr inbounds float, float* %a, i32 16

  %1 = load volatile float, float* %a
  %2 = load volatile float, float* %arrayidx1
  %3 = load volatile float, float* %arrayidx2
  %4 = load volatile float, float* %arrayidx3
  %5 = load volatile float, float* %arrayidx4
  %6 = load volatile float, float* %arrayidx5

  store float %6, float* %a
  store float %6, float* %arrayidx1
  store float %6, float* %arrayidx2
  store float %6, float* %arrayidx3
  store float %6, float* %arrayidx4
  store volatile float %6, float* %arrayidx5

  call void @noleafopt()

  ret void
}

; CHECK: rldf	[%o0], %f0
; CHECK: ld [%o0+4], %f0
; CHECK: ld [%o0+8], %f0
; CHECK: ld [%o0+12], %f0
; CHECK: ld [%o0+60], %f0
; CHECK: rldf	[%o1], %f0
; CHECK: rstf	%f0, [%o0]
; CHECK: st %f0, [%o0+4]
; CHECK: st %f0, [%o0+8]
; CHECK: st %f0, [%o0+12]
; CHECK: st %f0, [%o0+60]
; CHECK: rstf	%f0, [%o1]
define void @floatrego0imm(float* %a) #0 {
  %arrayidx1 = getelementptr inbounds float, float* %a, i32 1
  %arrayidx2 = getelementptr inbounds float, float* %a, i32 2
  %arrayidx3 = getelementptr inbounds float, float* %a, i32 3
  %arrayidx4 = getelementptr inbounds float, float* %a, i32 15
  %arrayidx5 = getelementptr inbounds float, float* %a, i32 16

  %1 = load volatile float, float* %a
  %2 = load volatile float, float* %arrayidx1
  %3 = load volatile float, float* %arrayidx2
  %4 = load volatile float, float* %arrayidx3
  %5 = load volatile float, float* %arrayidx4
  %6 = load volatile float, float* %arrayidx5

  store float %6, float* %a
  store float %6, float* %arrayidx1
  store float %6, float* %arrayidx2
  store float %6, float* %arrayidx3
  store float %6, float* %arrayidx4
  store volatile float %6, float* %arrayidx5

  ret void
}

declare void @noleafopt() #1

attributes #0 = { nounwind }
