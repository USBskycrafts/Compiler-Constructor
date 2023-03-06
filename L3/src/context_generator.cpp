
#include <queue>
#include "L3.hpp"
#include "context_generator.hpp"
#include "utils.hpp"

namespace L3 {
  ContextGenerator::ContextGenerator(Function* f) {
    generate_contexts(f);
  }

  std::vector<std::vector<Instruction*>> ContextGenerator::get_contexts() {
    return contexts;
  }

  void ContextGenerator::generate_contexts(Function* f) {
    contexts.push_back({});
    for(auto inst : f->get_instructions()) {
      this->visit(inst);
    }
    std::vector<std::vector<Instruction*>> tmp_contexts;
    for(auto context : contexts) {
      if(!context.empty()) {
        tmp_contexts.push_back(context);
      }
    }
    contexts = tmp_contexts;
    for(auto context : contexts) {
      for(auto inst : context) {
        DEBUG_COUT << inst->to_string();
      }
    }
  }



  void ContextGenerator::visit(Item* item) {

  }

  void ContextGenerator::visit(Variable* var) {

  }

  void ContextGenerator::visit(Number* num) {

  }

  void ContextGenerator::visit(Operator* op) {

  }

  void ContextGenerator::visit(Label* label) {

  }

  void ContextGenerator::visit(FunctionName* name) {

  }

  void ContextGenerator::visit(Instruction* inst) {
    DEBUG_COUT << inst->to_string() << std::endl;
    inst->accept(this);
  }

  void ContextGenerator::visit(AssignInst* inst) {
    auto& context = contexts.back();
    context.push_back(inst);
    DEBUG_COUT << inst->to_string() << "---->with contexts: " << contexts.size() 
    << " and context: " << context.size() << std::endl;
  }

  void ContextGenerator::visit(StoreInst* inst) {
    auto& context = contexts.back();
    context.push_back(inst);
    DEBUG_COUT << inst->to_string() << "---->with contexts: " << contexts.size() 
    << " and context: " << context.size() << std::endl;
  }

  void ContextGenerator::visit(LoadInst* inst) {
    auto& context = contexts.back();
    context.push_back(inst);
    DEBUG_COUT << inst->to_string() << "---->with contexts: " << contexts.size() 
    << " and context: " << context.size() << std::endl;
  }

  void ContextGenerator::visit(BinaryOperator* inst) {
    auto& context = contexts.back();
    context.push_back(inst);
    DEBUG_COUT << inst->to_string() << "---->with contexts: " << contexts.size() 
    << " and context: " << context.size() << std::endl;
  }

  void ContextGenerator::visit(CmpInst* inst) {
    auto& context = contexts.back();
    context.push_back(inst);
    DEBUG_COUT << inst->to_string() << "---->with contexts: " << contexts.size() 
    << " and context: " << context.size() << std::endl;
  }

  void ContextGenerator::visit(LabelInst* inst) {
    DEBUG_COUT << contexts.size() << std::endl;
    std::vector<Instruction*> context = {inst};
    contexts.push_back(context);
    DEBUG_COUT << inst->to_string() << "---->with contexts: " << contexts.size() 
    << " and context: " << context.size() << std::endl;
    contexts.push_back({});
  }

  void ContextGenerator::visit(CallInst* inst)  {
    std::vector<Instruction*> context = {inst};
    contexts.push_back(context);
    DEBUG_COUT << inst->to_string() << "---->with contexts: " << contexts.size() 
    << " and context: " << context.size() << std::endl;
    contexts.push_back({});
  }

  void ContextGenerator::visit(ReturnInst* inst) {
    auto& context = contexts.back();
    context.push_back(inst);
    DEBUG_COUT << inst->to_string() << "---->with contexts: " << contexts.size() 
    << " and context: " << context.size() << std::endl;
    contexts.push_back({});
  }

  void ContextGenerator::visit(BranchInst* inst) {
    auto& context = contexts.back();
    context.push_back(inst);
    DEBUG_COUT << inst->to_string() << "---->with contexts: " << contexts.size() 
    << " and context: " << context.size() << std::endl;
    contexts.push_back({});
  }

  void ContextGenerator::visit(Function* f) {
    
  }

}