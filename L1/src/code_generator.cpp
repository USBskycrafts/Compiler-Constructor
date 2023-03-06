#include <string>
#include <iostream>
#include <fstream>

#include "code_generator.h"

using namespace std;

namespace L1{
  void generate_code(Program p){

    /* 
     * Open the output file.
     */ 
    std::ofstream outputFile;
    outputFile.open("prog.S");
   
    /* 
     * Generate target code
     */ 
    //TODO
    const std::string reg_push = 
    ".text\n"
    " .globl go\n"
    "go:\n"
    " pushq %rbx\n"
    " pushq %rbp\n"
    " pushq %r12\n"
    " pushq %r13\n"
    " pushq %r14\n"
    " pushq %r15\n";

    const std::string reg_pop = 
    " popq %r15\n"
    " popq %r14\n"
    " popq %r13\n"
    " popq %r12\n"
    " popq %rbp\n"
    " popq %rbx\n"
    " retq\n";

    auto l = p.entryPointLabel;
    l[0] = '_';
    outputFile << reg_push << " call " + l + "\n";
    outputFile << reg_pop;
    for(auto f : p.functions) {
      outputFile << generate_function_code(f);
    }

    /* 
     * Close the output file.
     */ 
    outputFile.close();
   
    return;
  }

  std::string generate_function_code(Function *f) {
    std::string function_code;

    auto l = f->name;
    l[0] = '_';
    function_code += l + ":\n";
    auto rsp_sub = f->locals * 8;
    if(rsp_sub != 0) {
      function_code += " subq $" + to_string(rsp_sub) + ", %rsp\n";
    }
    for(auto inst : f->instructions) {
      if(inst->inst_type == Instruction_type::ret) {
        auto rsp_add = (max(0L, f->arguments - 6) + f->locals) * 8;
        if(rsp_add != 0) {
          function_code += " addq $" + to_string(rsp_add) + ", %rsp\n";
        }
      }
      function_code += " " + generate_instruction_code(inst) + "\n";
    }
    return function_code;
  }

  std::string generate_instruction_code(Instruction *inst) {
    std::string inst_code;
    switch (inst->inst_type) {
    case ret:
    {
      auto inst_ret = (Instruction_ret*)inst;
      inst_code = "retq";
      break;
    }
    case assign:
    {
      auto inst_assign = (Instruction_assignment*)inst;
      if(inst_assign->s->item_type == cmp) {
        auto c = (Cmp_operation*)inst_assign->s;
        inst_code = generate_component(c);
        if(c->t1->item_type == reg) {
          inst_code += "\n";
          if(c->op->op == "<") {
            inst_code += " setl ";
          } else if(c->op->op == "<=") {
            inst_code += " setle ";
          } else if(c->op->op == "=") {
            inst_code += " sete ";
          }
          inst_code += "%" + register_q2b((Register*)(inst_assign->d)) + "\n";
          inst_code += " movzbq %" + register_q2b((Register*)(inst_assign->d)) + ", " + generate_component(inst_assign->d);
        } else if(c->t2->item_type == reg) {
          inst_code += "\n";
          if(c->op->op == "<") {
            inst_code += " setg ";
          } else if(c->op->op == "<=") {
            inst_code += " setge ";
          } else if(c->op->op == "=") {
            inst_code += " sete ";
          }
          inst_code += "%" + register_q2b((Register*)(inst_assign->d)) + "\n";
          inst_code += " movzbq %" + register_q2b((Register*)(inst_assign->d)) + ", " + generate_component(inst_assign->d);
        } else {
          auto n1 = (Number*)c->t1, n2 = (Number*)c->t2;
          if(n1->value < n2->value && c->op->op == "<"
          || n1->value <= n2-> value && c->op->op == "<="
          || n1->value == n2->value && c->op->op == "="
          ) {
            inst_code += "movq $1, " + generate_component(inst_assign->d);
          } else {
            inst_code += "movq $0, " + generate_component(inst_assign->d);
          }
        }
        break;
      }
      inst_code = "movq ";
      if(inst_assign->s->item_type == function_name) {
        inst_code += "$";
      }
      inst_code += generate_component(inst_assign->s) + ", " 
                  + generate_component(inst_assign->d);
      break;
    }
    case inst_label:
    {
      auto inst_l = (Instruction_label*)inst;
      auto l_name = inst_l->l->name;
      l_name[0] = '_';
      inst_code =  l_name + ":";
      break;
    }
    case arithmetic_operation:
    {
      auto inst_a = (Instruction_arithmetic_operation*)inst;
      if(inst_a->op->op == "<<=" || inst_a->op->op == ">>=") {
        if(inst_a->t2->item_type == reg) {
          auto r = (Register*)inst_a->t2;
          inst_code = generate_component(inst_a->op) + " " 
          + "%"  + register_q2b(r) + ", " + generate_component(inst_a->t1);
        } else {
          inst_code = generate_component(inst_a->op) + " " + generate_component(inst_a->t2) + ", "
          + generate_component(inst_a->t1);
        }
        break;
      }
      inst_code = generate_component(inst_a->op)
       + " " + generate_component(inst_a->t2) + ", " + generate_component(inst_a->t1);
      break;
    }
    case cjump:
    {
      auto inst_cjump = (Instruction_conditional_jump_operation*)inst;
      auto c = inst_cjump->cmp;
      inst_code = generate_component(c) + "\n ";
      if(c->t1->item_type == reg) {
        if(c->op->op == "<=") {
          inst_code += "jle";
        } else if(c->op->op == "<") {
          inst_code += "jl";
        } else if(c->op->op == "=") {
          inst_code += "je";
        }
        auto l_name = inst_cjump->l->name;
        l_name[0] = '_';
        inst_code += " " + l_name;
      } else if(c->t2->item_type == reg) {
        if(c->op->op == "<=") {
          inst_code += "jge";
        } else if(c->op->op == "<") {
          inst_code += "jg";
        } else if(c->op->op == "=") {
          inst_code += "je";
        }
        auto l_name = inst_cjump->l->name;
        l_name[0] = '_';
        inst_code += " " + l_name;
      } else {
        auto n1 = (Number*)c->t1, n2 = (Number*)c->t2;
        if(n1->value < n2->value && c->op->op == "<"
        || n1->value <= n2->value && c->op->op == "<=" 
        || n1->value == n2->value && c->op->op == "="
        ) {
          auto l_name = inst_cjump->l->name;
          l_name[0] = '_';
          inst_code += " jmp " + l_name;
        } else {
          inst_code += "";
        }
      }
      break;
    }
    case jump:
    {
      auto inst_jmp = (Instruction_jump_operation*)inst;
      auto l_name = inst_jmp->l->name;
      l_name[0] = '_';
      inst_code = "jmp " + l_name;
      break;
    }
    case self_operation:
    {
      auto inst_self = (Instruction_self_operation*)inst;
      inst_code = generate_component(inst_self->op) + " " + generate_component(inst_self->r);
      break;
    }
    case call:
    {
      auto inst_c = (Instruction_call*)inst;
      auto rsp_sub = max(0LL, inst_c->val->value - 6) * 8 + 8;
      auto call_addr = inst_c->name;
      if(call_addr->item_type == reg) {

        inst_code = "subq $" + to_string(rsp_sub) + ", %rsp\n";
        inst_code += "jmp *" + generate_component(call_addr);

      } else {
        auto f_name = (Function_name*)(call_addr);
        auto func_name = f_name->name;
        if(func_name[0] == '@') {
          inst_code = "subq $" + to_string(rsp_sub) + ", %rsp\n";
          func_name[0] = '_';
          inst_code += " jmp " + func_name;
        } else {
          if(func_name == "print") {
            inst_code += "call print";
          } else if(func_name == "allocate") {
            inst_code += "call allocate";
          } else if(func_name == "input") {
            inst_code += "call input";
          } else if(func_name == "tensor-error") {
            switch(inst_c->val->value) 
            {
              case 1:
                inst_code += "call array_tensor_error_null";
                break;
              case 3:
                inst_code += "call array_error";
                break;
              case 4:
                inst_code += "call tensor_error";
              default:
                break;
            }
          }
        }
      }
      break;
    }
    case leaq:
    {
      auto inst_l = (Instruction_leaq_operation*)inst;

      inst_code = "leaq (" + generate_component(inst_l->r2) + ", " +
                  generate_component(inst_l->r3) + ", " + to_string(inst_l->n->value) +
                  + "), " + generate_component(inst_l->r1);
      break;
    }
    default:
      std::cerr << "instruction generate error occur" << std::endl;
      exit(-1);
    }
    return inst_code;
  }

  std::string generate_component(Item *t) {
    std::string comp_code;
    switch (t->item_type) {
      case reg:
      {
        auto r = (Register*)t;
        comp_code = "%" + r->name;
        break;
      }
      case memory_location:
      {
        auto m = (Memory_address*)t;
        comp_code = to_string(m->value->value) + "(" + generate_component(m->reg) + ")";
        break;
      }
      case number:
      {
        auto n = (Number*)t;
        comp_code = "$" + to_string(n->value);
        break;
      }
      case label:
      {
        auto l = (Label*)t;
        auto label_name = l->name;
        label_name[0] = '_';
        comp_code = "$" + label_name;
        break;
      }
      case cmp:
      {
        auto c = (Cmp_operation*)t;
        auto t1 = c->t1, t2 = c->t2;
        if(t1->item_type == reg) {
          comp_code = "cmpq " + generate_component(t2) + ", " + generate_component(t1);
        } else if(t2->item_type == reg) {
          comp_code = "cmpq " + generate_component(t1) + ", " + generate_component(t2);
        } else {
          comp_code = "";
        }
        break;
      }
      case op:
      {
        auto o = (Operator*)t;
        if(o->op == "+=") {
          comp_code = "addq";
        } else if(o->op == "-=") {
          comp_code = "subq";
        } else if(o->op == "*=") {
          comp_code = "imulq";
        } else if(o->op == "&=") {
          comp_code = "andq";
        } else if(o->op == "<<=") {
          comp_code = "salq";
        } else if(o->op == ">>=") {
          comp_code = "sarq";
        } else if(o->op == "++") {
          comp_code = "inc";
        } else if(o->op == "--") {
          comp_code = "dec";
        } else {
          std::cerr << "undefined operator of "  << o->op << std::endl;
          exit(-1);
        }
        break;
      }
      case function_name:
      {
        auto n = (Function_name*)t;
        auto l = n->name;
        if(l[0] == '@') {
          l[0]  = '_';
        }
        comp_code = l;
        break;
      }
      default:
        std::cerr << "component generate error occur" << std::endl;
        std::cerr << "item type is " << t->item_type << std::endl;
        exit(-1);
    }
    return comp_code;
  }

  std::string register_q2b(Register* reg) {
    std::string rb;
    auto reg_name = reg->name;
    if(reg_name[1] >= '0' && reg_name[1] <= '9') {
      rb = reg_name + "b";
    } else if(reg_name[2] == 'x') {
      rb = reg_name.substr(1, 2);
      rb[1] = 'l';
    } else {
      rb = reg_name.substr(1, 2) + "l";
    }
    return rb;
  }
}
