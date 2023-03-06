
#include <queue>

#include "L3.hpp"
#include "control_flow_analysis.hpp"
#include "liveness_analysis.hpp"
#include "utils.hpp"

namespace L3 {
  LivenessAnalysis::LivenessAnalysis(Function* f) {
    f_ = f;
    analyze_ = new ControlFlowAnalysis(f);
    GenerateBitSet();
    GenerateInAndOut();
  }

  void LivenessAnalysis::GenerateBitSet() {
    unsigned counter = 0;
    DEBUG_COUT << "\n" << f_->to_string();
    for(auto variable : f_->get_locals()) {
      DEBUG_COUT << "---> has variable " << variable->to_string() << std::endl;
      std::string mask(f_->get_locals().size(), '0');
      mask[counter++] = '1';
      bitset bits(mask);
      DEBUG_COUT << bits << " with bits size " << bits.size() << std::endl;
      variable_mapper[variable] = bits;
    }
    for(auto inst : f_->get_instructions()) {
      auto local_size = f_->get_locals().size();
      gen_[inst] = bitset(local_size, 0);
      kill_[inst] = bitset(local_size, 0);
      DEBUG_COUT << inst->to_string();
      for(auto def : inst->get_defs()) {
        if(auto variable = dynamic_cast<Variable*>(def)) {
          kill_[inst] |= variable_mapper[variable];
          DEBUG_COUT << "---->kills " << variable->to_string() << std::endl;
          DEBUG_COUT << "---->bit" << variable_mapper[variable] << std::endl;
        }
      }
      for(auto use : inst->get_uses()) {
        if(auto variable = dynamic_cast<Variable*>(use)) {
          DEBUG_COUT << "---->gens " << variable->to_string() << std::endl;
          gen_[inst] |= variable_mapper[variable];
          DEBUG_COUT << "---->bit" << variable_mapper[variable] << std::endl;
        }
      }
    }
  }

  void LivenessAnalysis::GenerateInAndOut() {
    std::queue<Instruction*> work_list;
    auto local_size = f_->get_locals().size();
    DEBUG_COUT << "local size: " << local_size << std::endl;
    for(auto inst : f_->get_instructions()) {
      work_list.push(inst);
      in_[inst] = bitset(local_size, 0);
      out_[inst] = bitset(local_size, 0);
    }
    while(!work_list.empty()) {
      auto in = bitset(local_size, 0);
      auto out = bitset(local_size, 0);
      auto inst = work_list.front();
      work_list.pop();
      auto successors = analyze_->GetSuccessors(inst);
      for(auto successor : successors) {
        out |= in_[successor];
      }
      in = gen_[inst] | (out - kill_[inst]);
      if(in != in_[inst] or out != out_[inst]) {
        auto predecessors = analyze_->GetPredecessors(inst);
        for(auto predecessor : predecessors) {
          work_list.push(predecessor);
        }
        in_[inst] = in;
        out_[inst] = out;
      }
    }
  }
  std::set<Variable*> LivenessAnalysis::GetInSet(Instruction* inst) {
    auto in_bits = in_.at(inst);
    std::set<Variable*> in_set;
    for(auto [variable, bit] : variable_mapper) {
      if((in_bits & bit).any()) {
        in_set.insert(variable);
      }
    }
    return in_set;
  }

  std::set<Variable*> LivenessAnalysis::GetOutSet(Instruction* inst) {
    auto out_bits = out_.at(inst);
    std::set<Variable*> out_set;
    for(auto [variable, bit] : variable_mapper) {
      if((out_bits & bit).any()) {
        out_set.insert(variable);
      }
    }
    return out_set;
  }
}

