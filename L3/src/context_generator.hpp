#pragma once
#include <vector>

#include "L3.hpp"
#include "visitor.hpp"

namespace L3 {
  class ContextGenerator : public Visitor {
    public:
      ContextGenerator() {}
      ContextGenerator(Function* f);
      virtual ~ContextGenerator() {}
      std::vector<std::vector<Instruction*>> get_contexts();
    private:
      void generate_contexts(Function* f);
      std::vector<std::vector<Instruction*>> contexts;

      void visit(Item* item) override;
      void visit(Variable* var) override;
      void visit(Number* num) override;
      void visit(Operator* op) override;
      void visit(Label* label) override;
      void visit(FunctionName* name) override;

      void visit(Instruction* inst) override;
      void visit(AssignInst* inst) override;
      void visit(StoreInst* inst) override;
      void visit(LoadInst* inst) override;
      void visit(BinaryOperator* inst) override;
      void visit(CmpInst* inst) override;
      void visit(LabelInst* inst) override;
      void visit(ReturnInst* inst) override;
      void visit(CallInst* inst) override;
      void visit(BranchInst* inst) override;
      void visit(Function* f) override;
  };
}