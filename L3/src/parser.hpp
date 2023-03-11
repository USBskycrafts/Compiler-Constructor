#pragma once

#include <set>
#include <map>
#include "L3.hpp"

namespace L3 {
  Program parse_file(char* fileName);

  struct LabelComparator {
    bool operator()(const Label* l1, const Label* l2) const {
      return l1->name < l2->name;
    }
  };
  
  using LabelSet = std::set<Label*, LabelComparator>;
  extern std::map<Function*, LabelSet> labels;
}