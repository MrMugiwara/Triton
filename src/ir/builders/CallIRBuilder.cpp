#include <iostream>
#include <sstream>
#include <stdexcept>

#include "CallIRBuilder.h"
#include "Registers.h"
#include "SMT2Lib.h"
#include "SymbolicElement.h"


CallIRBuilder::CallIRBuilder(uint64_t address, const std::string &disassembly):
  BaseIRBuilder(address, disassembly) {
}


static SymbolicElement *alignStack(AnalysisProcessor &ap, uint64_t writeSize)
{
  SymbolicElement     *se;
  std::stringstream   expr, op1, op2;
  uint64_t            symReg = ap.getRegSymbolicID(ID_RSP);

  /*
   * Create the SMT semantic.
   */
  if (symReg != UNSET)
    op1 << "#" << std::dec << symReg;
  else
    op1 << smt2lib::bv(ap.getRegisterValue(ID_RSP), writeSize * REG_SIZE);

  op2 << smt2lib::bv(REG_SIZE, writeSize * REG_SIZE);

  expr << smt2lib::bvsub(op1.str(), op2.str());

  /* Create the symbolic element */
  se = ap.createRegSE(expr, ID_RSP, "Aligns stack");

  /* Apply the taint */
  se->isTainted = ap.isRegTainted(ID_RSP);

  return se;
}


void CallIRBuilder::reg(AnalysisProcessor &ap, Inst &inst) const {
  std::cout << "TODO" << std::endl;
  //OneOperandTemplate::stop(this->disas);
}


void CallIRBuilder::imm(AnalysisProcessor &ap, Inst &inst) const {
  std::cout << "TODO" << std::endl;
  //OneOperandTemplate::stop(this->disas);
}


void CallIRBuilder::mem(AnalysisProcessor &ap, Inst &inst) const {
  SymbolicElement   *se;
  std::stringstream expr1;//, expr2;
  //uint64_t          imm       = std::get<1>(this->operands[1]);
  uint64_t          memDst    = std::get<1>(this->operands[0]); // The dst memory write
  uint32_t          writeSize = std::get<2>(this->operands[0]);

  /* Create the SMT semantic side effect */
  inst.addElement(alignStack(ap, writeSize));

  /* Create the SMT semantic */
  /* *RSP =  Next_RIP */
  expr1 << smt2lib::bv(this->nextAddress, writeSize * REG_SIZE);

  /* Create the symbolic element */
  se = ap.createMemSE(expr1, memDst, "Saved RIP");

  /* Apply the taint */
  ap.assignmentSpreadTaintMemImm(se, memDst, writeSize);

  /* Add the symbolic element to the current inst */
  inst.addElement(se);

// TODO: How really works XED_CALL ??
//
//  /* Create the SMT semantic */
//  /* RIP = imm */
//  expr2 << smt2lib::bv(imm, writeSize * REG_SIZE);
//
//  /* Create the symbolic element */
//  se = ap.createRegSE(expr2, ID_RIP, "RIP");
//
//  /* Apply the taint */
//  ap.assignmentSpreadTaintRegImm(se, ID_RIP);
//
//  /* Add the symbolic element to the current inst */
//  inst.addElement(se);
}


void CallIRBuilder::none(AnalysisProcessor &ap, Inst &inst) const {
  OneOperandTemplate::stop(this->disas);
}


Inst *CallIRBuilder::process(AnalysisProcessor &ap) const {
  this->checkSetup();

  Inst *inst = new Inst(ap.getThreadID(), this->address, this->disas);

  try {
    this->templateMethod(ap, *inst, this->operands, "CALL");
    ap.incNumberOfExpressions(inst->numberOfElements()); /* Used for statistics */
  }
  catch (std::exception &e) {
    delete inst;
    throw;
  }

  return inst;
}

