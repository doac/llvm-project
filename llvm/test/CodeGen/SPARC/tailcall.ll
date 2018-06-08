; RUN: llc < %s -mtriple=sparc -verify-machineinstrs | FileCheck %s

; CHECK-LABEL: simple_leaf
; CHECK: mov %o7, %g1
; CHECK: call foo
; CHECK: mov %g1, %o7

define i32 @simple_leaf(i32 %i) #0 {
entry:
  %call = tail call i32 @foo(i32 %i)
  ret i32 %call
}

; CHECK-LABEL: simple_standard
; CHECK: save %sp, -96, %sp
; CHECK: call foo
; CHECK: restore

define i32 @simple_standard(i32 %i) #1 {
entry:
  %call = tail call i32 @foo(i32 %i)
  ret i32 %call
}

; CHECK-LABEL: extra_arg_leaf
; CHECK: mov 12, %o1
; CHECK: mov %o7, %g1
; CHECK: call foo2
; CHECK: mov %g1, %o7

define i32 @extra_arg_leaf(i32 %i) #0 {
entry:
  %call = tail call i32 @foo2(i32 %i, i32 12)
  ret i32 %call
}

; CHECK-LABEL: extra_arg_standard
; CHECK: save %sp, -96, %sp
; CHECK: call foo2
; CHECK: restore %g0, 12, %o1

define i32 @extra_arg_standard(i32 %i) #1 {
entry:
  %call = tail call i32 @foo2(i32 %i, i32 12)
  ret i32 %call
}

; Perform tail call optimization for external symbol.

; CHECK-LABEL: caller_extern
; CHECK: mov  %o7, %g1
; CHECK: call memcpy
; CHECK: mov  %g1, %o7

define void @caller_extern(i8* %src) optsize #0 {
entry:
  tail call void @llvm.memcpy.p0i8.p0i8.i32(
    i8* getelementptr inbounds ([2 x i8],
    [2 x i8]* @dest, i32 0, i32 0),
    i8* %src, i32 7, i1 false)
  ret void
}

; Perform tail call optimization for function pointer.

; CHECK-LABEL: func_ptr_test
; CHECK: jmp %o0
; CHECK: nop

define i32 @func_ptr_test(i32 ()* nocapture %func_ptr) #0 {
entry:
  %call = tail call i32 %func_ptr() #1
  ret i32 %call
}

; CHECK-LABEL: func_ptr_test2
; CHECK: save %sp, -96, %sp
; CHECK: mov 10, %i3
; CHECK: mov %i0, %i4
; CHECK: mov %i1, %i0
; CHECK: jmp %i4
; CHECK: restore %g0, %i3, %o1
define i32 @func_ptr_test2(i32 (i32, i32, i32)* nocapture %func_ptr,
                           i32 %r, i32 %q) #1 {
entry:
  %call = tail call i32 %func_ptr(i32 %r, i32 10, i32 %q) #1
  ret i32 %call
}


; Do not tail call optimize if stack is used to pass parameters.

; CHECK-LABEL: caller_args
; CHECK: ret

define i32 @caller_args() #0 {
entry:
  %r = tail call i32 @foo7(i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6)
  ret i32 %r
}

; Byval parameters hand the function a pointer directly into the stack area
; we want to reuse during a tail call. Do not tail call optimize functions with
; byval parameters.

; CHECK-LABEL: caller_byval
; CHECK: ret

define i32 @caller_byval() #0 {
entry:
  %a = alloca i32*
  %r = tail call i32 @callee_byval(i32** byval %a)
  ret i32 %r
}

; Perform tail call optimization for sret function.

; CHECK-LABEL: sret_test
; CHECK: mov  %o7, %g1
; CHECK: call sret_func
; CHECK: mov  %g1, %o7

define void @sret_test(%struct.a* noalias sret %agg.result) #0 {
entry:
  tail call void bitcast (void (%struct.a*)* @sret_func to
                          void (%struct.a*)*)(%struct.a* sret %agg.result)
  ret void
}

; Do not tail call if either caller or callee returns
; a struct and the other does not. Returning a large
; struct will generate a memcpy as the tail function.

; CHECK-LABEL: ret_large_struct
; CHECK: jmp %i7+12

define void @ret_large_struct(%struct.big* noalias sret %agg.result) #0 {
entry:
  %0 = bitcast %struct.big* %agg.result to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 4 %0, i8* align 4 bitcast (%struct.big* @bigstruct to i8*), i32 400, i1 false)
  ret void
}

; Test register + immediate pattern.

; CHECK-LABEL: addri_test
; CHECK: jmp %o0+4

define void @addri_test(i32 %ptr) #0 {
entry:
  %add = add nsw i32 %ptr, 4
  %0 = inttoptr i32 %add to void ()*
  tail call void %0() #1
  ret void
}

%struct.a = type { i32, i32 }
@dest = global [2 x i8] zeroinitializer

%struct.big = type { [100 x i32] }
@bigstruct = global %struct.big zeroinitializer

declare void @llvm.memcpy.p0i8.p0i8.i32(i8*, i8*, i32, i1)
declare void @sret_func(%struct.a* sret)
declare i32 @callee_byval(i32** byval %a)
declare i32 @foo(i32)
declare i32 @foo2(i32, i32)
declare i32 @foo7(i32, i32, i32, i32, i32, i32, i32)

attributes #0 = { nounwind "disable-tail-calls"="false"
                  "no-frame-pointer-elim"="false" }
attributes #1 = { nounwind "disable-tail-calls"="false"
                  "no-frame-pointer-elim"="true" }
