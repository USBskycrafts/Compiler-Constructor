
#include "interfernce_graph.hpp"
#include "L2.hpp"
#include "liveness_analysis.hpp"
#include "utils.hpp"

namespace L2 {
  InterferenceGraph::InterferenceGraph(Function* f)
  : f(f), analyzer(new LivenessAnalysis(f)) {
    Generate();
  }

  void InterferenceGraph::Generate() {
    auto in_mapper = analyzer->GetInSet();
    auto out_mapper = analyzer->GetOutSet();
    auto kill_mapper = analyzer->GetKillSet();

    auto registers = Register::getGeneralPurposeRegisters();
    for(auto r1 : registers) {
      for(auto r2 : registers) {
        if(r1 != r2) {
          value_mapper[r1].insert(r2);
          value_mapper[r2].insert(r1);
          DEBUG_COUT << r1->get_code() << " " << r2->get_code() << " interfere" << std::endl;
        }  
      }
    }

    for(auto inst : f->instructions) {
      auto in_set = in_mapper[inst];
      auto out_set = out_mapper[inst];
      auto kill_set = kill_mapper[inst];
      // for(auto value : in_set) {
      //   value_mapper[value] = {};
      // }
      // for(auto value : out_set) {
      //   value_mapper[value] = {};
      // }
      // for(auto value : kill_set) {
      //   value_mapper[value] = {};
      // }
      for(auto v1 : in_set) {
        for(auto v2 : in_set) {
          if(v1 != v2) {
            value_mapper[v1].insert(v2);
            value_mapper[v2].insert(v1);
            DEBUG_COUT << v1->get_code() << " " << v2->get_code() << " interfere" << std::endl;
          }
        }
      }
      for(auto v1 : out_set) {
        for(auto v2 : out_set) {
          if(v1 != v2) {
            value_mapper[v1].insert(v2);
            value_mapper[v2].insert(v1);
            DEBUG_COUT << v1->get_code() << " " << v2->get_code() << " interfere" << std::endl;
          }
        }
      }

      for(auto v1 : kill_set) {
        for(auto v2 : out_set) {
          if(v1 != v2) {
            value_mapper[v1].insert(v2);
            value_mapper[v2].insert(v1);
            DEBUG_COUT << v1->get_code() << " " << v2->get_code() << " interfere" << std::endl;
          }
        }
      }

      if(inst->get_type() == Instruction::kArithmetic) {
        auto exp = dynamic_cast<InstructionArithmetic*>(inst);
        auto v2 = exp->get_items().second;
        auto op = exp->get_operator();
        if(v2->get_type() == Item::kVariable 
        && (op->get_code() == "<<="
        || op ->get_code() == ">>=")) {
          auto value = dynamic_cast<Variable*>(v2);
          for(auto reg : registers) {
            if(reg->get_code() != "rcx") {
              value_mapper[reg].insert(value);
              value_mapper[value].insert(reg);
              DEBUG_COUT << reg->get_code() << " " << value->get_code() << " interfere" << std::endl;
            }
            
          }
        }
      }
    }
  }

  std::map<Value*, std::set<Value*>> InterferenceGraph::getValueMapper() {
    return value_mapper;
  }
}