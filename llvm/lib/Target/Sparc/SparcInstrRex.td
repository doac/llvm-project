//===----------------------- SparcInstrRex.td - REX -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains instruction formats, definitions and patterns needed for
// the REX instruction set extension.
//
// A full description of the REX instruction set is available at
// http://www.gaisler.com/doc/antn/GRLIB-TN-0001.pdf.
//
//===----------------------------------------------------------------------===//

class InstRex<dag outs, dag ins, string asmstr, list<dag> pattern>
    : Instruction {
  let Namespace = "SP";

  let Predicates      = [IsREX];
  dag OutOperandList  = outs;
  dag InOperandList   = ins;
  let AsmString       = asmstr;
  let Pattern         = pattern;

  let AddedComplexity = 60;

  let DecoderNamespace = "SparcRex";
  field bits<48> SoftFail = 0;
}

// Base class for 16-bit REX instructions
class InstRex16<dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstRex<outs, ins, asmstr, pattern> {
  field bits<16> Inst;
  let Size = 2;
  bits<2> op;
  let Inst{15-14} = op;
}

// Base class for 32-bit REX instructions
class InstRex32<dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstRex<outs, ins, asmstr, pattern> {
  field bits<32> Inst;
  let Size = 4;
  bits<2> op;
  let Inst{31-30} = op;
}

// Base class for 48-bit REX instructions
class InstRex48<string asmstr, bits<5> rop4, Operand immop,
                Operand regop, list<dag> pattern>
    : InstRex<(outs regop:$rd), (ins immop:$imm32), asmstr, pattern> {
  field bits<48> Inst;
  bits<32> imm32;
  bits<4> rd;
  let Size = 6;
  let Inst{47-46} = 0b10;
  let Inst{45-42} = rd;
  let Inst{41}    = 1;
  let Inst{40-37} = 0b1111;
  let Inst{36-32} = rop4;
  let Inst{31-0}  = imm32;
}

// For 16-bit REX instruction with rs and rd operands
class RexFR<bits<2> opval, bits<4> rop3val,
            bits<1> rop3lval, dag outs, dag ins, string asmstr,
            list<dag> pattern>
    : InstRex16<outs, ins, asmstr, pattern> {
  bits<4> rs;
  bits<4> rd;
  let op          = opval;
  let Inst{13-10} = rd;
  let Inst{9}     = 0;
  let Inst{8-5}   = rop3val;
  let Inst{4}     = rop3lval;
  let Inst{3-0}   = rs;
}

// For 16-bit REX instruction with immediate and rd operands
class RexFI<bits<2> opval, bits<1> imm, bits<4> rop3val,
            dag outs, dag ins, string asmstr,
            list<dag> pattern>
    : InstRex16<outs, ins, asmstr, pattern> {
  bits<4> rd;
  bits<5> imm5;
  let op          = opval;
  let Inst{13-10} = rd;
  let Inst{9}     = imm;
  let Inst{8-5}   = rop3val;
  let Inst{4-0}   = imm5;
}

// For 16-bit REX instruction with only rd operand
class RexFO<bits<2> opval, bits<1> imm, bits<4> rop3val, bits<5> rop4val,
            dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstRex16<outs, ins, asmstr, pattern> {
  bits<4> rd;
  let op          = opval;
  let Inst{13-10} = rd;
  let Inst{9}     = imm;
  let Inst{8-5}   = rop3val;
  let Inst{4-0}   = rop4val;
}

// For 16-bit REX instructions with fixed register + immediate and rd operands
class RexFF<bits<1> isStore, bits<1> isFloat,
             dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstRex16<outs, ins, asmstr, pattern> {
  bits<4> rd;
  bits<2> rs;
  bits<7> addr;
  let op = 0b11;
  let Inst{13-10} = rd;
  let Inst{9} = 0b1;
  let Inst{8} = isStore;
  let Inst{7-6} = addr{6-5};
  let Inst{5} = isFloat;
  let Inst{4-0} = addr{4-0};
}

// Transforms

def Log2 : SDNodeXForm<imm, [{
 return CurDAG->getTargetConstant(Log2_32((unsigned)N->getZExtValue()),
        SDLoc(N), MVT::i32);
}]>;

def NLog2 : SDNodeXForm<imm, [{
 return CurDAG->getTargetConstant(Log2_32(~(unsigned)N->getZExtValue()),
        SDLoc(N), MVT::i32);
}]>;

// Asm operands

class ImmAsmOperandClass : AsmOperandClass {
  let RenderMethod = "addImmOperands";
}

class RegAsmOperandClass : AsmOperandClass {
  let RenderMethod = "addRegOperands";
}

def Imm5AsmOperand       : ImmAsmOperandClass { let Name = "Imm5";       }
def SImm5AsmOperand      : ImmAsmOperandClass { let Name = "SImm5";      }
def Imm3AsmOperand       : ImmAsmOperandClass { let Name = "Imm3";       }
def NImm13AsmOperand     : ImmAsmOperandClass { let Name = "NImm13";     }
def SImm21AsmOperand     : ImmAsmOperandClass { let Name = "SImm21";     }
def BrTarget8AsmOperand  : ImmAsmOperandClass { let Name = "BrTarget8";  }
def BrTarget24AsmOperand : ImmAsmOperandClass { let Name = "BrTarget24"; }

def RexMEMfiAsmOperand   : AsmOperandClass {
  let Name = "MEMfi";
  let RenderMethod = "addMEMriOperands";
}
def RexMEMffiAsmOperand  : AsmOperandClass {
  let Name = "MEMffi";
  let RenderMethod = "addMEMriOperands";
}

def RexMEMREXrAsmOperand   : AsmOperandClass    { let Name = "MEMREXr";       }
def RexIntRegAsmOperand    : RegAsmOperandClass { let Name = "RexIntReg";     }
def RexIntPairAsmOperand   : RegAsmOperandClass { let Name = "RexIntPairReg"; }
def RexFPRegAsmOperand     : RegAsmOperandClass { let Name = "RexFPReg";      }
def RexDFPRegAsmOperand    : RegAsmOperandClass { let Name = "RexDFPReg";     }
def IntRegAsRexAsmOperand  : RegAsmOperandClass { let Name = "IntReg";        }

// Operands

def IntRegsAsRex : Operand<iPTR> {
  let MIOperandInfo = (ops IntRegs);
  let EncoderMethod = "getRexIntRegEncoding";
  let DecoderMethod = "DecodeRexIntRegsRegisterClass";
  let ParserMatchClass = IntRegAsRexAsmOperand;
}

def RexIntRegsOp : Operand<iPTR> {
  let MIOperandInfo = (ops RexIntRegs);
  let EncoderMethod = "getRexIntRegEncoding";
  let DecoderMethod = "DecodeRexIntRegsRegisterClass";
  let ParserMatchClass = RexIntRegAsmOperand;
}

def RexIntPairOp : Operand<v2i32> {
  let MIOperandInfo = (ops RexIntPair);
  let EncoderMethod = "getRexIntRegEncoding";
  let DecoderMethod = "DecodeRexIntPairRegisterClass";
  let ParserMatchClass = RexIntPairAsmOperand;
}

def RexFPRegsOp : Operand<f32> {
  let MIOperandInfo = (ops RexFPRegs);
  let EncoderMethod = "getRexFPRegEncoding";
  let DecoderMethod = "DecodeRexFPRegsRegisterClass";
  let ParserMatchClass = RexFPRegAsmOperand;
}

def RexDFPRegsOp : Operand<f64> {
  let MIOperandInfo = (ops RexDFPRegs);
  let EncoderMethod = "getRexFPRegEncoding";
  let DecoderMethod = "DecodeRexDFPRegsRegisterClass";
  let ParserMatchClass = RexDFPRegAsmOperand;
}

def ADDRr : Operand<iPTR> {
  let MIOperandInfo = (ops RexIntRegsOp:$rs);
  let ParserMatchClass = RexMEMREXrAsmOperand;
  let EncoderMethod = "getRexIntRegEncoding";
  let DecoderMethod = "DecodeRexIntRegsRegisterClass";
}

class RegImmOperand : Operand<iPTR> {
  let PrintMethod   = "printMemOperand";
  let MIOperandInfo = (ops RexIntRegsOp:$rs, i32imm:$imm5);
  let EncoderMethod = "getADDRfiOpValue";
  let DecoderMethod = "DecodeADDRfi";
}

def ADDRfi  : RegImmOperand { let ParserMatchClass =  RexMEMfiAsmOperand; }
def ADDRffi : RegImmOperand { let ParserMatchClass =  RexMEMffiAsmOperand; }

def Imm3Op   : Operand<iPTR> { let ParserMatchClass = Imm3AsmOperand; }
def Imm5Op   : Operand<iPTR> { let ParserMatchClass = Imm5AsmOperand; }
def SImm5Op  : Operand<iPTR> {
  let ParserMatchClass = SImm5AsmOperand;
  let DecoderMethod = "DecodeSIMM5";
}
def NImm13Op : Operand<iPTR> {
  let ParserMatchClass = NImm13AsmOperand;
  let DecoderMethod = "DecodeSIMM13";
}
def SImm21Op : Operand<iPTR> {
  let ParserMatchClass = SImm21AsmOperand;
  let DecoderMethod = "DecodeSIMM21";
}
def Imm32Op  : Operand<iPTR> { let EncoderMethod = "getImm32OpValue";   }

// Leaf patterns

def imm32  : ImmLeaf<i32, [{}]>;
def simm21 : ImmLeaf<i32, [{ return isInt<21>(Imm);      }]>;
def imm5   : ImmLeaf<i32, [{ return isUInt<5>(Imm);      }]>;
def simm5  : ImmLeaf<i32, [{ return isInt<5>(Imm);       }]>;
def pow2   : ImmLeaf<i32, [{ return isPowerOf2_32(Imm);  }], Log2>;
def npow2  : ImmLeaf<i32, [{ return isPowerOf2_32(~Imm); }], NLog2>;
def masklo : ImmLeaf<i32, [{ return isMask_32(Imm);      }], Log2>;

// Instructions

// Section 6.2.1 - SAVEREX and ADDREX

class EnterREX<string OpcStr, bits<6> op3val>
    : F3<(outs IntRegs:$rd),
         (ins IntRegs:$rs1, NImm13Op : $nimm13 ),
         !strconcat(OpcStr, " $rs1, $nimm13, $rd"), []> {
  bits<13> nimm13;
  let op         = 0b10;
  let op3        = op3val;
  let Inst{13}   = 0;
  let Inst{12-0} = nimm13;
}

def SAVEREX : EnterREX<"saverex", 0b111100>;
def ADDREX :  EnterREX<"addrex",  0b000000>;

// Section 6.3.1 - Branch on Integer Condition Codes - Short

def brtarget8 : Operand<OtherVT> {
  let DecoderMethod = "DecodeShortRexBranch";
  let EncoderMethod = "getRexShortBranchTargetOpValue";
  let ParserMatchClass = BrTarget8AsmOperand;
}

class RexBranch<bit isFloat, dag ins, string asmstr, list<dag> pattern>
    : InstRex16<(outs), ins, asmstr, pattern> {
  bits<8> disp8;
  bits<4> cond;
  let op           = 0b00;
  let Inst{9}      = 0;
  let Inst{8}      = isFloat;
  let Inst{13-10}  = cond;
  let Inst{7-0}    = disp8;
  let isBranch     = 1;
  let isTerminator = 1;
}

let cond = 8, isBarrier = 1 in
def RBA : RexBranch<0, (ins brtarget8:$disp8),
                    "rba\t$disp8",
                    []>;

let Uses = [ICC] in
def RBCOND : RexBranch<0, (ins brtarget8:$disp8, CCOp:$cond),
                       "rb$cond\t$disp8",
                       []>;

// Section 6.3.2 - Branch on Floating-Point Condition Codes - Short

let Uses = [FCC0] in
def RFBCOND : RexBranch<1, (ins brtarget8:$disp8, CCOp:$cond),
                        "rfb$cond\t$disp8",
                        []>;

// Section 6.3.3 - Arithmetic operations - Accumulator with register

// isConvertibleToThreeAddress = 1
let Constraints = "$rd = $rsc" in
class RexArith<string OpcStr, bits<4> rop3, bits<1> rop3l, list<dag> pattern>
    : RexFR<0b10, rop3, rop3l, (outs RexIntRegsOp:$rd),
            (ins RexIntRegsOp:$rs, RexIntRegsOp:$rsc),
            !strconcat(OpcStr, "\t$rs, $rd"), pattern>;

let isCommutable = 1 in {
  def RADD  : RexArith<"radd",  0b0000, 0b0,
                       [(set i32:$rd, (add i32:$rsc, i32:$rs))]>;
  def RAND  : RexArith<"rand",  0b0010, 0b0,
                       [(set i32:$rd, (and i32:$rsc, i32:$rs))]>;
  def ROR   : RexArith<"ror",   0b0100, 0b0,
                       [(set i32:$rd, (or  i32:$rsc, i32:$rs))]>;
  def RXOR  : RexArith<"rxor",  0b0110, 0b0,
                       [(set i32:$rd, (xor i32:$rsc, i32:$rs))]>;
}

def RSUB  : RexArith<"rsub",  0b0001, 0b0,
                     [(set i32:$rd, (sub i32:$rsc, i32:$rs))]>;
def RANDN : RexArith<"randn", 0b1010, 0b0,
                     [(set i32:$rd, (and i32:$rsc, (not i32:$rs)))]>;
def RORN  : RexArith<"rorn",  0b1100, 0b0,
                     [(set i32:$rd, (or  i32:$rsc, (not i32:$rs)))]>;
def RSLL  : RexArith<"rsll",  0b1011, 0b0,
                     [(set i32:$rd, (shl i32:$rsc, i32:$rs))]>;
def RSRL  : RexArith<"rsrl",  0b1101, 0b0,
                     [(set i32:$rd, (srl i32:$rsc, i32:$rs))]>;

let Defs = [ICC] in {
  let isCommutable = 1 in {
    def RADDCC  : RexArith<"raddcc",  0b0000, 0b1,
                           [(set i32:$rd, (addc i32:$rsc, i32:$rs))]>;
    def RANDCC  : RexArith<"randcc",  0b0010, 0b1, []>;
    def RORCC   : RexArith<"rorcc",   0b0100, 0b1, []>;
    def RXORCC  : RexArith<"rxorcc",  0b0110, 0b1, []>;
  }

  def RSUBCC  : RexArith<"rsubcc",  0b0001, 0b1,
                         [(set i32:$rd, (subc i32:$rsc, i32:$rs))]>;
  def RANDNCC : RexArith<"randncc", 0b1010, 0b1, []>;
  def RORNCC  : RexArith<"rorncc",  0b1100, 0b1, []>;
}

// Section 6.3.4 - Arithmetic operations - Accumulator with immediate

let Constraints = "$rd = $rs" in
class RexArithImm<string OpcStr, bits<4> rop3, Operand immop,
                  list<dag> pattern>
    : RexFI<0b10, 0b1, rop3,
            (outs RexIntRegsOp:$rd),
            (ins RexIntRegsOp:$rs, immop:$imm5),
            !strconcat(OpcStr, "\t$imm5, $rd"),
            pattern>;

let Defs = [ICC] in
def RADDCCi : RexArithImm<"raddcc", 0b0000, SImm5Op,
                          [(set i32:$rd, (addc i32:$rs, simm5:$imm5))]>;
def RSLLi   : RexArithImm<"rsll",   0b1011, Imm5Op,
                          [(set i32:$rd, (shl i32:$rs,  imm5:$imm5))]>;
def RSRLi   : RexArithImm<"rsrl",   0b1101, Imm5Op,
                          [(set i32:$rd, (srl i32:$rs,  imm5:$imm5))]>;

// Section 6.3.5 - Comparison with register

let Defs = [ICC], isCompare = 1 in
def RCMPrr : RexFR<0b10, 0b1011, 0b1,
                   (outs), (ins RexIntRegsOp:$rs, RexIntRegsOp:$rd),
                   "rcmp\t$rd, $rs",
                   []>;

// Section 6.3.6 - Comparison with immediate

let Defs = [ICC], isCompare = 1 in
def RCMPri : RexFI<0b10, 0b1, 0b1000,
                   (outs), (ins RexIntRegsOp:$rd, SImm5Op:$imm5),
                   "rcmp\t$rd, $imm5",
                   []>;

// Section 6.3.7 - Constant assignment

class RexConstant<string OpcStr, bits<4> op3val,
                  Operand immop, list<dag> pattern>
    : RexFI<0b10, 0b1, op3val,
            (outs RexIntRegsOp:$rd),
            (ins immop:$imm5 ),
            !strconcat(OpcStr, "\t$imm5, $rd"),
            pattern>;

def RSET5 : RexConstant<"rset5", 0b0001, SImm5Op,
                        [(set i32:$rd, (i32 simm5:$imm5))]>;
def RONE  : RexConstant<"rone",  0b0101, Imm5Op,
                        [(set i32:$rd, (i32 pow2:$imm5))]>;

// Section 6.3.8 - Bit-mask operations

let Constraints = "$rd = $rs" in
class RexBitmask<string OpcStr, bits<4> op3val, list<dag> pattern>
    : RexFI<0b10, 0b1, op3val, (outs RexIntRegsOp:$rd),
            (ins Imm5Op:$imm5, RexIntRegsOp:$rs ),
            !strconcat(OpcStr, "\t$imm5, $rd"), pattern>;

def RSETBIT  : RexBitmask<"rsetbit", 0b0100,
                          [(set i32:$rd, (or i32:$rs, pow2:$imm5))]>;
def RCLRBIT  : RexBitmask<"rclrbit", 0b1010,
                          [(set i32:$rd, (and i32:$rs, npow2:$imm5))]>;
def RINVBIT  : RexBitmask<"rinvbit", 0b0110,
                          [(set i32:$rd, (xor i32:$rs, pow2:$imm5))]>;
def RMASKLO  : RexBitmask<"rmasklo", 0b0010,
                          [(set i32:$rd, (and i32:$rs, masklo:$imm5))]>;

let Defs = [ICC] in
def RTSTBIT : RexFI<0b10, 0b1, 0b0011, (outs), (ins Imm5Op:$imm5,
                    RexIntRegsOp:$rd),
                    "rtstbit\t$imm5, $rd",
                    []>;

// Section 6.3.9 - Register to register copy

def RMOV : InstRex16<(outs IntRegsAsRex:$rd),
                     (ins IntRegsAsRex:$rs),
                     "rmov\t$rs, $rd",
                     []> {
  bits<5> rd;
  bits<5> rs;
  let op            = 0b10;
  let Inst{13-10}   = rd{3-0};
  let Inst{9}       = 0;
  let Inst{8-6}     = 0b100;
  let Inst{5}       = rd{4};
  let Inst{4}       = rs{4};
  let Inst{3-0}     = rs{3-0};
}

// Section 6.3.10 - Negation

let Constraints = "$rs = $rd" in
class RexNeg<string OpcStr, bits<5> rop4val, list<dag> pattern>
    : RexFO<0b10, 0b1, 0b1110, rop4val,
            (outs RexIntRegsOp:$rd),
            (ins RexIntRegsOp:$rs),
            !strconcat(OpcStr, "\t$rd"),
            pattern>;

def RNEG : RexNeg<"rneg", 0b00100, [(set i32:$rd, (sub (i32 0), i32:$rs))]>;
def RNOT : RexNeg<"rnot", 0b00101, [(set i32:$rd, (not i32:$rs))]>;

// Section 6.3.11 - Return instructions

class RetInst<string asmstr, bits<5> rop4>
    : RexFO<0b10, 0b1, 0b1110, rop4, (outs), (ins Imm5Op:$offset),
            asmstr, []> {
  let rd           = 0b0000;
  let isReturn     = 1;
  let isTerminator = 1;
  let isBarrier    = 1;
}

def RRETREST : RetInst<"rretrest", 0b00000>;
def RRETL    : RetInst<"rretl", 0b00001>;

// Section 6.3.12 and 6.3.14 - Load/Store 8/16/32/64-bit - One register
// With or without auto-incrementing

multiclass RexStore<string OpcStr, bits<4> rop3val, SDPatternOperator OpNode,
                    Operand RegOp, ValueType Ty> {

def r:     RexFR<0b11, rop3val, 0,
                 (outs), (ins ADDRr:$rs, RegOp:$rd),
                 !strconcat(OpcStr, "\t$rd, [$rs]"),
                 []>;

let Constraints = "$rs = $rd2" in
def inc : RexFR<0b11, rop3val, 1,
                (outs ADDRr:$rd2), (ins ADDRr:$rs, RegOp:$rd ),
                !strconcat(OpcStr, "inc\t$rd, [$rs]"),
                []>;
}

defm RST   : RexStore<"rst",   0b1000, store,         RexIntRegsOp, i32>;
defm RSTB  : RexStore<"rstb",  0b1010, truncstorei8,  RexIntRegsOp, i32>;
defm RSTH  : RexStore<"rsth",  0b1100, truncstorei16, RexIntRegsOp, i32>;
defm RSTD  : RexStore<"rstd",  0b1110, store,         RexIntPairOp, v2i32>;
defm RSTF  : RexStore<"rstf",  0b1001, store,         RexFPRegsOp,  f32>;
defm RSTDF : RexStore<"rstdf", 0b1101, store,         RexDFPRegsOp, f64>;

multiclass RexLoad<string OpcStr, bits<4> rop3val, SDPatternOperator OpNode,
                   Operand RegOp, ValueType Ty> {

def r:      RexFR<0b11, rop3val, 0,
                  (outs RegOp:$rd), (ins ADDRr:$rs),
                  !strconcat(OpcStr, "\t[$rs], $rd"),
                  []>;

let Constraints = "$rs = $rd2" in
def inc :  RexFR<0b11, rop3val, 1,
                 (outs RegOp:$rd, ADDRr:$rd2), (ins ADDRr:$rs),
                 !strconcat(OpcStr, "inc\t[$rs], $rd"),
                 []>;
}

defm RLD   : RexLoad<"rld",   0b0000, load,          RexIntRegsOp, i32>;
defm RLDUB : RexLoad<"rldub", 0b0010, zextloadi8,    RexIntRegsOp, i32>;
defm RLDUH : RexLoad<"rlduh", 0b0100, zextloadi16,   RexIntRegsOp, i32>;
defm RLDD  : RexLoad<"rldd",  0b0110, load,          RexIntPairOp, v2i32>;
defm RLDF  : RexLoad<"rldf",  0b0001, load,          RexFPRegsOp,  f32>;
defm RLDDF : RexLoad<"rlddf", 0b0101, load,          RexDFPRegsOp, f64>;

// Section 6.3.13 - Load/Store 32-bit - Fixed register plus immediate

class RexStoreFI<string OpcStr, bits<1> isFloat,
                 Operand RegImmOp, Operand RegOp>
    : RexFF<1, isFloat, (outs), (ins RegImmOp:$addr, RegOp:$rd), OpcStr, []>;

let mayStore = 1 in {
def RSTfi  : RexStoreFI<"rst\t$rd, [$addr]", 0, ADDRfi, RexIntRegsOp>;
def RSTFfi : RexStoreFI<"rstf\t$rd, [$addr]", 1, ADDRffi, RexFPRegsOp>;
}

class RexLoadFI<string OpcStr, bits<1> isFloat,
                Operand RegImmOp, Operand RegOp>
    : RexFF<0, isFloat, (outs RegOp:$rd), (ins RegImmOp:$addr), OpcStr, []>;

let mayLoad = 1 in {
def RLDfi  : RexLoadFI<"rld\t[$addr], $rd",  0, ADDRfi, RexIntRegsOp>;
def RLDFfi : RexLoadFI<"rldf\t[$addr], $rd", 1, ADDRffi, RexFPRegsOp>;
}

// Section 6.3.15 - Miscellaneous operations

class RexMisc<string OpStr, bits<5> rop4val, dag outs, dag ins> :
              RexFO<0b10, 0b1, 0b1110, rop4val, outs, ins, OpStr, []>;

let Defs = [O6], Uses = [O6] in {
def RPUSH : RexMisc<"rpush\t$rd", 0b00010, (outs), (ins RexIntRegsOp:$rd)>;
def RPOP  : RexMisc<"rpop\t$rd",  0b00011, (outs RexIntRegsOp:$rd), (ins)>;
}

def RTA   : RexMisc<"rta\t$rd",   0b00110, (outs), (ins Imm3Op:$rd)>;

let rd = 0b0000 in
def RLEAVE : RexMisc<"rleave", 0b00111, (outs), (ins)>;

def RGETPC : RexMisc<"rgetpc\t$rd", 0b01001, (outs RexIntRegsOp:$rd), (ins)>;

def RNOP : InstRex16<(outs), (ins), "nop", []> {
    let op = 0b00;
    let Inst{13-0} = 0;
}

// Section 6.4.1 - Branch on integer condition codes - Long

def brtarget24 : Operand<OtherVT> {
  let DecoderMethod = "DecodeLongRexBranch";
  let EncoderMethod = "getRexLongBranchTargetOpValue";
  let ParserMatchClass = BrTarget24AsmOperand;
}

class RexLongBranch<bit isFloat, dag ins, string asmstr, list<dag> pattern>
    : InstRex32<(outs), ins, asmstr, pattern> {
  bits<24> disp24;
  bits<4> cond;
  let op           = 0b00;
  let Inst{25}     = 1;
  let Inst{24}     = isFloat;
  let Inst{29-26}  = cond;
  let Inst{23-16}  = disp24{7-0};
  let Inst{15-0}   = disp24{23-8};
  let isBranch     = 1;
  let isTerminator = 1;
}

let cond = 8, isBarrier = 1 in
def RBAL : RexLongBranch<0, (ins brtarget24:$disp24),
                         "rba,l\t$disp24",
                         []>;

let Uses = [ICC] in
def RLBCOND : RexLongBranch<0, (ins brtarget24:$disp24, CCOp:$cond),
                            "rb$cond,l\t$disp24",
                            []>;

// Section 6.4.2 - Branch on floating-point condition codes - Long

let Uses = [FCC0] in
def RFLBCOND : RexLongBranch<1, (ins brtarget24:$disp24, CCOp:$cond),
                             "rfb$cond,l\t$disp24",
                             []>;

// Section 6.4.3 - Call and link

let isTerminator = 1, isBarrier = 1, isBranch = 1, isIndirectBranch = 1,
    rd = 8, simm13 = 1, isCodeGenOnly = 1, Predicates = [IsREX],
    AddedComplexity = 60 in
def BINDr : F3_2<2, 0b111000,
                 (outs), (ins IntRegs:$rs1),
                 "rjmp\t$rs1",
                 []>;

// Section 6.4.4 - Constant assignment

def RSET21 : InstRex32<(outs RexIntRegsOp:$rd),
                       (ins SImm21Op:$imm21 ),
                       "rset21\t$imm21, $rd",
                       [(set i32:$rd, (i32 simm21:$imm21))]> {
  bits<4> rd;
  bits<21> imm21;
  let op          = 0b10;
  let Inst{29-26} = rd;
  let Inst{25}    = 1;
  let Inst{24-21} = 0b0111;
  let Inst{20-16} = imm21{4-0};
  let Inst{15-0}  = imm21{20-5};
}

// Section 6.4.5 - Generic format 3 SPARC operations

// Section 6.4.6 - Floating point operations

// Section 6.5.1 - Set 32-bit constant

def RSET32   : InstRex48<"rset32\t$imm32, $rd",    0b01000,
                         Imm32Op, RexIntRegsOp,
                         [(set i32:$rd, (i32 imm32:$imm32))]>;
def RSET32PC : InstRex48<"rset32pc $imm32, $rd",  0b01001,
                         Imm32Op, RexIntRegsOp,
                         []>;

// Section 6.5.2 - Load from 32-bit address

def RLD32    : InstRex48<"rld32\t[$imm32], $rd",   0b01010,
                         Imm32Op, RexIntRegsOp,
                         []>;
def RLD32PC  : InstRex48<"rld32pc\t[$imm32], $rd", 0b01011,
                         Imm32Op,RexIntRegsOp,
                         []>;

// Patterns

let Predicates = [IsREX], AddedComplexity = 60 in {

def : Pat<(add i32:$rs, simm5:$imm5), (RADDCCi i32:$rs, simm5:$imm5)>;

}
