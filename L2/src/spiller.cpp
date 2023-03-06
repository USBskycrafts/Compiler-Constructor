#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

#include "spiller.hpp"
#include "L2.hpp"
#include "liveness_analysis.hpp"
#include "utils.hpp"
#include "printer.hpp"

namespace L2 {

  Spiller::Spiller(Function* f, std::string prefix)
  : f(f), prefix(prefix)/*, analyzer(new LivenessAnalysis(f))*/ {
    newF = new Function;
    newF->name = f->name;
    newF->arguments = f->arguments;
    newF->locals = f->locals;

  }

  Function* Spiller::spill(Variable* var) {
    if(var->get_code().find(prefix) != std::string::npos) {
      DEBUG_COUT << "Probably spill a spilled value, check the code for" << var->get_code() << std::endl;
    }
    this->spilled_var = var;
    for(auto inst : f->instructions) {
      this->visit(inst);
    }
    if(spill_counter) {
      newF->locals++;
    }
    return newF;
  }

  Function* Spiller::spill() {
    for(auto var : Variable::getAll()) {
      if(var->get_code().find(prefix) == std::string::npos) {
        DEBUG_COUT << " spilling " << var->get_code() << std::endl;
        f = spill(var);
        newF = new Function;
        newF->name = f->name;
        newF->arguments = f->arguments;
        newF->locals = f->locals;
        DEBUG_COUT << var->get_code() << " spilled finished" << std::endl;
      }   
    }
    return f;
  }


  void Spiller::visit(Item* item) {
    item->accept(this);
  }

  void Spiller::visit(Value* value) {
    value->accept(this);
  }

  void Spiller::visit(Register* reg) {
    parsed_item.push_back(reg);
  }

  void Spiller::visit(Variable* var) {
    parsed_item.push_back(var);
  }

  void Spiller::visit(MemoryAddress* address) {
    auto w = address->get_value();
    auto n = address->get_number();

    w->accept(this);
    n->accept(this);
    n = (Number*)parsed_item.back();
    parsed_item.pop_back();
    if(w == spilled_var) {
      auto rsp = Register::GetInstance("rsp");
      auto n = new Number(std::to_string(8 * newF->locals));
      auto m = new MemoryAddress(rsp, n);
      w = Variable::GetInstance(prefix + std::to_string(spill_counter));
      newF->instructions.push_back(new InstructionAssignment(w, m));
      spill_counter++;
    } else {
      w = (Value*)parsed_item.back();
    }  
    parsed_item.pop_back();
    parsed_item.push_back(new MemoryAddress(w, n));
  }

  void Spiller::visit(Comparison* cmp) {
    bool is_modified = false;
    auto v = cmp->get_items();
    auto op = cmp->get_operator();
    v.first->accept(this);
    op->accept(this);
    v.second->accept(this);

    auto v2 = parsed_item.back();
    if(v2 == spilled_var) {
      is_modified = true;
      auto rsp = Register::GetInstance("rsp");
      auto n = new Number(std::to_string(8 * newF->locals));
      auto m = new MemoryAddress(rsp, n);
      v2 = Variable::GetInstance(prefix + std::to_string(spill_counter));
      newF->instructions.push_back(new InstructionAssignment(v2, m));
    }
    parsed_item.pop_back();
    op = (Operator*)parsed_item.back();
    parsed_item.pop_back();
    auto v1 = parsed_item.back();
    if(v1 == spilled_var) {
      is_modified = true;
      auto rsp = Register::GetInstance("rsp");
      auto n = new Number(std::to_string(8 * newF->locals));
      auto m = new MemoryAddress(rsp, n);
      v1 = Variable::GetInstance(prefix + std::to_string(spill_counter));
      newF->instructions.push_back(new InstructionAssignment(v1, m));
    }
    parsed_item.pop_back();
    parsed_item.push_back(new Comparison(v1, op, v2));
    if(is_modified) {
      spill_counter++;
    }
  }

  void Spiller::visit(Number* num) {
    parsed_item.push_back(num);
  }

  void Spiller::visit(Operator* op) {
    parsed_item.push_back(op);
  }

  void Spiller::visit(Label* label) {
    parsed_item.push_back(label);
  }

  void Spiller::visit(FunctionName* name) {
    parsed_item.push_back(name);
  }

  void Spiller::visit(Instruction* inst) {
    inst->accept(this);
  }

  void Spiller::visit(InstructionArithmetic* inst) {
    auto items = inst->get_items();
    auto op = inst->get_operator();
    items.first->accept(this);
    op->accept(this);
    items.second->accept(this);
    bool is_modified = false;

    auto v2 = parsed_item.back();
    parsed_item.pop_back();
    op = (Operator*)parsed_item.back();
    parsed_item.pop_back();
    auto v1 = parsed_item.back();
    parsed_item.pop_back();

    if(dynamic_cast<MemoryAddress*>(items.first)
    and items.second == spilled_var) {
      auto m = dynamic_cast<MemoryAddress*>(items.first);
      if(m->get_value() == spilled_var) {
        spill_counter--;
        auto v = Variable::GetInstance(prefix + std::to_string(spill_counter));
        newF->instructions.push_back(new InstructionArithmetic(v1, op, v));
        spill_counter++;
        return;
      }
    }

    if(v2 == spilled_var) {
      is_modified = true;
      auto rsp = Register::GetInstance("rsp");
      auto n = new Number(std::to_string(8 * newF->locals));
      auto m = new MemoryAddress(rsp, n);
      v2 = Variable::GetInstance(prefix + std::to_string(spill_counter));
      newF->instructions.push_back(new InstructionAssignment(v2, m));
    }
    
    if(v1 == spilled_var) {
      is_modified = true;
      auto rsp = Register::GetInstance("rsp");
      auto n = new Number(std::to_string(8 * newF->locals));
      auto m = new MemoryAddress(rsp, n);
      v1 = Variable::GetInstance(prefix + std::to_string(spill_counter));
      if(v2 != v1) {
        newF->instructions.push_back(new InstructionAssignment(v1, m));
      }
      newF->instructions.push_back(new InstructionArithmetic(v1, op, v2));
      newF->instructions.push_back(new InstructionAssignment(m, v1));
    } else {
      newF->instructions.push_back(new InstructionArithmetic(v1, op, v2));
    }
    
    if(is_modified) {
      spill_counter++;
    }
  }

  void Spiller::visit(InstructionCall* inst) {
    auto u = inst->get_item();
    auto n = inst->get_number();

    u->accept(this);
    n->accept(this);

    n = (Number*)parsed_item.back();
    parsed_item.pop_back();
    u = parsed_item.back();
    if(u == spilled_var) {
      auto rsp = Register::GetInstance("rsp");
      auto n = new Number(std::to_string(8 * newF->locals));
      auto m = new MemoryAddress(rsp, n);
      u = Variable::GetInstance(prefix + std::to_string(spill_counter));
      newF->instructions.push_back(new InstructionAssignment(u, m));
    }
    newF->instructions.push_back(new InstructionCall(u, n));
    if(u == spilled_var) {
      spill_counter++;
    }
  }

  void Spiller::visit(InstructionConditionalJump* inst) {
    auto cmp = inst->get_comparison();
    auto l = inst->get_label();
    cmp->accept(this);
    l->accept(this);

    l = (Label*)parsed_item.back();
    parsed_item.pop_back();
    cmp = (Comparison*)parsed_item.back();
    parsed_item.pop_back();
    newF->instructions.push_back(new InstructionConditionalJump(cmp, l));
  }

  void Spiller::visit(InstructionJump* inst) {
    auto l = inst->get_label();
    l->accept(this);

    l = (Label*)parsed_item.back();
    parsed_item.pop_back();
    newF->instructions.push_back(new InstructionJump(l));
  }

  void Spiller::visit(InstructionLeaq* inst) {
    if(inst->get_items().size() != 3) {
      DEBUG_PRINT("leaq construct fault!");
      abort();
    }
    bool is_modified = false;
    auto items = inst->get_items();
    auto v1 = items[0], v2 = items[1], v3 = items[2];
    auto n = inst->get_number();

    // n->accept(this);
    // v3->accept(this);
    // v2->accept(this);
    // v1->accept(this);
    v1->accept(this);
    v2->accept(this);
    v3->accept(this);
    n->accept(this);

    n = (Number*)parsed_item.back();
    parsed_item.pop_back();
    v3 = parsed_item.back();
    parsed_item.pop_back();
    if(v3 == spilled_var) {
      is_modified = true;
      auto rsp = Register::GetInstance("rsp");
      auto n = new Number(std::to_string(8 * newF->locals));
      auto m = new MemoryAddress(rsp, n);
      v3 = Variable::GetInstance(prefix + std::to_string(spill_counter));
      newF->instructions.push_back(new InstructionAssignment(v3, m));
    }
    v2 = parsed_item.back();
    parsed_item.pop_back();
    if(v2 == spilled_var) {
      is_modified = true;
      auto rsp = Register::GetInstance("rsp");
      auto n = new Number(std::to_string(8 * newF->locals));
      auto m = new MemoryAddress(rsp, n);
      v2 = Variable::GetInstance(prefix + std::to_string(spill_counter));
      newF->instructions.push_back(new InstructionAssignment(v2, m));
    }
    v1 = parsed_item.back();
    parsed_item.pop_back();
    if(v1 == spilled_var) {
      is_modified = true;
      auto rsp = Register::GetInstance("rsp");
      auto offset = new Number(std::to_string(8 * newF->locals));
      auto m = new MemoryAddress(rsp, offset);
      v1 = Variable::GetInstance(prefix + std::to_string(spill_counter));
      newF->instructions.push_back(new InstructionLeaq(v1, v2, v3, n));
      DEBUG_COUT << v1->get_code() << " @ " <<  v2->get_code() << " " << v3->get_code() << " " << n->get_code() << std::endl;
      newF->instructions.push_back(new InstructionAssignment(m, v1));
    } else {
      newF->instructions.push_back(new InstructionLeaq(v1, v2, v3, n));
    }

    if(is_modified) {
      spill_counter++;
    }
  }

  void Spiller::visit(InstructionSelfOperation* inst) {
    auto w = inst->get_item();
    auto op = inst->get_operator();

    w->accept(this);
    op->accept(this);

    op = (Operator*)parsed_item.back();
    parsed_item.pop_back();
    if(w == spilled_var) {
      auto rsp = Register::GetInstance("rsp");
      auto n = new Number(std::to_string(8 * newF->locals));
      auto m = new MemoryAddress(rsp, n);
      w = Variable::GetInstance(prefix + std::to_string(spill_counter));
      newF->instructions.push_back(new InstructionAssignment(w, m));
      newF->instructions.push_back(new InstructionSelfOperation(w, op));
      newF->instructions.push_back(new InstructionAssignment(m, w));
      spill_counter++;
    } else {
      newF->instructions.push_back(new InstructionSelfOperation(w, op));
    }
  }

  void Spiller::visit(InstructionAssignment* inst) {
    bool is_modified = false;
    auto items = inst->get_items();
    auto v1 = items.first;
    auto v2 = items.second;

    v1->accept(this);
    v2->accept(this);

    v2 = parsed_item.back();
    parsed_item.pop_back();
    v1 = parsed_item.back();
    parsed_item.pop_back();

    if(dynamic_cast<MemoryAddress*>(items.first)
    and items.second == spilled_var) {
      auto m = dynamic_cast<MemoryAddress*>(items.first);
      if(m->get_value() == spilled_var) {
        spill_counter--;
        auto v = Variable::GetInstance(prefix + std::to_string(spill_counter));
        newF->instructions.push_back(new InstructionAssignment(v1, v));
        spill_counter++;
        return;
      }
    }
    
    if(v2 == spilled_var) {
      is_modified = true;
      auto rsp = Register::GetInstance("rsp");
      auto n = new Number(std::to_string(8 * newF->locals));
      auto m = new MemoryAddress(rsp, n);
      v2 = Variable::GetInstance(prefix + std::to_string(spill_counter));
      newF->instructions.push_back(new InstructionAssignment(v2, m));
    }

    if(v1 == spilled_var) {
      is_modified = true;
      auto rsp = Register::GetInstance("rsp");
      auto n = new Number(std::to_string(8 * newF->locals));
      auto m = new MemoryAddress(rsp, n);
      v1 = Variable::GetInstance(prefix + std::to_string(spill_counter));
      newF->instructions.push_back(new InstructionAssignment(v1, v2));
      newF->instructions.push_back(new InstructionAssignment(m, v1));
    } else {
       newF->instructions.push_back(new InstructionAssignment(v1, v2));
    }
    if(is_modified) {
      spill_counter++;
    }
  }

  void Spiller::visit(InstructionLabel* inst) {
    auto l = inst->get_label();
    l->accept(this);
    l = (Label*)parsed_item.back();
    parsed_item.pop_back();
    newF->instructions.push_back(new InstructionLabel(l));
  }

  void Spiller::visit(InstructionReturn* inst) {
    newF->instructions.push_back(new InstructionReturn);
  }

  void Spiller::visit(InstructionStackArg* inst) {
    auto item = inst->get_item();
    auto n = inst->get_number();

    item->accept(this);
    n->accept(this);

    n = (Number*)parsed_item.back();
    parsed_item.pop_back();
    item = parsed_item.back();
    parsed_item.pop_back();
    DEBUG_COUT << "in spilling stack-arg" << std::endl;
    DEBUG_COUT << item->get_code() << " " << spilled_var->get_code() << std::endl;
    if(item == spilled_var) {
      auto rsp = Register::GetInstance("rsp");
      auto offset = new Number(std::to_string(8 * newF->locals));
      auto m = new MemoryAddress(rsp, offset);
      item = Variable::GetInstance(prefix + std::to_string(spill_counter));
      newF->instructions.push_back(new InstructionStackArg(item, n));
      newF->instructions.push_back(new InstructionAssignment(m, item));
      spill_counter++;
    } else {
      newF->instructions.push_back(inst);
    }
  }

  void Spiller::visit(Function* f) {
    for(auto inst : f->instructions) {
      inst->accept(this);
    }
  }

}