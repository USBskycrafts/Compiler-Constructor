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

#include "parser.hpp"
#include "liveness-analysis.hpp"
#include "utils.hpp"
#include "tiling.hpp"


bool log_open = false;

void print_help (char *progName){
  std::cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] SOURCE" << std::endl;
  return ;
}

int main(
  int argc, 
  char **argv
  ){
  auto enable_code_generator = true;
  int32_t optLevel = 0;
  bool verbose;

  /* 
   * Check the compiler arguments.
   */
  if( argc < 2 ) {
    print_help(argv[0]);
    return 1;
  }
  int32_t opt;
  while ((opt = getopt(argc, argv, "vg:O:")) != -1) {
    switch (opt){
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
  auto p = L3::parse_file(argv[optind]);
  /*
   * Code optimizations (optional)
   */

  /* 
   * Print the source program.
   */
  if (verbose){
    log_open = true;
  }

  /*
   * Generate x86_64 assembly.
   */
  for(auto f : p.functions) {
    LivenessAnalysis::Analyze(f);
    auto contexts = Tiling::GenerateContext(f);
    for(auto context : contexts) {
      auto trees = Tiling::GenerateTree(context);
      trees = Tiling::MergingTree(trees);
    }
  }
  

  return 0;
}
