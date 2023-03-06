#include "L1.h"

namespace L1 {

  Register::Register (const std::string reg_name)
    : name {reg_name}{
      item_type = Item_type::reg;
  }

  Number::Number(const long long num)
    : value {num} {
      item_type = Item_type::number;
  }

  Memory_address::Memory_address(Register *reg, Number *num)
    : reg {reg}, value {num} {
      item_type = Item_type::memory_location;
  }


  Label::Label(const std::string label_name)
    : name {label_name} {
      item_type = Item_type::label;
  }

  Function_name::Function_name(const std::string function_name) 
    : name {function_name} {
      item_type = Item_type::function_name;
  }


  Operator::Operator(const std::string op)
    : op {op} {
      item_type = Item_type::op; 
  }

  Cmp_operation::Cmp_operation(Item *t1, Operator *op, Item *t2)
    : t1 {t1}, op {op}, t2 {t2} {
     item_type = Item_type::cmp; 
  }

  Instruction_ret::Instruction_ret() {
    inst_type = Instruction_type::ret;
  }

  Instruction_assignment::Instruction_assignment (Item *dst, Item *src)
    : s {src},
      d {dst} {
      inst_type = Instruction_type::assign;
  }

  Instruction_call::Instruction_call (Item *name, Number *num)
    : name {name},
      val {num} {
      inst_type = Instruction_type::call;
  }

  Instruction_label::Instruction_label (Label *l) 
    : l{l} {
      inst_type = Instruction_type::inst_label;
  }

  Instruction_arithmetic_operation::Instruction_arithmetic_operation (Item *t1, Operator *op, Item *t2)
    : t1 {t1}, op {op}, t2 {t2} {
      inst_type = Instruction_type::arithmetic_operation;
  }

  Instruction_self_operation::Instruction_self_operation(Register *r, Operator *op)
    : r {r}, op {op} {
      inst_type = Instruction_type::self_operation;
  }

  Instruction_conditional_jump_operation::Instruction_conditional_jump_operation(Cmp_operation *cmp, Label *l)
    : cmp {cmp}, l {l} {
      inst_type = Instruction_type::cjump; 
  }

  Instruction_jump_operation::Instruction_jump_operation(Label *l)
    : l {l} {
      inst_type = Instruction_type::jump;
  }

  Instruction_leaq_operation::Instruction_leaq_operation(Register *r1, Register *r2, Register *r3, Number *n)
    : r1 {r1}, r2 {r2}, r3 {r3}, n {n} {
      inst_type = Instruction_type::leaq;
  }

}
