#pragma once

#include "L3.hpp"
#include "visitor.hpp"
#include <sstream>

namespace L3 {
  class CodeGenerator : public Visitor {};


  class NaiveGenerator : CodeGenerator {
    public:
      NaiveGenerator() {}
      NaiveGenerator(Function* f);
      virtual ~NaiveGenerator() {}
      std::string get_L2Code();
    private:
      std::stringstream code;
      static int label_counter;
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
      void visit(BranchInst* inst) override;
      void visit(CallInst* inst) override;
      void visit(Function* f) override;
  };
}