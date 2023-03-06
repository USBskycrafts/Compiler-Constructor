#pragma once

#include <map>
#include <vector>
#include <memory>

namespace IR {

  class Item;
  class Var;
  class Instruction;
  class BasicBlock;
  class Function;
  class Program;

  class Item {
    public:
      virtual std::string ToString() = 0;
      virtual std::string GetType() = 0;
  };

  class Var : public Item {
    public:
      virtual std::shared_ptr<Function> GetFunction();
      virtual void SetFunction(std::shared_ptr<Function> parent);
    protected:
      std::shared_ptr<Function> parent_;
  };

  

  class Code : public Var {
    public:
      Code(std::string code);
      std::string ToString() override;
      std::string GetType() override;
    private:    
      std::string code_;
  };

  class Int64 : public Var {
    public:
      Int64(std::string code);
      std::string ToString() override;
      std::string GetType() override;
    private:
      std::string code_;
  };

  class Tuple : public Var {
    public:
      Tuple(std::string code);
      std::string ToString() override;
      std::string GetType() override;
      void SetLength(std::shared_ptr<Item> length);
    private:
      std::string code_;
      std::shared_ptr<Item> length_;
  };

  class Array : public Var {
    public:
      Array(std::string code, unsigned dims);
      std::string ToString() override;
      std::string GetType() override;
      void SetDim(unsigned index, std::shared_ptr<Item> dim);
    private:
      std::string code_;
      std::vector<std::shared_ptr<Item>> dimensions_;
  };


  class Operator : public Item {
    public:
      Operator(std::string code);
      std::string ToString() override;
      std::string GetType() override;
    private:
      std::string code_;
  };

  class Number : public Item {
    public:
      Number(std::string code);
      std::string ToString() override;
      std::string GetType() override;
    private: 
      std::string code_;
  };

  class Label : public Item {
    public:
      Label(std::string code);
      std::string ToString() override;
      std::string GetType() override;
    private:    
      std::string code_;
  };


  class FunctionName : public Item {
    public:
      FunctionName(std::string code);
      std::string ToString() override;
      std::string GetType() override;
    private:    
      std::string code_;
  };
  


  class Instruction {
    public:
      virtual std::string ToString() = 0;
      virtual std::string GenerateCode() = 0;
      virtual std::shared_ptr<BasicBlock> GetParent();
      virtual void SetParent(std::shared_ptr<BasicBlock> parent);
    protected:
      std::shared_ptr<BasicBlock> parent_;
  };


  class DeclareInst : public Instruction {
    public:
      DeclareInst(std::shared_ptr<Var> variable);
      std::string ToString() override;
      std::string GenerateCode() override;
    private:
      std::shared_ptr<Var> variable_;
  };

  class AssignInst : public Instruction {
    public:
      AssignInst(std::shared_ptr<Var> lhs, std::shared_ptr<Item> rhs);
      std::string ToString() override;
      std::string GenerateCode() override;
    private:
      std::shared_ptr<Var> lhs_;
      std::shared_ptr<Item> rhs_;
  };

  class BinaryOperator : public Instruction {
    public:
      BinaryOperator(std::shared_ptr<Var> lhs,
        std::shared_ptr<Item> operand1,
        std::shared_ptr<Operator> op,
        std::shared_ptr<Item> operand2);
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      std::shared_ptr<Var> lhs_;
      std::shared_ptr<Item> operand1_, operand2_;
      std::shared_ptr<Operator> op_;
  };

  class LoadArray : public Instruction {
    public:
      LoadArray(std::shared_ptr<Var> lhs,
        std::shared_ptr<Array> rhs,
        std::vector<std::shared_ptr<Item>> index);
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      std::shared_ptr<Var> lhs_;
      std::shared_ptr<Array> rhs_;
      std::vector<std::shared_ptr<Item>> index_;
  };

  class StoreArray : public Instruction {
    public:
      StoreArray(std::shared_ptr<Array> lhs,
        std::vector<std::shared_ptr<Item>> index,
        std::shared_ptr<Item> rhs);
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      std::shared_ptr<Array> lhs_;
      std::vector<std::shared_ptr<Item>> index_;
      std::shared_ptr<Item> rhs_;
  };

  class LoadTuple : public Instruction {
    public:
      LoadTuple(std::shared_ptr<Var> lhs,
        std::shared_ptr<Tuple> rhs,
        std::shared_ptr<Item> index);
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      std::shared_ptr<Var> lhs_;
      std::shared_ptr<Tuple> rhs_;
      std::shared_ptr<Item> index_;
  };

  class StoreTuple : public Instruction {
    public:
      StoreTuple(std::shared_ptr<Tuple> lhs,
        std::shared_ptr<Item> index,
        std::shared_ptr<Item> rhs);
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      std::shared_ptr<Tuple> lhs_;
      std::shared_ptr<Item> index_;
      std::shared_ptr<Item> rhs_;
  };

  class ArrayLength : public Instruction {
    public:
      ArrayLength(std::shared_ptr<Var> lhs,
        std::shared_ptr<Array> rhs,
        std::shared_ptr<Item> dim);
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      std::shared_ptr<Var> lhs_;
      std::shared_ptr<Array> rhs_;
      std::shared_ptr<Item> dim_;
  };

  class TupleLength : public Instruction {
    public:
      TupleLength(std::shared_ptr<Var> lhs,
        std::shared_ptr<Tuple> rhs);
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      std::shared_ptr<Var> lhs_;
      std::shared_ptr<Tuple> rhs_;
  };

  class CallInst : public Instruction {
    public:
      CallInst(std::shared_ptr<Var> ret_value,
        std::shared_ptr<Item> f_name,
        std::vector<std::shared_ptr<Item>> operands);
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      std::shared_ptr<Var> ret_value_;
      std::shared_ptr<Item> f_name_;
      std::vector<std::shared_ptr<Item>> operands_;
  };

  class ArrayAllocate : public Instruction {
    public:
      ArrayAllocate(std::shared_ptr<Array> lhs,
        std::vector<std::shared_ptr<Item>> args);
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      std::shared_ptr<Var> lhs_;
      std::vector<std::shared_ptr<Item>> args_;
  };

  class TupleAllocate : public Instruction {
    public:
      TupleAllocate(std::shared_ptr<Tuple> lhs,
        std::shared_ptr<Item> dim);
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      std::shared_ptr<Var> lhs_;
      std::shared_ptr<Item> dim_;
  };

  class ReturnInst : public Instruction {
    public:
      ReturnInst(std::shared_ptr<Item>ret_value);
      std::string GenerateCode() override;
      std::string ToString() override;
    private:
      std::shared_ptr<Item> ret_value_;
  };

  class BranchInst : public Instruction {
    public:
      BranchInst(std::shared_ptr<Item> mark,
        std::vector<std::shared_ptr<Label>> labels);
      std::string GenerateCode() override;
      std::string ToString() override;
      std::vector<std::shared_ptr<Label>> GetSuccessors();
    private:
      std::shared_ptr<Item> mark_;
      std::vector<std::shared_ptr<Label>> labels_;
  };



  struct BasicBlock {
    std::string GenerateCode();
    std::string ToString();
    std::shared_ptr<Function> GetParent();
    void SetParent(std::shared_ptr<Function> parent);
    std::shared_ptr<Label> name;
    std::vector<std::shared_ptr<Instruction>> instructions;
    std::shared_ptr<Instruction> terminator;
    std::shared_ptr<Function> parent;
  };

  struct Function {
    std::string ToString();
    std::string GenerateCode();
    std::shared_ptr<Program> GetParent();
    void SetParent(std::shared_ptr<Function> parent);
    //std::shared_ptr<Item> ret;
    std::string ret;
    std::shared_ptr<FunctionName> function_name;
    std::vector<std::shared_ptr<Item>> args;
    std::map<std::string, std::shared_ptr<Item>> variables_mapper;
    std::vector<std::shared_ptr<BasicBlock>> basic_blocks;
    std::map<std::string, std::shared_ptr<BasicBlock>> bbs_mapper;
    std::shared_ptr<Program> parent;
  };

  struct Program {
    std::string ToString();
    std::string GenerateCode();
    std::vector<std::shared_ptr<Function>> functions;
  };

  
}