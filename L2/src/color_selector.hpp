#pragma once
#include <stack>
#include <set>
#pragma GCC optimize("O3","unroll-loops")
#include "L2.hpp"
#include "interfernce_graph.hpp"
#include "spiller.hpp"

namespace L2 {
  class ColorSelector {
    public:
      ColorSelector(Function* f);
      Variable* getSpilledValue();
      std::map<Value*, unsigned> getColorMap();
    private:
      void allocateRegisterColor();
      bool allocateVariableColor();
      void eraseNode();
      bool assignColor();
      InterferenceGraph* graph;
      Function* f;
      Variable* spilled_value = nullptr;
      std::map<Value*, unsigned> color_mapper;
      std::map<Value*, std::set<Value*>> value_mapper;
      std::stack<std::pair<Value*, std::set<Value*>>> node_stack;
  };
}