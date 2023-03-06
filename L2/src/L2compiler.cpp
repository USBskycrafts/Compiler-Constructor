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

#include "L2.hpp"
#include "code_generator.hpp"
#include "interfernce_graph.hpp"
#include "liveness_analysis.hpp"
#include "parser.hpp"
#include "printer.hpp"
#include "register_allocator.hpp"
#include "spiller.hpp"
#include "utils.hpp"


void print_help (char *progName){
  std::cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] [-s] [-l] [-i] SOURCE" << std::endl;
  return ;
}

int main(
  int argc, 
  char **argv
  ){
  auto enable_code_generator = true;
  auto spill_only = false;
  auto interference_only = false;
  auto liveness_only = false;
  int32_t optLevel = 3;

  /* 
   * Check the compiler arguments.
   */
  auto verbose = false;
  if( argc < 2 ) {
    print_help(argv[0]);
    return 1;
  }
  int32_t opt;
  int64_t functionNumber = -1;
  while ((opt = getopt(argc, argv, "vg:O:sli")) != -1) {
    switch (opt){

      case 'l':
        liveness_only = true;
        break ;

      case 'i':
        interference_only = true;
        break ;

      case 's':
        spill_only = true;
        break ;

      case 'O':
        optLevel = strtoul(optarg, NULL, 0);
        break ;

      case 'g':
        enable_code_generator = (strtoul(optarg, NULL, 0) == 0) ? false : true ;
        break ;

      case 'v':
        verbose = true;
        break ;

      default:
        print_help(argv[0]);
        return 1;
    }
  }

  /*
   * Parse the input file.
   */
  L2::Program p;
  L2::SpillInput input;
  if (spill_only){

    /* 
     * Parse an L2 function and the spill arguments.
     */
    input = L2::parse_spill_file(argv[optind]);
 
  } else if (liveness_only){

    /*
     * Parse an L2 function.
     */
    p = L2::parse_function_file(argv[optind]);
   
  } else if (interference_only){

    /*
     * Parse an L2 function.
     */
    p = L2::parse_function_file(argv[optind]);

  } else {

    /* 
     * Parse the L2 program.
     */
    p = L2::parse_file(argv[optind]);
  }

  /*
   * Special cases.
   */
  if (spill_only){

    /*
     * Spill.
     */
    //TODO
    L2::Variable *var = input.var;
    L2::Spiller spiller(input.f, input.prefix);
    auto f = spiller.spill(var);
    /*
     * Dump the L2 code.
     */
    //TODO
    L2::Printer printer;
    printer.visit(f);
    return 0;
  }

  /*
   * Liveness test.
   */
  if (liveness_only){
    //TODO
    auto analyzer = L2::LivenessAnalysis(p.functions.back());
    auto in = analyzer.GetInSet();
    auto out = analyzer.GetOutSet();
    auto f = p.functions.back();

    std::cout << "(\n";
    std::cout << "(in\n";
    L2::Printer printer;
    // for(auto [inst, set] : in) {
    //   std::cout << "(";
    //   for(auto iter = set.begin(); iter != set.end(); iter++) {
    //     if(iter != set.begin()) {
    //       std::cout << " ";
    //     }
    //     printer.visit(*iter);
    //   } 
    //   std::cout << ")\n";
    // }
    for(auto inst : f->instructions) {
      std::cout << "(";
      auto set = in.at(inst);
      for(auto iter = set.begin(); iter != set.end(); iter++) {
        if(iter != set.begin()) {
          std::cout << " ";
        }
        printer.visit(*iter);
      } 
      std::cout << ")\n";
    }
    std::cout << ")\n\n";
    std::cout << "(out\n";

    // for(auto [inst, set] : out) {
    //   std::cout << "(";
    //   for(auto iter = set.begin(); iter != set.end(); iter++) {
    //     if(iter != set.begin()) {
    //       std::cout << " ";
    //     }
    //     printer.visit(*iter);
    //   }
    //   std::cout << ")\n"; 
    // }
    for(auto inst : f->instructions) {
      std::cout << "(";
      //printer.visit(inst);
      auto set = out.at(inst);
      for(auto iter = set.begin(); iter != set.end(); iter++) {
        if(iter != set.begin()) {
          std::cout << " ";
        }
        printer.visit(*iter);
      } 
      std::cout << ")\n";
    }
    std::cout << ")\n\n";
    std::cout << ")\n";
    return 0;
  }

  /*
   * Interference graph test.
   */
  if (interference_only){
    //TODO
    L2::InterferenceGraph* graph = new L2::InterferenceGraph(p.functions.back());
    auto value_mapper = graph->getValueMapper();
    L2::Printer printer;
    for(auto [value, value_set] : value_mapper) {
      printer.visit(value);
      std::cout << " ";
      for(auto v : value_set) {
        printer.visit(v);
        std::cout << " ";
      }
      std::cout << std::endl;
    }
    return 0;
  }

  L2::RegisterAllocator* allocator;
  L2::Printer printer;
  freopen("prog.L1", "w", stdout); 
  std::cout << "("  << p.entryPointLabel << std::endl;
  for(auto f : p.functions) {
    allocator = new L2::RegisterAllocator(f);
    auto color_mapper = allocator->GetColorMap();  
    //printer.visit(allocator->getFunction());
    auto generator = new L2::CodeGenerator(allocator->getFunction());
    auto generated_f = generator->getFunction();
    printer.visit(generated_f);
    std::cout << std::endl;
  }
  std::cout << ")\n";

  /*
   * Generate the target code.
   */
  if (enable_code_generator){
    //TODO
  }

  return 0;
}
