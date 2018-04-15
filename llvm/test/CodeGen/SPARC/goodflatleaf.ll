; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "E-m:e-p:32:32-i64:64-f128:64-n32-S64"
target triple = "sparc-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind
define dso_local void @_Z10example10aPsS_S_PiS0_S0_(i16* noalias nocapture %sa, i16* noalias nocapture readonly %sb, i16* noalias nocapture readonly %sc, i32* noalias nocapture %ia, i32* noalias nocapture readonly %ib, i32* noalias nocapture readonly %ic) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.017 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %ib, i32 %i.017
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %arrayidx1 = getelementptr inbounds i32, i32* %ic, i32 %i.017
  %1 = load i32, i32* %arrayidx1, align 4, !tbaa !2
  %add = add nsw i32 %1, %0
  %arrayidx2 = getelementptr inbounds i32, i32* %ia, i32 %i.017
  store i32 %add, i32* %arrayidx2, align 4, !tbaa !2
  %arrayidx3 = getelementptr inbounds i16, i16* %sb, i32 %i.017
  %2 = load i16, i16* %arrayidx3, align 2, !tbaa !6
  %arrayidx4 = getelementptr inbounds i16, i16* %sc, i32 %i.017
  %3 = load i16, i16* %arrayidx4, align 2, !tbaa !6
  %add6 = add i16 %3, %2
  %arrayidx8 = getelementptr inbounds i16, i16* %sa, i32 %i.017
  store i16 %add6, i16* %arrayidx8, align 2, !tbaa !6
  %inc = add nuw nsw i32 %i.017, 1
  %exitcond = icmp eq i32 %inc, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { noinline norecurse  "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-features"="+flat" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (trunk 339995) (llvm/trunk 340735)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"short", !4, i64 0}
