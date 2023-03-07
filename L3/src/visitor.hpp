#pragma once

#include "L3.hpp"

namespace L3 {
  class Visitor {
    public:
      virtual void visit(Item* item) = 0;
      virtual void visit(Number* item) = 0;
      virtual void visit(FunctionName* item) = 0;
      virtual void visit(Label* item) = 0;
      virtual void visit(Variable* item) = 0;
      virtual void visit(Operator* item) = 0;
      virtual void visit(Instruction* inst) = 0;
      virtual void visit(CallInst* inst) = 0;
      virtual void visit(AssignInst* inst) = 0;
      virtual void visit(BinaryOperator* inst) = 0;
      virtual void visit(LoadInst* inst) = 0;
      virtual void visit(StoreInst* inst) = 0;
      virtual void visit(ReturnInst* inst) = 0;
      virtual void visit(BranchInst* inst) = 0;
      virtual void visit(LabelInst* inst) = 0;
  };
} 
