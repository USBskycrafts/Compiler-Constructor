#pragma once


#include <map>
#pragma GCC optimize("O3","unroll-loops")
#include "L2.hpp"
#include "visitor.hpp"


namespace L2 {
  class ControlFlowGraph : Visitor {
    public:
      ControlFlowGraph(Function* f);
      std::map<Instruction*, std::set<Instruction*>> getSuccessors();
      std::map<Instruction*, std::set<Instruction*>> getPredecessors();
    private:
      Function* f_;
      std::map<Instruction*, std::set<Instruction*>> successors, predecessors;
      std::vector<Instruction*> parsed_instructions;

      void visit(Item* item) override;
      void visit(Value* value) override;
      void visit(Register* reg) override;
      void visit(Variable* var) override;
      void visit(MemoryAddress* address) override;
      void visit(Comparison* cmp) override;
      void visit(Number* num) override;
      void visit(Operator* op) override;
      void visit(Label* label) override;
      void visit(FunctionName* name) override;

      void visit(Instruction* inst) override;
      void visit(InstructionArithmetic* inst) override;
      void visit(InstructionCall* inst) override;
      void visit(InstructionConditionalJump* inst) override;
      void visit(InstructionJump* inst) override;
      void visit(InstructionLeaq* inst) override;
      void visit(InstructionSelfOperation* inst) override;
      void visit(InstructionAssignment* inst) override;
      void visit(InstructionLabel* inst) override;
      void visit(InstructionReturn* inst) override;
      void visit(InstructionStackArg* inst) override;
      void visit(Function* f) override;
  };
}