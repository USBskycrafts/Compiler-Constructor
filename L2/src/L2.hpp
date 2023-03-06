#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>



namespace L2 {

  class Visitor;
  class ControlFlowGraph;

  class Element {
    public:
      virtual void accept(Visitor* visitor) = 0;
  };

  class Entry : public Element {
    
  };


  class Item : public Entry {
    public:
      Item(const std::string name);
      Item() {}
      virtual std::string get_code() const;
      enum ItemType {
        kRegister = 0, kVariable, kNumber, kLabel, kFunctionName, kMemoryAddress, kOperator, kComparison
      };
      ItemType get_type() const;
      friend std::ostream& operator<<(std::ostream& stream, const Item *item);
      bool operator==(const Item& other);
      bool operator<(const Item& other);
      void accept(Visitor* visitor) override;

    protected:
      std::string code_;
      ItemType type_;
  };

  class Value : public Item {
    public:
      Value() = default;
      Value(std::string name);
      void accept(Visitor* visitor) override;
  };

  class Register : public Value {
    public:
      static Register* GetInstance(const std::string& name);
      void accept(Visitor* visitor) override;
      static int countRegister();
      static std::vector<Register*> getAllCallerSavedRegisters();
      static std::vector<Register*> getAllCalleeSavedRegisters();
      static std::vector<Register*> getGeneralPurposeRegisters();

      enum RegisterType {
        rdi, rsi, rdx, rcx, r8, r9,
        rax, rbx, rbp, r10, r11, r12, r13, r14, r15,
        rsp
      };  
      RegisterType register_type_;
      const std::unordered_map<std::string, RegisterType> reg_name_mapper = 
      {
        {"rsi", rsi}, {"rdi", rdi}, {"rdx", rdx}, {"rcx", rcx}, {"r8", r8}, {"r9", r9},
        {"rax", rax}, {"rbx", rbx}, {"rbp", rbp}, {"r10", r10}, {"r11", r11}, {"r12", r12},
        {"r13", r13}, {"r14", r14}, {"r15", r15}, {"rsp", rsp}
      };
    private:
      Register() = default;
      Register(const std::string name);
      static std::set<Register*> registers_;
  };

  class Variable : public Value {
    public:
      static Variable* GetInstance(const std::string& name);
      void accept(Visitor* visitor) override;
      static std::set<Variable*> getAll();
      static int countVariable();
    private:
      Variable(const std::string name);
      Variable() = default;
      static std::set<Variable*> variables_;
  };

  class Number : public Item {
    public:
      int64_t get_value() const;
      Number(const std::string value);
      void accept(Visitor* visitor) override;
    private:
      int64_t value_;
  };

  class MemoryAddress : public Item {
    public:
      MemoryAddress(Value* r, Number* n);
      void accept(Visitor* visitor) override;
      Value* get_value() const;
      Number* get_number() const;
    private:
      Value* v_;
      Number* n_;
  };

  class Operator : public Item {
    public:
      Operator(const std::string symbol);
      void accept(Visitor* visitor) override;
  };

  class Label : public Item {
    public:
      Label(const std::string name);
      void accept(Visitor* visitor) override;
  };

  class FunctionName : public Item {
    public:
      FunctionName(const std::string name);
      void accept(Visitor* visitor) override;
  };

  class Comparison : public Item {
    public:
      Comparison(Item* v1, Operator* o, Item* v2);
      std::pair<Item*, Item*> get_items() const;
      Operator* get_operator() const;
      void accept(Visitor* visitor) override;
    private:
      Item* v1_;
      Item* v2_;
      Operator* o_;
  };


  class Instruction : public Entry {
    public:
      enum InstructionType {
        kReturn = 0, kCall, kAssignment, kLabel, kArithmetic,
        kSelfOperation, kConditionalJump, kJump, kLeaq, kStackArg 
      };
      void accept(Visitor* visitor) override;
      InstructionType get_type() const;
      virtual std::ostream& print(std::ostream& stream) = 0;
    protected:
      Instruction();
      InstructionType type_;  
  };

  class InstructionReturn : public Instruction {
    public:
      InstructionReturn();
      std::ostream& print(std::ostream& stream) override;
      void accept(Visitor* visitor) override;
  };

  class InstructionCall : public Instruction {
    public:
      InstructionCall(Item* i, Number* n);
      Item* get_item() const;
      Number* get_number() const;
      std::ostream& print(std::ostream& stream) override;
      void accept(Visitor* visitor) override;
    private:
      Item* i_;
      Number* n_;
  };

  class InstructionAssignment : public Instruction {
    public:
      InstructionAssignment(Item* d, Item* s);
      std::pair<Item*, Item*> get_items() const;
      void accept(Visitor* visitor) override;
      std::ostream& print(std::ostream& stream) override;
    private:
      Item* d_;
      Item* s_;
  };

  class InstructionLabel : public Instruction {
    public:
      InstructionLabel(Label* l);
      Label* get_label() const;
      void accept(Visitor* visitor) override;
      std::ostream& print(std::ostream& stream) override;
    private:
      Label* l_;
  };

  class InstructionArithmetic : public Instruction {
    public:
      InstructionArithmetic(Item* i1, Operator* o, Item* i2);
      std::pair<Item*, Item*> get_items() const;
      Operator* get_operator() const;
      void accept(Visitor* visitor) override;
      std::ostream& print(std::ostream& stream) override;
    private:
      Item* i1_;
      Item* i2_;
      Operator* o_;
  };

  class InstructionSelfOperation : public Instruction {
    public:
      InstructionSelfOperation(Item* i, Operator* o);
      Item* get_item() const;
      Operator* get_operator() const;
      void accept(Visitor* visitor) override;
      std::ostream& print(std::ostream& stream) override;
    private:
      Item* i_;
      Operator *o_;
  };

  class InstructionConditionalJump : public Instruction {
    public:
      InstructionConditionalJump(Comparison* c, Label* l);
      Comparison* get_comparison() const;
      Label* get_label() const;
      void accept(Visitor* visitor) override;
      std::ostream& print(std::ostream& stream) override;
    private:
      Comparison* c_;
      Label* l_;
  };

  class InstructionJump : public Instruction {
    public:
      InstructionJump(Label* l);
      Label* get_label() const;
      void accept(Visitor* visitor) override;
      std::ostream& print(std::ostream& stream) override;
    private:
      Label* l_;
  };

  class InstructionLeaq : public Instruction {
    public:
      InstructionLeaq(Item* i1, Item* i2, Item* i3, Number* n);
      std::vector<Item*> get_items() const;
      Number* get_number() const;
      void accept(Visitor* visitor) override;
      std::ostream& print(std::ostream& stream) override;
    private:
      Item* i1_;
      Item* i2_;
      Item* i3_;
      Number* n_;
  };

  class InstructionStackArg : public Instruction {
    public:
      InstructionStackArg(Item* i, Number* n);
      Number* get_number() const;
      Item* get_item() const;
      void accept(Visitor* visitor) override;
      std::ostream& print(std::ostream& stream) override;
    private:
      Number* n_;
      Item* i_;
  };


  class Function : Entry {
    public:
      Function();
      std::string name;
      int64_t arguments;
      int64_t locals;
      std::vector<Instruction*> instructions;
      void accept(Visitor* visitor) override;
      std::map<Instruction*, std::set<Instruction*>> getSuccessors();
      std::map<Instruction*, std::set<Instruction*>> getPredecessors();
    private:
      ControlFlowGraph* cfg;
  };

  class Program {
    public:
      std::string entryPointLabel;
      std::vector<Function*> functions;
  };
}