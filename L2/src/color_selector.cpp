#include <cstdlib>
#include <ios>
#include <queue>
#include <bitset>
#include "color_selector.hpp"
#include "L2.hpp"
#include "interfernce_graph.hpp"
#include "spiller.hpp"
#include "utils.hpp"
#include "printer.hpp"

namespace L2 {

  struct cmp {
    public:
      bool operator()(std::pair<Value*, std::set<Value*>> a, std::pair<Value*, std::set<Value*>> b) {
      return a.second.size() < b.second.size();
      }
  };
 

  ColorSelector::ColorSelector(Function* f) 
  : f(f), graph(new InterferenceGraph(f)) {
    value_mapper = graph->getValueMapper();
    allocateRegisterColor();
    allocateVariableColor();
  }

  void ColorSelector::allocateRegisterColor() {
    unsigned color = 1;
    for(auto reg : Register::getGeneralPurposeRegisters()) {
      color_mapper[reg] = color;
      //DEBUG_COUT << reg->get_code() << " " << std::hex << color << std::endl;
      color <<= 1;
    }
  }

  bool ColorSelector::allocateVariableColor() {
    if(spilled_value) {
      return false;
    }
    eraseNode();
    return assignColor();
  }

  bool ColorSelector::assignColor() {
    DEBUG_COUT << node_stack.size() << std::endl;
    while(!node_stack.empty()) {
      auto [value, v_set] = node_stack.top();
      node_stack.pop();
      unsigned color = 0;
      DEBUG_COUT << "in :" << value << std::endl;
      for(auto v : v_set) {
        color |= color_mapper[v];
        DEBUG_COUT << v->get_code() << " bit is : " << std::bitset<sizeof(color_mapper[v]) * 8>(color_mapper[v]) << std::endl;
        DEBUG_COUT << "now color is :" <<  std::bitset<sizeof(color) * 8>(color) << std::endl;
      }
      // if(color == ~(1 << 15)) {
      //   spilled_value = dynamic_cast<Variable*>(value);
      //   if(spilled_value == nullptr) {
      //     DEBUG_COUT << "check the code!" << std::endl;
      //     abort();
      //   }
      //   return false;
      // } else {
      //   color = ~color;
      //   unsigned bit = 1;
      //   while(!(color & 1)) {
      //     bit <<= 1;
      //     color = color >> 1;
      //   }
      //   DEBUG_COUT << value->get_code() << ": bitset is " << std::bitset<sizeof(bit) * 8>(bit) << std::endl;
      //   color_mapper[value] = bit;
      // }
      int i = 1;
      for(i = 1; i <= 1 << 14; i <<= 1) {
        if((i & color) == 0) {
          color_mapper[value] = i;
          DEBUG_COUT << value->get_code() << ": bitset is " << std::bitset<sizeof(i) * 8>(i) << std::endl;
          break;
        }
      }
      if(i == (1 << 15)) {
        spilled_value = dynamic_cast<Variable*>(value);
        if(spilled_value == nullptr) {
          DEBUG_COUT << "check the code" << std::endl;
          abort();
        }
        return false;
      }
    }
    return true;
  }


  Variable* ColorSelector::getSpilledValue() {
    if(allocateVariableColor()) {
      return nullptr;
    } else {
      return this->spilled_value;
    }
   }


  void ColorSelector::eraseNode() {
    bool has_leaved = false;
    std::queue<Value*> list;
    do {
      has_leaved = false;
      for(auto [value, v_set] : value_mapper) {
        auto size = Register::getGeneralPurposeRegisters().size();
        if(v_set.size() < size && value->get_type() == Item::kVariable) {
          has_leaved = true;
          node_stack.push({value, v_set});
          color_mapper[value] = 0;
          DEBUG_COUT << node_stack.size() << std::endl;
          DEBUG_COUT << value->get_code() << std::endl;
          // for(auto v : v_set) {
          //   std::cout << v->get_code() << " ";
          // }
          // std::cout << std::endl;
          list.push(value);
          for(auto v : v_set) {
            value_mapper[v].erase(value);
          }
        }
      }
      while(!list.empty()) {
        auto v = list.front();
        list.pop();
        value_mapper.erase(v);
      }
    }while(has_leaved);
    // do {
    //   unsigned max_size = 0;
    //   Value* removed_node = nullptr;
    //   has_leaved = false;
    //   for(auto [v, v_set] : value_mapper) {
    //     if(v->get_type() == Item::kVariable) {
    //       has_leaved = true;
    //       if(max_size < v_set.size()) {
    //         removed_node = v;
    //         max_size = v_set.size();
    //       }
    //     }
    //   }
    //   if(removed_node) {
    //     node_stack.push({removed_node, value_mapper[removed_node]});
    //   }
    //   for(auto v : value_mapper[removed_node]) {
    //     value_mapper[v].erase(removed_node);
    //   }
    //   list.push(removed_node);
    // }while(has_leaved);
    std::priority_queue<std::pair<Value*, std::set<Value*>>, 
    std::vector<std::pair<Value*, std::set<Value*>>>, cmp> work_list;
    for(auto [v, v_set] : value_mapper) {
      if(v->get_type() == Item::kVariable) {
        work_list.push({v, v_set});
      }
    }
    while(!work_list.empty()) {
      auto [v, v_set] = work_list.top();
      DEBUG_COUT << v->get_code()  << "  " << v_set.size()  << std::endl;
      work_list.pop();
      node_stack.push({v, v_set});
      color_mapper[v] = 0;
      for(auto vv : v_set) {
        DEBUG_COUT << vv->get_code() << std::endl;
        value_mapper[vv].erase(v);
      }
      value_mapper.erase(v);
    }
  }

  std::map<Value*, unsigned> ColorSelector::getColorMap() {
    if(spilled_value) {
      return {{nullptr, 0}};
    }
    return color_mapper;
  }
}