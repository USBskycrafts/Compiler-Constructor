#pragma once

#include <memory>
#include <map>
#include "L3.hpp"
#include "visitor.hpp"

namespace LivenessAnalysis {

  struct AnalysisResult {
    std::set<L3::Variable> ins, outs, gens, kills;
  };

  class ControlFlowGenerator : public L3::Visitor {
    public:
      ControlFlowGenerator(L3::Function* function) : function_(function), N(function->instructions.size()) {}
      void GenerateSuccessors();
      void GeneratePredecessors();
      //visit generate successors
      virtual void visit(L3::Item* item) override;
      virtual void visit(L3::Number* item) override;
      virtual void visit(L3::FunctionName* item) override;
      virtual void visit(L3::Label* item) override;
      virtual void visit(L3::Variable* item) override;
      virtual void visit(L3::Operator* item) override;
      virtual void visit(L3::Instruction* inst) override;
      virtual void visit(L3::CallInst* inst) override;
      virtual void visit(L3::AssignInst* inst) override;
      virtual void visit(L3::BinaryOperator* inst) override;
      virtual void visit(L3::LoadInst* inst) override;
      virtual void visit(L3::StoreInst* inst) override;
      virtual void visit(L3::ReturnInst* inst) override;
      virtual void visit(L3::BranchInst* inst) override;
      virtual void visit(L3::LabelInst* inst) override;
    private:
      std::shared_ptr<L3::Function> function_;
      unsigned i = 0;
      const unsigned N ;
  };

  void SetControlFlow(L3::Function* function);

  AnalysisResult* GenerateInAndOut(L3::Function* function);
}