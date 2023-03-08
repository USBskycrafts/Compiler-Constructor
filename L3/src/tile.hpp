#pragma once
#include <map>
#include <vector>

#include "L3.hpp"

namespace Tiling {

  class Node {
    public:
      explicit Node(const std::string& code) : code_(std::move(code)) {}
      std::string GetCode() { return code_; }
      std::vector<Node*> GetChildren() { return children_; }
      void InsertChildren(Node* child) { children_.emplace_back(child); }
    private:
      std::string code_;
      std::vector<Node*> children_;
  };

  class Tree {
    public:
      Tree(Node* root) : root_(root) {}
      std::vector<Node*> GetLeaves();
      void SetGeneratedCode(Node* node, std::string code) { codes_[node].emplace_back(code); }
      std::vector<std::string> GetRootCode() { return codes_[root_.get()]; }
      Node* GetRoot() { return root_.get(); }
    private:
      std::map<Node*,  std::vector<std::string>> codes_;
      std::shared_ptr<Node> root_;
  };

  struct Context {
    std::map<Tree*, std::vector<std::string>> generated_codes;
    std::vector<Tree*> trees;
    //used in generate context
    std::map<L3::Instruction*, Tree*> tree_mapper;
    L3::Function* function;
  };

  class Tile {

  };

  void GenerateContext(L3::Function* function);

  void MergingTree(Context* context);
}