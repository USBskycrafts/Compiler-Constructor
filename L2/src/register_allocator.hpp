#pragma once


#include "code_generator.hpp"
#include "register_allocator.hpp"
#include "L2.hpp"
#include "color_selector.hpp"
#include "spiller.hpp"


namespace L2 {
  class RegisterAllocator {
    public:
      RegisterAllocator(Function* f);
      std::map<Value*, unsigned> GetColorMap();
      Function* getFunction();
    private:
      Function* f_orig;
      Function* f_;
      std::map<Value*, unsigned> color_mapper;
      std::set<Variable*> pre_variables;
  };
}