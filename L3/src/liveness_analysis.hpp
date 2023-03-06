#pragma once
#pragma GCC optimize("O3","unroll-loops")
#include <boost/dynamic_bitset/dynamic_bitset.hpp>

#include "liveness_analysis.hpp"
#include "control_flow_analysis.hpp"
#include "L3.hpp"

namespace L3 {
  class LivenessAnalysis {
    using bitset = boost::dynamic_bitset<>;
    public:
      LivenessAnalysis() = delete;
      LivenessAnalysis(Function* f);
      std::set<Variable*> GetInSet(Instruction* inst);
      std::set<Variable*> GetOutSet(Instruction* inst);
    private:
      Function* f_;
      ControlFlowAnalysis* analyze_;
      std::map<Instruction*, bitset> gen_, kill_, in_, out_;
      std::map<Variable*, bitset> variable_mapper;
      void GenerateBitSet();
      void GenerateInAndOut();
  };
}