; RUN: llc < %s -verify-machineinstrs -march=sparc -mattr=+rex | FileCheck %s

@g = common global [32 x i32] zeroinitializer
@h = common global [32 x float] zeroinitializer
@i = common global [32 x double] zeroinitializer

; CHECK: st {{.+}}, [%fp+-4]             ! 4-byte Folded Spill
; CHECK: ld [%fp+-4], {{.+}}             ! 4-byte Folded Reload
define void @spilli32() #0 {
entry:
  %0 = load volatile i32, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 0), align 16
  %1 = load volatile i32, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 1), align 4
  %2 = load volatile i32, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 2), align 8
  %3 = load volatile i32, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 3), align 4
  %4 = load volatile i32, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 4), align 16
  %5 = load volatile i32, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 5), align 4
  %6 = load volatile i32, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 6), align 8
  %7 = load volatile i32, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 7), align 4
  %8 = load volatile i32, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 8), align 16
  %9 = load volatile i32, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 9), align 4
  %10 = load volatile i32, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 10), align 8
  %11 = load volatile i32, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 11), align 4
  %12 = load volatile i32, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 12), align 16
  %13 = load volatile i32, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 13), align 4
  store volatile i32 %1, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 0), align 16
  store volatile i32 %2, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 1), align 4
  store volatile i32 %3, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 2), align 8
  store volatile i32 %4, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 3), align 4
  store volatile i32 %5, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 4), align 16
  store volatile i32 %6, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 5), align 4
  store volatile i32 %7, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 6), align 8
  store volatile i32 %8, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 7), align 4
  store volatile i32 %9, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 8), align 16
  store volatile i32 %10, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 9), align 4
  store volatile i32 %11, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 10), align 8
  store volatile i32 %12, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 11), align 4
  store volatile i32 %13, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 12), align 16
  store volatile i32 %0, i32* getelementptr inbounds ([32 x i32], [32 x i32]* @g, i64 0, i64 31), align 4

  call void @func()

  ret void
}

; CHECK: st {{.+}}, [%fp+-4]                ! 4-byte Folded Spill
; CHECK: ld [%fp+-4], {{.+}}                ! 4-byte Folded Reload
define void @spillfloat() #0 {
entry:
  %0 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 0), align 16
  %1 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 1), align 4
  %2 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 2), align 8
  %3 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 3), align 4
  %4 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 4), align 16
  %5 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 5), align 4
  %6 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 6), align 8
  %7 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 7), align 4
  %8 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 8), align 16
  %9 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 9), align 4
  %10 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 10), align 8
  %11 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 11), align 4
  %12 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 12), align 16
  %13 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 13), align 4
  %14 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 14), align 8
  %15 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 15), align 8
  %16 = load volatile float, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 16), align 8
  store volatile float %1, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 0), align 16
  store volatile float %2, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 1), align 4
  store volatile float %3, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 2), align 8
  store volatile float %4, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 3), align 4
  store volatile float %5, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 4), align 16
  store volatile float %6, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 5), align 4
  store volatile float %7, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 6), align 8
  store volatile float %8, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 7), align 4
  store volatile float %9, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 8), align 16
  store volatile float %10, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 9), align 4
  store volatile float %11, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 10), align 8
  store volatile float %12, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 11), align 4
  store volatile float %13, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 12), align 16
  store volatile float %14, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 13), align 4
  store volatile float %15, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 14), align 4
  store volatile float %16, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 15), align 4
  store volatile float %0, float* getelementptr inbounds ([32 x float], [32 x float]* @h, i64 0, i64 31), align 4

  call void @func()

  ret void
}

; CHECK: std {{.+}}, [%fp+-8]               ! 8-byte Folded Spill
; CHECK: ldd [%fp+-8], {{.+}}               ! 8-byte Folded Reload
define void @spilldouble() #0 {
entry:
  %0 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 0), align 16
  %1 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 1), align 4
  %2 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 2), align 8
  %3 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 3), align 4
  %4 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 4), align 16
  %5 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 5), align 4
  %6 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 6), align 8
  %7 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 7), align 4
  %8 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 8), align 16
  %9 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 9), align 4
  %10 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 10), align 8
  %11 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 11), align 4
  %12 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 12), align 16
  %13 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 13), align 4
  %14 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 14), align 8
  %15 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 15), align 8
  %16 = load volatile double, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 16), align 8
  store volatile double %1, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 0), align 16
  store volatile double %2, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 1), align 4
  store volatile double %3, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 2), align 8
  store volatile double %4, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 3), align 4
  store volatile double %5, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 4), align 16
  store volatile double %6, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 5), align 4
  store volatile double %7, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 6), align 8
  store volatile double %8, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 7), align 4
  store volatile double %9, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 8), align 16
  store volatile double %10, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 9), align 4
  store volatile double %11, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 10), align 8
  store volatile double %12, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 11), align 4
  store volatile double %13, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 12), align 16
  store volatile double %14, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 13), align 4
  store volatile double %15, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 14), align 4
  store volatile double %16, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 15), align 4
  store volatile double %0, double* getelementptr inbounds ([32 x double], [32 x double]* @i, i64 0, i64 31), align 4

  call void @func()

  ret void
}


declare void @func() #1

attributes #0 = { nounwind }
