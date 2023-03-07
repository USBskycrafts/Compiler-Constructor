#pragma once
#include <map>

#pragma GCC optimize("O3","unroll-loops")
#include "interfernce_graph.hpp"
#include "L2.hpp"
#include "liveness_analysis.hpp"

namespace L2 {
  class InterferenceGraph {
    public:
      InterferenceGraph(Function* f);  
      std::map<Value*, std::set<Value*>> getValueMapper();
    private:
      std::map<Value*, std::set<Value*>> value_mapper;
      void Generate();
      Function* f;
      LivenessAnalysis* analyzer;
  };
}