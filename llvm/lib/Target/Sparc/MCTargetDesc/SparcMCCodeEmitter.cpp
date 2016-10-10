//===-- SparcMCCodeEmitter.cpp - Convert Sparc code to machine code -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the SparcMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SparcFixupKinds.h"
#include "SparcMCExpr.h"
#include "SparcMCTargetDesc.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <cstdint>

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");

namespace {

class SparcMCCodeEmitter : public MCCodeEmitter {
  const MCInstrInfo &MCII;
  MCContext &Ctx;

public:
  SparcMCCodeEmitter(const MCInstrInfo &mcii, MCContext &ctx)
      : MCII(mcii), Ctx(ctx) {}
  SparcMCCodeEmitter(const SparcMCCodeEmitter &) = delete;
  SparcMCCodeEmitter &operator=(const SparcMCCodeEmitter &) = delete;
  ~SparcMCCodeEmitter() override = default;

  bool isREX(const MCSubtargetInfo &STI) const {
    return STI.getFeatureBits()[Sparc::FeatureREX];
  }

  void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  // getBinaryCodeForInstr - TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// getMachineOpValue - Return binary encoding of operand. If the machine
  /// operand requires relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getCallTargetOpValue(const MCInst &MI, unsigned OpNo,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;
  unsigned getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;
  unsigned getADDRfiOpValue(const MCInst &MI, unsigned OpNo,
                            SmallVectorImpl<MCFixup> &Fixups,
                            const MCSubtargetInfo &STI) const;
  unsigned getRexShortBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const;
  unsigned getRexLongBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const;
  unsigned getBranchPredTargetOpValue(const MCInst &MI, unsigned OpNo,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const;
  unsigned getBranchOnRegTargetOpValue(const MCInst &MI, unsigned OpNo,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       const MCSubtargetInfo &STI) const;
private:
  uint64_t computeAvailableFeatures(const FeatureBitset &FB) const;
  void verifyInstructionPredicates(const MCInst &MI,
                                   uint64_t AvailableFeatures) const;
  unsigned getImm32OpValue(const MCInst &MI, unsigned OpNo,
                           SmallVectorImpl<MCFixup> &Fixups,
                           const MCSubtargetInfo &STI) const;
  unsigned getRexIntRegEncoding(const MCInst &MI, unsigned OpNo,
                                SmallVectorImpl<MCFixup> &Fixups,
                                const MCSubtargetInfo &STI) const;
  unsigned getRexFPRegEncoding(const MCInst &MI, unsigned OpNo,
                               SmallVectorImpl<MCFixup> &Fixups,
                               const MCSubtargetInfo &STI) const;
};

} // end anonymous namespace

void SparcMCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  verifyInstructionPredicates(MI,
                              computeAvailableFeatures(STI.getFeatureBits()));

  uint64_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);

  support::endianness Endian = Ctx.getAsmInfo()->isLittleEndian() ? support::little
    : support::big;

  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());

  // Get byte count of instruction
  unsigned Size = Desc.getSize();

  switch (Size) {
  default:
    llvm_unreachable("Unhandled encodeInstruction length!");
  case 2:
    support::endian::write<uint16_t>(OS, Bits, Endian);
    break;
  case 4:
    support::endian::write<uint32_t>(OS, Bits, Endian);
    break;
  case 6:
    support::endian::write<uint16_t>(OS, Bits >> 32, Endian);
    support::endian::write<uint32_t>(OS, Bits, Endian);
  }

  unsigned tlsOpNo = 0;
  switch (MI.getOpcode()) {
  default: break;
  case SP::TLS_CALL:   tlsOpNo = 1; break;
  case SP::TLS_ADDrr:
  case SP::TLS_ADDXrr:
  case SP::TLS_LDrr:
  case SP::TLS_LDXrr:  tlsOpNo = 3; break;
  }
  if (tlsOpNo != 0) {
    const MCOperand &MO = MI.getOperand(tlsOpNo);
    uint64_t op = getMachineOpValue(MI, MO, Fixups, STI);
    assert(op == 0 && "Unexpected operand value!");
    (void)op; // suppress warning.
  }

  ++MCNumEmitted;  // Keep track of the # of mi's emitted.
}

unsigned SparcMCCodeEmitter::
getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                  SmallVectorImpl<MCFixup> &Fixups,
                  const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  if (MO.isImm())
    return MO.getImm();

  assert(MO.isExpr());
  const MCExpr *Expr = MO.getExpr();
  if (const SparcMCExpr *SExpr = dyn_cast<SparcMCExpr>(Expr)) {
    MCFixupKind Kind = (MCFixupKind)SExpr->getFixupKind();
    Fixups.push_back(MCFixup::create(0, Expr, Kind));
    return 0;
  }

  int64_t Res;
  if (Expr->evaluateAsAbsolute(Res))
    return Res;

  llvm_unreachable("Unhandled expression!");
  return 0;
}

unsigned SparcMCCodeEmitter::
getCallTargetOpValue(const MCInst &MI, unsigned OpNo,
                     SmallVectorImpl<MCFixup> &Fixups,
                     const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isImm())
    return MO.getImm() >> 2;
  if (MO.isReg())
    return getMachineOpValue(MI, MO, Fixups, STI);

  if (MI.getOpcode() == SP::TLS_CALL) {
    // No fixups for __tls_get_addr. Will emit for fixups for tls_symbol in
    // encodeInstruction.
#ifndef NDEBUG
    // Verify that the callee is actually __tls_get_addr.
    const SparcMCExpr *SExpr = dyn_cast<SparcMCExpr>(MO.getExpr());
    assert(SExpr && SExpr->getSubExpr()->getKind() == MCExpr::SymbolRef &&
           "Unexpected expression in TLS_CALL");
    const MCSymbolRefExpr *SymExpr = cast<MCSymbolRefExpr>(SExpr->getSubExpr());
    assert(SymExpr->getSymbol().getName() == "__tls_get_addr" &&
           "Unexpected function for TLS_CALL");
#endif
    return 0;
  }

  MCFixupKind fixupKind = (MCFixupKind)Sparc::fixup_sparc_call30;

  if (const SparcMCExpr *SExpr = dyn_cast<SparcMCExpr>(MO.getExpr())) {
    if (SExpr->getKind() == SparcMCExpr::VK_Sparc_WPLT30)
      fixupKind = (MCFixupKind)Sparc::fixup_sparc_wplt30;
  }

  const MCExpr *FixupExpression = MO.getExpr();

  if (isREX(STI))
    FixupExpression = MCBinaryExpr::createAdd(
        MO.getExpr(), MCConstantExpr::create(+2, Ctx), Ctx);

  Fixups.push_back(MCFixup::create(0, FixupExpression, fixupKind));

  return 0;
}

unsigned SparcMCCodeEmitter::
getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                  SmallVectorImpl<MCFixup> &Fixups,
                  const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isReg() || MO.isImm())
    return getMachineOpValue(MI, MO, Fixups, STI);

  Fixups.push_back(MCFixup::create(0, MO.getExpr(),
                                   (MCFixupKind)Sparc::fixup_sparc_br22));
  return 0;
}

unsigned
SparcMCCodeEmitter::getADDRfiOpValue(const MCInst &MI, unsigned OpNo,
                                     SmallVectorImpl<MCFixup> &Fixups,
                                     const MCSubtargetInfo &STI) const {

  const MCOperand &MO1 = MI.getOperand(OpNo);
  const MCOperand &MO2 = MI.getOperand(OpNo + 1);
  unsigned RegVal;
  unsigned EncodedValue;

  switch (MO1.getReg()) {
  case SP::I6: // FP
    RegVal = 0;
    break;
  case SP::O6: // SP
    RegVal = 1;
    break;
  case SP::I0:
    RegVal = 2;
    break;
  case SP::O0:
    RegVal = 3;
    break;
  default:
    llvm_unreachable("Unsupported register");
  }

  EncodedValue = RegVal << 5;
  EncodedValue |= MO2.getImm() >> 2;

  return EncodedValue;
}

unsigned SparcMCCodeEmitter::getRexShortBranchTargetOpValue(
    const MCInst &MI, unsigned OpNo, SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);

  assert(!MO.isReg());

  if (MO.isImm()) {
    unsigned value = getMachineOpValue(MI, MO, Fixups, STI);
    assert(!(value & 1));
    return value >> 1;
  }

  Fixups.push_back(
      MCFixup::create(0, MO.getExpr(), (MCFixupKind)Sparc::fixup_sparc_br8));
  return 0;
}

unsigned SparcMCCodeEmitter::getRexLongBranchTargetOpValue(
    const MCInst &MI, unsigned OpNo, SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);

  assert(!MO.isReg());

  if (MO.isImm()) {
    unsigned value = getMachineOpValue(MI, MO, Fixups, STI);
    assert(!(value & 1));
    return value >> 1;
  }

  Fixups.push_back(
      MCFixup::create(0, MO.getExpr(), (MCFixupKind)Sparc::fixup_sparc_br24));
  return 0;
}

unsigned SparcMCCodeEmitter::getBranchPredTargetOpValue(
    const MCInst &MI, unsigned OpNo, SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isReg() || MO.isImm())
    return getMachineOpValue(MI, MO, Fixups, STI);

  Fixups.push_back(MCFixup::create(0, MO.getExpr(),
                                   (MCFixupKind)Sparc::fixup_sparc_br19));
  return 0;
}

unsigned SparcMCCodeEmitter::
getBranchOnRegTargetOpValue(const MCInst &MI, unsigned OpNo,
                           SmallVectorImpl<MCFixup> &Fixups,
                           const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isReg() || MO.isImm())
    return getMachineOpValue(MI, MO, Fixups, STI);

  Fixups.push_back(MCFixup::create(0, MO.getExpr(),
                                   (MCFixupKind)Sparc::fixup_sparc_br16_2));
  Fixups.push_back(MCFixup::create(0, MO.getExpr(),
                                   (MCFixupKind)Sparc::fixup_sparc_br16_14));

  return 0;
}

unsigned SparcMCCodeEmitter::getImm32OpValue(const MCInst &MI, unsigned OpNo,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isImm())
    return MO.getImm();

  assert(MO.isExpr());
  const MCExpr *Expr = MO.getExpr();
  Fixups.push_back(
      MCFixup::create(2, Expr, (MCFixupKind)Sparc::fixup_sparc_32));
  return 0;
}

static const unsigned RexIntRegsMapping[32] = {
    24, 25, 26, 27, 28, 29, 30, 31, 0, 1, 2,  3,  20, 21, 22, 23,
    16, 17, 18, 19, 4,  5,  6,  7,  8, 9, 10, 11, 12, 13, 14, 15};

static const unsigned RexFloatRegsMapping[32] = {
    8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 4,  5,  6,  7,
    0, 1, 2,  3,  20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};

unsigned
SparcMCCodeEmitter::getRexIntRegEncoding(const MCInst &MI, unsigned OpNo,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const {

  const MCOperand &MO = MI.getOperand(OpNo);
  assert(MO.isReg());

  unsigned regval = Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  return RexIntRegsMapping[regval];
}

unsigned
SparcMCCodeEmitter::getRexFPRegEncoding(const MCInst &MI, unsigned OpNo,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const {

  const MCOperand &MO = MI.getOperand(OpNo);
  assert(MO.isReg());

  unsigned regval = Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  return RexFloatRegsMapping[regval];
}

#define ENABLE_INSTR_PREDICATE_VERIFIER

#include "SparcGenMCCodeEmitter.inc"

MCCodeEmitter *llvm::createSparcMCCodeEmitter(const MCInstrInfo &MCII,
                                              const MCRegisterInfo &MRI,
                                              MCContext &Ctx) {
  return new SparcMCCodeEmitter(MCII, Ctx);
}
