//===-- SparcRegisterInfo.cpp - SPARC Register Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the SPARC implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "SparcRegisterInfo.h"
#include "Sparc.h"
#include "SparcMachineFunctionInfo.h"
#include "SparcSubtarget.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "SparcGenRegisterInfo.inc"

SparcRegisterInfo::SparcRegisterInfo() : SparcGenRegisterInfo(SP::O7) {}

const MCPhysReg*
SparcRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  const SparcSubtarget &Subtarget = MF->getSubtarget<SparcSubtarget>();
  if (Subtarget.useFlatRegisterMode())
    return CSR_Flat_SaveList;
  else
    return CSR_SaveList;
}

const uint32_t *
SparcRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                        CallingConv::ID CC) const {
  return CSR_RegMask;
}

const uint32_t*
SparcRegisterInfo::getRTCallPreservedMask(CallingConv::ID CC) const {
  return RTCSR_RegMask;
}

bool SparcRegisterInfo::isConstantPhysReg(unsigned PhysReg) const {
  return PhysReg == SP::G0;
}

BitVector SparcRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const SparcSubtarget &Subtarget = MF.getSubtarget<SparcSubtarget>();
  const SparcFrameLowering *TFI = getFrameLowering(MF);

  if (Subtarget.useFlatRegisterMode() && TFI->hasFP(MF))
    markSuperRegs(Reserved, SP::L7);

  if (Subtarget.isREX())
    markSuperRegs(Reserved, SP::L0);

  if (Subtarget.reserveRegG2())
    markSuperRegs(Reserved, SP::G2);
  if (Subtarget.reserveRegG3())
    markSuperRegs(Reserved, SP::G3);
  if (Subtarget.reserveRegG4())
    markSuperRegs(Reserved, SP::G4);
  if (Subtarget.reserveRegG5())
    markSuperRegs(Reserved, SP::G5);
  if (Subtarget.reserveRegG6())
    markSuperRegs(Reserved, SP::G6);
  if (Subtarget.reserveRegG7())
    markSuperRegs(Reserved, SP::G7);

  markSuperRegs(Reserved, SP::O6);
  markSuperRegs(Reserved, SP::I6);
  markSuperRegs(Reserved, SP::I7);
  markSuperRegs(Reserved, SP::G0);
  markSuperRegs(Reserved, SP::O7);

  // Unaliased double registers are not available in non-V9 targets.
  if (!Subtarget.isV9()) {
    for (unsigned n = 0; n != 16; ++n) {
      for (MCRegAliasIterator AI(SP::D16 + n, this, true); AI.isValid(); ++AI)
        Reserved.set(*AI);
    }
  }

  // Reserve ASR1-ASR31
  for (unsigned n = 0; n < 31; n++)
    Reserved.set(SP::ASR1 + n);

  return Reserved;
}

bool SparcRegisterInfo::requiresVirtualBaseRegisters(
    const MachineFunction &MF) const {
  return true;
}

bool SparcRegisterInfo::isFrameOffsetLegal(const MachineInstr *MI,
                                           unsigned BaseReg,
                                           int64_t Offset) const {
  return (Offset >= -4096 && Offset <= 4095);
}

void SparcRegisterInfo::materializeFrameBaseRegister(MachineBasicBlock *MBB,
                                                     unsigned BaseReg,
                                                     int FrameIdx,
                                                     int64_t Offset) const {

  MachineBasicBlock::iterator Ins = MBB->begin();
  DebugLoc DL; // Defaults to "unknown"
  if (Ins != MBB->end())
    DL = Ins->getDebugLoc();

  const MachineFunction &MF = *MBB->getParent();
  const SparcSubtarget &Subtarget = MF.getSubtarget<SparcSubtarget>();
  const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
  const MCInstrDesc &MCID = TII.get(SP::ADDri);
  MachineRegisterInfo &MRI = MBB->getParent()->getRegInfo();
  MRI.constrainRegClass(BaseReg, TII.getRegClass(MCID, 0, this, MF));

  BuildMI(*MBB, Ins, DL, MCID, BaseReg).addFrameIndex(FrameIdx).addImm(Offset);
}
bool SparcRegisterInfo::needsFrameBaseReg(MachineInstr *MI,
                                          int64_t Offset) const {
  return !(Offset >= -4096 && Offset <= 4095);
}

void SparcRegisterInfo::resolveFrameIndex(MachineInstr &MI, unsigned BaseReg,
                                          int64_t Offset) const {
  unsigned FIOperandNum = 0;
  while (!MI.getOperand(FIOperandNum).isFI()) {
    ++FIOperandNum;
    assert(FIOperandNum < MI.getNumOperands() &&
           "Instr doesn't have FrameIndex operand!");
  }

  MI.getOperand(FIOperandNum).ChangeToRegister(BaseReg, false);
  unsigned OffsetOperandNo = FIOperandNum + 1;
  // getOffsetONFromFION(MI, FIOperandNum);
  Offset += MI.getOperand(OffsetOperandNo).getImm();
  MI.getOperand(OffsetOperandNo).ChangeToImmediate(Offset);

  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const SparcSubtarget &Subtarget = MF.getSubtarget<SparcSubtarget>();
  const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
  const MCInstrDesc &MCID = MI.getDesc();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  MRI.constrainRegClass(BaseReg, TII.getRegClass(MCID, FIOperandNum, this, MF));
}

const TargetRegisterClass*
SparcRegisterInfo::getPointerRegClass(const MachineFunction &MF,
                                      unsigned Kind) const {
  const SparcSubtarget &Subtarget = MF.getSubtarget<SparcSubtarget>();
  return Subtarget.is64Bit() ? &SP::I64RegsRegClass : &SP::IntRegsRegClass;
}

static void replaceFIRex(MachineFunction &MF, MachineBasicBlock::iterator II,
                         MachineInstr &MI, DebugLoc dl, unsigned FIOperandNum,
                         int Offset, unsigned FramePtr) {
  const SparcSubtarget &Subtarget = MF.getSubtarget<SparcSubtarget>();
  const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
  SparcMachineFunctionInfo *FuncInfo = MF.getInfo<SparcMachineFunctionInfo>();

  const bool FitsIn16bit = (Offset & ~0x7c) == 0;
  unsigned NewOp;
  bool Is16bit = false;

  switch (MI.getOpcode()) {
  case SP::RLDri:
    NewOp = FitsIn16bit ? SP::RLDfi : SP::LDri;
    Is16bit = FitsIn16bit;
    break;
  case SP::RSTri:
    NewOp = FitsIn16bit ? SP::RSTfi : SP::STri;
    Is16bit = FitsIn16bit;
    break;
  case SP::RLDFri:
    NewOp = FitsIn16bit ? SP::RLDFfi : SP::LDFri;
    Is16bit = FitsIn16bit;
    break;
  case SP::RSTFri:
    NewOp = FitsIn16bit ? SP::RSTFfi : SP::STFri;
    Is16bit = FitsIn16bit;
    break;
  case SP::LEA_ADDri:
    NewOp = SP::ADDri;
    break;
  default:
    NewOp = 0;
  }

  if (NewOp) {
    MI.setDesc(TII.get(NewOp));
  }

  if ((FitsIn16bit && Is16bit) || (isInt<7>(Offset) && !Is16bit)) {
    MI.getOperand(FIOperandNum).ChangeToRegister(FramePtr, false);
    MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
    return;
  }

  unsigned DstReg =
      MF.getRegInfo().createVirtualRegister(&SP::RexIntRegsRegClass);

  if (FuncInfo->isLeafProc())
    MF.getRegInfo().constrainRegClass(DstReg, &SP::LeafRegsRegClass);

  if (isInt<21>(Offset))
    BuildMI(*MI.getParent(), II, dl, TII.get(SP::RSET21), DstReg)
        .addImm(Offset);
  else
    BuildMI(*MI.getParent(), II, dl, TII.get(SP::RSET32), DstReg)
        .addImm(Offset);

  switch (MI.getOpcode()) {
  case SP::STri:
    NewOp = SP::STrr;
    break;
  case SP::LDri:
    NewOp = SP::LDrr;
    break;
  case SP::LDFri:
    NewOp = SP::LDFrr;
    break;
  case SP::STFri:
    NewOp = SP::STFrr;
    break;
  case SP::LDDri:
    NewOp = SP::LDDrr;
    break;
  case SP::STDri:
    NewOp = SP::STDrr;
    break;
  case SP::LDSHri:
    NewOp = SP::LDSHrr;
    break;
  case SP::STDFri:
    NewOp = SP::STDFrr;
    break;
  case SP::LDDFri:
    NewOp = SP::LDDFrr;
    break;
  case SP::ADDri:
    NewOp = SP::ADDrr;
    break;
  case SP::LDUBri:
    NewOp = SP::LDUBrr;
    break;
  case SP::LDUHri:
    NewOp = SP::LDUHrr;
    break;
  case SP::STHri:
    NewOp = SP::STHrr;
    break;
  case SP::LDSBri:
    NewOp = SP::LDSBrr;
    break;
  case SP::STBri:
    NewOp = SP::STBrr;
    break;
  default:
    NewOp = 0;
  }

  if (NewOp) {
    MI.setDesc(TII.get(NewOp));
  }
  MI.getOperand(FIOperandNum).ChangeToRegister(FramePtr, false);

  MI.getOperand(FIOperandNum + 1).ChangeToRegister(DstReg, false, false, true);
  return;
}

static void replaceFI(MachineFunction &MF, MachineBasicBlock::iterator II,
                      MachineInstr &MI, DebugLoc dl, unsigned FIOperandNum,
                      int Offset, unsigned FramePtr) {
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  const SparcSubtarget &Subtarget = MF.getSubtarget<SparcSubtarget>();

  if (Subtarget.isREX()) {
    replaceFIRex(MF, II, MI, dl, FIOperandNum, Offset, FramePtr);
    return;
  }

  // Replace frame index with a frame pointer reference.
  if (Offset >= -4096 && Offset <= 4095) {
    // If the offset is small enough to fit in the immediate field, directly
    // encode it.
    MI.getOperand(FIOperandNum).ChangeToRegister(FramePtr, false);
    MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
    return;
  }

  SparcMachineFunctionInfo *FuncInfo = MF.getInfo<SparcMachineFunctionInfo>();
  const TargetRegisterClass *RC =
      FuncInfo->isLeafProc() ? &SP::LeafRegsRegClass : &SP::IntRegsRegClass;
  unsigned ScratchReg = MF.getRegInfo().createVirtualRegister(RC);

  if (Offset >= 0) {
    // Emit nonnegaive immediates with sethi + or.
    // sethi %hi(Offset), %scratch
    // add %scratch, %fp, %scratch
    // Insert %scratch+%lo(offset) into the user.
    BuildMI(*MI.getParent(), II, dl, TII.get(SP::SETHIi), ScratchReg)
      .addImm(HI22(Offset));

    // Emit scratch = scratch + I6
    BuildMI(*MI.getParent(), II, dl, TII.get(SP::ADDrr), ScratchReg).addReg(ScratchReg)
      .addReg(FramePtr);
    // Insert: scratchreg+%lo(offset) into the user.
    MI.getOperand(FIOperandNum).ChangeToRegister(ScratchReg, false);
    MI.getOperand(FIOperandNum + 1).ChangeToImmediate(LO10(Offset));
    return;
  }

  // Emit Negative numbers with sethi + xor
  // sethi %hix(Offset), %scratch
  // xor  %scratch, %lox(offset), %scratch
  // add %scratch, %fp, %scratch
  // Insert: scratch + 0 into the user.
  BuildMI(*MI.getParent(), II, dl, TII.get(SP::SETHIi), ScratchReg)
    .addImm(HIX22(Offset));
  BuildMI(*MI.getParent(), II, dl, TII.get(SP::XORri), ScratchReg)
    .addReg(ScratchReg).addImm(LOX10(Offset));

  BuildMI(*MI.getParent(), II, dl, TII.get(SP::ADDrr), ScratchReg).addReg(ScratchReg)
    .addReg(FramePtr);
  // Insert: ScratchReg+%lo(offset) into the user.
  MI.getOperand(FIOperandNum).ChangeToRegister(ScratchReg, false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(0);
}

/// saveScavengerRegister - Spill the register so it can be used by the
/// register scavenger.
bool SparcRegisterInfo::saveScavengerRegister(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
    MachineBasicBlock::iterator &UseMI, const TargetRegisterClass *RC,
    unsigned Reg) const {

  const SparcSubtarget &Subtarget =
      MBB.getParent()->getSubtarget<SparcSubtarget>();

  if (!Subtarget.isREX())
    return false;

  const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
  DebugLoc DL;
  BuildMI(MBB, I, DL, TII.get(SP::RMOV))
      .addReg(SP::L0, RegState::Define)
      .addReg(Reg, RegState::Kill);
  BuildMI(MBB, UseMI, DL, TII.get(SP::RMOV))
      .addReg(Reg, RegState::Define)
      .addReg(SP::L0, RegState::Kill);

  return true;
}

void
SparcRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                       int SPAdj, unsigned FIOperandNum,
                                       RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");

  MachineInstr &MI = *II;
  DebugLoc dl = MI.getDebugLoc();
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  MachineFunction &MF = *MI.getParent()->getParent();
  const SparcSubtarget &Subtarget = MF.getSubtarget<SparcSubtarget>();
  const SparcFrameLowering *TFI = getFrameLowering(MF);

  unsigned FrameReg;
  int Offset;
  Offset = TFI->getFrameIndexReference(MF, FrameIndex, FrameReg);

  Offset += MI.getOperand(FIOperandNum + 1).getImm();

  if (!Subtarget.isV9() || !Subtarget.hasHardQuad()) {
    if (MI.getOpcode() == SP::STQFri) {
      const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
      unsigned SrcReg = MI.getOperand(2).getReg();
      unsigned SrcEvenReg = getSubReg(SrcReg, SP::sub_even64);
      unsigned SrcOddReg  = getSubReg(SrcReg, SP::sub_odd64);
      MachineInstr *StMI =
        BuildMI(*MI.getParent(), II, dl, TII.get(SP::STDFri))
        .addReg(FrameReg).addImm(0).addReg(SrcEvenReg);
      replaceFI(MF, II, *StMI, dl, 0, Offset, FrameReg);
      MI.setDesc(TII.get(SP::STDFri));
      MI.getOperand(2).setReg(SrcOddReg);
      Offset += 8;
    } else if (MI.getOpcode() == SP::LDQFri) {
      const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
      unsigned DestReg     = MI.getOperand(0).getReg();
      unsigned DestEvenReg = getSubReg(DestReg, SP::sub_even64);
      unsigned DestOddReg  = getSubReg(DestReg, SP::sub_odd64);
      MachineInstr *StMI =
        BuildMI(*MI.getParent(), II, dl, TII.get(SP::LDDFri), DestEvenReg)
        .addReg(FrameReg).addImm(0);
      replaceFI(MF, II, *StMI, dl, 1, Offset, FrameReg);

      MI.setDesc(TII.get(SP::LDDFri));
      MI.getOperand(0).setReg(DestOddReg);
      Offset += 8;
    }
  }

  replaceFI(MF, II, MI, dl, FIOperandNum, Offset, FrameReg);

}

unsigned SparcRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const SparcSubtarget &Subtarget = MF.getSubtarget<SparcSubtarget>();
  if (Subtarget.useFlatRegisterMode())
    return SP::L7;

  return SP::I6;
}

// Sparc has no architectural need for stack realignment support,
// except that LLVM unfortunately currently implements overaligned
// stack objects by depending upon stack realignment support.
// If that ever changes, this can probably be deleted.
bool SparcRegisterInfo::canRealignStack(const MachineFunction &MF) const {
  if (!TargetRegisterInfo::canRealignStack(MF))
    return false;

  // Sparc always has a fixed frame pointer register, so don't need to
  // worry about needing to reserve it. [even if we don't have a frame
  // pointer for our frame, it still cannot be used for other things,
  // or register window traps will be SADNESS.]

  // If there's a reserved call frame, we can use SP to access locals.
  if (getFrameLowering(MF)->hasReservedCallFrame(MF))
    return true;

  // Otherwise, we'd need a base pointer, but those aren't implemented
  // for SPARC at the moment.

  return false;
}
