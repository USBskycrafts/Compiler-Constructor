

#include "code_generator.hpp"
#include "L2.hpp"
#include "color_selector.hpp"
#include "utils.hpp"

namespace L2 {
  CodeGenerator::CodeGenerator(Function* f)
  : f(f), selector(new ColorSelector(f)) {
    newF = new Function;
    newF->arguments = f->arguments;
    newF->locals = f->locals;
    newF->name = f->name;
    this->generate();
  }

  Function* CodeGenerator::getFunction() {
    return this->newF;
  }


  Function* CodeGenerator::generate() {
    for(auto inst : f->instructions) {
      inst->accept(this);
    }
    return newF;
  }

 
  void CodeGenerator::visit(Item* item) {
    item->accept(this);
  }

  void CodeGenerator::visit(Value* value) {
    value->accept(this);
  }

  void CodeGenerator::visit(Register* reg) {
    parsed_items.push_back(reg);
  }

  void CodeGenerator::visit(Variable* var) {
    auto color_mapper = selector->getColorMap();
    DEBUG_COUT << var->get_code() << " is selecting color" << std::endl;
    if(color_mapper.count(var) == 0) {
      color_mapper[var] = 1;
    }
    auto color = color_mapper.at(var);
    for(auto reg : Register::getGeneralPurposeRegisters()) {
      if(color_mapper.at(reg) == color) {
        parsed_items.push_back(reg);
        return;
      }
    }
    DEBUG_COUT << var->get_code() << "hasn't had a name" << std::endl;
  }

  void CodeGenerator::visit(MemoryAddress* address) {
    auto m = address->get_value();
    m->accept(this);
    m = (Value*)parsed_items.back();
    parsed_items.pop_back();
    parsed_items.push_back(new MemoryAddress(m, address->get_number()));
  }

  void CodeGenerator::visit(Comparison* cmp) {
    auto m = cmp->get_items();
    m.first->accept(this);
    m.second->accept(this);

    auto v2 = parsed_items.back();
    parsed_items.pop_back();
    auto v1 = parsed_items.back();
    parsed_items.pop_back();
    parsed_items.push_back(new Comparison(v1, cmp->get_operator(), v2));
  }

  void CodeGenerator::visit(Number* num) {
    parsed_items.push_back(num);
  }

  void CodeGenerator::visit(Operator* op) {
    parsed_items.push_back(op);
  } 

  void CodeGenerator::visit(Label* label) {
     parsed_items.push_back(label);
  }

  void CodeGenerator::visit(FunctionName* name) {
    parsed_items.push_back(name);
  }

  void CodeGenerator::visit(Instruction* inst) {
    inst->accept(this);
  }

  void CodeGenerator::visit(InstructionArithmetic* inst) {
    auto items = inst->get_items();
    items.first->accept(this);
    items.second->accept(this);

    auto rhs = parsed_items.back();
    parsed_items.pop_back();
    auto lhs = parsed_items.back();
    parsed_items.pop_back();

    newF->instructions.push_back(
      new InstructionArithmetic(lhs, inst->get_operator(), rhs));
  }

  void CodeGenerator::visit(InstructionCall* inst) {
    auto u = inst->get_item();
    u->accept(this);

    u = parsed_items.back();
    parsed_items.pop_back();
    newF->instructions.push_back(new InstructionCall(u, inst->get_number()));
  }

  void CodeGenerator::visit(InstructionConditionalJump* inst) {
    auto cmp = inst->get_comparison();
    auto op = cmp->get_operator();
    cmp->accept(this);
    cmp = (Comparison*)parsed_items.back();
    parsed_items.pop_back();
    newF->instructions.push_back(new InstructionConditionalJump(cmp, inst->get_label()));
  }

  void CodeGenerator::visit(InstructionJump* inst) {
    newF->instructions.push_back(inst);
  }

  void CodeGenerator::visit(InstructionLeaq* inst) {
    auto items = inst->get_items();
    auto v1 = items[0], v2 = items[1], v3 = items[2];
    auto n = inst->get_number();

    v1->accept(this);
    v2->accept(this);
    v3->accept(this);

    v3 = parsed_items.back();
    parsed_items.pop_back();
    v2 = parsed_items.back();
    parsed_items.pop_back();
    v1 = parsed_items.back();
    parsed_items.pop_back();

    newF->instructions.push_back(new InstructionLeaq(v1, v2, v3, n));
  }

  void CodeGenerator::visit(InstructionSelfOperation* inst) {
    auto w = inst->get_item();
    w->accept(this);

    w = parsed_items.back();
    parsed_items.pop_back();

    newF->instructions.push_back(new InstructionSelfOperation(w, inst->get_operator()));
  }

  void CodeGenerator::visit(InstructionAssignment* inst) {
    auto items = inst->get_items();
    auto v1 = items.first, v2 = items.second;

    v1->accept(this);
    v2->accept(this);

    v2 = parsed_items.back();
    parsed_items.pop_back();
    v1 = parsed_items.back();
    parsed_items.pop_back();
    newF->instructions.push_back(new InstructionAssignment(v1, v2));
  }

  void CodeGenerator::visit(InstructionLabel* inst) {
    newF->instructions.push_back(inst);
  }

  void CodeGenerator::visit(InstructionReturn* inst) {
    DEBUG_COUT << newF->instructions.size() << std::endl;
    newF->instructions.push_back(inst);
    DEBUG_COUT << newF->instructions.size() << std::endl;
  }

  void CodeGenerator::visit(InstructionStackArg* inst) {
    //代码生成有问题
    DEBUG_COUT << "generate stack-arg\n";
    auto w = inst->get_item();
    DEBUG_COUT << "w is : " << w->get_code() << std::endl;
    w->accept(this);

    w = parsed_items.back();
    parsed_items.pop_back();
    auto num = new Number(std::to_string(newF->locals * 8+ inst->get_number()->get_value()));
    auto mem = new MemoryAddress(Register::GetInstance("rsp"), num);
    newF->instructions.push_back(new InstructionAssignment(w, mem));
    // newF->instructions.push_back(new InstructionStackArg(w, inst->get_number()));
  }

  void CodeGenerator::visit(Function* f) {
    for(auto inst : f->instructions) {
      inst->accept(this);
    }
  }

}