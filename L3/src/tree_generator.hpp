#pragma once
#ifndef DEBUG
#pragma GCC optimize("O3", "inline")
#endif


#include <stack>
#include "L3.hpp"
#include "context_generator.hpp"
#include "tree.hpp"

namespace L3 {
  class TreeGenerator : Visitor {
    public:
      TreeGenerator() = delete;
      TreeGenerator(std::vector<Instruction*> context, std::map<Instruction*, std::set<Variable*>> out_set_mapper);
      std::vector<Node*> GetTrees();
      std::string CodeGenerate();
    private:
      void GenerateTrees();
      void MergeTrees();
      std::vector<Instruction*> context_;
      std::vector<Node*> trees_;
      std::stack<Node*> parsed_nodes;
      std::map<Instruction*, std::set<Variable*>> out_set_mapper_, gen_set_mapper, kill_set_mapper_;
      std::map<Node*, std::set<Variable*>> node_defs_mapper, node_uses_mapper;
      std::map<Node*, std::set<Instruction*>> instructions_mapper_;


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