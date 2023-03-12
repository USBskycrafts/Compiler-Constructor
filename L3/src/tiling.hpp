#pragma once

#include <vector>
#include <unordered_map>
#include <set>
#include "L3.hpp"
#include "visitor.hpp"

namespace Tiling {

  struct Context {
    std::vector<L3::Instruction*> instructions;
  };

  struct VariableComparator {
    bool operator()(const L3::Variable* v1, const L3::Variable* v2) const {
      return v1->name < v2->name;
    }
  };

  struct Node {
    std::string code;
    std::vector<Node*> children;
    std::vector<std::string> generated_code;
  };

  using VariableSet = std::set<L3::Variable*, VariableComparator>;

  struct Tree {
    VariableSet defs, uses, ins, outs;
    Node* root;
  };

  class TreeGenerator : public L3::Visitor {
    public:
      void visit(L3::Item* item) override;
      void visit(L3::Variable* var) override;
      void visit(L3::Number* num) override;
      void visit(L3::Operator* op) override;
      void visit(L3::Label* label) override;
      void visit(L3::FunctionName* name) override;
      void visit(L3::Instruction* inst) override;
      void visit(L3::AssignInst* inst) override;
      void visit(L3::BinaryOperator* inst) override;
      void visit(L3::ReturnInst* inst) override;
      void visit(L3::LabelInst* inst) override;
      void visit(L3::BranchInst* inst) override;
      void visit(L3::LoadInst* inst) override;
      void visit(L3::StoreInst* inst) override;
      void visit(L3::CallInst* inst) override;
      Tree* tree;
  };

  std::vector<Context> GenerateContext(L3::Function* function);

  std::vector<Tree*> GenerateTree(Context context);

  Tree* GenerateTree(L3::Instruction* inst);

  std::vector<Tree*> MergingTree(std::vector<Tree*> trees);

  class Tile {
    public:
      virtual std::vector<Node*> Matches(Node* node) = 0;
      virtual std::vector<std::string> GenerateCode(Node* node) = 0;
  };

  class AssignTile : public Tile {
    public:
      std::vector<Node*> Matches(Node* node) override;
      std::vector<std::string> GenerateCode(Node* node) override;
  };

  class BinaryTile : public Tile {
    public:
      std::vector<Node*> Matches(Node* node) override;
      std::vector<std::string> GenerateCode(Node* node) override;
  };
}