; RUN: llc < %s -march=sparc -mattr=+rex -relocation-model=pic | FileCheck %s

@gvar1 = global i32 0, align 4
@gvar2 = global i32 0, align 4

define i32 @read_gvar() #0 {
entry:
  %0 = load i32, i32* @gvar1
  %1 = load i32, i32* @gvar2
  %2 = add i32 %0, %1
  ret i32 %2
}

attributes #0 = { nounwind }
