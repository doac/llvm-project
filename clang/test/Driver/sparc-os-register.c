// RUN: %clang -target sparc -ffixed-g2 -### %s 2> %t
// RUN: FileCheck --check-prefix=CHECK-FIXED-G2 < %t %s
// CHECK-FIXED-G2: "-target-feature" "+reserve-reg-g2"

// RUN: %clang -target sparc -ffixed-g3 -### %s 2> %t
// RUN: FileCheck --check-prefix=CHECK-FIXED-G3 < %t %s
// CHECK-FIXED-G3: "-target-feature" "+reserve-reg-g3"

// RUN: %clang -target sparc -ffixed-g4 -### %s 2> %t
// RUN: FileCheck --check-prefix=CHECK-FIXED-G4 < %t %s
// CHECK-FIXED-G4: "-target-feature" "+reserve-reg-g4"

// RUN: %clang -target sparc -ffixed-g5 -### %s 2> %t
// RUN: FileCheck --check-prefix=CHECK-FIXED-G5 < %t %s
// CHECK-FIXED-G5: "-target-feature" "+reserve-reg-g5"

// RUN: %clang -target sparc -fcall-used-g5 -### %s 2> %t
// RUN: FileCheck --check-prefix=CHECK-USED-G5 < %t %s
// CHECK-USED-G5: "-target-feature" "+use-reg-g5"

// RUN: %clang -target sparc -fcall-used-g6 -### %s 2> %t
// RUN: FileCheck --check-prefix=CHECK-USED-G6 < %t %s
// CHECK-USED-G6: "-target-feature" "+use-reg-g6"

// RUN: %clang -target sparc -fcall-used-g7 -### %s 2> %t
// RUN: FileCheck --check-prefix=CHECK-USED-G7 < %t %s
// CHECK-USED-G7: "-target-feature" "+use-reg-g7"
