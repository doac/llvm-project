//===-- SparcFrameLowering.cpp - Sparc Frame Information ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Sparc implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "SparcFrameLowering.h"
#include "SparcInstrInfo.h"
#include "SparcMachineFunctionInfo.h"
#include "SparcSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/CodeGen/RegisterScavenging.h"

using namespace llvm;

static cl::opt<bool>
DisableLeafProc("disable-sparc-leaf-proc",
                cl::init(false),
                cl::desc("Disable Sparc leaf procedure optimization."),
                cl::Hidden);

SparcFrameLowering::SparcFrameLowering(const SparcSubtarget &ST)
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown,
                          ST.is64Bit() ? 16 : 8, 0, ST.is64Bit() ? 16 : 8) {}

void SparcFrameLowering::emitSPAdjustment(MachineFunction &MF,
                                          MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator MBBI,
                                          int NumBytes,
                                          unsigned ADDrr,
                                          unsigned ADDri) const {

  DebugLoc dl;
  unsigned ScratchReg;
  const SparcInstrInfo &TII =
      *static_cast<const SparcInstrInfo *>(MF.getSubtarget().getInstrInfo());

  if (NumBytes >= -4096 && NumBytes < 4096) {
    BuildMI(MBB, MBBI, dl, TII.get(ADDri), SP::O6)
      .addReg(SP::O6).addImm(NumBytes);
    return;
  }

  ScratchReg = MF.getRegInfo().createVirtualRegister(&SP::LeafRegsRegClass);

  // Emit this the hard way.
  if (NumBytes >= 0) {
    // Emit nonnegative numbers with sethi + or.
    // sethi %hi(NumBytes), %scratch
    // or %scratch, %lo(NumBytes), %scratch
    // add %sp, %scratch, %sp
    BuildMI(MBB, MBBI, dl, TII.get(SP::SETHIi), ScratchReg)
      .addImm(HI22(NumBytes));
    BuildMI(MBB, MBBI, dl, TII.get(SP::ORri), ScratchReg)
      .addReg(ScratchReg).addImm(LO10(NumBytes));
    BuildMI(MBB, MBBI, dl, TII.get(ADDrr), SP::O6)
      .addReg(SP::O6).addReg(ScratchReg);
    return ;
  }

  // Emit negative numbers with sethi + xor.
  // sethi %hix(NumBytes), %scratch
  // xor %scratch, %lox(NumBytes), %scratch
  // add %sp, %scratch, %sp
  BuildMI(MBB, MBBI, dl, TII.get(SP::SETHIi), ScratchReg)
    .addImm(HIX22(NumBytes));
  BuildMI(MBB, MBBI, dl, TII.get(SP::XORri), ScratchReg)
    .addReg(ScratchReg).addImm(LOX10(NumBytes));
  BuildMI(MBB, MBBI, dl, TII.get(ADDrr), SP::O6)
    .addReg(SP::O6).addReg(ScratchReg);
}

void SparcFrameLowering::emitPrologue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  SparcMachineFunctionInfo *FuncInfo = MF.getInfo<SparcMachineFunctionInfo>();

  assert(&MF.front() == &MBB && "Shrink-wrapping not yet supported");
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const SparcSubtarget &Subtarget = MF.getSubtarget<SparcSubtarget>();
  const SparcInstrInfo &TII =
      *static_cast<const SparcInstrInfo *>(Subtarget.getInstrInfo());
  const SparcRegisterInfo &RegInfo =
      *static_cast<const SparcRegisterInfo *>(Subtarget.getRegisterInfo());
  MachineBasicBlock::iterator MBBI = MBB.begin();
  // Debug location must be unknown since the first debug location is used
  // to determine the end of the prologue.
  DebugLoc dl;
  bool NeedsStackRealignment = RegInfo.needsStackRealignment(MF);

  // FIXME: unfortunately, returning false from canRealignStack
  // actually just causes needsStackRealignment to return false,
  // rather than reporting an error, as would be sensible. This is
  // poor, but fixing that bogosity is going to be a large project.
  // For now, just see if it's lied, and report an error here.
  if (!NeedsStackRealignment && MFI.getMaxAlignment() > getStackAlignment())
    report_fatal_error("Function \"" + Twine(MF.getName()) + "\" required "
                       "stack re-alignment, but LLVM couldn't handle it "
                       "(probably because it has a dynamic alloca).");

  // Get the number of bytes to allocate from the FrameInfo
  int NumBytes = (int) MFI.getStackSize();

  unsigned SAVEri = SP::SAVEri;
  unsigned SAVErr = SP::SAVErr;

  if (Subtarget.useFlatRegisterMode()) {
    emitFlatPrologue(MF, MBB);
    SAVEri = SP::ADDri;
    SAVErr = SP::ADDrr;
  }

  if (FuncInfo->isLeafProc()) {
    if (NumBytes == 0)
      return;
    SAVEri = SP::ADDri;
    SAVErr = SP::ADDrr;
  }

  // The SPARC ABI is a bit odd in that it requires a reserved 92-byte
  // (128 in v9) area in the user's stack, starting at %sp. Thus, the
  // first part of the stack that can actually be used is located at
  // %sp + 92.
  //
  // We therefore need to add that offset to the total stack size
  // after all the stack objects are placed by
  // PrologEpilogInserter calculateFrameObjectOffsets. However, since the stack needs to be
  // aligned *after* the extra size is added, we need to disable
  // calculateFrameObjectOffsets's built-in stack alignment, by having
  // targetHandlesStackFrameRounding return true.


  // Add the extra call frame stack size, if needed. (This is the same
  // code as in PrologEpilogInserter, but also gets disabled by
  // targetHandlesStackFrameRounding)
  if (MFI.adjustsStack() && hasReservedCallFrame(MF))
    NumBytes += MFI.getMaxCallFrameSize();

  // Adds the SPARC subtarget-specific spill area to the stack
  // size. Also ensures target-required alignment.
  NumBytes = Subtarget.getAdjustedFrameSize(NumBytes);

  // Finally, ensure that the size is sufficiently aligned for the
  // data on the stack.
  if (MFI.getMaxAlignment() > 0) {
    NumBytes = alignTo(NumBytes, MFI.getMaxAlignment());
  }

  // Update stack size with corrected value.
  MFI.setStackSize(NumBytes);

  emitSPAdjustment(MF, MBB, MBBI, -NumBytes, SAVErr, SAVEri);

  if (!Subtarget.useFlatRegisterMode()) {
    unsigned regFP = RegInfo.getDwarfRegNum(SP::I6, true);

    // Emit ".cfi_def_cfa_register 30".
    unsigned CFIIndex =
        MF.addFrameInst(MCCFIInstruction::createDefCfaRegister(nullptr, regFP));
    BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
        .addCFIIndex(CFIIndex);

    // Emit ".cfi_window_save".
    CFIIndex = MF.addFrameInst(MCCFIInstruction::createWindowSave(nullptr));
    BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
        .addCFIIndex(CFIIndex);

    unsigned regInRA = RegInfo.getDwarfRegNum(SP::I7, true);
    unsigned regOutRA = RegInfo.getDwarfRegNum(SP::O7, true);
    // Emit ".cfi_register 15, 31".
    CFIIndex = MF.addFrameInst(
        MCCFIInstruction::createRegister(nullptr, regOutRA, regInRA));
    BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
        .addCFIIndex(CFIIndex);

  } else {
    unsigned CFIIndex = MF.addFrameInst(
        MCCFIInstruction::createDefCfaOffset(nullptr, -NumBytes));
    BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
        .addCFIIndex(CFIIndex);
  }

  if (NeedsStackRealignment) {
    int64_t Bias = Subtarget.getStackPointerBias();
    unsigned regUnbiased;
    if (Bias) {
      // This clobbers G1 which we always know is available here.
      regUnbiased = SP::G1;
      // add %o6, BIAS, %g1
      BuildMI(MBB, MBBI, dl, TII.get(SP::ADDri), regUnbiased)
        .addReg(SP::O6).addImm(Bias);
    } else
      regUnbiased = SP::O6;

    // andn %regUnbiased, MaxAlign-1, %regUnbiased
    int MaxAlign = MFI.getMaxAlignment();
    BuildMI(MBB, MBBI, dl, TII.get(SP::ANDNri), regUnbiased)
      .addReg(regUnbiased).addImm(MaxAlign - 1);

    if (Bias) {
      // add %g1, -BIAS, %o6
      BuildMI(MBB, MBBI, dl, TII.get(SP::ADDri), SP::O6)
        .addReg(regUnbiased).addImm(-Bias);
    }
  }
}

MachineBasicBlock::iterator SparcFrameLowering::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const {
  if (!hasReservedCallFrame(MF)) {
    MachineInstr &MI = *I;
    int Size = MI.getOperand(0).getImm();
    if (MI.getOpcode() == SP::ADJCALLSTACKDOWN)
      Size = -Size;

    if (Size)
      emitSPAdjustment(MF, MBB, I, Size, SP::ADDrr, SP::ADDri);
  }
  return MBB.erase(I);
}


void SparcFrameLowering::emitEpilogue(MachineFunction &MF,
                                  MachineBasicBlock &MBB) const {
  SparcMachineFunctionInfo *FuncInfo = MF.getInfo<SparcMachineFunctionInfo>();
  const SparcSubtarget &Subtarget = MF.getSubtarget<SparcSubtarget>();
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  const SparcInstrInfo &TII =
      *static_cast<const SparcInstrInfo *>(MF.getSubtarget().getInstrInfo());
  DebugLoc dl = MBBI->getDebugLoc();

  if (Subtarget.useFlatRegisterMode()) {
    emitFlatEpilogue(MF, MBB);
    return;
  }

  assert((MBBI->getOpcode() == SP::RETL || MBBI->getOpcode() == SP::TAIL_CALL ||
          MBBI->getOpcode() == SP::TAIL_CALLri) &&
         "Can only put epilog before 'retl' or 'tail_call' instruction!");

  if (!FuncInfo->isLeafProc()) {
    BuildMI(MBB, MBBI, dl, TII.get(SP::RESTORErr), SP::G0).addReg(SP::G0)
      .addReg(SP::G0);
    return;
  }
  MachineFrameInfo &MFI = MF.getFrameInfo();

  int NumBytes = (int) MFI.getStackSize();
  if (NumBytes != 0)
    emitSPAdjustment(MF, MBB, MBBI, NumBytes, SP::ADDrr, SP::ADDri);

  // Preserve return address in %o7
  if (MBBI->getOpcode() == SP::TAIL_CALL) {
    MBB.addLiveIn(SP::O7);
    BuildMI(MBB, MBBI, dl, TII.get(SP::ORrr), SP::G1)
        .addReg(SP::G0)
        .addReg(SP::O7);
    BuildMI(MBB, MBBI, dl, TII.get(SP::ORrr), SP::O7)
        .addReg(SP::G0)
        .addReg(SP::G1);
  }
}

bool SparcFrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  // Reserve call frame if there are no variable sized objects on the stack.
  return !MF.getFrameInfo().hasVarSizedObjects();
}

// hasFP - Return true if the specified function should have a dedicated frame
// pointer register.  This is true if the function has variable sized allocas or
// if frame pointer elimination is disabled.
bool SparcFrameLowering::hasFP(const MachineFunction &MF) const {
  const TargetRegisterInfo *RegInfo = MF.getSubtarget().getRegisterInfo();

  const MachineFrameInfo &MFI = MF.getFrameInfo();
  return MF.getTarget().Options.DisableFramePointerElim(MF) ||
      RegInfo->needsStackRealignment(MF) ||
      MFI.hasVarSizedObjects() ||
      MFI.isFrameAddressTaken();
}


int SparcFrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI,
                                               unsigned &FrameReg) const {
  const SparcSubtarget &Subtarget = MF.getSubtarget<SparcSubtarget>();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const SparcRegisterInfo *RegInfo = Subtarget.getRegisterInfo();
  const SparcMachineFunctionInfo *FuncInfo = MF.getInfo<SparcMachineFunctionInfo>();
  bool isFixed = MFI.isFixedObjectIndex(FI);

  // Addressable stack objects are accessed using neg. offsets from
  // %fp, or positive offsets from %sp.
  bool UseFP;

  // Sparc uses FP-based references in general, even when "hasFP" is
  // false. That function is rather a misnomer, because %fp is
  // actually always available, unless isLeafProc.
  if (FuncInfo->isLeafProc()) {
    // If there's a leaf proc, all offsets need to be %sp-based,
    // because we haven't caused %fp to actually point to our frame.
    UseFP = false;
  } else if (Subtarget.useFlatRegisterMode()) {
    UseFP = hasFP(MF) && !RegInfo->needsStackRealignment(MF);
  } else if (isFixed) {
    // Otherwise, argument access should always use %fp.
    UseFP = true;
  } else if (RegInfo->needsStackRealignment(MF)) {
    // If there is dynamic stack realignment, all local object
    // references need to be via %sp, to take account of the
    // re-alignment.
    UseFP = false;
  } else {
    // Finally, default to using %fp.
    UseFP = true;
  }

  int64_t FrameOffset = MF.getFrameInfo().getObjectOffset(FI) +
      Subtarget.getStackPointerBias();

  if (UseFP && !hasFP(MF) && (FrameOffset < -4096) &&
      (FrameOffset + MF.getFrameInfo().getStackSize()) <= 4095)
    UseFP = false;

  if (UseFP) {
    FrameReg = RegInfo->getFrameRegister(MF);
    return FrameOffset;
  } else {
    FrameReg = SP::O6; // %sp
    return FrameOffset + MF.getFrameInfo().getStackSize();
  }
}

static bool LLVM_ATTRIBUTE_UNUSED verifyLeafProcRegUse(MachineRegisterInfo *MRI)
{

  for (unsigned reg = SP::I0; reg <= SP::I7; ++reg)
    if (MRI->isPhysRegUsed(reg))
      return false;

  for (unsigned reg = SP::L0; reg <= SP::L7; ++reg)
    if (MRI->isPhysRegUsed(reg))
      return false;

  return true;
}

bool SparcFrameLowering::isLeafProc(MachineFunction &MF) const
{

  MachineRegisterInfo &MRI = MF.getRegInfo();
  MachineFrameInfo    &MFI = MF.getFrameInfo();

  return !(MFI.hasCalls()                  // has calls
           || MRI.isPhysRegUsed(SP::L0)    // Too many registers needed
           || MRI.isPhysRegUsed(SP::O6)    // %sp is used
           || hasFP(MF));                  // need %fp
}

void SparcFrameLowering::remapRegsForLeafProc(MachineFunction &MF) const {
  MachineRegisterInfo &MRI = MF.getRegInfo();
  // Remap %i[0-7] to %o[0-7].
  for (unsigned reg = SP::I0; reg <= SP::I7; ++reg) {
    if (!MRI.isPhysRegUsed(reg))
      continue;

    unsigned mapped_reg = reg - SP::I0 + SP::O0;

    // Replace I register with O register.
    MRI.replaceRegWith(reg, mapped_reg);

    // Also replace register pair super-registers.
    if ((reg - SP::I0) % 2 == 0) {
      unsigned preg = (reg - SP::I0) / 2 + SP::I0_I1;
      unsigned mapped_preg = preg - SP::I0_I1 + SP::O0_O1;
      MRI.replaceRegWith(preg, mapped_preg);
    }
  }

  // Rewrite MBB's Live-ins.
  for (MachineFunction::iterator MBB = MF.begin(), E = MF.end();
       MBB != E; ++MBB) {
    for (unsigned reg = SP::I0_I1; reg <= SP::I6_I7; ++reg) {
      if (!MBB->isLiveIn(reg))
        continue;
      MBB->removeLiveIn(reg);
      MBB->addLiveIn(reg - SP::I0_I1 + SP::O0_O1);
    }
    for (unsigned reg = SP::I0; reg <= SP::I7; ++reg) {
      if (!MBB->isLiveIn(reg))
        continue;
      MBB->removeLiveIn(reg);
      MBB->addLiveIn(reg - SP::I0 + SP::O0);
    }
  }

  assert(verifyLeafProcRegUse(&MRI));
#ifdef EXPENSIVE_CHECKS
  MF.verify(0, "After LeafProc Remapping");
#endif
}

void SparcFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                              BitVector &SavedRegs,
                                              RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);

  const SparcSubtarget &Subtarget = MF.getSubtarget<SparcSubtarget>();

  if (!DisableLeafProc && isLeafProc(MF)) {
    SparcMachineFunctionInfo *MFI = MF.getInfo<SparcMachineFunctionInfo>();
    MFI->setLeafProc(true);

    if (!Subtarget.useFlatRegisterMode())
      remapRegsForLeafProc(MF);
  }

  if (Subtarget.useFlatRegisterMode()) {
    const MachineFrameInfo &MFI = MF.getFrameInfo();
    const MachineRegisterInfo &MRI = MF.getRegInfo();

    if (MFI.hasCalls() || MRI.isPhysRegModified(SP::O7)
        || MFI.isReturnAddressTaken())
      SavedRegs.set(SP::I7);

    // Landing pads will modify I0 and I1
    if (!MF.getLandingPads().empty()) {
        SavedRegs.set(SP::I0);
        SavedRegs.set(SP::I1);
    }

    assert(!MRI.isPhysRegUsed(SP::I6));

    if (hasFP(MF))
      SavedRegs.set(Subtarget.getRegisterInfo()->getFrameRegister(MF));
  }
}

bool SparcFrameLowering::assignCalleeSavedSpillSlots(
    MachineFunction &MF, const TargetRegisterInfo *TRI,
    std::vector<CalleeSavedInfo> &CSI) const {
  // All callee saved registers have fixed stack locations.
  return true;
}

bool SparcFrameLowering::restoreCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    std::vector<CalleeSavedInfo> &CSI, const TargetRegisterInfo *TRI) const {
  return true;
}

bool SparcFrameLowering::spillCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    const std::vector<CalleeSavedInfo> &CSI,
    const TargetRegisterInfo *TRI) const {
  // Handled in emitFlatPrologue() for the flat register window model.
  return true;
}

void SparcFrameLowering::emitFlatPrologue(MachineFunction &MF,
                                          MachineBasicBlock &MBB) const {
  const SparcSubtarget &Subtarget = MF.getSubtarget<SparcSubtarget>();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const SparcInstrInfo &TII =
      *static_cast<const SparcInstrInfo *>(Subtarget.getInstrInfo());
  const SparcRegisterInfo &RegInfo =
      *static_cast<const SparcRegisterInfo *>(Subtarget.getRegisterInfo());
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc dl;

  const std::vector<CalleeSavedInfo> &CSI = MFI.getCalleeSavedInfo();

  BitVector Saved(RegInfo.getNumRegs());
  DebugLoc DL;

  for (auto &CS : CSI)
    Saved.set(CS.getReg());

  for (unsigned i = 0; i < 16; i++) {
    unsigned Reg = i < 8 ? SP::L0 + i : SP::I0 + i - 8;

    if (Saved.test(Reg)) {
      if (i % 2 == 0 && Saved.test(Reg + 1)) {
        unsigned DReg = i < 8 ? SP::L0_L1 + i / 2 : SP::I0_I1 + (i - 8) / 2;
        BuildMI(MBB, MBBI, DL, TII.get(SP::STDri))
            .addReg(SP::O6)
            .addImm(i * 4)
            .addReg(DReg)
            .setMIFlag(MachineInstr::FrameSetup);

        unsigned CFIIndex = MF.addFrameInst(MCCFIInstruction::createOffset(
            nullptr, RegInfo.getDwarfRegNum(Reg, true), i * 4));
        BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
            .addCFIIndex(CFIIndex);
        CFIIndex = MF.addFrameInst(MCCFIInstruction::createOffset(
            nullptr, RegInfo.getDwarfRegNum(Reg + 1, true), i * 4 + 4));
        BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
            .addCFIIndex(CFIIndex);

        i++;
      } else {
        BuildMI(MBB, MBBI, DL, TII.get(SP::STri))
            .addReg(SP::O6)
            .addImm(i * 4)
            .addReg(Reg)
            .setMIFlag(MachineInstr::FrameSetup);
        unsigned CFIIndex = MF.addFrameInst(MCCFIInstruction::createOffset(
            nullptr, RegInfo.getDwarfRegNum(Reg, true), i * 4));
        BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
            .addCFIIndex(CFIIndex);
      }
    }
  }

  if (Saved.test(SP::I7)) {
    if (!MBB.isLiveIn(SP::O7))
      MBB.addLiveIn(SP::O7);
    BuildMI(MBB, MBBI, DL, TII.get(SP::ORrr))
        .addReg(SP::I7, RegState::Define)
        .addReg(SP::G0)
        .addReg(SP::O7);
    unsigned regInRA = RegInfo.getDwarfRegNum(SP::I7, true);
    unsigned regOutRA = RegInfo.getDwarfRegNum(SP::O7, true);
    // Emit ".cfi_register 15, 31".
    unsigned CFIIndex = MF.addFrameInst(
        MCCFIInstruction::createRegister(nullptr, regOutRA, regInRA));
    BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
        .addCFIIndex(CFIIndex);
  }

  if (Saved.test(SP::I6)) {
    llvm_unreachable("Frame pointer can not be used in flat mode");
  }

  if (hasFP(MF)) {
    BuildMI(MBB, MBBI, DL, TII.get(SP::ORrr))
        .addReg(Subtarget.getRegisterInfo()->getFrameRegister(MF),
                RegState::Define)
        .addReg(SP::G0)
        .addReg(SP::O6);
  }

  return;
}

void SparcFrameLowering::emitFlatEpilogue(MachineFunction &MF,
                                          MachineBasicBlock &MBB) const {
  const SparcSubtarget &Subtarget = MF.getSubtarget<SparcSubtarget>();
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  const SparcInstrInfo &TII =
      *static_cast<const SparcInstrInfo *>(MF.getSubtarget().getInstrInfo());
  DebugLoc dl = MBBI->getDebugLoc();

  assert(Subtarget.useFlatRegisterMode());

  MachineFrameInfo &MFI = MF.getFrameInfo();
  int NumBytes = (int)MFI.getStackSize();

  const std::vector<CalleeSavedInfo> &CSI = MFI.getCalleeSavedInfo();

  BitVector Saved(Subtarget.getRegisterInfo()->getNumRegs());

  for (auto &CS : CSI)
    Saved.set(CS.getReg());

  bool TailCall = MBBI->getOpcode() == SP::TAIL_CALL;

  // Restore return address from I7.
  if (Saved.test(SP::I7)) {
    BuildMI(MBB, MBBI, dl, TII.get(SP::ORrr))
        .addReg(TailCall ? SP::G2 : SP::O7, RegState::Define)
        .addReg(SP::G0)
        .addReg(SP::I7);
  }

  unsigned FrameReg = SP::O6;
  bool RestoreSP = false;

  if (hasFP(MF)) {
    NumBytes = 0;
    FrameReg = SP::G1;
    RestoreSP = true;
    Subtarget.getRegisterInfo()->getFrameRegister(MF);
    BuildMI(MBB, MBBI, dl, TII.get(SP::ORrr))
        .addReg(SP::G1, RegState::Define)
        .addReg(SP::G0)
        .addReg(Subtarget.getRegisterInfo()->getFrameRegister(MF));
  }

  if (NumBytes + 64 > 4095) {
    BuildMI(MBB, MBBI, dl, TII.get(SP::SETHIi), SP::G1).addImm(HI22(NumBytes));
    BuildMI(MBB, MBBI, dl, TII.get(SP::ORri), SP::G1)
        .addReg(SP::G1)
        .addImm(LO10(NumBytes));
    BuildMI(MBB, MBBI, dl, TII.get(SP::ADDrr), SP::G1)
        .addReg(SP::O6)
        .addReg(SP::G1);
    NumBytes = 0;
    FrameReg = SP::G1;
    RestoreSP = true;
  }

  for (unsigned i = 0; i < 16; i++) {
    unsigned Reg = i < 8 ? SP::L0 + i : SP::I0 + i - 8;

    if (Saved.test(Reg)) {
      if (i % 2 == 0 && Saved.test(Reg + 1)) {
        unsigned DReg = i < 8 ? SP::L0_L1 + i / 2 : SP::I0_I1 + (i - 8) / 2;
        BuildMI(MBB, MBBI, dl, TII.get(SP::LDDri))
            .addReg(DReg, RegState::Define)
            .addReg(FrameReg)
            .addImm(i * 4 + NumBytes);
        i++;
      } else
        BuildMI(MBB, MBBI, dl, TII.get(SP::LDri))
            .addReg(Reg, RegState::Define)
            .addReg(FrameReg)
            .addImm(i * 4 + NumBytes);
    }
  }

  if (RestoreSP) {
    BuildMI(MBB, MBBI, dl, TII.get(SP::ORrr))
        .addReg(SP::O6, RegState::Define)
        .addReg(SP::G0)
        .addReg(SP::G1);
  } else if (NumBytes != 0)
    emitSPAdjustment(MF, MBB, MBBI, NumBytes, SP::ADDrr, SP::ADDri);

  if (TailCall)
    BuildMI(MBB, MBBI, dl, TII.get(SP::ORrr))
        .addReg(SP::O7, RegState::Define)
        .addReg(SP::G0)
        .addReg(SP::G2);

  return;
}

void SparcFrameLowering::processFunctionBeforeFrameFinalized(
    MachineFunction &MF, RegScavenger *RS) const {
  const TargetRegisterInfo *RegInfo = MF.getSubtarget().getRegisterInfo();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  const TargetRegisterClass *RC = &SP::IntRegsRegClass;
  if (!isInt<13>(MFI.estimateStackSize(MF))) {
    int RegScavFI = MFI.CreateStackObject(
        RegInfo->getSpillSize(*RC), RegInfo->getSpillAlignment(*RC), false);
    RS->addScavengingFrameIndex(RegScavFI);
  }
}
