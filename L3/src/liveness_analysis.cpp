#include "liveness_analysis.hpp"
#include <algorithm>
#include <queue>

namespace LivenessAnalysis {
  

  void SetControlFlow(L3::Function* function) {
    auto cfg = new ControlFlowGenerator(function);
    cfg->GenerateSuccessors();
    cfg->GeneratePredecessors();
  }

  std::map<L3::Instruction*, AnalysisResult*> GenerateInAndOut(L3::Function* function) {
    std::map<L3::Instruction*, AnalysisResult*> results;
    static std::map<L3::Function*, std::map<L3::Instruction*, AnalysisResult*>> recorder;
    if(recorder.count(function)) {
      return recorder[function];
    }
    SetControlFlow(function);
    std::queue<L3::Instruction*> inst_list;
    for(auto inst : function->instructions) {
      auto result =  new AnalysisResult;
      result->gens.emplace(inst->GetUses());
      result->kills.emplace(inst->GetDefs());
      results[inst.get()] = result;
      inst_list.push(inst.get());
    }
    while(!inst_list.empty()) {
      auto inst = inst_list.front();
      inst_list.pop();
      std::set<L3::Variable> in, out;
      for(auto suc : inst->GetSuccessors()) {
        out.emplace(results[suc.lock().get()]->ins);
      }
      std::set_difference(out.begin(), out.end(), 
          results[inst]->kills.begin(), results[inst]->kills.end(), in.begin());
      std::merge(results[inst]->gens.begin(), results[inst]->gens.end(),
          in.begin(), in.end(), in.begin());
      if(in != results[inst]->ins || out != results[inst]->outs) {
        for(auto pred : inst->GetPredecessors()) {
          inst_list.push(pred.lock().get());
        }
      }
    }
    recorder[function] = results;
    return results;
  }


  void ControlFlowGenerator::GenerateSuccessors() {
    for(auto inst : function_->instructions) {
      inst->accept(this);
    }
  }

  void ControlFlowGenerator::GeneratePredecessors() {
    for(auto inst : function_->instructions) {
      auto suc_s = inst->GetSuccessors();
      for(auto suc : suc_s) {
        suc.lock()->InsertPredecessor(inst);
      }
    }
  }

  void ControlFlowGenerator::visit(L3::Item* item) {}
  void ControlFlowGenerator::visit(L3::Number* item) {}
  void ControlFlowGenerator::visit(L3::FunctionName* item) {}
  void ControlFlowGenerator::visit(L3::Label* item) {}
  void ControlFlowGenerator::visit(L3::Variable* item) {}
  void ControlFlowGenerator::visit(L3::Operator* item) {}

  void ControlFlowGenerator::visit(L3::Instruction* inst) {
    inst->accept(this);
  }

  void ControlFlowGenerator::visit(L3::CallInst* inst) {
    if(i >= N) {
      return;
    }
    if(inst->GetCalleeName() != "tensor-error") {
      inst->InsertSuccessor(function_->instructions[++i]);
    }
  }

  void ControlFlowGenerator::visit(L3::AssignInst* inst) {
    if(i >= N) {
      return;
    }
    inst->InsertSuccessor(function_->instructions[++i]);
  }

  void ControlFlowGenerator::visit(L3::BinaryOperator* inst) {
    if(i >= N) {
      return;
    }
    inst->InsertSuccessor(function_->instructions[++i]);
  }

  void ControlFlowGenerator::visit(L3::LoadInst* inst) {
    if(i >= N) {
      return;
    }
    inst->InsertSuccessor(function_->instructions[++i]);
  }

  void ControlFlowGenerator::visit(L3::StoreInst* inst) {
    if(i >= N) {
      return;
    }
    inst->InsertSuccessor(function_->instructions[++i]);
  }

  void ControlFlowGenerator::visit(L3::ReturnInst* inst) {
    return;
  }

  void ControlFlowGenerator::visit(L3::BranchInst* inst) {
    if(i >= N) {
      return;
    }
    auto inst_s = function_->instructions;
    for(auto inst_iter : inst_s) {
      if(auto label = std::dynamic_pointer_cast<L3::LabelInst>(inst_iter)) {
        if(label->GetLabelName() == inst->GetLabelName()) {
          inst->InsertSuccessor(label);
          break;
        }
      }
    }
    // check if mark is nullptr
    if(!inst->GetUses().empty()) {
      inst->InsertSuccessor(function_->instructions[++i]);
    }
  }

  void ControlFlowGenerator::visit(L3::LabelInst* inst) {
    inst->InsertSuccessor(function_->instructions[++i]);
  }

}