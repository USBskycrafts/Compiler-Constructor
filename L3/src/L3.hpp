#pragma once


#include <string>
#include <set>
#include <vector>
#include <map>


namespace L3 {
  class Visitor;
  class Element {
    public:
      virtual void accept(Visitor* visitor) = 0;
  };

  class Item : Element {
    public:
      virtual ~Item() {}
      virtual std::string to_string() = 0;
      template<typename T> static std::string to_string(T* item);
      void accept(Visitor* visitor) override;
    protected:
      std::string symbols;
  };

  class Variable : public Item {
    public:
      virtual ~Variable() {}
      Variable() {}
      std::string to_string() override;
      static Variable* get_variable(const std::string& name);
      void accept(Visitor* visitor) override;
    private:
      static std::map<std::string, Variable*> variable_mapper;
  };


  class Operator : public Item {
    public:
      std::string to_string() override;
      static Operator* get_operator(const std::string& name);
      void accept(Visitor* visitor) override;
    private:
      static std::map<std::string, Operator*> op_mapper;
  };

  class Label : public Item {
    public:
      std::string to_string() override;
      static Label* get_label(const std::string& name);
      void accept(Visitor* visitor) override;
      //static void re_label();
      static int re_labeled_count;
    private:
      static std::map<std::string, Label*> label_mapper;
  };

  class Number : public Item {
    public:
      std::string to_string() override;
      static Number* get_number(const std::string& name);
      void accept(Visitor* visitor) override;
    private:
      static std::map<std::string, Number*> number_mapper;
  };

  class FunctionName : public Item {
    public:
      std::string to_string() override;
      static FunctionName* get_name(const std::string &name);
      void accept(Visitor* visitor) override;
    private:
      static std::map<std::string, FunctionName*> function_name_mapper;
  };


  class Instruction : public Element {
    public:
      virtual ~Instruction() {}
      virtual std::string to_string() = 0;
      virtual bool has_item(Item* item);
      //every time replace should generate a new Instruction
      virtual std::set<Item*> get_defs();
      virtual std::set<Item*> get_uses();
      virtual std::vector<Item*> get_items();
      void accept(Visitor* visitor) override;
    protected:
      template<typename T> static T* copy(const T* inst);
      template<typename T> static T* replace(const T* inst, Item* old_item, Item* new_item);
    protected:
      std::vector<Item*> items;
      std::set<Item*> defs, uses;
  };


  class AssignInst : public Instruction {
    public:
      AssignInst() {}
      AssignInst(Variable* l_value, Item* r_value);
      AssignInst* operator=(const AssignInst* inst);
      AssignInst* replace(Item* old_item, Item* new_item) const;
      std::string to_string() override;
      void accept(Visitor* visitor) override;
  };

  class StoreInst : public Instruction {
    public:
      StoreInst() {}
      StoreInst(Variable* l_value, Item* r_value);
      StoreInst* operator=(const StoreInst* inst);
      StoreInst* replace(Item* old_item, Item* new_item) const;
      std::string to_string() override;
      void accept(Visitor* visitor) override;
  };

  class LoadInst : public Instruction {
    public:
      LoadInst() {}
      LoadInst(Variable* l_value, Variable* r_value);
      LoadInst* operator=(const LoadInst* inst);
      LoadInst* replace(Item* old_item, Item* new_item) const;
      std::string to_string() override;
      void accept(Visitor* visitor) override;
  };

  class BinaryOperator : public Instruction {
    public:
      BinaryOperator() {}
      BinaryOperator(Variable* l_value, Item* i1, Operator* op, Item* i2);
      BinaryOperator* operator=(const BinaryOperator* inst);
      BinaryOperator* replace(Item* old_item, Item* new_item) const;
      std::string to_string() override;
      void accept(Visitor* visitor) override;
  };

  class CmpInst : public Instruction {
    public:
      CmpInst() {}
      CmpInst(Variable* l_value, Item* i1, Operator* op, Item* i2);
      CmpInst* operator=(const CmpInst* inst);
      CmpInst* replace(Item* old_item, Item* new_item) const;
      std::string to_string() override;
      void accept(Visitor* visitor) override;
  };

  class LabelInst : public Instruction {
    public:
      LabelInst() {}
      LabelInst(Label* label);
      LabelInst* operator=(const LabelInst* inst);
      LabelInst* replace(Item* old_item, Item* new_item) const;
      std::string to_string() override;
      void accept(Visitor* visitor) override;
  };

  class CallInst : public Instruction {
    public:
      CallInst() {}
      CallInst(Variable* ret_value, Item* callee, std::vector<Item*> arguments);
      CallInst* operator=(const CallInst* inst);
      CallInst* replace(Item* old_item, Item* new_item) const;
      std::string to_string() override;
      std::vector<Item*> get_arguments();
      Variable* get_ret_value();
      Item* get_callee();
      static std::vector<Item*> parsed_args;
      static Variable* parsed_ret_value;
      static Item* parsed_callee;
      void accept(Visitor* visitor) override;
    private:
      std::vector<Item*> arguments;
      Variable* ret_value;
      Item* callee;
  };

  class BranchInst : public Instruction {
    public:
      BranchInst() {}
      BranchInst(Item* flag, Label* label);
      BranchInst* operator=(const BranchInst* inst);
      BranchInst* replace(Item* old_item, Item* new_item) const;
      std::string to_string() override;
      void accept(Visitor* visitor) override;
  };

  class ReturnInst : public Instruction {
    public:
      ReturnInst() {}
      ReturnInst(Variable* variable);
      ReturnInst* operator=(const ReturnInst* inst);
      ReturnInst* replace(Item* old_item, Item* new_item) const;
      std::string to_string() override;
      void accept(Visitor* visitor) override;
  };

  class Function : Element {
    public: 
      Function* operator=(const Function* f);
      std::string to_string();
      void set_name(const std::string& name);
      void add_argument(Variable* arg);
      void add_instruction(Instruction* instruction);
      std::string get_name();
      std::vector<Variable*> get_arguments();
      std::set<Variable*> get_locals();
      std::vector<Instruction*> get_instructions();
      void accept(Visitor* visitor) override;
      void add_local();
    private:
      std::string name;
      std::vector<Variable*>arguments;
      std::set<Variable*> locals;
      std::vector<Instruction*> instructions;

  };

  class Program {
    public:
      std::vector<Function*> get_functions();
      void set_function(Function* f);
    private:
      std::vector<Function*> functions;
  };
}