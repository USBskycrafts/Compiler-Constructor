#include <cstdint>
#include <queue>
#include <iostream>
#include <cstdlib>

#include "liveness_analysis.hpp"
#include "L2.hpp"
#include "printer.hpp"
#include "utils.hpp"



namespace L2 {

  LivenessAnalysis::LivenessAnalysis(Function* f) : 
  f_(f),
  value_counter(15 + Variable::countVariable()), 
  cfg(new ControlFlowGraph(f)) {
    Analysis(f);
  }

  std::map<Instruction*, std::set<Value*>> LivenessAnalysis::GetInSet() {
    std::map<Instruction*, std::set<Value*>> in;
    //DEBUG_COUT << this->in.size() << " ";
    for(auto [inst, bitset] : this->in) {
      DEBUG_COUT << "in_set: ";
      for(auto [item, i_bitset] : value_mapper) {
        if(!(i_bitset & bitset).none()) {
          in[inst].insert(item);
          DEBUG_COUT << item->get_code() << " ";
        }
      }
      DEBUG_COUT << std::endl;
      if(in[inst].size() == 0) in[inst] = {};
    }
    //DEBUG_COUT << in.size() << " " << f_->instructions.size() << std::endl;
    return in;
  }

  std::map<Instruction*, std::set<Value*>> LivenessAnalysis::GetOutSet() {
     std::map<Instruction*, std::set<Value*>> out;
     //DEBUG_COUT << this->out.size() << " ";
     for(auto [inst, bitset] : this->out) {
      DEBUG_COUT << "out_set: "; 
      for(auto [item, i_bitset] : value_mapper) {
        auto mask = i_bitset & bitset;
        if(mask.any()) {
          out[inst].insert(item);
          DEBUG_COUT << item->get_code() << " ";
        }
      }
      DEBUG_COUT << std::endl;
      if(out[inst].size() == 0) out[inst] = {};
     }
     //DEBUG_COUT << out.size() << " " << f_->instructions.size() << std::endl;
     return out;
  }

  std::map<Instruction*, std::set<Value*>> LivenessAnalysis::GetKillSet() {
    std::map<Instruction*, std::set<Value*>> kill;
    for(auto [inst, bitset] : this->kill) {
      DEBUG_COUT << "kill_set: ";
      for(auto [item, i_bitset] : value_mapper) {
        auto mask = i_bitset & bitset;
        if(mask.any()) {
          kill[inst].insert(item);
          DEBUG_COUT << item->get_code() << " ";
        }
      }
      DEBUG_COUT << std::endl;
      if(kill[inst].size() == 0) kill[inst] = {};
    }
    return kill;
  }

  std::map<Instruction*, std::set<Value*>> LivenessAnalysis::GetGenSet() {
    std::map<Instruction*, std::set<Value*>> gen;
    for(auto [inst, bitset] : this->gen) {
      DEBUG_COUT << "gen_set: ";
      for(auto [item, i_bitset] : value_mapper) {
        auto mask = i_bitset & bitset;
        if(mask.any()) {
          gen[inst].insert(item);
          DEBUG_COUT << item->get_code() << " ";
        }
      }
      DEBUG_COUT << std::endl;
      if(gen[inst].size() == 0) gen[inst] = {};
    }
    return gen;
  }

  void LivenessAnalysis::Analysis(Function* f) {
    for(auto inst : f->instructions) {
      gen[inst].resize(value_counter, 0);
      kill[inst].resize(value_counter, 0);
      in[inst].resize(value_counter, 0);
      out[inst].resize(value_counter, 0);
      this->visit(inst);
    }
    setInOutSet();
  }

  boost::dynamic_bitset<> LivenessAnalysis::GenerateBitSet(Value* item) {
    if(value_mapper.count(item) == 0) {
      std::string mask = std::string(value_counter, '0');
      mask[encoded_value_counter] = '1';
      Bitset bit(mask);
      encoded_value_counter++;
      value_mapper[item] = bit;
    }
    DEBUG_COUT << item->get_code() << " " << value_mapper[item] << std::endl;
    return value_mapper[item];
  }

  void LivenessAnalysis::setInOutSet() {
    DEBUG_PRINT("in set in and out");
    std::priority_queue<Instruction*> list;
    for(auto inst : f_->instructions) {
      list.push(inst);
    }
    while(!list.empty()) {
      auto inst = list.top();
      list.pop();
      Bitset in(value_counter, 0), out(value_counter, 0);
      auto successors = cfg->getSuccessors();
      for(auto successor : successors[inst]) {
        out |= this->in[successor];
      }
      in = gen[inst] | (out - kill[inst]);
      DEBUG_COUT << "instruction: " << inst << " "<< in << " " << out << std::endl;
      // inst->print(std::cerr);
      // std::cerr << std::endl;
      if(this->in[inst] != in || this->out[inst] != out) {
        auto predecessors = cfg->getPredecessors();
        for(auto predecessor :predecessors[inst]) {
          list.push(predecessor);
        }
        //list.push(inst);
      }  
      this->in[inst] = in;
      this->out[inst] = out;
    }
    DEBUG_PRINT("in and out set finished");
  }


  void LivenessAnalysis::visit(Item* item) {
    item->accept(this);
  }

  void LivenessAnalysis::visit(Value* value) {
    value->accept(this);
  }

  void LivenessAnalysis::visit(Register* reg) {
    if(reg->get_code() == "rsp") {
      return;
    }
    parsed_values.push_back(reg);
    GenerateBitSet(reg);
  }

  void LivenessAnalysis::visit(Variable* var) {
    parsed_values.push_back(var);
    GenerateBitSet(var);
  }

  void LivenessAnalysis::visit(MemoryAddress* address) {
    auto m = address->get_value();
    m->accept(this);
  }

  void LivenessAnalysis::visit(Comparison* cmp) {
    auto items = cmp->get_items();
    items.first->accept(this);
    items.second->accept(this);
  }

  void LivenessAnalysis::visit(Number* num) {
    
  }

  void LivenessAnalysis::visit(Operator* op ) {

  }

  void LivenessAnalysis::visit(Label* label) {

  }

  void LivenessAnalysis::visit(FunctionName* name) {

  }

  void LivenessAnalysis::visit(Instruction* inst) {
    inst->accept(this);
  }

  void LivenessAnalysis::visit(InstructionArithmetic* inst) {
    auto v = inst->get_items();
    v.first->accept(this);
    for(auto value : parsed_values) {
      if(value == v.first) {
        kill[inst] |= value_mapper[value];
        gen[inst] |= value_mapper[value];
      } else {
        gen[inst] |= value_mapper[value];
      }
    }
    parsed_values.clear();
    v.second->accept(this);
    for(auto value : parsed_values) {
      gen[inst] |= value_mapper[value];
    }
    parsed_values.clear();
  }

  void LivenessAnalysis::visit(InstructionCall* inst) {
    auto u = inst->get_item();
    u->accept(this);
    for(auto value : parsed_values) {
      gen[inst] |= value_mapper[value];
    }
    parsed_values.clear();

    auto n = inst->get_number();
    auto registers = Register::getAllCallerSavedRegisters();
    for(auto i = 0LL; i < std::min(n->get_value(), (int64_t)6); i++) {
      this->visit(registers[i]);
    }
    for(auto value : parsed_values) {
      gen[inst] |= value_mapper[value];
    }
    parsed_values.clear();

    for(auto reg : registers) {
      this->visit(reg);
    }
    for(auto value : parsed_values) {
      kill[inst] |= value_mapper[value];
    }
    parsed_values.clear();
  }

  void LivenessAnalysis::visit(InstructionConditionalJump* inst) {
    auto cmp = inst->get_comparison();
    auto items = cmp->get_items();
    items.first->accept(this);
    items.second->accept(this);
    for(auto value : parsed_values) {
      gen[inst] |= value_mapper[value];
    }
    parsed_values.clear();
  }

  void LivenessAnalysis::visit(InstructionJump* inst) {
    
  }

  void LivenessAnalysis::visit(InstructionLeaq* inst) {
    auto items = inst->get_items();
    if(items.size() != 3) {
      DEBUG_PRINT("leaq inst generated error");
      abort();
    }
    auto v1 = items[0], v2 = items[1], v3 = items[2];
    v1->accept(this);
    // auto v = parsed_values.back();
    // parsed_values.pop_back();
    // kill[inst] |= value_mapper[v];

    v2->accept(this);
    // v = parsed_values.back();
    // parsed_values.pop_back();
    // gen[inst] |= value_mapper[v];
    v3->accept(this);
    // v = parsed_values.back();
    // parsed_values.pop_back();
    // gen[inst] |= value_mapper[v];
    for(int i = 0; i < parsed_values.size(); i++) {
      auto v = parsed_values[i];
      if(i == 0) {
        kill[inst] |= value_mapper[v];
      } else {
        gen[inst] |= value_mapper[v];
      }
    }
    parsed_values.clear();
  }

  void LivenessAnalysis::visit(InstructionSelfOperation* inst) {
    auto w = inst->get_item();
    w->accept(this);
    for(auto value : parsed_values) {
      gen[inst] |= value_mapper[value];
      kill[inst] |= value_mapper[value];
    }
    parsed_values.clear();
  }

  void LivenessAnalysis::visit(InstructionAssignment* inst) {
    auto items = inst->get_items();
    items.first->accept(this);
    for(auto value : parsed_values) {
      if(value == items.first) {
        kill[inst] |= value_mapper[value];
      } else {
        gen[inst] |= value_mapper[value];
      }
    }
    parsed_values.clear();
    items.second->accept(this);
    for(auto value : parsed_values) {
      gen[inst] |= value_mapper[value];
    }
    parsed_values.clear();
  }

  void LivenessAnalysis::visit(InstructionLabel* inst) {

  }

  void LivenessAnalysis::visit(InstructionReturn* inst) {
    auto registers = Register::getAllCalleeSavedRegisters();
    for(auto reg : registers) {
      this->visit(reg);
    }
    this->visit(Register::GetInstance("rax"));
    for(auto value : parsed_values) {
      gen[inst] |= value_mapper[value];
    }
    parsed_values.clear();
  }

  void LivenessAnalysis::visit(InstructionStackArg* inst) {
    auto i = inst->get_item();
    this->visit(i);
    for(auto value : parsed_values) {
      kill[inst] |= value_mapper[value];
    }
    parsed_values.clear();
  }

  void LivenessAnalysis::visit(Function* f) {
    for(auto inst : f->instructions) {
      inst->accept(this);
    }
  }
}