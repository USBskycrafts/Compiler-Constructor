#include <sstream>
#include <algorithm>

#include "L3.hpp"
#include "utils.hpp"
#include "visitor.hpp"

namespace L3 {

  std::map<std::string, Variable*> Variable::variable_mapper;
  std::map<std::string, Operator*> Operator::op_mapper;
  std::map<std::string, Label*> Label::label_mapper;
  std::map<std::string, Number*> Number::number_mapper;
  std::map<std::string, FunctionName*> FunctionName::function_name_mapper;

  std::vector<Item*> CallInst::parsed_args;
  Variable* CallInst::parsed_ret_value;
  Item* CallInst::parsed_callee;

  int Label::re_labeled_count = 0;


  void Item::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  std::string Item::to_string() {
    return this->symbols;
  }

  template <typename T>
  std::string Item::to_string(T* item) {
    return item->symbols;
  }

  Variable* Variable::get_variable(const std::string &name) {
    if(variable_mapper.count(name) == 0) {
      Variable* variable = new Variable;
      variable_mapper.insert({name, variable});
      variable->symbols = name;
    }
    return variable_mapper.at(name);
  }

  void Variable::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  std::string Variable::to_string() {
    return Item::to_string<Variable>(this);
  }

  Operator* Operator::get_operator(const std::string& name) {
    if(op_mapper.count(name) == 0) {
      Operator* op = new Operator;
      op_mapper.insert({name, op});
      op->symbols = name;
    }
    return op_mapper.at(name);
  }

  void Operator::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  std::string Operator::to_string() {
    return Item::to_string<Operator>(this);
  }

  Label* Label::get_label(const std::string& name) {
    if(label_mapper.count(name) == 0) {
      Label* label = new Label;
      label_mapper.insert({name, label});
      label->symbols = name;
    }
    return label_mapper.at(name);
  }

  // void Label::re_label() {
  //   std::string n_label;
  //   for(auto [name, label] : Label::label_mapper) {
  //     if(name.size() > n_label.size()) {
  //       n_label = name;
  //     }
  //   }
  //   std::vector<Label*> list;
  //   for(auto [name, label] : label_mapper) {
  //     list.push_back(label);
  //   }
  //   label_mapper.clear();
  //   for(auto label : list) {
  //     label->symbols = n_label + std::to_string(re_labeled_count++);
  //     DEBUG_COUT << label->symbols << std::endl;
  //     label_mapper[label->symbols] = label;
  //   }
  // }

  void Label::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  std::string Label::to_string() {
    return Item::to_string<Label>(this);
  }
  

  Number* Number::get_number(const std::string& name) {
    if(number_mapper.count(name) == 0) {
      Number* number = new Number;
      number_mapper.insert({name, number});
      number->symbols = name;
    }
    return number_mapper.at(name);
  }

  void Number::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  std::string Number::to_string() {
    return Item::to_string<Number>(this);
  }

  FunctionName* FunctionName::get_name(const std::string& name) {
    if(function_name_mapper.count(name) == 0) {
      FunctionName* func_name = new FunctionName;
      function_name_mapper.insert({name, func_name});
      func_name->symbols = name;
    }
    return function_name_mapper.at(name);
  }

  void FunctionName::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  std::string FunctionName::to_string() {
    return Item::to_string<FunctionName>(this);
  }

  bool Instruction::has_item(Item *item) {
    return std::count(items.begin(), items.end(), item);
  }

  void Instruction::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  std::set<Item*> Instruction::get_defs() {
    return this->defs;
  }

  std::set<Item*> Instruction::get_uses() {
    return this->uses;
  }

  std::vector<Item*> Instruction::get_items() {
    return this->items;
  }

  template<typename T> 
  T* Instruction::replace(const T* inst, Item* old_item, Item* new_item) {
    if(std::count(inst->items.begin(), inst->items.end(), old_item) == 0) {
      return nullptr;
    }
    auto inst_copy = new T;
    std::replace(inst_copy->items.begin(), inst_copy->items.end(), 
    old_item, new_item);
    inst_copy->defs.erase(old_item);
    inst_copy->defs.insert(new_item);
    inst_copy->uses.erase(old_item);
    inst_copy->uses.insert(new_item);
    return inst_copy;
  }

  template <typename T>
  T* Instruction::copy(const T* inst) {
    T* inst_copy = new T;
    inst_copy->defs = inst->defs;
    inst_copy->uses = inst->uses;
    inst_copy->items = inst->items;
    return inst_copy;
  }

  AssignInst::AssignInst(Variable* l_value, Item* r_value) {
    this->items.push_back(l_value);
    this->items.push_back(r_value);
    this->defs.insert(l_value);
    this->uses.insert(r_value);
  }

  void AssignInst::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  AssignInst* AssignInst::operator=(const AssignInst* inst) {
    return Instruction::copy<AssignInst>(inst);
  }

  AssignInst* AssignInst::replace(Item *old_item, Item *new_item) const {
    auto inst_copy = Instruction::replace<AssignInst>(this, old_item, new_item);
    return inst_copy;
  }

  std::string AssignInst::to_string() {
    std::stringstream ss;
    ss << this->items[0]->to_string() << " <- " << this->items[1]->to_string() << std::endl;
    return ss.str();
  }

  StoreInst::StoreInst(Variable* l_value, Item* r_value) {
    this->items.push_back(l_value);
    this->items.push_back(r_value);
    this->uses.insert(l_value);
    this->uses.insert(r_value);
  }

  void StoreInst::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  StoreInst* StoreInst::operator=(const StoreInst* inst) {
    return Instruction::copy<StoreInst>(inst);
  }

  StoreInst* StoreInst::replace(Item* old_item, Item* new_item) const {
    auto inst_copy = Instruction::replace<StoreInst>(this, old_item, new_item);
    return inst_copy; 
  }

  std::string StoreInst::to_string() {
    std::stringstream ss;
    ss << "store " << items[0]->to_string() << " <- " << items[1]->to_string() << std::endl;
    return ss.str();
  }

  LoadInst::LoadInst(Variable* l_value, Variable* r_value) {
    this->items.push_back(l_value);
    this->items.push_back(r_value);
    this->defs.insert(l_value);
    this->uses.insert(r_value);
  }

  void LoadInst::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  LoadInst* LoadInst::operator=(const LoadInst* inst) {
    return Instruction::copy<LoadInst>(inst);
  }

  LoadInst* LoadInst::replace(Item* old_item, Item* new_item) const {
    return Instruction::replace(this, old_item, new_item);
  }

  std::string LoadInst::to_string()  {
    std::stringstream ss;
    ss << items[0]->to_string() << " <- load " << items[1]->to_string() << std::endl;
    return ss.str();
  }


  BinaryOperator::BinaryOperator(Variable* l_value, Item* i1, Operator* op, Item* i2) {
    this->items.push_back(l_value);
    this->items.push_back(i1);
    this->items.push_back(op);
    this->items.push_back(i2);
    this->uses.insert(i1);
    this->uses.insert(i2);
    this->defs.insert(l_value);
  }

  void BinaryOperator::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  BinaryOperator* BinaryOperator::operator=(const BinaryOperator* inst) {
    return Instruction::copy<BinaryOperator>(inst);
  }

  BinaryOperator* BinaryOperator::replace(Item* old_item, Item* new_item) const {
    return Instruction::replace<BinaryOperator>(this, old_item, new_item);
  }

  std::string BinaryOperator::to_string() {
    std::stringstream ss;
    ss << items[0]->to_string() << " <- " << items[1]->to_string() << " "
    << items[2]->to_string() << " " << items[3]->to_string() << std::endl;
    return ss.str();
  }

  CmpInst::CmpInst(Variable* l_value, Item* i1, Operator* op, Item* i2) {
    this->items.push_back(l_value);
    this->items.push_back(i1);
    this->items.push_back(op);
    this->items.push_back(i2);
    this->uses.insert(i1);
    this->uses.insert(i2);
    this->defs.insert(l_value);
  }

  void CmpInst::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  CmpInst* CmpInst::operator=(const CmpInst* inst) {
    return Instruction::copy<CmpInst>(inst);
  }

  CmpInst* CmpInst::replace(Item* old_item, Item* new_item) const {
     return Instruction::replace<CmpInst>(this, old_item, new_item);
  }
  std::string CmpInst::to_string() {
    std::stringstream ss;
    ss << items[0]->to_string() << " <- " << items[1]->to_string() << " "
    << items[2]->to_string() << " " << items[3]->to_string() << std::endl;
    return ss.str();
  }

  LabelInst::LabelInst(Label* label) {
    this->items.push_back(label);
    this->uses.insert(label);
  }

  void LabelInst::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  LabelInst* LabelInst::operator=(const LabelInst* inst) {
    return Instruction::copy<LabelInst>(inst);
  }

  LabelInst* LabelInst::replace(Item* old_item, Item* new_item) const {
    return Instruction::replace<LabelInst>(this, old_item, new_item);
  }

  std::string LabelInst::to_string() {
    std::stringstream ss;
    ss << items[0]->to_string() << std::endl;
    return ss.str();
  }

  CallInst::CallInst(Variable* ret_value, Item* callee, std::vector<Item*> arguments) {
    // DEBUG_COUT << (ret_value == nullptr) << std::endl;
    if(ret_value != nullptr) {
      this->defs.insert(ret_value);
    }
    this->ret_value = ret_value;
    this->items.push_back(ret_value);
    if(auto variable = dynamic_cast<Variable*>(callee)) {
      this->uses.insert(callee);
    }
    this->callee = callee;
    this->items.push_back(callee);
    for(auto arg : arguments) {
      this->items.push_back(arg);
      this->uses.insert(arg);
      this->arguments.push_back(arg);
    }
  }

  void CallInst::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  CallInst* CallInst::operator=(const CallInst* inst) {
    auto inst_copy = Instruction::copy(inst);
    inst_copy->callee = inst->callee;
    inst_copy->arguments = inst->arguments;
    inst_copy->ret_value = inst->ret_value;
    return inst_copy;
  }

  CallInst* CallInst::replace(Item* old_item, Item* new_item) const {
    auto inst_copy = Instruction::replace(this, old_item, new_item);
    if(old_item == this->callee) {
      inst_copy->callee = new_item;
    }
    if(old_item == this->ret_value) {
      inst_copy->ret_value = (Variable*)new_item;
    }
    if(std::count(this->arguments.begin(), this->arguments.end(), old_item)) {
      std::replace(inst_copy->arguments.begin(), 
      inst_copy->arguments.end(), old_item, new_item);
    }
    return inst_copy;
  }

  std::string CallInst::to_string()  {
    std::stringstream ss;
    // DEBUG_COUT << (this->ret_value == nullptr) << std::endl;
    ss << (this->ret_value ? (ret_value->to_string() + " <- ") : "");
    ss << "call " << callee->to_string() << "(";
    for(int i = 0; i < arguments.size(); i++) {
      if(i != 0) {
        ss << ", ";
      }
      ss << arguments[i]->to_string();
    }
    // for(auto arg : arguments) {
    //   ss << arg->to_string() << " ";
    // }
    ss << ")" << std::endl;
    return ss.str();
  }

  std::vector<Item*> CallInst::get_arguments() {
    return this->arguments;
  }

  Variable* CallInst::get_ret_value() {
    return this->ret_value;
  }

  Item* CallInst::get_callee() {
    return this->callee;
  }

  BranchInst::BranchInst(Item* flag, Label* label) {
    if(flag != nullptr) {
      this->uses.insert(flag);
    }
    this->items.push_back(flag);
    this->items.push_back(label);
  }

  void BranchInst::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  BranchInst* BranchInst::operator=(const BranchInst* inst) {
    return Instruction::copy<BranchInst>(inst);
  }

  BranchInst* BranchInst::replace(Item* old_item, Item* new_item) const {
    return Instruction::replace(this, old_item, new_item);
  }

  std::string BranchInst::to_string() {
    std::stringstream ss;
    ss << "br " << (items[0] ? (items[0]->to_string() + " ") : "") 
    << items[1]->to_string() << std::endl;
    return ss.str();
  }

  ReturnInst::ReturnInst(Variable* variable) {
    if(variable) {
      uses.insert(variable);
    }
    items.push_back(variable);
  }

  void ReturnInst::accept(Visitor* visitor) {
    visitor->visit(this);
  }
  
  ReturnInst* ReturnInst::operator=(const ReturnInst* inst) {
    return Instruction::copy<ReturnInst>(inst);
  }
  
  ReturnInst* ReturnInst::replace(Item* old_item, Item* new_item) const {
    return Instruction::replace(this, old_item, new_item);
  } 
  
  std::string ReturnInst::to_string() {
    std::stringstream ss;
    ss << "return" << (items[0] ? (" " + items[0]->to_string()) : "") << std::endl;
    return ss.str();
  }

  Function* Function::operator=(const Function* f) {
    auto func_copy = new Function;
    func_copy->arguments = f->arguments;
    func_copy->locals = f->locals;
    func_copy->instructions = f->instructions;
    func_copy->name = f->name;
    return func_copy;
  }
  
  std::string Function::to_string() {
    std::stringstream ss;
    ss << "define " << this->get_name() << " (";
    for(int i = 0; i < arguments.size(); i++) {
      if(i != 0) {
        ss << ", ";
      }
      ss << arguments[i]->to_string();
    }
    // for(auto arg : arguments) {
    //   ss << arg->to_string() << " ";
    // }
    ss << ") {\n";
    for(auto inst : instructions) {
      ss << "  " << inst->to_string();
    }
    ss << "}" << std::endl;
    return ss.str();
  }
  void Function::set_name(const std::string& name) {
    this->name = name;
  }
  void Function::add_argument(Variable* arg) {
    this->arguments.push_back(arg);
  }

  void Function::add_local() {
    for(auto inst : instructions) {
      for(auto items : inst->get_items()) {
        if(auto variable = dynamic_cast<Variable*>(items)) {
          locals.insert(variable);
        }
      }
    }
  }
  
  void Function::add_instruction(Instruction* instruction) {
    instructions.push_back(instruction);
  }

  std::string Function::get_name() {
    return this->name;
  }
      
  std::vector<Variable*> Function::get_arguments() {
    return this->arguments;
  }
      
  std::set<Variable*> Function::get_locals() {
    return this->locals;
  }

  void Function::accept(Visitor *visitor) {
    visitor->visit(this);
  }
  
  std::vector<Instruction*> Function::get_instructions() {
    return this->instructions;
  }

  std::vector<Function*> Program::get_functions() {
    return this->functions;
  }
  
  void Program::set_function(Function* f) {
    this->functions.push_back(f);
  }

}