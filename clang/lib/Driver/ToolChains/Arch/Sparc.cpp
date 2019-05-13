//===--- Sparc.cpp - Tools Implementations ----------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Sparc.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Options.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Option/ArgList.h"

using namespace clang::driver;
using namespace clang::driver::tools;
using namespace clang;
using namespace llvm::opt;

const char *sparc::getSparcAsmModeForCPU(StringRef Name,
                                         const llvm::Triple &Triple) {
  if (Triple.getArch() == llvm::Triple::sparcv9) {
    return llvm::StringSwitch<const char *>(Name)
        .Case("niagara", "-Av9b")
        .Case("niagara2", "-Av9b")
        .Case("niagara3", "-Av9d")
        .Case("niagara4", "-Av9d")
        .Default("-Av9");
  } else {
    return llvm::StringSwitch<const char *>(Name)
        .Case("v7", "-Av7")
        .Case("v8", "-Av8")
        .Case("supersparc", "-Av8")
        .Case("sparclite", "-Asparclite")
        .Case("f934", "-Asparclite")
        .Case("hypersparc", "-Av8")
        .Case("sparclite86x", "-Asparclite")
        .Case("sparclet", "-Asparclet")
        .Case("tsc701", "-Asparclet")
        .Case("v9", "-Av8plus")
        .Case("ultrasparc", "-Av8plus")
        .Case("ultrasparc3", "-Av8plus")
        .Case("niagara", "-Av8plusb")
        .Case("niagara2", "-Av8plusb")
        .Case("niagara3", "-Av8plusd")
        .Case("niagara4", "-Av8plusd")
        .Case("ma2100", "-Aleon")
        .Case("ma2150", "-Aleon")
        .Case("ma2155", "-Aleon")
        .Case("ma2450", "-Aleon")
        .Case("ma2455", "-Aleon")
        .Case("ma2x5x", "-Aleon")
        .Case("ma2080", "-Aleon")
        .Case("ma2085", "-Aleon")
        .Case("ma2480", "-Aleon")
        .Case("ma2485", "-Aleon")
        .Case("ma2x8x", "-Aleon")
        .Case("myriad2", "-Aleon")
        .Case("myriad2.1", "-Aleon")
        .Case("myriad2.2", "-Aleon")
        .Case("myriad2.3", "-Aleon")
        .Case("leon", "-Av8")
        .Case("leon3", "-Aleon")
        .Case("leon3v7", "-Aleon")
        .Case("gr712rc", "-Aleon")
        .Case("leon4", "-Aleon")
        .Case("gr740", "-Aleon")
        .Case("gr716", "-Aleon")
        .Default("-Av8");
  }
}

sparc::FloatABI sparc::getSparcFloatABI(const Driver &D,
                                        const ArgList &Args) {
  sparc::FloatABI ABI = sparc::FloatABI::Invalid;
  if (Arg *A = Args.getLastArg(clang::driver::options::OPT_msoft_float,
                               options::OPT_mhard_float,
                               options::OPT_mfloat_abi_EQ)) {
    if (A->getOption().matches(clang::driver::options::OPT_msoft_float))
      ABI = sparc::FloatABI::Soft;
    else if (A->getOption().matches(options::OPT_mhard_float))
      ABI = sparc::FloatABI::Hard;
    else {
      ABI = llvm::StringSwitch<sparc::FloatABI>(A->getValue())
                .Case("soft", sparc::FloatABI::Soft)
                .Case("hard", sparc::FloatABI::Hard)
                .Default(sparc::FloatABI::Invalid);
      if (ABI == sparc::FloatABI::Invalid &&
          !StringRef(A->getValue()).empty()) {
        D.Diag(clang::diag::err_drv_invalid_mfloat_abi) << A->getAsString(Args);
        ABI = sparc::FloatABI::Hard;
      }
    }
  }

  // If unspecified, choose the default based on the platform.
  // Only the hard-float ABI on Sparc is standardized, and it is the
  // default. GCC also supports a nonstandard soft-float ABI mode, also
  // implemented in LLVM. However as this is not standard we set the default
  // to be hard-float.
  if (ABI == sparc::FloatABI::Invalid) {
    ABI = sparc::FloatABI::Hard;
  }

  return ABI;
}

void sparc::getSparcTargetFeatures(const Driver &D, const ArgList &Args,
                                   std::vector<StringRef> &Features) {
  sparc::FloatABI FloatABI = sparc::getSparcFloatABI(D, Args);
  if (FloatABI == sparc::FloatABI::Soft)
    Features.push_back("+soft-float");
  if (Args.hasFlag(options::OPT_mflat, options::OPT_mno_flat, false))
    Features.push_back("+flat");

  if (Args.hasArg(options::OPT_mfix_gr712rc)) {
    Features.push_back("+fix-tn0009");
    Features.push_back("+fix-tn0011");
    Features.push_back("+fix-tn0012");
    Features.push_back("+fix-tn0013");
  }

  if (Args.hasArg(options::OPT_mfix_ut700)) {
    Features.push_back("+fix-tn0009");
    Features.push_back("+fix-tn0010");
    Features.push_back("+fix-tn0013");
  }

  if (Args.hasFlag(options::OPT_ffixed_g2, options::OPT_fcall_used_g2, false))
    Features.push_back("+reserve-reg-g2");
  if (Args.hasFlag(options::OPT_ffixed_g3, options::OPT_fcall_used_g3, false))
    Features.push_back("+reserve-reg-g3");
  if (Args.hasFlag(options::OPT_ffixed_g4, options::OPT_fcall_used_g4, false))
    Features.push_back("+reserve-reg-g4");
  if (Args.hasFlag(options::OPT_ffixed_g5, options::OPT_fcall_used_g5, false))
    Features.push_back("+reserve-reg-g5");
  if (Args.hasFlag(options::OPT_fcall_used_g5, options::OPT_ffixed_g5, false))
    Features.push_back("+use-reg-g5");
  if (Args.hasFlag(options::OPT_fcall_used_g6, options::OPT_ffixed_g6, false))
    Features.push_back("+use-reg-g6");
  if (Args.hasFlag(options::OPT_fcall_used_g7, options::OPT_ffixed_g7, false))
    Features.push_back("+use-reg-g7");
}
