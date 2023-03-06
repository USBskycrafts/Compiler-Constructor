#include <iostream>

#include "printer.hpp"
#include "L2.hpp"
#include "utils.hpp"


namespace L2 {


  void Printer::visit(Item* item) {
    item->accept(this);
  }

  void Printer::visit(Value* value) {
    value->accept(this);
  }

  void Printer::visit(Register* reg) {
    std::cout << reg->get_code();
  }

  void Printer::visit(Variable* var) {
    std::cout << var->get_code();
  }

  void Printer::visit(MemoryAddress* address) {
    std::cout << address->get_code();
  }

  void Printer::visit(Comparison* cmp) {
    std::cout << cmp->get_code();
  }

  void Printer::visit(Number* num) {
    std::cout << num->get_code();
  }

  void Printer::visit(Operator* op) {
    std::cout << op->get_code();
  }

  void Printer::visit(Label* label) {
    std::cout << label->get_code();
  }

  void Printer::visit(FunctionName* name) {
    std::cout << name->get_code();
  }

  void Printer::visit(Instruction* inst) {
    inst->accept(this);
  }

  void Printer::visit(InstructionArithmetic* inst) {
    auto item = inst->get_items();
    auto op = inst->get_operator();

    item.first->accept(this);
    std::cout << " ";
    op->accept(this);
    std::cout << " ";
    item.second->accept(this);
    std::cout << std::endl;
  }

  void Printer::visit(InstructionCall* inst) {
    auto u = inst->get_item();
    auto n = inst->get_number();

    std::cout << "call ";
    u->accept(this);
    std::cout << " ";
    n->accept(this);
    std::cout << std::endl;
  }

  void Printer::visit(InstructionConditionalJump* inst) {
    auto cmp = inst->get_comparison();
    auto l = inst->get_label();

    std::cout << "cjump ";
    cmp->accept(this);
    std::cout << " ";
    l->accept(this);
    std::cout << std::endl;
  }

  void Printer::visit(InstructionJump* inst) {
    auto l = inst->get_label();
    std::cout << "goto ";
    l->accept(this);
    std::cout << std::endl;
  }

  void Printer::visit(InstructionLeaq* inst) {
    auto items = inst->get_items();
    auto n = inst->get_number();
    for(int i = 0; i < 3; i++) {
      if(i == 1) {
        std::cout << "@ ";
      }
      items[i]->accept(this);
      std::cout << " ";
    }
    n->accept(this);
    std::cout << std::endl;
  }

  void Printer::visit(InstructionSelfOperation* inst) {
    auto w = inst->get_item();
    auto op = inst->get_operator();
    w->accept(this);
    op->accept(this);
    std::cout << std::endl;
  }

  void Printer::visit(InstructionAssignment* inst) {
    auto item = inst->get_items();
    item.first->accept(this);
    std::cout << " <- ";
    item.second->accept(this);
    std::cout << std::endl;
  }

  void Printer::visit(InstructionLabel* inst) {
    auto l = inst->get_label();
    l->accept(this);
    std::cout << std::endl;
  }

  void Printer::visit(InstructionReturn* inst) {
    std::cout << "return\n";
  }

  void Printer::visit(InstructionStackArg* inst) {
    inst->get_item()->accept(this);
    std::cout << " <- stack-arg ";
    auto n = inst->get_number();
    std::cout << std::endl;
  }

  void Printer::visit(Function* f) {
    std::cout << "(" << f->name << std::endl;
    std::cout << "\t" << f->arguments << " " << f->locals << std::endl;
    for(auto inst : f->instructions) {
      std::cout << '\t';
      inst->accept(this);
    }
    std::cout << ')' << std::endl;
  }

  void Printer::visit(Program* p) {
    std::cout << "(" << p->entryPointLabel << std::endl;
    for(auto f : p->functions) {
      f->accept(this);
    }
    std::cout << ')' << std::endl;
  }

}