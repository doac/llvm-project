//===- SparcTargetTransformInfo.h - Sparc specific TTI ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// This file a TargetTransformInfo::Concept conforming object specific to the
/// Sparc target machine. It uses the target's detailed information to
/// provide more precise answers to certain TTI queries, while letting the
/// target independent and default TTI implementations handle the rest.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPARC_SPARCTARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_SPARC_SPARCTARGETTRANSFORMINFO_H

#include "Sparc.h"
#include "SparcSubtarget.h"
#include "SparcTargetMachine.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/BasicTTIImpl.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class SparcTTIImpl : public BasicTTIImplBase<SparcTTIImpl> {
  using BaseT = BasicTTIImplBase<SparcTTIImpl>;
  using TTI = TargetTransformInfo;

  friend BaseT;

  const SparcSubtarget *ST;
  const SparcTargetLowering *TLI;

  const SparcSubtarget *getST() const { return ST; }
  const SparcTargetLowering *getTLI() const { return TLI; }

public:
  explicit SparcTTIImpl(const SparcTargetMachine *TM, const Function &F)
      : BaseT(TM, F.getParent()->getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}

  /// \name Scalar TTI Implementations
  /// @{

  bool isLSRCostLess(TargetTransformInfo::LSRCost &C1,
                     TargetTransformInfo::LSRCost &C2);

  /// @}
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SPARC_SPARCTARGETTRANSFORMINFO_H
