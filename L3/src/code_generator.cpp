
#include "code_generator.hpp"
#include "L3.hpp"
#include <string>

namespace L3 {

  int NaiveGenerator::label_counter = 0;

  NaiveGenerator::NaiveGenerator(Function* f) {
    this->visit(f);
  }

  std::string NaiveGenerator::get_L2Code() {
    return code.str();
  }

  void NaiveGenerator::visit(Item* item) {

  }

  void NaiveGenerator::visit(Variable* var) {

  }

  void NaiveGenerator::visit(Number* num) {

  }

  void NaiveGenerator::visit(Operator* op) {

  }

  void NaiveGenerator::visit(Label* label) {

  }

  void NaiveGenerator::visit(FunctionName* name) {

  }

  void NaiveGenerator::visit(Instruction* inst) {
    inst->accept(this);
  }

  void NaiveGenerator::visit(AssignInst* inst) {
    code << inst->get_items()[0]->to_string() 
    << " <- " << inst->get_items()[1]->to_string() << std::endl;
  }

  void NaiveGenerator::visit(StoreInst* inst) {
    code << "mem " << inst->get_items()[0]->to_string()
    << " 0 <- " << inst->get_items()[1]->to_string() << std::endl;
  }

  void NaiveGenerator::visit(LoadInst* inst) {
    code << inst->get_items()[0]->to_string() << " <- mem "
    << inst->get_items()[1]->to_string() << " 0 " << std::endl;
  }

  void NaiveGenerator::visit(BinaryOperator* inst) {
    auto d = inst->get_items()[0];
    auto s1 = inst->get_items()[1], s2 = inst->get_items()[3];
    auto op = inst->get_items()[2];

    code << (d->to_string() + "__") << " <- " << s1->to_string() << "\n"
    << (d->to_string() + "__") << " " << (op->to_string() + "= ") << s2->to_string()
    << "\n" << d->to_string() << " <- " << (d->to_string() + "__") << std::endl;
  }

  void NaiveGenerator::visit(CmpInst* inst) {
    auto cmp = (Operator*)inst->get_items()[2];
    if(cmp->to_string()[0] == '>') {
      auto op = cmp->to_string();
      op[0] = '<';
      code << inst->get_items()[0]->to_string() <<
      " <- " << inst->get_items()[3]->to_string() << " "
      << op << " " << inst->get_items()[1]->to_string() 
      << std::endl; 
    } else {
      code << inst->get_items()[0]->to_string() <<
      " <- " << inst->get_items()[1]->to_string() << " "
      << inst->get_items()[2]->to_string() << " "
      << inst->get_items()[3]->to_string() << std::endl; 
    }
  }

  void NaiveGenerator::visit(LabelInst* inst) {
    code << inst->to_string() << std::endl;
  }

  void NaiveGenerator::visit(ReturnInst* inst) {
    if(inst->get_items()[0] != nullptr) {
      code << "rax <- " << inst->get_items()[0]->to_string() << std::endl;
    }
    code << "return" << std::endl;
  }

  void NaiveGenerator::visit(BranchInst* inst) {
    if(inst->get_items()[0] == nullptr) {
      code << "goto " << inst->get_items()[1]->to_string() << std::endl;
    } else {
      code << "cjump 1 = " << inst->get_items()[0]->to_string()
      << " " << inst->get_items()[1]->to_string() << std::endl;
    }
  }

  void NaiveGenerator::visit(CallInst* inst) {
    auto callee = inst->get_callee();
    auto arguments = inst->get_arguments();
    auto ret_value = inst->get_ret_value();
    for(int i = 1; i <= arguments.size(); i++) {
      switch (i) {
        case 1: {
          code << "rdi <- " << arguments[i - 1]->to_string() << std::endl;
          break;
        }
        case 2: {
          code << "rsi <- " << arguments[i - 1]->to_string() << std::endl;
          break;
        }
        case 3: {
          code << "rdx <- " << arguments[i - 1]->to_string() << std::endl;
          break;
        }
        case 4: {
          code << "rcx <- " << arguments[i - 1]->to_string() << std::endl;
          break;
        }
        case 5: {
          code << "r8 <- " << arguments[i - 1]->to_string() << std::endl;
          break;
        }
        case 6: {
          code << "r9 <- " << arguments[i - 1]->to_string() << std::endl;
          break;
        }
        default: {
          int offset = 40 - 8 * i;
          code << "mem rsp " << std::to_string(offset) << " <- "
          << arguments[i - 1]->to_string() << std::endl;
          break;
        }
      }
    }
    auto callee_label = callee->to_string();
    if(callee_label[0] == '@' || callee_label[0] == '%') {
      callee_label[0] = ':';
      callee_label += "_ret" + std::to_string(label_counter++);
      code << "mem rsp -8 <- " << callee_label << std::endl;
      code << "call " << callee->to_string() << " " << arguments.size() << std::endl;
      code << callee_label << std::endl;
    } else {
      code << "call " << callee->to_string() << " " << arguments.size() << std::endl;
    }
    if(ret_value) {
      code << ret_value->to_string() << " <- rax" << std::endl;
    }
  }

  void NaiveGenerator::visit(Function* f) {
    auto arguments = f->get_arguments();
    for(int i = 1; i <= arguments.size(); i++) {
      switch (i) {
        case 1: {
          code << arguments[i - 1]->to_string() << " <- rdi " << std::endl;
          break;
        }
        case 2: {
          code << arguments[i - 1]->to_string() << " <- rsi " << std::endl;
          break;
        }
        case 3: {
          code << arguments[i - 1]->to_string() << " <- rdx " << std::endl;
          break;
        }
        case 4: {
          code << arguments[i - 1]->to_string() << " <- rcx " << std::endl;
          break;
        }
        case 5: {
          code << arguments[i - 1]->to_string() << " <- r8 " << std::endl;
          break;
        }
        case 6: {
          code << arguments[i - 1]->to_string() << " <- r9 " << std::endl;
          break;
        }
        default: {
          code << arguments[i - 1]->to_string() << " <- stack-arg "
          << std::to_string((arguments.size() - i) * 8) << std::endl;
          break;
        }
      }
    }
    for(auto inst : f->get_instructions()) {
      this->visit(inst);
    }
  }

}
