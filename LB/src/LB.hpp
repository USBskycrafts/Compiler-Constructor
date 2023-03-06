#pragma once


#include <string>
#include <vector>
#include <map>

namespace LB {

  class Item;
  class Variable;
  class Scope;
  class Function;
  class Program;

  
  class Item {
    public:
      virtual ~Item() = default;
      std::string GetCode() { return code_; }
      void SetCode(std::string code) { code_ = code; }
    private:
      std::string code_;
  };

  class Number : public Item {
    public:
      explicit Number(std::string code) { SetCode(code); }
  };

  class Label : public Item {
    public:
      explicit Label(std::string code) { SetCode(code); }
  };

  class Operator : public Item {
    public:
      explicit Operator(std::string code) { SetCode(code); }
  };

  class FunctionName : public Item {
    public:
      explicit FunctionName(std::string code) { SetCode(code); }
  };

  class Variable : public Item {
    public:
      virtual ~Variable() = default;
      Scope* GetScope() { return scope_; }
      void SetScope(Scope* scope) { scope_ = scope; }
      std::string GetType() { return type_; }
      void SetType(std::string type) {type_ = type; }
    private:
      Scope* scope_;
      std::string type_;
  };

  class Int64 : public Variable {
    public:
      explicit Int64(std::string code) {
        SetCode(code);
        SetType("int64");
      }
  };

  class Code : public Variable {
    public:
      explicit Code(std::string code);
  };

  class Array : public Variable {
    public:
      explicit Array(std::string code, const unsigned dim);
      std::vector<Item*> GetIndexes() { return indexes_; }
      void InsertIndex(Item* index) { indexes_.emplace_back(index); }
    private:
      std::vector<Item*> indexes_;
  };

  class Tuple : public Variable {
    public:
      explicit Tuple(std::string code);
  };

  class Comparator {
    public:
      explicit Comparator(Item* t1, Operator* cmp, Item* t2) : t1_(t1), t2_(t2), cmp_(cmp) {}
      std::string GetCode();
    private:
      Item* t1_;
      Item* t2_;
      Operator* cmp_;
  };

  class WhileStatement;

  class Instruction {
    public:
      virtual std::string GenerateCode() = 0;
      virtual std::string ToString() = 0;
      Scope* GetScope() { return scope_; }
      void SetScope(Scope* scope) { scope_ = scope; }
      WhileStatement* GetLoop() { return loop_; }
      void SetLoop(WhileStatement* loop) { loop_ = loop; }
    private:
      Scope* scope_;
      WhileStatement* loop_;
  };

  class Scope : public Instruction {
    public:
      std::string GenerateCode() override;
      std::string ToString() override;
      std::string GetUniqueName(std::string name) { return name_mapper_.at(name); }
      void SetUniqueName(std::string name, std::string u_name) { 
        if(name_mapper_.count(name)) {
          abort();
        }
        name_mapper_[name] = u_name; 
      }
      Variable* GetVar(std::string var_name) { return vars_mapper_.at(var_name); }
      void SetVarName(std::string name, Variable* var) {
        if(vars_mapper_.count(name)) {
          abort();
        }
        vars_mapper_[name] = var;
      }
      void InsertInstructions(Instruction* inst) { instructions_.emplace_back(inst); }
      std::vector<Instruction*> GetInstructions() { return instructions_; }
    private:
      std::map<std::string, std::string> name_mapper_;
      std::vector<Instruction* > instructions_;
      std::map<std::string, Variable*> vars_mapper_;
  };

  class DeclareInst : public Instruction {
    public:
      explicit DeclareInst(std::vector<Variable*> vars) : vars_(vars) {}
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      std::vector<Variable*> vars_;
  };

  class AssignInst : public Instruction {
    public:
      AssignInst(Variable* lhs, Item* rhs) : lhs_(lhs), rhs_(rhs) {}
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      Variable* lhs_;
      Item* rhs_;
  };

  class BinaryOperator : public Instruction {
    public:
      BinaryOperator(Variable* lhs, Item* t1, Operator* op, Item* t2)
          : lhs_(lhs), t1_(t1), op_(op), t2_(t2) {}
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      Variable* lhs_;
      Item* t1_;
      Operator* op_;
      Item* t2_;
  };

  class LabelInst : public Instruction {
    public:
      explicit LabelInst(Label* label) : label_(label) {}
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      Label* label_;
  };

  class IfStatement : public Instruction {
    public:
      IfStatement(Comparator* cmp, Label* t_label, Label* f_label)
          : cmp_(cmp), true_label_(t_label), false_label_(f_label) {}
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      Comparator* cmp_;
      Label* true_label_;
      Label* false_label_;
  };

  class JumpInst : public Instruction {
    public:
      explicit JumpInst(Label* label) : label_(label) {}
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      Label* label_;
  };

  class ReturnInst : public Instruction {
    public:
      explicit ReturnInst(Item* ret) : ret_(ret) {}
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      Item* ret_;
  };

  class WhileStatement : public Instruction {
    public:
      WhileStatement(Comparator* cmp, Label* t_label, Label* f_label, Label* c_label)
        : cmp_(cmp), true_label_(t_label), false_label_(f_label), cond_label_(c_label) {}
      std::string GenerateCode() override;
      std::string ToString() override;

      std::string GetTrueLabel() { return true_label_->GetCode(); }
      std::string GetFalseLabel() { return false_label_->GetCode(); }
      std::string GetCondLabel() { return cond_label_->GetCode(); }
    private:
      Comparator* cmp_;
      Label* true_label_;
      Label* false_label_;
      Label* cond_label_;
  };


  class Continue : public Instruction {
    public:
      std::string GenerateCode() override;
      std::string ToString() override;
  };

  class Break : public Instruction {
    public:
      std::string GenerateCode() override;
      std::string ToString() override;
  };

  class LoadContainer : public Instruction {
    public:
      LoadContainer(Variable* lhs, Variable* rhs, std::vector<Item*> indexes)
          : lhs_(lhs), rhs_(rhs), indexes_(indexes) {}
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      Variable* lhs_;
      Variable* rhs_;
      std::vector<Item*> indexes_;
  };

  class StoreContainer : public Instruction {
    public:
      StoreContainer(Variable* lhs, std::vector<Item*> indexes, Item* rhs)
          : lhs_(lhs), rhs_(rhs), indexes_(indexes) {}
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      Variable* lhs_;
      Item* rhs_;
      std::vector<Item*> indexes_;
  };

  class GetLength : public Instruction {
    public:
      GetLength(Variable* lhs, Variable* rhs, Item* index)
          : lhs_(lhs), rhs_(rhs), index_(index) {}
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      Variable* lhs_;
      Variable* rhs_;
      Item* index_;
  };

  class CallInst : public Instruction {
    public:
      CallInst(Variable* ret, FunctionName* callee, std::vector<Item*> args)
          : ret_(ret), callee_(callee), args_(args) {}
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      Variable* ret_;
      FunctionName* callee_;
      std::vector<Item*> args_;
  };

  class AllocateArray : public Instruction {
    public:
      AllocateArray(Array* lhs, std::vector<Item*> args)
          : lhs_(lhs), args_(args) {}
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      Array* lhs_;
      std::vector<Item*> args_;
  };

  class AllocateTuple : public Instruction {
    public:
      AllocateTuple(Tuple* lhs, Item* index)
          : lhs_(lhs), index_(index) {}
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      Tuple* lhs_;
      Item* index_;
  };

  struct Function {
    std::string type, callee;
    std::vector<Variable*> args;
    Scope* scope;
  };

  struct Program {
    std::vector<Function*> functions;
  };
  

}