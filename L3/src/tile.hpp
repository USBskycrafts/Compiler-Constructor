#pragma once
#include "L3.hpp"

namespace Tiling {

  class Node {

  };

  class Tree {

  };

  class Context {
    public:

    private:
      std::vector<std::string> generated_codes_;
  };

  class Tile {

  };

  void GenerateContext(L3::Function* function);

  void MergingTree(Context* context);
}