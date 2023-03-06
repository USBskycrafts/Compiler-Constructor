#pragma once

#include "iostream"

#ifdef DEBUG_
  #define DEBUG_PRINT(...)                                                                           \
   fprintf(stdout, "DEBUG %s : in function %s, line %d ", __FILE__, __FUNCTION__, __LINE__);   \
   fprintf(stdout, __VA_ARGS__);                                                               \
   fprintf(stdout, "\n");
  #define DEBUG_COUT                                                                  \
  std::cout << "DEBUG " << __FILE__ << " : in function " << __FUNCTION__ << ", line " \
  << __LINE__ << " "
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_COUT 0 && std::cout
#endif
