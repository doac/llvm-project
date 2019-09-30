; ModuleID = 'bugpoint-reduced-simplified.bc'
source_filename = "/home/cederman/tests/leon-tctest/tests/gcc/gcc.c-torture/execute/pr56866.c"
target datalayout = "E-m:e-p:32:32-i64:64-f128:64-n32-S64"
target triple = "sparc"

define dso_local void @main() #0 {
entry:
  %rq = alloca [256 x i64], align 8
  %ri = alloca [256 x i32], align 4
  %ws = alloca [256 x i16], align 2
  %rs = alloca [256 x i16], align 2
  %wc = alloca [256 x i8], align 1
  %rc = alloca [256 x i8], align 1
  %arraydecay6 = getelementptr inbounds [256 x i8], [256 x i8]* %wc, i32 0, i32 0
  call void asm sideeffect "", "imr,imr,imr,imr,~{memory}"(i64* undef, i32* undef, i16* undef, i8* %arraydecay6) #1, !srcloc !1
  unreachable
}

attributes #0 = { "target-features"="+rex" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 8.0.0 "}
!1 = !{i32 577}
