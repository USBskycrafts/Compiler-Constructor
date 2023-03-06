#pragma once

#include "L2.hpp"


namespace L2 {
  class Visitor {
    public:
      virtual void visit(Item* item) = 0;
      virtual void visit(Value* value) = 0;
      virtual void visit(Register* reg) = 0;
      virtual void visit(Variable* var) = 0;
      virtual void visit(MemoryAddress* address) = 0;
      virtual void visit(Comparison* cmp) = 0;
      virtual void visit(Number* num) = 0;
      virtual void visit(Operator* op) = 0;
      virtual void visit(Label* label) = 0;
      virtual void visit(FunctionName* name) = 0;

      virtual void visit(Instruction* inst) = 0;
      virtual void visit(InstructionArithmetic* inst) = 0;
      virtual void visit(InstructionCall* inst) = 0;
      virtual void visit(InstructionConditionalJump* inst) = 0;
      virtual void visit(InstructionJump* inst) = 0;
      virtual void visit(InstructionLeaq* inst) = 0;
      virtual void visit(InstructionSelfOperation* inst) = 0;
      virtual void visit(InstructionAssignment* inst) = 0;
      virtual void visit(InstructionLabel* inst) = 0;
      virtual void visit(InstructionReturn* inst) = 0;
      virtual void visit(InstructionStackArg* inst) = 0;
      virtual void visit(Function* f) = 0;
  };
}