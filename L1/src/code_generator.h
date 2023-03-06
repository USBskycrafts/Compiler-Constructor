#pragma once

#include "L1.h"

namespace L1{

  void generate_code(Program p);

  std::string generate_function_code(Function *f);

  std::string generate_instruction_code(Instruction *inst);

  std::string generate_component(Item *t);

  std::string register_q2b(Register* reg);
}
