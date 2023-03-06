

#include "register_allocator.hpp"
#include "L2.hpp"
#include "code_generator.hpp"
#include "color_selector.hpp"
#include "spiller.hpp"
#include "utils.hpp"
#include "printer.hpp"
namespace L2 {


  Function* RegisterAllocator::getFunction() {
    return f_;
  }

  RegisterAllocator::RegisterAllocator(Function* f)
  : f_orig(f), f_(f) {
    ColorSelector* selector;
    selector = new ColorSelector(f_);
    std::string prefix = "%___S";
    do {
      DEBUG_COUT << "a new loop" << std::endl;
      if(auto spilled_variable = selector->getSpilledValue()) { 
        if(pre_variables.count(spilled_variable)) {
          Spiller* spiller = new Spiller(f_orig, "%___S");
          f_ = spiller->spill();
          DEBUG_COUT << "break the loop" << std::endl;
          break;
        }
        pre_variables.insert(spilled_variable);
        DEBUG_COUT << spilled_variable->get_code() << " " << prefix << std::endl;      
        if(spilled_variable->get_code().find(prefix) != std::string::npos) {
          DEBUG_COUT << "going to spill all" << std::endl;
          Spiller* spiller = new Spiller(f_orig, "%___S");
          f_ = spiller->spill();
          DEBUG_COUT << "break the loop" << std::endl;
          break;
        } else {
          DEBUG_COUT << "try to spill " << spilled_variable << std::endl;
          auto spiller = new Spiller(f, "%___S");
          f_ = spiller->spill(spilled_variable);
        } 
      } else {
        color_mapper = selector->getColorMap();
        DEBUG_COUT << "finished, getting colormap" << std::endl;
      }
      selector = new ColorSelector(f_);
    }while(selector->getSpilledValue());
    // Printer printer;
    // printer.visit(f_);
  }

  std::map<Value*, unsigned> RegisterAllocator::GetColorMap() {
    return color_mapper;
  }

}