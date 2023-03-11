#pragma once

#include "iostream"
#include <string>

extern bool log_open;

#ifdef DEBUG
  #define DEBUG_PRINT(...)                                                                           \
   fprintf(stderr, "DEBUG %s : in function %s, line %d ", __FILE__, __FUNCTION__, __LINE__);   \
   fprintf(stderr, __VA_ARGS__);                                                               \
   fprintf(stderr, "\n");
  #define DEBUG_COUT                                                                  \
  log_open && std::cerr << "DEBUG " << __FILE__ << " : in function " << __FUNCTION__ << ", line " \
  << __LINE__ << " "
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_COUT 0 && std::cout
#endif

#define INDENT(x) std::string(x, '\t')
