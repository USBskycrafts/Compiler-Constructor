#pragma once
#include <cstdint>
#pragma GCC optimize("O3","unroll-loops", "inline")
#include <climits>
#include <vector>
#include <map>
#include <string>

#include "L3.hpp"

namespace L3 {
  class Tile;

  class Node {
    public:
      virtual unsigned GetCost();
      virtual std::string GetCode();
      virtual std::vector<std::string> GetGeneratedCode();
      virtual std::vector<Node*> GetChildren();
      //    1. able to parsed 2. the children to continue parse 
      virtual void AddChild(Node* child);
      virtual void ReplaceChild(Node* old_node, Node* new_node);
      void parse();
    protected:
      Node* root = this;
      std::string code_;
      std::vector<Node*> children;
      unsigned cost = INT_MAX / 2;
      std::vector<std::string> generated_code;
  };

  class VariableNode : public Node {
    public:
      VariableNode() = delete;
      VariableNode(std::string variable);
  };

  class ValueNode : public Node {
    public:
      ValueNode() = delete;
      ValueNode(std::string item);
  };

  class NumberNode : public Node {
    public:
      NumberNode() = delete;
      NumberNode(std::string number);
  };

  class OperatorNode : public Node {
    public:
      OperatorNode() = delete;
      OperatorNode(std::string op);
  };

  class Tile {
    public:
      static Tile* GetTile(const std::string& name);
      static std::vector<Tile*> getAllTile();
      virtual unsigned GetCost();
      virtual std::pair<bool, std::vector<Node*>> accept(Node* root) = 0;
      virtual std::string GenerateCode(Node* root) = 0;
    protected:
      unsigned cost;
    private:
      static std::map<std::string, Tile*> tile_mapper;
  };

  class AssignTile : public Tile {
    public:
      AssignTile();
      std::pair<bool, std::vector<Node*>> accept(Node* root) override;
      std::string GenerateCode(Node* root) override;
  };

  class LoadTile : public Tile {
    public:
      LoadTile();
      std::pair<bool, std::vector<Node*>> accept(Node* root) override;
      std::string GenerateCode(Node* root) override;
  };

  class LoadWithOffsetTile : public Tile {
    public:
      LoadWithOffsetTile();
      std::pair<bool, std::vector<Node*>> accept(Node* root) override;
      std::string GenerateCode(Node* root) override;
  };

  class StoreTile : public Tile {
    public:
      StoreTile();
      std::pair<bool, std::vector<Node*>> accept(Node* root) override;
      std::string GenerateCode(Node* root) override;
  };

  class StoreWithOffsetTile : public Tile {
    public:
      StoreWithOffsetTile();
      std::pair<bool, std::vector<Node*>> accept(Node* root) override;
      std::string GenerateCode(Node* root) override;
  };

  class ReturnTile : public Tile {
    public:
      ReturnTile();
      std::pair<bool, std::vector<Node*>> accept(Node* root) override;
      std::string GenerateCode(Node* root) override;
  };

  class BranchTile : public Tile {
    public:
      BranchTile();
      std::pair<bool, std::vector<Node*>> accept(Node* root) override;
      std::string GenerateCode(Node* root) override;
  };

  class LabelTile : public Tile {
    public:
      LabelTile();
      std::pair<bool, std::vector<Node*>> accept(Node* root) override;
      std::string GenerateCode(Node* root) override;
  };

  class CmpTile : public Tile {
    public:
      CmpTile();
      std::pair<bool, std::vector<Node*>> accept(Node* root) override;
      std::string GenerateCode(Node* root) override;
  };

  class BinaryOpTile : public Tile {
    public:
      BinaryOpTile();
      std::pair<bool, std::vector<Node*>> accept(Node* root) override;
      std::string GenerateCode(Node* root) override;
  };

  class CallTile : public Tile {
    public:
      CallTile();
      std::pair<bool, std::vector<Node*>> accept(Node* root) override;
      std::string GenerateCode(Node* root) override;
    private:
      static unsigned count;
  };

  class LEALeftTile : public Tile {
    public:
      LEALeftTile();
      std::pair<bool, std::vector<Node*>> accept(Node* root) override;
      std::string GenerateCode(Node* root) override;
  };

  class LEARightTile : public Tile {
    public:
      LEARightTile();
      std::pair<bool, std::vector<Node*>> accept(Node* root) override;
      std::string GenerateCode(Node* root) override;
  };
}