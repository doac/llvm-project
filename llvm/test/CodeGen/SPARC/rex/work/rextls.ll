; RUN: llc < %s -march=sparc -mattr=+rex -relocation-model=pic | FileCheck %s

@gdvar1 = thread_local                local_unnamed_addr global i32 0, align 4
@gdvar2 = thread_local                local_unnamed_addr global i32 0, align 4
@ldvar1 = thread_local(localdynamic)  local_unnamed_addr global i32 0, align 4
@ldvar2 = thread_local(localdynamic)  local_unnamed_addr global i32 0, align 4
@ievar  = thread_local(initialexec)   local_unnamed_addr global i32 0, align 4
@levar  = thread_local(localexec)     local_unnamed_addr global i32 0, align 4

define i32 @read_gdvar() #0 {
entry:
  %0 = load i32, i32* @gdvar1
  %1 = load i32, i32* @gdvar2
  %2 = add i32 %0, %1
  ret i32 %2
}

define i32 @read_ldvar() #0 {
entry:
  %0 = load i32, i32* @ldvar1
  %1 = load i32, i32* @ldvar2
  %2 = add i32 %0, %1
  ret i32 %2
}

define i32 @read_ievar() #0 {
entry:
  %0 = load i32, i32* @ievar
  ret i32 %0
}

; CHECK:   rmov    %g7, %o0
; CHECK:  rset32  %tle_32(levar), %o1
; CHECK:  radd    %o0, %o1
; CHECK:  rld     [%o1], %o0

define i32 @read_levar() #0 {
entry:
  %0 = load i32, i32* @levar
  ret i32 %0
}

attributes #0 = { nounwind }
