#pragma once

#include "L2.hpp"

namespace L2 {

  struct SpillInput {
    Function* f;
    std::string prefix;
    Variable* var;
  };

  SpillInput parse_spill_file(char *fileName);

  Program parse_function_file(char *fileName);

  Program parse_file(char *fileName);
}