

#include "control_flow_analysis.hpp"
#include "L3.hpp"
#include "utils.hpp"

namespace L3 {
  ControlFlowAnalysis::ControlFlowAnalysis(Function* f) {
    f_ = f;
    for(auto inst : f->get_instructions()) {
      successor_mapper[inst] = {};
      predecessor_mapper[inst] = {};
    }
    GenerateSuccessors();
    GeneratePredecessors();
  }

  void ControlFlowAnalysis::GenerateSuccessors() {
    this->visit(f_);
  }

  void ControlFlowAnalysis::GeneratePredecessors() {
    for(auto [inst, successors] : successor_mapper) {
      for(auto successor : successors) {
        predecessor_mapper[successor].insert(inst);
        DEBUG_COUT << inst->to_string();
        DEBUG_COUT << "successor----> " << successor->to_string();
      }
    }
  }

  std::set<Instruction*> ControlFlowAnalysis::GetPredecessors(Instruction* inst) {
    return predecessor_mapper.at(inst);
  }

  std::set<Instruction*> ControlFlowAnalysis::GetSuccessors(Instruction* inst) {
    return successor_mapper.at(inst);
  }

  void ControlFlowAnalysis::visit(Item* item) {

  }

  void ControlFlowAnalysis::visit(Variable* var) {

  }

  void ControlFlowAnalysis::visit(Number* num) {

  }

  void ControlFlowAnalysis::visit(Operator* op) {

  }

  void ControlFlowAnalysis::visit(Label* label) {

  }

  void ControlFlowAnalysis::visit(FunctionName*) {

  }

  void ControlFlowAnalysis::visit(Instruction* inst) {
    inst->accept(this);
  }

  void ControlFlowAnalysis::visit(AssignInst* inst) {
    if(!parsed_instructions.empty()) {
      auto parsed = parsed_instructions.back();
      successor_mapper[inst].insert(parsed);
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowAnalysis::visit(StoreInst* inst) {
    if(!parsed_instructions.empty()) {
      auto parsed = parsed_instructions.back();
      successor_mapper[inst].insert(parsed);
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowAnalysis::visit(LoadInst* inst) {
    if(!parsed_instructions.empty()) {
      auto parsed = parsed_instructions.back();
      successor_mapper[inst].insert(parsed);
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowAnalysis::visit(BinaryOperator* inst) {
    if(!parsed_instructions.empty()) {
      auto parsed = parsed_instructions.back();
      successor_mapper[inst].insert(parsed);
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowAnalysis::visit(CmpInst* inst) {
    if(!parsed_instructions.empty()) {
      auto parsed = parsed_instructions.back();
      successor_mapper[inst].insert(parsed);
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowAnalysis::visit(LabelInst* inst) {
    if(!parsed_instructions.empty()) {
      auto parsed = parsed_instructions.back();
      successor_mapper[inst].insert(parsed);
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowAnalysis::visit(ReturnInst* inst) {
    parsed_instructions.push_back(inst);
  }

  void ControlFlowAnalysis::visit(BranchInst* inst) {
    for(auto iter : f_->get_instructions()) {
      if(auto label = dynamic_cast<LabelInst*>(iter)) {
        if(label->get_items()[0] == inst->get_items()[1]) {
          successor_mapper[inst].insert(label);
        }
      }
    }
    if(inst->get_items()[0] != nullptr) {
      auto parsed = parsed_instructions.back();
      successor_mapper[inst].insert(parsed);
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowAnalysis::visit(CallInst* inst) {
    auto callee = inst->get_callee();
    if(callee->to_string() != "tensor-error") {
      auto parsed = parsed_instructions.back();
      successor_mapper[inst].insert(parsed);
    }
    parsed_instructions.push_back(inst);
  }

  void ControlFlowAnalysis::visit(Function* f) {
    auto instructions = f->get_instructions();
    for(auto iter = instructions.rbegin(); iter != instructions.rend(); iter++) {
      (*iter)->accept(this);
    }
  }

}