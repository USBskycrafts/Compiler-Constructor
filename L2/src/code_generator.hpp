#pragma once

#include "L2.hpp"
#include "color_selector.hpp"

namespace L2 {
  class CodeGenerator : Visitor {
    public:
      CodeGenerator(Function* f);
      Function* getFunction();
    private:
      Function* newF;
      Function* f;
      std::vector<Item*> parsed_items;
      ColorSelector* selector;
      Function* generate();

      void visit(Item* item);
      void visit(Value* value);
      void visit(Register* reg);
      void visit(Variable* var);
      void visit(MemoryAddress* address);
      void visit(Comparison* cmp);
      void visit(Number* num);
      void visit(Operator* op);
      void visit(Label* label);
      void visit(FunctionName* name);
      void visit(Instruction* inst);
      void visit(InstructionArithmetic* inst);
      void visit(InstructionCall* inst);
      void visit(InstructionConditionalJump* inst);
      void visit(InstructionJump* inst);
      void visit(InstructionLeaq* inst);
      void visit(InstructionSelfOperation* inst);
      void visit(InstructionAssignment* inst);
      void visit(InstructionLabel* inst);
      void visit(InstructionReturn* inst);
      void visit(InstructionStackArg* inst);
      void visit(Function* f);
  };
}