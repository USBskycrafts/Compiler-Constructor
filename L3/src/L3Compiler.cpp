#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <iostream>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <unistd.h>
#include <iostream>
#include <assert.h>

#include "L3.hpp"
#include "L3Compiler.hpp"
#include "L3Compiler.hpp"
#include "code_generator.hpp"
#include "context_generator.hpp"
#include "liveness_analysis.hpp"
#include "tree_generator.hpp"
#include "utils.hpp"


int main(int argc, char **argv) {
    auto enable_code_generator = true;
    int32_t optLevel = 3;

    /* 
    * Check the compiler arguments.
    */
    auto verbose = false;
    int32_t opt;
    int64_t functionNumber = -1;

    /*
    * Parse the input file.
    */
    L3::Program p;
      /* 
      * Parse the L2 program.
      */
    p = L3::parse_file(argv[optind]);

    /*
    * DEBUG part
    */
    for(auto f : p.get_functions()) {
      auto analyze = L3::LivenessAnalysis(f);
      for(auto inst : f->get_instructions()) {
        DEBUG_COUT << inst->to_string();
        for(auto variable : analyze.GetInSet(inst)) {
          DEBUG_COUT << "in set ---> " << variable->to_string() << std::endl;
        }
        for(auto variable : analyze.GetOutSet(inst)) {
          DEBUG_COUT << "out set ---> " << variable->to_string() << std::endl;
        }
        DEBUG_COUT << std::endl;
      }
      
    }
    

    /*
    * Generate the target code.
    */
    if (enable_code_generator){
      //TODO
      // for(auto f : p.get_functions()) {
      //   std::cout << f->to_string() << std::endl;
      // }
      // for(auto f : p.get_functions()) {
      //   auto generator = new L3::ContextGenerator(f);
      // }
      std::stringstream code;
      /*
      * Naive Code Generator
      */
      // code << "(@main" << std::endl;
      // for(auto f : p.get_functions()) {
      //   code << "(" << f->get_name() << " " << f->get_arguments().size() << std::endl;
      //   auto code_generator = new L3::NaiveGenerator(f);
      //   code << code_generator->get_L2Code() << std::endl;
      //   code << ")" << std::endl;
      // }
      // code << ")" << std::endl;


      /*
      * Merged Code Generator
      */
      code << "(@main" << std::endl;
      for(auto f : p.get_functions()) {
        code << "(" << f->get_name() << " " << f->get_arguments().size() << std::endl;
        auto arguments = f->get_arguments();
        for(int i = 1; i <= arguments.size(); i++) {
          switch (i) {
            case 1: {
              code << arguments[i - 1]->to_string() << " <- rdi " << std::endl;
              break;
            }
            case 2: {
              code << arguments[i - 1]->to_string() << " <- rsi " << std::endl;
              break;
            }
            case 3: {
              code << arguments[i - 1]->to_string() << " <- rdx " << std::endl;
              break;
            }
            case 4: {
              code << arguments[i - 1]->to_string() << " <- rcx " << std::endl;
              break;
            }
            case 5: {
              code << arguments[i - 1]->to_string() << " <- r8 " << std::endl;
              break;
            }
            case 6: {
              code << arguments[i - 1]->to_string() << " <- r9 " << std::endl;
              break;
            }
            default: {
              code << arguments[i - 1]->to_string() << " <- stack-arg "
              << std::to_string((arguments.size() - i) * 8) << std::endl;
              break;
            }
          }
        }
        auto context_generator = new L3::ContextGenerator(f);
        auto analyzer = L3::LivenessAnalysis(f);
        for(auto context : context_generator->get_contexts()) {
          std::map<L3::Instruction*, std::set<L3::Variable*>> out_set_mapper_;
          for(auto inst : context) {
            auto out_set  = analyzer.GetOutSet(inst);
            out_set_mapper_[inst].insert(out_set.begin(), out_set.end());
          }
          auto tree_generator = new L3::TreeGenerator(context, out_set_mapper_);
          code << tree_generator->CodeGenerate() << std::endl;
        }
        code << ")" << std::endl;
      }
      code << ")" << std::endl;
      
    

      freopen("prog.L2", "w", stdout);
      std::cout << code.str() << std::endl;
    }

    return 0;
  }