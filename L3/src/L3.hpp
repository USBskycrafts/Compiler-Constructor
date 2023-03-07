#pragma once

#include <string>
#include <memory>
#include <set>
#include <vector>

namespace L3 {

  class Visitor;
  class Item {
    public:
      virtual void accept(Visitor* visitor);
      const std::string& GetCode() { return code_; }
      void SetCode(const std::string& code) { code_ = std::move(code); }
    private:
      std::string code_;
  };

  class Number : public Item {
    public:
      explicit Number(const std::string& code) { SetCode(std::move(code)); }
      void accept(Visitor* visitor) override;
  };

  class FunctionName : public Item {
    public:
      explicit FunctionName(const std::string& code) { SetCode(std::move(code)); }
      void accept(Visitor* visitor) override;
  };

  class Label : public Item {
    public:
      explicit Label(const std::string& code) { SetCode(std::move(code)); }
      void accept(Visitor* visitor) override;
  };

  class Variable : public Item {
    public:
      explicit Variable(const std::string& code) { SetCode(std::move(code)); }
      bool operator<(Variable& other) { return this->GetCode() < other.GetCode(); }
      void accept(Visitor* visitor) override;
  };

  class Operator : public Item {
    public:
      explicit Operator(const std::string& code) { SetCode(std::move(code)); }
      void accept(Visitor* visitor) override;
  };

  class Instruction {
    public:
      virtual void accept(Visitor* visitor);
      virtual std::vector<Variable> GetDefs() = 0;
      virtual std::vector<Variable> GetUses() = 0;

      void InsertSuccessor(std::shared_ptr<Instruction> suc) { successors_.push_back(suc); }
      std::vector<std::weak_ptr<Instruction>> GetSuccessors() { return successors_; }
      void InsertPredecessor(std::shared_ptr<Instruction> suc) { predecessors_.push_back(suc); }
      std::vector<std::weak_ptr<Instruction>> GetPredecessors() { return predecessors_; }
    private:
      std::vector<std::weak_ptr<Instruction>> successors_;
      std::vector<std::weak_ptr<Instruction>> predecessors_;
  };

  class AssignInst : public Instruction {
    public:
      explicit AssignInst(std::shared_ptr<Variable> dst, std::shared_ptr<Variable> src) 
          : dst_(dst), src_(src) {}
      std::vector<Variable> GetDefs() override;
      std::vector<Variable> GetUses() override;
    private:
      std::shared_ptr<Variable> dst_;
      std::shared_ptr<Item> src_;
  };

  class BinaryOperator : public Instruction {
    public:
      explicit BinaryOperator(Variable* dst, Item* operand1, Operator* op, Item* operand2) 
          : dst_(dst), operand1_(operand1), op_(op), operand2_(operand2) {}
      std::vector<Variable> GetDefs() override;
      std::vector<Variable> GetUses() override;
    private:
      std::shared_ptr<Variable> dst_;
      std::shared_ptr<Item> operand1_, operand2_;
      std::shared_ptr<Operator> op_;
  };

  class LoadInst : public Instruction {
    public:
      explicit LoadInst(Variable* dst, Variable* src) : dst_(dst), src_(src) {}
      std::vector<Variable> GetDefs() override;
      std::vector<Variable> GetUses() override;
    private:
      std::shared_ptr<Variable> dst_;
      std::shared_ptr<Variable> src_;
  };

  class StoreInst : public Instruction {
    public:
      explicit StoreInst(Variable* dst, Item* src) : dst_(dst), src_(src) {}
      std::vector<Variable> GetDefs() override;
      std::vector<Variable> GetUses() override;
    private:
      std::shared_ptr<Variable> dst_;
      std::shared_ptr<Item> src_;
  };

  class ReturnInst : public Instruction {
    public:
      explicit ReturnInst(Item* ret) : ret_(ret) {}
      std::vector<Variable> GetDefs() override;
      std::vector<Variable> GetUses() override;
    private:
      std::shared_ptr<Item> ret_;
  };

  class LabelInst : public Instruction {
    public:
      explicit LabelInst(Label* label) : label_(label) {}
      std::string GetLabelName() { return label_->GetCode(); }
      std::vector<Variable> GetDefs() { return {}; }
      std::vector<Variable> GetUses() { return {}; }
    private:
      std::shared_ptr<Label> label_;
  };

  class BranchInst : public Instruction {
    public:
      explicit BranchInst(Item* mark, Label* label) : mark_(mark), label_(label) {}
      std::vector<Variable> GetDefs() override;
      std::vector<Variable> GetUses() override;
      std::string GetLabelName() { return label_->GetCode(); }
    private:
      std::shared_ptr<Item> mark_;
      std::shared_ptr<Label> label_;
  };

  class CallInst : public Instruction {
    public:
      explicit CallInst(Variable* ret, Item* callee, std::vector<Item*> args)
          : ret_(ret), callee_(callee) {
        for(auto arg : args) {
          args_.push_back(std::shared_ptr<Item>(arg));
        }
      }

      std::string GetCalleeName() { return callee_->GetCode(); }
      std::vector<Variable> GetDefs() override;
      std::vector<Variable> GetUses() override;
    private:
      std::shared_ptr<Variable> ret_;
      std::shared_ptr<Item> callee_;
      std::vector<std::shared_ptr<Item>> args_;
  };

  struct Function {
    std::string name;
    std::vector<std::shared_ptr<Item>> args;
    std::vector<std::shared_ptr<Instruction>> instructions;
  };

  struct Program {
    std::vector<std::shared_ptr<Function>> functions;
  };
}