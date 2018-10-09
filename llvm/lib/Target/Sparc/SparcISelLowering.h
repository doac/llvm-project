//===-- SparcISelLowering.h - Sparc DAG Lowering Interface ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Sparc uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPARC_SPARCISELLOWERING_H
#define LLVM_LIB_TARGET_SPARC_SPARCISELLOWERING_H

#include "Sparc.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
  class SparcSubtarget;

  namespace SPISD {
    enum NodeType : unsigned {
      FIRST_NUMBER = ISD::BUILTIN_OP_END,
      CMPICC,      // Compare two GPR operands, set icc+xcc.
      CMPFCC,      // Compare two FP operands, set fcc.
      BRICC,       // Branch to dest on icc condition
      BRXCC,       // Branch to dest on xcc condition (64-bit only).
      BRFCC,       // Branch to dest on fcc condition
      SELECT_ICC,  // Select between two values using the current ICC flags.
      SELECT_XCC,  // Select between two values using the current XCC flags.
      SELECT_FCC,  // Select between two values using the current FCC flags.

      Hi, Lo,      // Hi/Lo operations, typically on a global address.

      FTOI,        // FP to Int within a FP register.
      ITOF,        // Int to FP within a FP register.
      FTOX,        // FP to Int64 within a FP register.
      XTOF,        // Int64 to FP within a FP register.

      CALL,        // A call instruction.
      RET_FLAG,    // Return with a flag operand.
      GLOBAL_BASE_REG, // Global base reg for PIC.
      FLUSHW,      // FLUSH register windows to stack.

      TAIL_CALL,   // Tail call

      TLS_ADD,     // For Thread Local Storage (TLS).
      TLS_LD,
      TLS_CALL
    };
  }

  class SparcTargetLowering : public TargetLowering {
    const SparcSubtarget *Subtarget;
  public:
    SparcTargetLowering(const TargetMachine &TM, const SparcSubtarget &STI);
    SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

    bool useSoftFloat() const override;

    /// computeKnownBitsForTargetNode - Determine which of the bits specified
    /// in Mask are known to be either zero or one and return them in the
    /// KnownZero/KnownOne bitsets.
    void computeKnownBitsForTargetNode(const SDValue Op,
                                       KnownBits &Known,
                                       const APInt &DemandedElts,
                                       const SelectionDAG &DAG,
                                       unsigned Depth = 0) const override;

    MachineBasicBlock *
    EmitInstrWithCustomInserter(MachineInstr &MI,
                                MachineBasicBlock *MBB) const override;

    const char *getTargetNodeName(unsigned Opcode) const override;

    ConstraintType getConstraintType(StringRef Constraint) const override;
    ConstraintWeight
    getSingleConstraintMatchWeight(AsmOperandInfo &info,
                                   const char *constraint) const override;
    void LowerAsmOperandForConstraint(SDValue Op,
                                      std::string &Constraint,
                                      std::vector<SDValue> &Ops,
                                      SelectionDAG &DAG) const override;

    unsigned
    getInlineAsmMemConstraint(StringRef ConstraintCode) const override {
      if (ConstraintCode == "o")
        return InlineAsm::Constraint_o;
      return TargetLowering::getInlineAsmMemConstraint(ConstraintCode);
    }

    std::pair<unsigned, const TargetRegisterClass *>
    getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                                 StringRef Constraint, MVT VT) const override;

    bool isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const override;
    MVT getScalarShiftAmountTy(const DataLayout &, EVT) const override {
      return MVT::i32;
    }

    unsigned getRegisterByName(const char* RegName, EVT VT,
                               SelectionDAG &DAG) const override;

    /// If a physical register, this returns the register that receives the
    /// exception address on entry to an EH pad.
    unsigned
    getExceptionPointerRegister(const Constant *PersonalityFn) const override {
      return SP::I0;
    }

    /// If a physical register, this returns the register that receives the
    /// exception typeid on entry to a landing pad.
    unsigned
    getExceptionSelectorRegister(const Constant *PersonalityFn) const override {
      return SP::I1;
    }

    SDValue getThreadPointerRegister(SelectionDAG &DAG) const;

    /// Return if the target supports combining a
    /// chain like:
    /// \code
    ///   %andResult = and %val1, #mask
    ///   %icmpResult = icmp %andResult, 0
    /// \endcode
    /// into a single machine instruction of a form like:
    /// \code
    ///   cc = test %register, #mask
    /// \endcode
    bool
    isMaskAndCmp0FoldingBeneficial(const Instruction &AndI) const override {
      return true;
    }

    /// Use bitwise logic to make pairs of compares more efficient. For example:
    /// and (seteq A, B), (seteq C, D) --> seteq (or (xor A, B), (xor C, D)), 0
    /// This should be true when it takes more than one instruction to lower
    /// setcc (cmp+set on x86 scalar), when bitwise ops are faster than logic on
    /// condition bits (crand on PowerPC), and/or when reducing cmp+br is a win.
    bool convertSetCCLogicToBitwiseLogic(EVT VT) const override {
      return true;
    }

    /// Return true if the target should transform:
    /// (X & Y) == Y ---> (~X & Y) == 0
    /// (X & Y) != Y ---> (~X & Y) != 0
    ///
    /// This may be profitable if the target has a bitwise and-not operation
    /// that sets comparison flags. A target may want to limit the
    /// transformation based on the type of Y or if Y is a constant.
    ///
    /// Note that the transform will not occur if Y is known to be a power-of-2
    /// because a mask and compare of a single bit can be handled by inverting
    /// the predicate, for example: (X & 8) == 8 ---> (X & 8) != 0
    bool hasAndNotCompare(SDValue Y) const override { return true; }

    /// Return true if the specified immediate is legal icmp immediate, that is
    /// the target has icmp instructions which can compare a register against
    /// the immediate without having to materialize the immediate into a
    /// register.
    bool isLegalICmpImmediate(int64_t val) const override {
      return isInt<13>(val);
    }

    /// Return true if the specified immediate is legal add immediate, that is
    /// the target has add instructions which can add a register with the
    /// immediate without having to materialize the immediate into a register.
    bool isLegalAddImmediate(int64_t val) const override {
      return isInt<13>(val);
    }

    /// Return true if it is beneficial to convert a load of a constant to
    /// just the constant itself.
    /// On some targets it might be more efficient to use a combination of
    /// arithmetic instructions to materialize the constant instead of loading
    /// it from a constant pool.
    bool shouldConvertConstantLoadToIntImm(const APInt &Imm,
                                           Type *Ty) const override;

    /// Override to support customized stack guard loading.
    bool useLoadStackGuardNode() const override;
    void insertSSPDeclarations(Module &M) const override;

    /// getSetCCResultType - Return the ISD::SETCC ValueType
    EVT getSetCCResultType(const DataLayout &DL, LLVMContext &Context,
                           EVT VT) const override;

    SDValue
    LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                         const SmallVectorImpl<ISD::InputArg> &Ins,
                         const SDLoc &dl, SelectionDAG &DAG,
                         SmallVectorImpl<SDValue> &InVals) const override;
    SDValue LowerFormalArguments_32(SDValue Chain, CallingConv::ID CallConv,
                                    bool isVarArg,
                                    const SmallVectorImpl<ISD::InputArg> &Ins,
                                    const SDLoc &dl, SelectionDAG &DAG,
                                    SmallVectorImpl<SDValue> &InVals) const;
    SDValue LowerFormalArguments_64(SDValue Chain, CallingConv::ID CallConv,
                                    bool isVarArg,
                                    const SmallVectorImpl<ISD::InputArg> &Ins,
                                    const SDLoc &dl, SelectionDAG &DAG,
                                    SmallVectorImpl<SDValue> &InVals) const;

    SDValue
      LowerCall(TargetLowering::CallLoweringInfo &CLI,
                SmallVectorImpl<SDValue> &InVals) const override;
    SDValue LowerCall_32(TargetLowering::CallLoweringInfo &CLI,
                         SmallVectorImpl<SDValue> &InVals) const;
    SDValue LowerCall_64(TargetLowering::CallLoweringInfo &CLI,
                         SmallVectorImpl<SDValue> &InVals) const;

    SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                        const SmallVectorImpl<ISD::OutputArg> &Outs,
                        const SmallVectorImpl<SDValue> &OutVals,
                        const SDLoc &dl, SelectionDAG &DAG) const override;
    SDValue LowerReturn_32(SDValue Chain, CallingConv::ID CallConv,
                           bool IsVarArg,
                           const SmallVectorImpl<ISD::OutputArg> &Outs,
                           const SmallVectorImpl<SDValue> &OutVals,
                           const SDLoc &DL, SelectionDAG &DAG) const;
    SDValue LowerReturn_64(SDValue Chain, CallingConv::ID CallConv,
                           bool IsVarArg,
                           const SmallVectorImpl<ISD::OutputArg> &Outs,
                           const SmallVectorImpl<SDValue> &OutVals,
                           const SDLoc &DL, SelectionDAG &DAG) const;

    SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerGlobalTLSAddress(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerConstantPool(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerBlockAddress(SDValue Op, SelectionDAG &DAG) const;

    SDValue withTargetFlags(SDValue Op, unsigned TF, SelectionDAG &DAG) const;
    SDValue makeHiLoPair(SDValue Op, unsigned HiTF, unsigned LoTF,
                         SelectionDAG &DAG) const;
    SDValue makeAddress(SDValue Op, SelectionDAG &DAG) const;

    SDValue LowerF128_LibCallArg(SDValue Chain, ArgListTy &Args, SDValue Arg,
                                 const SDLoc &DL, SelectionDAG &DAG) const;
    SDValue LowerF128Op(SDValue Op, SelectionDAG &DAG,
                        const char *LibFuncName,
                        unsigned numArgs) const;
    SDValue LowerF128Compare(SDValue LHS, SDValue RHS, unsigned &SPCC,
                             const SDLoc &DL, SelectionDAG &DAG) const;

    SDValue LowerINTRINSIC_WO_CHAIN(SDValue Op, SelectionDAG &DAG) const;

    SDValue PerformBITCASTCombine(SDNode *N, DAGCombinerInfo &DCI) const;

    SDValue bitcastConstantFPToInt(ConstantFPSDNode *C, const SDLoc &DL,
                                   SelectionDAG &DAG) const;

    SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const override;

    bool IsEligibleForTailCallOptimization(CCState &CCInfo,
                                           CallLoweringInfo &CLI,
                                           MachineFunction &MF) const;

    bool ShouldShrinkFPConstant(EVT VT) const override {
      // Do not shrink FP constpool if VT == MVT::f128.
      // (ldd, call _Q_fdtoq) is more expensive than two ldds.
      return VT != MVT::f128;
    }

    bool shouldInsertFencesForAtomic(const Instruction *I) const override {
      // FIXME: We insert fences for each atomics and generate
      // sub-optimal code for PSO/TSO. (Approximately nobody uses any
      // mode but TSO, which makes this even more silly)
      return true;
    }

    AtomicExpansionKind shouldExpandAtomicRMWInIR(AtomicRMWInst *AI) const override;

    void ReplaceNodeResults(SDNode *N,
                            SmallVectorImpl<SDValue>& Results,
                            SelectionDAG &DAG) const override;

    MachineBasicBlock *expandSelectCC(MachineInstr &MI, MachineBasicBlock *BB,
                                      unsigned BROpcode) const;
  };
} // end namespace llvm

#endif    // SPARC_ISELLOWERING_H
