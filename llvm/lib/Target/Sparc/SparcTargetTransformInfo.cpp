//===- SparcTargetTransformInfo.cpp - Sparc specific TTI ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "SparcTargetTransformInfo.h"
#include "SparcSubtarget.h"
#include "SparcTargetMachine.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/CodeGen/CostTable.h"
#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/MachineValueType.h"
#include "llvm/Target/TargetMachine.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>

using namespace llvm;

#define DEBUG_TYPE "sparctti"

int SparcTTIImpl::getIntImmCodeSizeCost(unsigned Opcode, unsigned Idx,
                                        const APInt &Imm, Type *Ty) {
  return getIntImmCost(Imm, Ty);
}

int SparcTTIImpl::getIntImmCost(const APInt &Imm, Type *Ty) {
  unsigned Bits = Ty->getPrimitiveSizeInBits();
  if (Bits == 0 || Imm.getActiveBits() > 32)
    return  2 * TTI::TCC_Basic;
    
  if (isInt<13>(Imm.getSExtValue()))
    return TTI::TCC_Free;
  return TTI::TCC_Basic;
}

int SparcTTIImpl::getIntImmCost(unsigned Opcode, unsigned Idx, const APInt &Imm,
                                Type *Ty) {
  return getIntImmCost(Imm, Ty);
}

bool SparcTTIImpl::isLSRCostLess(TargetTransformInfo::LSRCost &C1,
                                 TargetTransformInfo::LSRCost &C2) {
  return std::tie(C1.Insns, C1.NumRegs, C1.AddRecCost, C1.NumIVMuls,
                  C1.NumBaseAdds, C1.ScaleCost, C1.ImmCost, C1.SetupCost) <
         std::tie(C2.Insns, C2.NumRegs, C2.AddRecCost, C2.NumIVMuls,
                  C2.NumBaseAdds, C2.ScaleCost, C2.ImmCost, C2.SetupCost);
}
