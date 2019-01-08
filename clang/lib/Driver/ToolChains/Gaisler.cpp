//===--- Gaisler.cpp - Gaisler ToolChain Implementations --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Gaisler.h"
#include "CommonArgs.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/ArgList.h"

using namespace clang::driver;
using namespace clang::driver::toolchains;
using namespace clang;
using namespace llvm::opt;

using tools::addPathIfExists;

static bool findGaislerMultilibs(const Driver &D, const llvm::Triple &TargetTriple,
                              StringRef Path, const ArgList &Args,
                              DetectedMultilibs &Result) {


  // GCC and LLVM have slightly different multilib options.
  // We need to show the ones LLVM support when doing --print-multi-libs
  // so these are defined in the gccSuffix below as this is what is printed.
  // These are also the ones used to select the correct BSP path.
  // However some of the LLVM multilibs will not have a direct match in libgcc,
  // which we need to set the correct library paths. In these cases we use the
  // includeSuffix to point to the best matching libgcc library.

  Multilib Default;
  Result.Multilibs.push_back(Default);

  Multilib soft = Multilib()
                       .gccSuffix("/soft")
                       .includeSuffix("/soft")
                       .flag("+msoft-float");
  Result.Multilibs.push_back(soft);

  Multilib leon3 = Multilib()
                       .gccSuffix("/leon3")
                       .includeSuffix("/leon3")
                       .flag("+mcpu=leon3");
  Result.Multilibs.push_back(leon3);

  Multilib leon3_flat = Multilib()
                       .gccSuffix("/leon3/flat")
                       .includeSuffix("/leon3/flat")
                       .flag("+mcpu=leon3")
                       .flag("+mflat");
  Result.Multilibs.push_back(leon3_flat);

  // Soft targets use different directories on GCC and Clang
  Multilib leon3_soft, leon3_soft_flat, gr740_soft, gr712rc_soft;
  if (TargetTriple.getOS() == llvm::Triple::RTEMS) {
    leon3_soft = Multilib()
                       .gccSuffix("/soft/leon3")
                       .includeSuffix("/soft/leon3")
                       .flag("+mcpu=leon3")
                       .flag("+msoft-float");
    leon3_soft_flat = Multilib()
                       .gccSuffix("/soft/leon3/flat")
                       .includeSuffix("/soft/leon3/flat")
                       .flag("+mcpu=leon3")
                       .flag("+msoft-float")
		       .flag("+mflat");
    gr740_soft = Multilib()
                       .gccSuffix("/gr740/soft")
                       .includeSuffix("/soft/leon3")
                       .flag("+mcpu=gr740")
                       .flag("+msoft-float");
    gr712rc_soft = Multilib()
                       .gccSuffix("/gr712rc/soft")
                       .includeSuffix("/soft/leon3/mfix-gr712rc")
                       .flag("+mcpu=gr712rc")
                       .flag("+msoft-float");
  } else {
    leon3_soft = Multilib()
                       .gccSuffix("/leon3/soft")
                       .includeSuffix("/leon3/soft")
                       .flag("+mcpu=leon3")
                       .flag("+msoft-float");
    leon3_soft_flat = Multilib()
                       .gccSuffix("/leon3/soft/flat")
                       .includeSuffix("/leon3/soft/flat")
                       .flag("+mcpu=leon3")
                       .flag("+msoft-float")
		       .flag("+mflat");
    gr740_soft = Multilib()
                       .gccSuffix("/gr740/soft")
                       .includeSuffix("/leon3/soft")
                       .flag("+mcpu=gr740")
                       .flag("+msoft-float");
    gr712rc_soft = Multilib()
                       .gccSuffix("/gr712rc/soft")
                       .includeSuffix("/leon3/mfix-gr712rc/soft")
                       .flag("+mcpu=gr712rc")
                       .flag("+msoft-float");

  }
  Result.Multilibs.push_back(leon3_soft);
  Result.Multilibs.push_back(leon3_soft_flat);
  Result.Multilibs.push_back(gr740_soft);
  Result.Multilibs.push_back(gr712rc_soft);

  /*  Multilib leon3_rex = Multilib()
                       .gccSuffix("/leon3/rex")
                       .includeSuffix("/leon3")
                       .flag("+mcpu=leon3")
                       .flag("+mrex");
  Result.Multilibs.push_back(leon3_rex);

  Multilib leon3_rex_soft = Multilib()
                       .gccSuffix("/leon3/rex/soft")
                       .includeSuffix("/leon3/soft")
                       .flag("+mcpu=leon3")
                       .flag("+mrex")
                       .flag("+msoft-float");
  Result.Multilibs.push_back(leon3_rex_soft);

  Multilib rex = Multilib()
                       .gccSuffix("/rex")
                       .includeSuffix("/")
                       .flag("+mrex");
  Result.Multilibs.push_back(rex);

  Multilib rex_soft = Multilib()
                       .gccSuffix("/rex/soft")
                       .includeSuffix("/soft")
                       .flag("+mrex")
                       .flag("+msoft-float");
  Result.Multilibs.push_back(rex_soft);
  */
  Multilib gr740 = Multilib()
                       .gccSuffix("/gr740")
                       .includeSuffix("/leon3")
                       .flag("+mcpu=gr740");
  Result.Multilibs.push_back(gr740);

  Multilib gr712rc = Multilib()
                       .gccSuffix("/gr712rc")
                       .includeSuffix("/leon3/mfix-gr712rc")
                       .flag("+mcpu=gr712rc");
  Result.Multilibs.push_back(gr712rc);

  // find the selected multilib

  // first get the cpu kind
  llvm::StringRef cpu = Args.getLastArgValue(options::OPT_mcpu_EQ, "");

  // should be handled by the below if statements
  // however we set to it a default in case it is not handled.
  Result.SelectedMultilib = Default;

  if (cpu.empty()) {
    if (Args.hasArg(options::OPT_msoft_float)) {
        /*        if (Args.hasArg(options::OPT_mrex)) {
          Result.SelectedMultilib = rex_soft;
          } else {*/
          Result.SelectedMultilib = soft;
          //        }
          /*    } else if (Args.hasArg(options::OPT_mrex)) {
                Result.SelectedMultilib = rex;*/
    } else {
      Result.SelectedMultilib = Default;
    }
  } else if (cpu.equals_lower("leon3")) {
    if (Args.hasArg(options::OPT_msoft_float)) {
        /*        if (Args.hasArg(options::OPT_mrex)) {
          Result.SelectedMultilib = leon3_rex_soft;
          } else {*/
          if (Args.hasArg(options::OPT_mflat)) {
            Result.SelectedMultilib = leon3_soft_flat;
          } else {
            Result.SelectedMultilib = leon3_soft;
          }
          //  }
    /*    } else if (Args.hasArg(options::OPT_mrex)) {
          Result.SelectedMultilib = leon3_rex;*/
    } else if (Args.hasArg(options::OPT_mflat)) {
      Result.SelectedMultilib = leon3_flat;
    } else {
      Result.SelectedMultilib = leon3;
    }
  } else if (cpu.equals_lower("gr740")) {
    if (Args.hasArg(options::OPT_msoft_float)) {
      Result.SelectedMultilib = gr740_soft;
    } else {
      Result.SelectedMultilib = gr740;
    }
  } else if (cpu.equals_lower("gr712rc")) {
    if (Args.hasArg(options::OPT_msoft_float)) {
      Result.SelectedMultilib = gr712rc_soft;
    } else {
      Result.SelectedMultilib = gr712rc;
    }
  }

  return true;

}

GaislerToolChain::GaislerToolChain(const Driver &D, const llvm::Triple &Triple,
                                 const ArgList &Args)
  : Generic_ELF(D, Triple, Args) {

  DetectedMultilibs Result;
  findGaislerMultilibs(D, Triple, "", Args, Result);
  Multilibs = Result.Multilibs;
  SelectedMultilib = Result.SelectedMultilib;

  // prefix might end up with sparc-gaisler--elf- for BCC. We do not want the
  // double dashes, so define it manually for BCC.
  std::string prefix = Triple.str();
  if (Triple.getOS() != llvm::Triple::RTEMS) {
    prefix = "sparc-gaisler-elf";
  }

  GCCInstallation.init(Triple, Args, {prefix});
  if (GCCInstallation.isValid()) {
    // This directory contains crt{i,n,begin,end}.o as well as libgcc.
    // These files are tied to a particular version of gcc.
    SmallString<128> CompilerSupportDir(GCCInstallation.getInstallPath());
    addPathIfExists(D, CompilerSupportDir + SelectedMultilib.includeSuffix(),
                    getFilePaths());
  }

  // Select the BSP based on the qbsp flag.
  // Defaults to leon3 if not set
  if (Args.hasArg(options::OPT_qbsp)) {
    bsp = Args.getLastArgValue(options::OPT_qbsp, "NONEXISTENTPATH").str();
    if (Triple.getOS() == llvm::Triple::RTEMS) {
      addPathIfExists(D, D.Dir + "/../" + prefix + "/" + bsp +
                       "/lib/", getFilePaths());
    } else {
      addPathIfExists(D, D.Dir + "/../" + prefix + "/bsp/" + bsp +
                       SelectedMultilib.gccSuffix(), getFilePaths());
    }
  } else if (Triple.getOS() != llvm::Triple::RTEMS) {
    bsp = "leon3";
    addPathIfExists(D, D.Dir + "/../" + prefix + "/bsp/" + bsp +
                       SelectedMultilib.gccSuffix(), getFilePaths());
  } else {
    bsp = "NONEXISTENTPATH";
  }

  // newlib is expected to be here
  addPathIfExists(D, D.Dir + "/../" + prefix + "/lib" +
                     SelectedMultilib.gccSuffix(), getFilePaths());

  // Ensures paths included with -B are added
  for (auto &I : D.PrefixDirs) {
    addPathIfExists(D, I, getFilePaths());
  }
}

Tool *GaislerToolChain::buildLinker() const {
  return new tools::gaisler::Linker(*this);
}

ToolChain::CXXStdlibType
GaislerToolChain::GetCXXStdlibType(const ArgList &Args) const {
  Arg *A = Args.getLastArg(options::OPT_stdlib_EQ);
  if (!A)
    return ToolChain::CST_Libstdcxx;

  StringRef Value = A->getValue();
  if (Value != "libstdc++")
    getDriver().Diag(diag::err_drv_invalid_stdlib_name) << A->getAsString(Args);

  return ToolChain::CST_Libstdcxx;
}

void GaislerToolChain::AddClangSystemIncludeArgs(const ArgList &DriverArgs,

                                                ArgStringList &CC1Args) const {
  // prefix might end up with sparc-gaisler--elf- for BCC. We do not want the
  // double dashes, so define it manually for BCC.
  std::string prefix = getTriple().str();
  if (getTriple().getOS() != llvm::Triple::RTEMS) {
    prefix = "sparc-gaisler-elf";
  }

  // newlib should be here
  if (!DriverArgs.hasArg(options::OPT_nostdinc) &&
      DriverArgs.hasArg(options::OPT_qnano)) {
     addSystemInclude(DriverArgs, CC1Args, getDriver().Dir +
                                          "/../" + prefix +
                                          "/include/newlib-nano");
  }

  if (!DriverArgs.hasArg(options::OPT_nostdinc)) {
     addSystemInclude(DriverArgs, CC1Args, getDriver().Dir +
                                          "/../" + prefix +
                                          "/include");
  }

  if (getTriple().getOS() == llvm::Triple::RTEMS) {
    // RCC specific include files
    addSystemInclude(DriverArgs, CC1Args, getDriver().Dir +
                                          "/../" + prefix + "/"
                                           + bsp + "/lib/include/");
  } else {
    // BCC specific include files
    addSystemInclude(DriverArgs, CC1Args, getDriver().Dir +
                                          "/../" + prefix + "/bsp/" + bsp +
                                          "/include/");
  }

  // ensure includes from -B flags are included
  if (DriverArgs.hasArg(options::OPT_qrtems)) {
    for (auto &I : getDriver().PrefixDirs) {
      addSystemInclude(DriverArgs, CC1Args, I + "/include");
    }
  }

  // make sure we don't include files from /usr/include or other local systems
  CC1Args.push_back("-nostdsysteminc");
}

void GaislerToolChain::AddClangCXXStdlibIncludeArgs(const ArgList &DriverArgs,
                                               ArgStringList &CC1Args) const {
  // prefix might end up with sparc-gaisler--elf- for BCC. We do not want the
  // double dashes, so define it manually for BCC.
  std::string prefix = getTriple().str();
  if (getTriple().getOS() != llvm::Triple::RTEMS) {
    prefix = "sparc-gaisler-elf";
  }

  addSystemInclude(DriverArgs, CC1Args, GCCInstallation.getInstallPath() +
                   "/include/c++/");
  addSystemInclude(DriverArgs, CC1Args, GCCInstallation.getInstallPath() +
                   "/include/c++/" + prefix + "/" +
                    SelectedMultilib.includeSuffix());
  addSystemInclude(DriverArgs, CC1Args, GCCInstallation.getInstallPath() +
                   "/include/c++/backward");
}

void GaislerToolChain::AddCXXStdlibLibArgs(const ArgList &Args,
                                 ArgStringList &CmdArgs) const {

  if (GetCXXStdlibType(Args) == ToolChain::CST_Libstdcxx) {
    CmdArgs.push_back("-lstdc++");
  }
}

GaislerVxWorksToolChain::GaislerVxWorksToolChain(const Driver &D, const llvm::Triple &Triple,
                                 const ArgList &Args)
  : Generic_ELF(D, Triple, Args) {

}

Tool *GaislerVxWorksToolChain::buildLinker() const {
  return new tools::gaisler::VxWorksLinker(*this);
}

void tools::gaisler::Linker::ConstructJob(Compilation &C, const JobAction &JA,
                               const InputInfo &Output,
                               const InputInfoList &Inputs, const ArgList &Args,
                               const char *LinkingOutput) const {

  const auto &TC =
      static_cast<const toolchains::GaislerToolChain &>(getToolChain());
  const llvm::Triple &T = TC.getTriple();
  ArgStringList CmdArgs;
  bool UseStartfiles =
      !Args.hasArg(options::OPT_nostdlib, options::OPT_nostartfiles);
  bool UseDefaultLibs =
      !Args.hasArg(options::OPT_nostdlib, options::OPT_nodefaultlibs);
  // Silence warning if the args contain both -nostdlib and -stdlib=.
  Args.getLastArg(options::OPT_stdlib_EQ);
  const Driver &D = getToolChain().getDriver(); 
  // The remaining logic is mostly like gnutools::Linker::ConstructJob,
  // but we never pass through a --sysroot option and various other bits.
  // For example, there are no sanitizers (yet) nor gold linker.

  if (Args.hasArg(options::OPT_s)) // Pass the 'strip' option.
    CmdArgs.push_back("-s");

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  // RTEMS/RCC
  if (T.getOS() == llvm::Triple::RTEMS) {
    if (UseStartfiles) {
      if (! Args.hasArg(options::OPT_qbsp) && ! Args.hasArg(options::OPT_qrtems) ) {
        CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath("crt0.o")));
      }
      if (Args.hasArg(options::OPT_qbsp) || Args.hasArg(options::OPT_qrtems)) {
        CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath("crti.o")));
        CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath("crtbegin.o")));
      }
    }

    TC.AddFilePathLibArgs(Args, CmdArgs);

    Args.AddAllArgs(CmdArgs, {options::OPT_L, options::OPT_T_Group,
                              options::OPT_e, options::OPT_s, options::OPT_t,
                              options::OPT_Z_Flag, options::OPT_r});

    AddLinkerInputs(getToolChain(), Inputs, Args, CmdArgs, JA);
    //if (D.CCCIsCXX()) {
    // For some reason D.CCIsCXX is failing. Use driver Name attribute instead.
    // Note that the binary must be a copy - not a symlink!
    if (D.Name.find("clang++") != std::string::npos)
      TC.AddCXXStdlibLibArgs(Args, CmdArgs);

    if (UseDefaultLibs) {
      CmdArgs.push_back("-lgcc");
      CmdArgs.push_back("--start-group");
      // only include if qbsp/qrtems is specified
      if (Args.hasArg(options::OPT_qbsp) || Args.hasArg(options::OPT_qrtems)) {
        CmdArgs.push_back("-lrtemsbsp");
        CmdArgs.push_back("-lrtemscpu");
      }
      CmdArgs.push_back("-latomic");
      CmdArgs.push_back("-lc");
      CmdArgs.push_back("-lgcc"); // circularly dependent on rtems
      CmdArgs.push_back("--end-group");
    }

    if(!Args.hasArg(options::OPT_T) && !Args.hasArg(options::OPT_qnolinkcmds)) {
      // only include linkcmds if it is found correctly
      std::string Path = TC.GetFilePath("linkcmds");
      if (Path != "linkcmds") {
        CmdArgs.push_back(Args.GetOrMakeJoinedArgString((Args.size()+1),"-T",
                                                        TC.GetFilePath("linkcmds")));
      }
    }

    CmdArgs.push_back("-lgcc");

    if (UseStartfiles && ( Args.hasArg(options::OPT_qbsp) ||
        Args.hasArg(options::OPT_qrtems)) ) {
      CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath("crtend.o")));
      CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath("crtn.o")));
    }

  } else { // BCC
    if (UseStartfiles) {
      if (Args.hasArg(options::OPT_qsvt)) {
        CmdArgs.push_back(
            Args.MakeArgString(TC.GetFilePath("trap_table_svt.S.o")));
      } else {
        CmdArgs.push_back(
            Args.MakeArgString(TC.GetFilePath("trap_table_mvt.S.o")));
      }

      CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath("crt0.S.o")));

      CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath("crti.o")));
      CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath("crtbegin.o")));
    }

    TC.AddFilePathLibArgs(Args, CmdArgs);

    Args.AddAllArgs(CmdArgs, {options::OPT_L, options::OPT_T_Group,
                              options::OPT_e, options::OPT_s, options::OPT_t,
                              options::OPT_Z_Flag, options::OPT_r});

    AddLinkerInputs(getToolChain(), Inputs, Args, CmdArgs, JA);
    if (D.CCCIsCXX()) {
      TC.AddCXXStdlibLibArgs(Args, CmdArgs);
    }

    if (UseDefaultLibs) {
      CmdArgs.push_back("-lgcc");
      CmdArgs.push_back("--start-group");
      CmdArgs.push_back("-lbcc");
      if(Args.hasArg(options::OPT_qnano)) {
        CmdArgs.push_back("-lc_nano");
      } else {
        CmdArgs.push_back("-lc");
      }
      CmdArgs.push_back("--end-group");
      CmdArgs.push_back("-lgcc");
    }

    if(!Args.hasArg(options::OPT_T) && !Args.hasArg(options::OPT_qnolinkcmds)) {
      // only include linkcmds if it is found correctly
      std::string Path = TC.GetFilePath("linkcmds");
      if (Path != "linkcmds") {
        CmdArgs.push_back(Args.GetOrMakeJoinedArgString((Args.size()+1),"-T",
                                                        TC.GetFilePath("linkcmds")));
      }
    }

    if (UseStartfiles) {
      CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath("crtend.o")));
      CmdArgs.push_back(Args.MakeArgString(TC.GetFilePath("crtn.o")));
    }
  } // end BCC

  // prefix might end up with sparc-gaisler--elf- for BCC. We do not want the
  // double dashes, so define it manually.
  std::string prefix = T.str();
  if (T.getOS() != llvm::Triple::RTEMS) {
    prefix = "sparc-gaisler-elf";
  }

  std::string linker_binary = prefix + "-ld";

  std::string Exec =
      Args.MakeArgString(TC.GetProgramPath(linker_binary.c_str()));
  C.addCommand(llvm::make_unique<Command>(JA, *this, Args.MakeArgString(Exec),
                                          CmdArgs, Inputs));

}

void tools::gaisler::VxWorksLinker::ConstructJob(Compilation &C, const JobAction &JA,
                               const InputInfo &Output,
                               const InputInfoList &Inputs, const ArgList &Args,
                               const char *LinkingOutput) const {

  const toolchains::GaislerToolChain &TC = static_cast<const toolchains::GaislerToolChain &>(getToolChain());
  const Driver &D = TC.getDriver();

  ArgStringList CmdArgs;
  // Silence warning for "clang -g foo.o -o foo"
  Args.ClaimAllArgs(options::OPT_g_Group);
  // and "clang -emit-llvm foo.o -o foo"
  Args.ClaimAllArgs(options::OPT_emit_llvm);
  // and for "clang -w foo.o -o foo". Other warning options are already
  // handled somewhere else.
  Args.ClaimAllArgs(options::OPT_w);

  if (Args.hasArg(options::OPT_rdynamic))
    CmdArgs.push_back("-export-dynamic");

  if (Args.hasArg(options::OPT_s))
    CmdArgs.push_back("-s");

  CmdArgs.push_back("--eh-frame-hdr");

  CmdArgs.push_back("-m");
  CmdArgs.push_back("elf32_sparc_vxworks");

  if (Args.hasArg(options::OPT_static))
    CmdArgs.push_back("-static");
  else if (Args.hasArg(options::OPT_shared)) {
    CmdArgs.push_back("-shared");
  }

  CmdArgs.push_back("-o");
  CmdArgs.push_back(Output.getFilename());

  Args.AddAllArgs(CmdArgs, options::OPT_L);
  Args.AddAllArgs(CmdArgs, options::OPT_u);

  TC.AddFilePathLibArgs(Args, CmdArgs);

  if (D.isUsingLTO()) {
    assert(!Inputs.empty() && "Must have at least one input.");
    AddGoldPlugin(TC, Args, CmdArgs, Output, Inputs[0],
                  D.getLTOMode() == LTOK_Thin);
  }

  if (Args.hasArg(options::OPT_Z_Xlinker__no_demangle))
    CmdArgs.push_back("--no-demangle");

  AddLinkerInputs(TC, Inputs, Args, CmdArgs, JA);

  // Silence warnings when linking C code with a C++ '-stdlib' argument.
  Args.ClaimAllArgs(options::OPT_stdlib_EQ);

  const char *Exec = Args.MakeArgString(TC.GetProgramPath("ldsparc"));
  C.addCommand(llvm::make_unique<Command>(JA, *this, Exec, CmdArgs, Inputs));
}
