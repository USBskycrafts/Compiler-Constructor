#pragma once

#include <string>
#include <vector>

namespace L3 {
  enum class ItemType {
    kVariable,
    kLabel,
    kFunctionName,
    kOperator,
    kNumber
  };

  class Item {
    public:
      std::string name;
      ItemType type;
  };

  class Label : public Item {
    public:
      Label(std::string name);
  };

  class FunctionName : public Item{
    public:
      FunctionName(std::string name);
  };

  class Variable : public Item {
    public:
      Variable(std::string name);
  };

  class Operator : public Item {
    public:
      Operator(std::string name);
  };

  class Number : public Item {
    public:
      Number(std::string name);
  };

  class Instruction {
    public:
      virtual std::vector<Variable*> GetDefs() = 0;
      virtual std::vector<Variable*> GetUses() = 0;
      virtual std::string ToString() = 0;
      std::string type;
  };

  class AssignInst : public Instruction {
    public:
      AssignInst(Variable* lhs, Item* rhs) : lhs(lhs), rhs(rhs) { type = "AssignInst"; }
      std::vector<Variable*> GetDefs() override;
      std::vector<Variable*> GetUses() override;
      std::string ToString() override;
      Variable* lhs;
      Item* rhs;
  };

  class BinaryOperator : public Instruction {
    public:
      BinaryOperator(Variable* lhs, Item* t1, Operator* op, Item* t2);
      std::vector<Variable*> GetDefs() override;
      std::vector<Variable*> GetUses() override;
      std::string ToString() override;
      Variable* lhs;
      std::tuple<Item*, Operator*, Item*> rhs;
  };

  class ReturnInst : public Instruction {
    public:
      ReturnInst(Item* ret_val) : ret_val(ret_val) { type = "ReturnInst"; }
      std::vector<Variable*> GetDefs() override;
      std::vector<Variable*> GetUses() override;
      std::string ToString() override;
      Item* ret_val;
  };

  class LabelInst : public Instruction {
    public:
      LabelInst(Label* label) : label(label) { type = "LabelInst"; }
      std::vector<Variable*> GetDefs() override;
      std::vector<Variable*> GetUses() override;
      std::string ToString() override;
      Label* label;
  };

  class BranchInst : public Instruction {
    public:
      BranchInst(Item* t, Label* label) : t(t), label(label) { type = "BranchInst"; }
      std::vector<Variable*> GetDefs() override;
      std::vector<Variable*> GetUses() override;
      std::string ToString() override;
      Item* t;
      Label* label;
  };

  class LoadInst : public Instruction {
    public:
      LoadInst(Variable* lhs, Variable* rhs) : lhs(lhs), rhs(rhs) { type = "LoadInst"; }
      std::vector<Variable*> GetDefs() override;
      std::vector<Variable*> GetUses() override;
      std::string ToString() override;
      Variable* lhs;
      Variable* rhs;
  };

  class StoreInst : public Instruction {
    public:
      StoreInst(Variable* lhs, Item* rhs) : lhs(lhs), rhs(rhs) { type = "StoreInst"; }
      std::vector<Variable*> GetDefs() override;
      std::vector<Variable*> GetUses() override;
      std::string ToString() override;
      Variable* lhs;
      Item* rhs;
  };

  class CallInst : public Instruction {
    public:
      CallInst(Variable* lhs, Item* name, std::vector<Item*> args)
          : lhs(lhs), callee(name), args(args) { type = "CallInst"; }
      std::vector<Variable*> GetDefs() override;
      std::vector<Variable*> GetUses() override;
      std::string ToString() override;
      Variable* lhs;
      Item* callee;
      std::vector<Item*> args;
  };

  class Function {
    public:
      std::vector<Instruction*> instructions;
      FunctionName* name;
      std::vector<Item*> args;
  };


  class Program {
    public:
      std::vector<Function*> functions;
  };
}