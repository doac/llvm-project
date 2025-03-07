//===-- SparcInstrInfo.cpp - Sparc Instruction Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Sparc implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "SparcInstrInfo.h"
#include "Sparc.h"
#include "SparcMachineFunctionInfo.h"
#include "SparcSubtarget.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"
#include "MCTargetDesc/SparcMCExpr.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "SparcGenInstrInfo.inc"

// Pin the vtable to this file.
void SparcInstrInfo::anchor() {}

SparcInstrInfo::SparcInstrInfo(SparcSubtarget &ST)
    : SparcGenInstrInfo(SP::ADJCALLSTACKDOWN, SP::ADJCALLSTACKUP), RI(),
      Subtarget(ST) {}

/// isLoadFromStackSlot - If the specified machine instruction is a direct
/// load from a stack slot, return the virtual or physical register number of
/// the destination along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than loading from the stack slot.
unsigned SparcInstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                             int &FrameIndex) const {
  if (MI.getOpcode() == SP::LDri || MI.getOpcode() == SP::LDXri ||
      MI.getOpcode() == SP::LDFri || MI.getOpcode() == SP::LDDFri ||
      MI.getOpcode() == SP::LDQFri || MI.getOpcode() == SP::RLDri ||
      MI.getOpcode() == SP::RLDFri) {
    if (MI.getOperand(1).isFI() && MI.getOperand(2).isImm() &&
        MI.getOperand(2).getImm() == 0) {
      FrameIndex = MI.getOperand(1).getIndex();
      return MI.getOperand(0).getReg();
    }
  }
  return 0;
}

/// isStoreToStackSlot - If the specified machine instruction is a direct
/// store to a stack slot, return the virtual or physical register number of
/// the source reg along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than storing to the stack slot.
unsigned SparcInstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                            int &FrameIndex) const {
  if (MI.getOpcode() == SP::STri || MI.getOpcode() == SP::STXri ||
      MI.getOpcode() == SP::STFri || MI.getOpcode() == SP::STDFri ||
      MI.getOpcode() == SP::STQFri || MI.getOpcode() == SP::RSTri ||
      MI.getOpcode() == SP::RSTFri) {
    if (MI.getOperand(0).isFI() && MI.getOperand(1).isImm() &&
        MI.getOperand(1).getImm() == 0) {
      FrameIndex = MI.getOperand(0).getIndex();
      return MI.getOperand(2).getReg();
    }
  }
  return 0;
}

static bool IsIntegerCC(unsigned CC)
{
  return  (CC <= SPCC::ICC_VC);
}

static SPCC::CondCodes GetOppositeBranchCondition(SPCC::CondCodes CC)
{
  switch(CC) {
  case SPCC::ICC_A:    return SPCC::ICC_N;
  case SPCC::ICC_N:    return SPCC::ICC_A;
  case SPCC::ICC_NE:   return SPCC::ICC_E;
  case SPCC::ICC_E:    return SPCC::ICC_NE;
  case SPCC::ICC_G:    return SPCC::ICC_LE;
  case SPCC::ICC_LE:   return SPCC::ICC_G;
  case SPCC::ICC_GE:   return SPCC::ICC_L;
  case SPCC::ICC_L:    return SPCC::ICC_GE;
  case SPCC::ICC_GU:   return SPCC::ICC_LEU;
  case SPCC::ICC_LEU:  return SPCC::ICC_GU;
  case SPCC::ICC_CC:   return SPCC::ICC_CS;
  case SPCC::ICC_CS:   return SPCC::ICC_CC;
  case SPCC::ICC_POS:  return SPCC::ICC_NEG;
  case SPCC::ICC_NEG:  return SPCC::ICC_POS;
  case SPCC::ICC_VC:   return SPCC::ICC_VS;
  case SPCC::ICC_VS:   return SPCC::ICC_VC;

  case SPCC::FCC_A:    return SPCC::FCC_N;
  case SPCC::FCC_N:    return SPCC::FCC_A;
  case SPCC::FCC_U:    return SPCC::FCC_O;
  case SPCC::FCC_O:    return SPCC::FCC_U;
  case SPCC::FCC_G:    return SPCC::FCC_ULE;
  case SPCC::FCC_LE:   return SPCC::FCC_UG;
  case SPCC::FCC_UG:   return SPCC::FCC_LE;
  case SPCC::FCC_ULE:  return SPCC::FCC_G;
  case SPCC::FCC_L:    return SPCC::FCC_UGE;
  case SPCC::FCC_GE:   return SPCC::FCC_UL;
  case SPCC::FCC_UL:   return SPCC::FCC_GE;
  case SPCC::FCC_UGE:  return SPCC::FCC_L;
  case SPCC::FCC_LG:   return SPCC::FCC_UE;
  case SPCC::FCC_UE:   return SPCC::FCC_LG;
  case SPCC::FCC_NE:   return SPCC::FCC_E;
  case SPCC::FCC_E:    return SPCC::FCC_NE;

  case SPCC::CPCC_A:   return SPCC::CPCC_N;
  case SPCC::CPCC_N:   return SPCC::CPCC_A;
  case SPCC::CPCC_3:   LLVM_FALLTHROUGH;
  case SPCC::CPCC_2:   LLVM_FALLTHROUGH;
  case SPCC::CPCC_23:  LLVM_FALLTHROUGH;
  case SPCC::CPCC_1:   LLVM_FALLTHROUGH;
  case SPCC::CPCC_13:  LLVM_FALLTHROUGH;
  case SPCC::CPCC_12:  LLVM_FALLTHROUGH;
  case SPCC::CPCC_123: LLVM_FALLTHROUGH;
  case SPCC::CPCC_0:   LLVM_FALLTHROUGH;
  case SPCC::CPCC_03:  LLVM_FALLTHROUGH;
  case SPCC::CPCC_02:  LLVM_FALLTHROUGH;
  case SPCC::CPCC_023: LLVM_FALLTHROUGH;
  case SPCC::CPCC_01:  LLVM_FALLTHROUGH;
  case SPCC::CPCC_013: LLVM_FALLTHROUGH;
  case SPCC::CPCC_012:
      // "Opposite" code is not meaningful, as we don't know
      // what the CoProc condition means here. The cond-code will
      // only be used in inline assembler, so this code should
      // not be reached in a normal compilation pass.
      llvm_unreachable("Meaningless inversion of co-processor cond code");
  }
  llvm_unreachable("Invalid cond code");
}

static bool isUncondBranchOpcode(int Opc) {
  return Opc == SP::BA || Opc == SP::RBA;
}

static bool isCondBranchOpcode(int Opc) {
  return Opc == SP::FBCOND || Opc == SP::BCOND || Opc == SP::RBCOND ||
         Opc == SP::RFBCOND;
}

static bool isIndirectBranchOpcode(int Opc) {
  return Opc == SP::BINDrr || Opc == SP::BINDri;
}

static void parseCondBranch(MachineInstr *LastInst, MachineBasicBlock *&Target,
                            SmallVectorImpl<MachineOperand> &Cond) {
  Cond.push_back(MachineOperand::CreateImm(LastInst->getOperand(1).getImm()));
  Target = LastInst->getOperand(0).getMBB();
}

bool SparcInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                   MachineBasicBlock *&TBB,
                                   MachineBasicBlock *&FBB,
                                   SmallVectorImpl<MachineOperand> &Cond,
                                   bool AllowModify) const {
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  if (I == MBB.end())
    return false;

  if (!isUnpredicatedTerminator(*I))
    return false;

  // Get the last instruction in the block.
  MachineInstr *LastInst = &*I;
  unsigned LastOpc = LastInst->getOpcode();

  // If there is only one terminator instruction, process it.
  if (I == MBB.begin() || !isUnpredicatedTerminator(*--I)) {
    if (isUncondBranchOpcode(LastOpc)) {
      TBB = LastInst->getOperand(0).getMBB();
      return false;
    }
    if (isCondBranchOpcode(LastOpc)) {
      // Block ends with fall-through condbranch.
      parseCondBranch(LastInst, TBB, Cond);
      return false;
    }
    return true; // Can't handle indirect branch.
  }

  // Get the instruction before it if it is a terminator.
  MachineInstr *SecondLastInst = &*I;
  unsigned SecondLastOpc = SecondLastInst->getOpcode();

  // If AllowModify is true and the block ends with two or more unconditional
  // branches, delete all but the first unconditional branch.
  if (AllowModify && isUncondBranchOpcode(LastOpc)) {
    while (isUncondBranchOpcode(SecondLastOpc)) {
      LastInst->eraseFromParent();
      LastInst = SecondLastInst;
      LastOpc = LastInst->getOpcode();
      if (I == MBB.begin() || !isUnpredicatedTerminator(*--I)) {
        // Return now the only terminator is an unconditional branch.
        TBB = LastInst->getOperand(0).getMBB();
        return false;
      } else {
        SecondLastInst = &*I;
        SecondLastOpc = SecondLastInst->getOpcode();
      }
    }
  }

  // If there are three terminators, we don't know what sort of block this is.
  if (SecondLastInst && I != MBB.begin() && isUnpredicatedTerminator(*--I))
    return true;

  // If the block ends with a B and a Bcc, handle it.
  if (isCondBranchOpcode(SecondLastOpc) && isUncondBranchOpcode(LastOpc)) {
    parseCondBranch(SecondLastInst, TBB, Cond);
    FBB = LastInst->getOperand(0).getMBB();
    return false;
  }

  // If the block ends with two unconditional branches, handle it.  The second
  // one is not executed.
  if (isUncondBranchOpcode(SecondLastOpc) && isUncondBranchOpcode(LastOpc)) {
    TBB = SecondLastInst->getOperand(0).getMBB();
    return false;
  }

  // ...likewise if it ends with an indirect branch followed by an unconditional
  // branch.
  if (isIndirectBranchOpcode(SecondLastOpc) && isUncondBranchOpcode(LastOpc)) {
    I = LastInst;
    if (AllowModify)
      I->eraseFromParent();
    return true;
  }

  // Otherwise, can't handle this.
  return true;
}

unsigned SparcInstrInfo::insertBranch(MachineBasicBlock &MBB,
                                      MachineBasicBlock *TBB,
                                      MachineBasicBlock *FBB,
                                      ArrayRef<MachineOperand> Cond,
                                      const DebugLoc &DL,
                                      int *BytesAdded) const {
  assert(TBB && "insertBranch must not be told to insert a fallthrough");
  assert((Cond.size() == 1 || Cond.size() == 0) &&
         "Sparc branch conditions should have one component!");
  assert(!BytesAdded && "code size not handled");

  unsigned UncondBranchOpcode = Subtarget.isREX() ? SP::RBA : SP::BA;
  unsigned CondBranchOpcode = Subtarget.isREX() ? SP::RBCOND : SP::BCOND;
  unsigned CondFBranchOpcode = Subtarget.isREX() ? SP::RFBCOND : SP::FBCOND;

  if (Cond.empty()) {
    assert(!FBB && "Unconditional branch with multiple successors!");
    BuildMI(&MBB, DL, get(UncondBranchOpcode)).addMBB(TBB);
    return 1;
  }

  // Conditional branch
  unsigned CC = Cond[0].getImm();

  if (IsIntegerCC(CC))
    BuildMI(&MBB, DL, get(CondBranchOpcode)).addMBB(TBB).addImm(CC);
  else
    BuildMI(&MBB, DL, get(CondFBranchOpcode)).addMBB(TBB).addImm(CC);
  if (!FBB)
    return 1;

  BuildMI(&MBB, DL, get(UncondBranchOpcode)).addMBB(FBB);
  return 2;
}

unsigned SparcInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                      int *BytesRemoved) const {
  assert(!BytesRemoved && "code size not handled");

  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count = 0;
  while (I != MBB.begin()) {
    --I;

    if (I->isDebugInstr())
      continue;

    if (I->getOpcode() != SP::BA && I->getOpcode() != SP::BCOND &&
        I->getOpcode() != SP::FBCOND && I->getOpcode() != SP::RBA &&
        I->getOpcode() != SP::RBCOND && I->getOpcode() != SP::RFBCOND)
      break; // Not a branch

    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }
  return Count;
}

bool SparcInstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  assert(Cond.size() == 1);
  SPCC::CondCodes CC = static_cast<SPCC::CondCodes>(Cond[0].getImm());
  Cond[0].setImm(GetOppositeBranchCondition(CC));
  return false;
}

void SparcInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator I,
                                 const DebugLoc &DL, unsigned DestReg,
                                 unsigned SrcReg, bool KillSrc) const {
  unsigned numSubRegs = 0;
  unsigned movOpc     = 0;
  const unsigned *subRegIdx = nullptr;
  bool ExtraG0 = false;

  const unsigned DW_SubRegsIdx[]  = { SP::sub_even, SP::sub_odd };
  const unsigned DFP_FP_SubRegsIdx[]  = { SP::sub_even, SP::sub_odd };
  const unsigned QFP_DFP_SubRegsIdx[] = { SP::sub_even64, SP::sub_odd64 };
  const unsigned QFP_FP_SubRegsIdx[]  = { SP::sub_even, SP::sub_odd,
                                          SP::sub_odd64_then_sub_even,
                                          SP::sub_odd64_then_sub_odd };

  if (SP::IntRegsRegClass.contains(DestReg, SrcReg)) {
    if (Subtarget.isREX())
      BuildMI(MBB, I, DL, get(SP::RMOV), DestReg)
          .addReg(SrcReg, getKillRegState(KillSrc));
    else
      BuildMI(MBB, I, DL, get(SP::ORrr), DestReg)
          .addReg(SP::G0)
          .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (SP::IntPairRegClass.contains(DestReg, SrcReg)) {
    subRegIdx  = DW_SubRegsIdx;
    numSubRegs = 2;
    movOpc     = SP::ORrr;
    ExtraG0 = true;
  } else if (SP::FPRegsRegClass.contains(DestReg, SrcReg))
    BuildMI(MBB, I, DL, get(SP::FMOVS), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc));
  else if (SP::DFPRegsRegClass.contains(DestReg, SrcReg)) {
    if (Subtarget.isV9()) {
      BuildMI(MBB, I, DL, get(SP::FMOVD), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
    } else {
      // Use two FMOVS instructions.
      subRegIdx  = DFP_FP_SubRegsIdx;
      numSubRegs = 2;
      movOpc     = SP::FMOVS;
    }
  } else if (SP::QFPRegsRegClass.contains(DestReg, SrcReg)) {
    if (Subtarget.isV9()) {
      if (Subtarget.hasHardQuad()) {
        BuildMI(MBB, I, DL, get(SP::FMOVQ), DestReg)
          .addReg(SrcReg, getKillRegState(KillSrc));
      } else {
        // Use two FMOVD instructions.
        subRegIdx  = QFP_DFP_SubRegsIdx;
        numSubRegs = 2;
        movOpc     = SP::FMOVD;
      }
    } else {
      // Use four FMOVS instructions.
      subRegIdx  = QFP_FP_SubRegsIdx;
      numSubRegs = 4;
      movOpc     = SP::FMOVS;
    }
  } else if (SP::ASRRegsRegClass.contains(DestReg) &&
             SP::IntRegsRegClass.contains(SrcReg)) {
    BuildMI(MBB, I, DL, get(SP::WRASRrr), DestReg)
        .addReg(SP::G0)
        .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (SP::IntRegsRegClass.contains(DestReg) &&
             SP::ASRRegsRegClass.contains(SrcReg)) {
    BuildMI(MBB, I, DL, get(SP::RDASR), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  } else
    llvm_unreachable("Impossible reg-to-reg copy");

  if (numSubRegs == 0 || subRegIdx == nullptr || movOpc == 0)
    return;

  const TargetRegisterInfo *TRI = &getRegisterInfo();
  MachineInstr *MovMI = nullptr;

  for (unsigned i = 0; i != numSubRegs; ++i) {
    unsigned Dst = TRI->getSubReg(DestReg, subRegIdx[i]);
    unsigned Src = TRI->getSubReg(SrcReg,  subRegIdx[i]);
    assert(Dst && Src && "Bad sub-register");

    MachineInstrBuilder MIB = BuildMI(MBB, I, DL, get(movOpc), Dst);
    if (ExtraG0)
      MIB.addReg(SP::G0);
    MIB.addReg(Src);
    MovMI = MIB.getInstr();
  }
  // Add implicit super-register defs and kills to the last MovMI.
  MovMI->addRegisterDefined(DestReg, TRI);
  if (KillSrc)
    MovMI->addRegisterKilled(SrcReg, TRI);
}

void SparcInstrInfo::
storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                    unsigned SrcReg, bool isKill, int FI,
                    const TargetRegisterClass *RC,
                    const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();

  MachineFunction *MF = MBB.getParent();
  const MachineFrameInfo &MFI = MF->getFrameInfo();
  MachineMemOperand *MMO = MF->getMachineMemOperand(
      MachinePointerInfo::getFixedStack(*MF, FI), MachineMemOperand::MOStore,
      MFI.getObjectSize(FI), MFI.getObjectAlignment(FI));

  if (Subtarget.isREX()) {
    if (SP::RexIntRegsRegClass.hasSubClassEq(RC)) {
      BuildMI(MBB, I, DL, get(SP::RSTri))
          .addFrameIndex(FI)
          .addImm(0)
          .addReg(SrcReg, getKillRegState(isKill))
          .addMemOperand(MMO);
      return;
    }

    if (RC == &SP::RexFPRegsRegClass) {
      BuildMI(MBB, I, DL, get(SP::RSTFri))
          .addFrameIndex(FI)
          .addImm(0)
          .addReg(SrcReg, getKillRegState(isKill))
          .addMemOperand(MMO);
      return;
    }
  }
  // On the order of operands here: think "[FrameIdx + 0] = SrcReg".
  if (RC == &SP::I64RegsRegClass)
    BuildMI(MBB, I, DL, get(SP::STXri)).addFrameIndex(FI).addImm(0)
      .addReg(SrcReg, getKillRegState(isKill)).addMemOperand(MMO);
  else if (RC == &SP::IntRegsRegClass || RC == &SP::LeafRegsRegClass)
    BuildMI(MBB, I, DL, get(SP::STri)).addFrameIndex(FI).addImm(0)
      .addReg(SrcReg, getKillRegState(isKill)).addMemOperand(MMO);
  else if (SP::IntPairRegClass.hasSubClassEq(RC))
    BuildMI(MBB, I, DL, get(SP::STDri)).addFrameIndex(FI).addImm(0)
      .addReg(SrcReg, getKillRegState(isKill)).addMemOperand(MMO);
  else if (RC == &SP::FPRegsRegClass)
    BuildMI(MBB, I, DL, get(SP::STFri)).addFrameIndex(FI).addImm(0)
      .addReg(SrcReg,  getKillRegState(isKill)).addMemOperand(MMO);
  else if (SP::DFPRegsRegClass.hasSubClassEq(RC))
    BuildMI(MBB, I, DL, get(SP::STDFri)).addFrameIndex(FI).addImm(0)
      .addReg(SrcReg,  getKillRegState(isKill)).addMemOperand(MMO);
  else if (SP::QFPRegsRegClass.hasSubClassEq(RC))
    // Use STQFri irrespective of its legality. If STQ is not legal, it will be
    // lowered into two STDs in eliminateFrameIndex.
    BuildMI(MBB, I, DL, get(SP::STQFri)).addFrameIndex(FI).addImm(0)
      .addReg(SrcReg,  getKillRegState(isKill)).addMemOperand(MMO);
  else
    llvm_unreachable("Can't store this register to stack slot");
}

void SparcInstrInfo::
loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                     unsigned DestReg, int FI,
                     const TargetRegisterClass *RC,
                     const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();

  MachineFunction *MF = MBB.getParent();
  const MachineFrameInfo &MFI = MF->getFrameInfo();
  MachineMemOperand *MMO = MF->getMachineMemOperand(
      MachinePointerInfo::getFixedStack(*MF, FI), MachineMemOperand::MOLoad,
      MFI.getObjectSize(FI), MFI.getObjectAlignment(FI));

  if (Subtarget.isREX()) {

    if (SP::RexIntRegsRegClass.hasSubClassEq(RC)) {
      BuildMI(MBB, I, DL, get(SP::RLDri), DestReg)
          .addFrameIndex(FI)
          .addImm(0)
          .addMemOperand(MMO);
      return;
    }

    if (RC == &SP::RexFPRegsRegClass) {
      BuildMI(MBB, I, DL, get(SP::RLDFri), DestReg)
          .addFrameIndex(FI)
          .addImm(0)
          .addMemOperand(MMO);
      return;
    }
  }

  if (RC == &SP::I64RegsRegClass)
    BuildMI(MBB, I, DL, get(SP::LDXri), DestReg).addFrameIndex(FI).addImm(0)
      .addMemOperand(MMO);
  else if (RC == &SP::IntRegsRegClass || RC == &SP::LeafRegsRegClass)
    BuildMI(MBB, I, DL, get(SP::LDri), DestReg).addFrameIndex(FI).addImm(0)
      .addMemOperand(MMO);
  else if (SP::IntPairRegClass.hasSubClassEq(RC))
    BuildMI(MBB, I, DL, get(SP::LDDri), DestReg).addFrameIndex(FI).addImm(0)
      .addMemOperand(MMO);
  else if (RC == &SP::FPRegsRegClass)
    BuildMI(MBB, I, DL, get(SP::LDFri), DestReg).addFrameIndex(FI).addImm(0)
      .addMemOperand(MMO);
  else if (SP::DFPRegsRegClass.hasSubClassEq(RC))
    BuildMI(MBB, I, DL, get(SP::LDDFri), DestReg).addFrameIndex(FI).addImm(0)
      .addMemOperand(MMO);
  else if (SP::QFPRegsRegClass.hasSubClassEq(RC))
    // Use LDQFri irrespective of its legality. If LDQ is not legal, it will be
    // lowered into two LDDs in eliminateFrameIndex.
    BuildMI(MBB, I, DL, get(SP::LDQFri), DestReg).addFrameIndex(FI).addImm(0)
      .addMemOperand(MMO);
  else
    llvm_unreachable("Can't load this register from stack slot");
}

unsigned SparcInstrInfo::getGlobalBaseReg(MachineFunction *MF) const
{
  SparcMachineFunctionInfo *SparcFI = MF->getInfo<SparcMachineFunctionInfo>();
  unsigned GlobalBaseReg = SparcFI->getGlobalBaseReg();
  if (GlobalBaseReg != 0)
    return GlobalBaseReg;

  // Insert the set of GlobalBaseReg into the first MBB of the function
  MachineBasicBlock &FirstMBB = MF->front();
  MachineBasicBlock::iterator MBBI = FirstMBB.begin();
  MachineRegisterInfo &RegInfo = MF->getRegInfo();

  const TargetRegisterClass *PtrRC =
    Subtarget.is64Bit() ? &SP::I64RegsRegClass : &SP::IntRegsRegClass;
  if (Subtarget.isREX())
    PtrRC =  &SP::RexIntRegsRegClass;
  GlobalBaseReg = RegInfo.createVirtualRegister(PtrRC);

  DebugLoc dl;

  if (Subtarget.isREX())
    BuildMI(FirstMBB, MBBI, dl, get(SP::RSET32PC), GlobalBaseReg)
      .addExternalSymbol("_GLOBAL_OFFSET_TABLE_", SparcMCExpr::VK_Sparc_R_DISP32);
  else
    BuildMI(FirstMBB, MBBI, dl, get(SP::GETPCX), GlobalBaseReg);
  SparcFI->setGlobalBaseReg(GlobalBaseReg);
  return GlobalBaseReg;
}

bool SparcInstrInfo::isSchedulingBoundary(const MachineInstr &MI,
                                          const MachineBasicBlock *MBB,
                                          const MachineFunction &MF) const {

  if (MI.getOpcode() == SP::RESTORErr || MI.getOpcode() == SP::RESTOREri)
    return true;

  const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();
  if (MI.modifiesRegister(SP::O7, TRI))
    return true;

  return TargetInstrInfo::isSchedulingBoundary(MI, MBB, MF);
}

bool SparcInstrInfo::analyzeCompare(const MachineInstr &MI, unsigned &SrcReg,
                                    unsigned &SrcReg2, int &CmpMask,
                                    int &CmpValue) const {
  switch (MI.getOpcode()) {
  default:
    break;
  case SP::CMPri:
    SrcReg = MI.getOperand(0).getReg();
    SrcReg2 = 0;
    CmpMask = ~0;
    CmpValue = MI.getOperand(1).getImm();
    return (CmpValue == 0);
  case SP::CMPrr:
    SrcReg = MI.getOperand(0).getReg();
    SrcReg2 = MI.getOperand(1).getReg();
    CmpMask = ~0;
    CmpValue = 0;
    return SrcReg2 == SP::G0;
  }

  return false;
}

bool SparcInstrInfo::optimizeCompareInstr(
    MachineInstr &CmpInstr, unsigned SrcReg, unsigned SrcReg2, int CmpMask,
    int CmpValue, const MachineRegisterInfo *MRI) const {

  // Get the unique definition of SrcReg.
  MachineInstr *MI = MRI->getUniqueVRegDef(SrcReg);
  if (!MI)
    return false;

  // Only optimize if defining and comparing instruction in same block.
  if (MI->getParent() != CmpInstr.getParent())
    return false;

  unsigned newOpcode;
  switch(MI->getOpcode()) {
  case SP::ANDNrr:  newOpcode = SP::ANDNCCrr; break;
  case SP::ANDNri:  newOpcode = SP::ANDNCCri; break;
  case SP::ANDrr:   newOpcode = SP::ANDCCrr;  break;
  case SP::ANDri:   newOpcode = SP::ANDCCri;  break;
  case SP::ORrr:    newOpcode = SP::ORCCrr;   break;
  case SP::ORri:    newOpcode = SP::ORCCri;   break;
  case SP::ORNCCrr: newOpcode = SP::ORNCCrr;  break;
  case SP::ORNri:   newOpcode = SP::ORNCCri;  break;
  case SP::XORrr:   newOpcode = SP::XORCCrr;  break;
  case SP::XNORri:  newOpcode = SP::XNORCCri; break;
  case SP::XNORrr:  newOpcode = SP::XNORCCrr; break;
  case SP::ADDrr:   newOpcode = SP::ADDCCrr;  break;
  case SP::ADDri:   newOpcode = SP::ADDCCri;  break;
  case SP::SUBrr:   newOpcode = SP::SUBCCrr;  break;
  case SP::SUBri:   newOpcode = SP::SUBCCri;  break;
  default:
    return false;
  }

  bool isSafe = false;
  bool isRegUsed = false;
  MachineBasicBlock::iterator I = MI;
  MachineBasicBlock::iterator C = CmpInstr;
  MachineBasicBlock::iterator E = CmpInstr.getParent()->end();
  const TargetRegisterInfo *TRI = &getRegisterInfo();

  // If ICC is used or modified between MI and CmpInstr we cannot optimize.
  while (++I != C) {
    if (I->modifiesRegister(SP::ICC, TRI) || I->readsRegister(SP::ICC, TRI))
      return false;
    if (I->readsRegister(SrcReg, TRI))
      isRegUsed = true;
  }

  while (++I != E) {
    // Only allow branching on equality.
    if (I->readsRegister(SP::ICC, TRI)) {
      if ((I->getOpcode() != SP::BCOND) ||
          (I->getOperand(1).getImm() != SPCC::ICC_E &&
           I->getOperand(1).getImm() != SPCC::ICC_NE))
        return false;
    } else if (I->modifiesRegister(SP::ICC, TRI)) {
      isSafe = true;
      break;
    }
  }

  if (!isSafe) {
    MachineBasicBlock *MBB = CmpInstr.getParent();
    for (MachineBasicBlock::succ_iterator SI = MBB->succ_begin(),
                                          SE = MBB->succ_end();
         SI != SE; ++SI)
      if ((*SI)->isLiveIn(SP::ICC))
        return false;
  }

  // If the result is not needed use the %g0 register.
  if (!isRegUsed && CmpInstr.getOperand(0).isKill())
    MI->getOperand(0).setReg(SP::G0);

  MI->setDesc(get(newOpcode));
  MI->addRegisterDefined(SP::ICC);
  CmpInstr.eraseFromParent();

  return true;
}

static bool isLoadRIInstruction(unsigned Opcode)
{
  switch (Opcode) {
  default:
    return false;
  case SP::LDSBri:
  case SP::LDSHri:
  case SP::LDUBri:
  case SP::LDUHri:
  case SP::LDri:
  case SP::LDDri:
  case SP::LDFri:
  case SP::LDDFri:
    return true;
  }
}

bool SparcInstrInfo::areLoadsFromSameBasePtr(SDNode *Load1, SDNode *Load2,
                                             int64_t &Offset1,
                                             int64_t &Offset2) const {
  if (!Load1->isMachineOpcode() || !Load2->isMachineOpcode())
    return false;

  if (!isLoadRIInstruction(Load1->getMachineOpcode()) ||
      !isLoadRIInstruction(Load2->getMachineOpcode()))
    return false;

  // Check if base addresses and chain operands match.
  if (Load1->getOperand(0) != Load2->getOperand(0) ||
      Load1->getOperand(2) != Load2->getOperand(2))
    return false;

  // Determine the offsets.
  if (isa<ConstantSDNode>(Load1->getOperand(1)) &&
      isa<ConstantSDNode>(Load2->getOperand(1))) {
    Offset1 = cast<ConstantSDNode>(Load1->getOperand(1))->getSExtValue();
    Offset2 = cast<ConstantSDNode>(Load2->getOperand(1))->getSExtValue();
    return true;
  }

  return false;
}

bool SparcInstrInfo::shouldScheduleLoadsNear(SDNode *Load1, SDNode *Load2,
                                             int64_t Offset1, int64_t Offset2,
                                             unsigned NumLoads) const {
  if (Offset2 - Offset1 > 32)
    return false;
  return true;
}

bool SparcInstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  case TargetOpcode::LOAD_STACK_GUARD: {
    assert(Subtarget.isTargetLinux() &&
           "Only Linux target is expected to contain LOAD_STACK_GUARD");
    if (!Subtarget.reserveRegG7())
      report_fatal_error("Stack protector requires register %g7");
    // offsetof(tcbhead_t, stack_guard) from sysdeps/sparc/nptl/tls.h in glibc.
    const int64_t Offset = Subtarget.is64Bit() ? 0x28 : 0x14;
    MI.setDesc(get(Subtarget.is64Bit() ? SP::LDXri : SP::LDri));
    MachineInstrBuilder(*MI.getParent()->getParent(), MI)
        .addReg(SP::G7)
        .addImm(Offset);
    return true;
  }
  }

  if (!Subtarget.isREX())
    return false;

  unsigned Reg;
  int32_t Imm;

  switch (MI.getOpcode()) {
  default:
    return false;
  case SP::RLDri:
    Reg = MI.getOperand(1).getReg();
    Imm = MI.getOperand(2).getImm();
    if ((Imm & ~0x7c) == 0 && (Reg == SP::O0 || Reg == SP::I0))
      MI.setDesc(get(SP::RLDfi));
    else
      MI.setDesc(get(SP::LDri));
    break;
  case SP::RSTri:
    Reg = MI.getOperand(0).getReg();
    Imm = MI.getOperand(1).getImm();
    if ((Imm & ~0x7c) == 0 && (Reg == SP::O0 || Reg == SP::I0))
      MI.setDesc(get(SP::RSTfi));
    else
      MI.setDesc(get(SP::STri));
    break;
  case SP::RLDFri:
    Reg = MI.getOperand(1).getReg();
    Imm = MI.getOperand(2).getImm();
    if ((Imm & ~0x7c) == 0 && (Reg == SP::I0))
      MI.setDesc(get(SP::RLDFfi));
    else
      MI.setDesc(get(SP::LDFri));
    break;
  case SP::RSTFri:
    Reg = MI.getOperand(0).getReg();
    Imm = MI.getOperand(1).getImm();
    if ((Imm & ~0x7c) == 0 && (Reg == SP::I0))
      MI.setDesc(get(SP::RSTFfi));
    else
      MI.setDesc(get(SP::STFri));
    break;
  }

  return true;
}
