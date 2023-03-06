#pragma once
#ifndef DEBUG
#pragma GCC optimize("O3","unroll-loops")
#endif
#include <map>

#include "L3.hpp"
#include "visitor.hpp"

namespace L3 {
  class ControlFlowAnalysis : Visitor {
    public:
      ControlFlowAnalysis() = delete;
      ControlFlowAnalysis(Function* f);
      std::set<Instruction*> GetPredecessors(Instruction* inst);
      std::set<Instruction*> GetSuccessors(Instruction* inst);
    private:
      std::map<Instruction*, std::set<Instruction*>> successor_mapper, predecessor_mapper;
      std::vector<Instruction*> parsed_instructions;
      Function* f_;
      void GenerateSuccessors();
      void GeneratePredecessors();
      void visit(Item* item) override;
      void visit(Variable* var) override;
      void visit(Number* num) override;
      void visit(Operator* op) override;
      void visit(Label* label) override;
      void visit(FunctionName*) override;
      void visit(Instruction* inst) override;
      void visit(AssignInst* inst) override;
      void visit(StoreInst* inst) override;
      void visit(LoadInst* inst) override;
      void visit(BinaryOperator* inst) override;
      void visit(CmpInst* inst) override;
      void visit(LabelInst* inst) override;
      void visit(ReturnInst* inst) override;
      void visit(BranchInst* inst) override;
      void visit(CallInst* inst) override;
      void visit(Function* f) override;
  };
}