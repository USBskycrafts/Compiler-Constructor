#pragma once

#include <vector>
#include <string>

namespace L1 {

  enum Item_type {
    reg = 0,
    number,
    call_keyword,
    label,
    function_name,
    lib_function_name,
    const_name,
    memory_location,
    op,
    cmp
  };

  class Item {
    public:
      Item_type item_type;
  };

  class Register : public Item {
    public:
      Register(const std::string reg_name);
      const std::string name;
  };

  class Number : public Item {
    public:
      Number(const long long num);
      const long long value;
  };

  class Memory_address : public Item {
    public:
      Memory_address(Register *reg, Number *num);
      Register *reg;
      Number *value;
  };

  class Operator : public Item {
    public:
      Operator(const std::string op);
      const std::string op;
  };

  class Label : public Item {
    public:
      Label(const std::string label_name);
      const std::string name;
  };


  class Function_name : public Item {
    public:
      Function_name(const std::string function_name);
      const std::string name;
  };


  class Cmp_operation : public Item {
    public:
      Cmp_operation(Item *t1, Operator *op, Item *t2);
      Item *t1, *t2;
      Operator *op;
  };
  /*
   * Instruction interface.
   */

  enum Instruction_type {
    ret = 0,
    call,
    assign,
    inst_label,
    arithmetic_operation,
    self_operation,
    cjump,
    jump,
    leaq
  };

  class Instruction{
    public:
      Instruction_type inst_type;
  };

  /*
   * Instructions.
   */
  class Instruction_ret : public Instruction{
    public:
      Instruction_ret();
  };

  class Instruction_assignment : public Instruction{
    public:
      Instruction_assignment(Item *dst, Item *src);
      Item *s;
      Item *d;
  };

  class Instruction_call : public Instruction {
    public:
      Instruction_call(Item *name, Number *val);
      Item *name;
      Number *val; 
  };

  class Instruction_label : public Instruction {
    public:
      Instruction_label(Label *l);
      Label *l;
  };

  class Instruction_arithmetic_operation : public Instruction {
    public:
      Instruction_arithmetic_operation(Item *t1, Operator *op, Item *t2);
      Item *t1, *t2;
      Operator *op;
  };

  class Instruction_self_operation : public Instruction {
    public:
      Instruction_self_operation(Register *r, Operator *op);
      Register *r;
      Operator *op;
  };

  class Instruction_conditional_jump_operation : public Instruction {
    public:
      Instruction_conditional_jump_operation(Cmp_operation *cmp, Label *l);
      Cmp_operation *cmp;
      Label *l;
  };

  class Instruction_jump_operation : public Instruction {
    public:
      Instruction_jump_operation(Label *l);
      Label *l;
  };

  class Instruction_leaq_operation : public Instruction {
    public:
      Instruction_leaq_operation(Register *r1, Register *r2, Register *r3, Number *n);
      Register *r1, *r2, *r3;
      Number *n;
  };


  /*
   * Function.
   */
  class Function{
    public:
      std::string name;
      int64_t arguments;
      int64_t locals;
      std::vector<Instruction *> instructions;
  };

  class Program{
    public:
      std::string entryPointLabel;
      std::vector<Function *> functions;
  };

}
