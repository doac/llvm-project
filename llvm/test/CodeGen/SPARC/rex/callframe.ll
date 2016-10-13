; RUN: llc < %s -march=sparc -mattr=+rex | FileCheck %s

; Normal functions with increasing stack usage

; CHECK-LABEL: nostack
; CHECK:       saverex %sp, -96, %sp
; CHECK:       rretrest
define i32 @nostack() #0 {
 %call = call i32 @func()
 ret i32 %call
}

; CHECK-LABEL: smallstack
; CHECK:       saverex %sp, -96, %sp
; CHECK:       rretrest
define i32 @smallstack() #0 {
 %data = alloca [1 x i32], align 4
 %call = call i32 @func()
 ret i32 %call
}

; CHECK-LABEL: mediumstack
; CHECK:       saverex %sp, -4096, %sp
; CHECK-NEXT:  add %sp, -40, %sp
; CHECK:       rretrest
define i32 @mediumstack() #0 {
 %data = alloca [1010 x i32], align 4
 %call = call i32 @func()
 ret i32 %call
}

; CHECK-LABEL: bigstack
; CHECK:       saverex %sp, -4096, %sp
; CHECK-NEXT:  rset21 -4192, %i0
; CHECK-NEXT:  add %i0, %sp, %sp
; CHECK:       rretrest
define i32 @bigstack() #0 {
 %data = alloca [2048 x i32], align 4
 %call = call i32 @func()
 ret i32 %call
}

; CHECK-LABEL: biggerstack
; CHECK:       saverex %sp, -4096, %sp
; CHECK-NEXT:  rset32 -4190304, %i0
; CHECK-NEXT:  add %i0, %sp, %sp
; CHECK:       rretrest
define i32 @biggerstack() #0 {
 %data = alloca [1048576 x i32], align 4
 %call = call i32 @func()
 ret i32 %call
}

; Leaf functions with increasing stack usage

; CHECK-LABEL: nostackleaf
; CHECK:       enterrex
; CHECK:       rretl
define void @nostackleaf() #0 {
 ret void
}

; CHECK-LABEL: smallstackleaf
; CHECK:       addrex %sp, -96, %sp
; CHECK:       rset21 96, %o0
; CHECK-NEXT:  add %o0, %sp, %sp
; CHECK-NEXT:  rretl
define void @smallstackleaf() #0 {
 %data = alloca [1 x i32], align 4
 ret void
}

; CHECK-LABEL: mediumstackleaf
; CHECK:       addrex %sp, -4096, %sp
; CHECK-NEXT:  add %sp, -40, %sp
; CHECK:       rset21 4136, %o0
; CHECK-NEXT:  add %o0, %sp, %sp
; CHECK-NEXT:  rretl
define void @mediumstackleaf() #0 {
 %data = alloca [1010 x i32], align 4
 ret void
}

; CHECK-LABEL: bigstackleaf
; CHECK:       addrex %sp, -4096, %sp
; CHECK-NEXT:  rset21 -4192, %o0
; CHECK-NEXT:  add %o0, %sp, %sp
; CHECK:       rset21 8288, %o0
; CHECK-NEXT:  add %o0, %sp, %sp
; CHECK-NEXT:  rretl
define void @bigstackleaf() #0 {
 %data = alloca [2048 x i32], align 4
 ret void
}

; CHECK-LABEL: biggerstackleaf
; CHECK:       addrex %sp, -4096, %sp
; CHECK-NEXT:  rset32 -4190304, %o0
; CHECK-NEXT:  add %o0, %sp, %sp
; CHECK:       rset32 4194400, %o0
; CHECK-NEXT:  add %o0, %sp, %sp
; CHECK-NEXT:  rretl
define void @biggerstackleaf() #0 {
 %data = alloca [1048576 x i32], align 4
 ret void
}

declare i32 @func() #1

attributes #0 = { nounwind }
