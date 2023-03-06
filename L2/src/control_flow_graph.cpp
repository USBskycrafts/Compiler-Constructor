
#include "control_flow_graph.hpp"
#include "printer.hpp"
#include "utils.hpp"
#include <exception>

namespace L2 {
  ControlFlowGraph::ControlFlowGraph(Function* f) : f_(f) {
    for(auto iter = f->instructions.rbegin(); iter != f->instructions.rend(); iter++) {
      predecessors[*iter] = {};
      successors[*iter] = {};
      this->visit(*iter);
    }
    DEBUG_COUT << f->instructions.size() << " " << successors.size() << std::endl;
    for(auto [inst, inst_set] : successors) {
      for(auto inst_suc : inst_set) {
        predecessors[inst_suc].insert(inst);
      }
    }

    
    // for(auto inst : f->instructions) {
    //   std::cout << "--------\n";
    //   Printer printer;
    //   printer.visit(inst);
    //   for(auto s : successors.at(inst)) {
    //     std::cout << "suc:";
    //     printer.visit(s);
    //   } 
    //   for(auto s : predecessors.at(inst)) {
    //     std::cout << "pre: ";
    //     printer.visit(s);
    //   } 
    //   std::cout << "--------\n";
    // }


  }

  std::map<Instruction*, std::set<Instruction*>> ControlFlowGraph::getSuccessors() {
    return successors;
  }

  std::map<Instruction*, std::set<Instruction*>> ControlFlowGraph::getPredecessors() {
    return predecessors;
  }


  void ControlFlowGraph::visit(Item* item) {

  }

  void ControlFlowGraph::visit(Value* value) {

  }

  void ControlFlowGraph::visit(Register* reg) {

  }

  void ControlFlowGraph::visit(Variable* var) {

  }

  void ControlFlowGraph::visit(MemoryAddress* address) {

  }

  void ControlFlowGraph::visit(Comparison* cmp) {

  }

  void ControlFlowGraph::visit(Number* num) {

  }

  void ControlFlowGraph::visit(Operator* op) {

  }

  void ControlFlowGraph::visit(Label* label) {

  }

  void ControlFlowGraph::visit(FunctionName* name) {

  }

  void ControlFlowGraph::visit(Instruction* inst) {
    inst->accept(this);
  }

  void ControlFlowGraph::visit(InstructionArithmetic* inst) {
    if(!parsed_instructions.empty()) {
      auto successor = parsed_instructions.back();
      successors[inst].insert(successor);
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowGraph::visit(InstructionCall* inst) {
    auto u = inst->get_item();
    if(u->get_type() == Item::kFunctionName && u->get_code() == "tensor-error") {
      successors[inst] = {};
      parsed_instructions.push_back(inst);
      return;
    }
    if(!parsed_instructions.empty()) {
      auto successor = parsed_instructions.back();
      successors[inst].insert(successor);
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowGraph::visit(InstructionConditionalJump* inst) {
    for(auto i : f_->instructions) {
      if(i->get_type() == Instruction::kLabel) {
        auto label = dynamic_cast<InstructionLabel*>(i);
        try {
          if(label->get_label()->get_code() == inst->get_label()->get_code()) {
            DEBUG_PRINT("find the same label");
            successors[inst].insert(label);
          }
        } catch (std::exception_ptr) {
          DEBUG_PRINT("Instruction Label NULL");
        }
      }
    }

    if(!parsed_instructions.empty()) {
      auto successor = parsed_instructions.back();
      successors[inst].insert(successor);
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowGraph::visit(InstructionJump* inst) {
    for(auto i : f_->instructions) {
      if(i->get_type() == Instruction::kLabel) {
        auto label = dynamic_cast<InstructionLabel*>(i);
        try {
          if(label->get_label()->get_code() == inst->get_label()->get_code()) {
            successors[inst].insert(label);
          }
        } catch (std::exception_ptr) {
          DEBUG_PRINT("Instruction Label NULL");
        }
      }
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowGraph::visit(InstructionLeaq* inst) {
    if(!parsed_instructions.empty()) {
      auto successor = parsed_instructions.back();
      successors[inst].insert(successor);
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowGraph::visit(InstructionSelfOperation* inst) {
    if(!parsed_instructions.empty()) {
      auto successor = parsed_instructions.back();
      successors[inst].insert(successor);
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowGraph::visit(InstructionAssignment* inst) {
    if(!parsed_instructions.empty()) {
      auto successor = parsed_instructions.back();
      successors[inst].insert(successor);
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowGraph::visit(InstructionLabel* inst) {
    if(!parsed_instructions.empty()) {
      auto successor = parsed_instructions.back();
      successors[inst].insert(successor);
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowGraph::visit(InstructionReturn* inst) {
    successors[inst] = {};
    parsed_instructions.push_back(inst);
  }

  void ControlFlowGraph::visit(InstructionStackArg* inst) {
    if(!parsed_instructions.empty()) {
      auto successor = parsed_instructions.back();
      successors[inst].insert(successor);
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowGraph::visit(Function* f) {
    for(auto inst : f->instructions) {
      inst->accept(this);
    }
  }

}