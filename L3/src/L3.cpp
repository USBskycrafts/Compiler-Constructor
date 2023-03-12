#include "L3.hpp"

#include <vector>
#include <sstream>
#include "utils.hpp"
#include "visitor.hpp"

namespace L3 {
  void Item::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  Label::Label(std::string name) {
    this->name = name;
    this->type = ItemType::kLabel;
  }

  FunctionName::FunctionName(std::string name) {
    this->name = name;
    this->type = ItemType::kFunctionName;
  }

  Operator::Operator(std::string name) {
    this->name = name;
    this->type = ItemType::kOperator;
  }

  Variable::Variable(std::string name) {
    this->name = name;
    this->type = ItemType::kVariable;
  }

  Number::Number(std::string name) {
    this->name = name;
    this->type = ItemType::kNumber;
  }

   void Instruction::accept(Visitor* visitor) {
    visitor->visit(this);
  }

  std::vector<Variable*> AssignInst::GetDefs() {
    return {lhs};
  }

  std::vector<Variable*> AssignInst::GetUses() {
    if(rhs->type == ItemType::kVariable) {
      return {(Variable*)rhs};
    } else {
      return {};
    }
  }

  std::string AssignInst::ToString() {
    std::stringstream code;
    code << INDENT(1) << lhs->name << " <- " << rhs->name << std::endl;
    return code.str();
  }

  BinaryOperator::BinaryOperator(Variable* lhs, Item* t1, Operator* op, Item* t2) {
    this->lhs = lhs;
    rhs = std::make_tuple(t1, op, t2);
    type = "BinaryOperator";
  }

  std::vector<Variable*> BinaryOperator::GetDefs() {
    return {lhs};
  }

  std::vector<Variable*> BinaryOperator::GetUses() {
    auto t1 = std::get<0>(rhs);
    auto t2 = std::get<2>(rhs);
    std::vector<Variable*> uses;
    if(t1->type == ItemType::kVariable) {
      uses.emplace_back((Variable*)t1);
    }
    if(t2->type == ItemType::kVariable) {
      uses.emplace_back((Variable*)t2);
    }
    return uses;
  }

  std::string BinaryOperator::ToString() {
    std::stringstream code;
    code << INDENT(1) << lhs->name << " <- "
    << std::get<0>(rhs)->name << " " << std::get<1>(rhs)->name << " "
    << std::get<2>(rhs)->name << std::endl;
    return code.str();
  }

  std::vector<Variable*> ReturnInst::GetDefs() {
    return {};
  }

  std::vector<Variable*> ReturnInst::GetUses() {
    if(ret_val && ret_val->type == ItemType::kVariable) {
      return {(Variable*)ret_val};
    } else {
      return {};
    }
  }

  std::string ReturnInst::ToString() {
    std::stringstream code;
    code << INDENT(1) << "return";
    if(ret_val) {
      code << " " << ret_val->name;
    }
    code << std::endl;
    return code.str();
  }

  std::vector<Variable*> LabelInst::GetDefs() {
    return {};
  }

  std::vector<Variable*> LabelInst::GetUses() {
    return {};
  }

  std::string LabelInst::ToString() {
    std::stringstream code;
    code << INDENT(1) << label->name << std::endl;
    return code.str();
  }

  std::vector<Variable*> BranchInst::GetDefs() {
    return {};
  }

  std::vector<Variable*> BranchInst::GetUses() {
    if(t && t->type == ItemType::kVariable) {
      return {(Variable*)t};
    } else {
      return {};
    }
  }

  std::string BranchInst::ToString() {
    std::stringstream code;
    code << INDENT(1) << "br ";
    if(t != nullptr) {
      code << t->name << " ";
    }
    code << label->name << std::endl;
    return code.str();
  }

  std::vector<Variable*> LoadInst::GetDefs() {
    return {lhs};
  }

  std::vector<Variable*> LoadInst::GetUses() {
    return {rhs};
  }

  std::string LoadInst::ToString() {
    std::stringstream code;
    code << INDENT(1) << lhs->name << " <- load " << rhs->name << std::endl;
    return code.str();
  }

  std::vector<Variable*> StoreInst::GetDefs() {
    return {lhs};
  }

  std::vector<Variable*> StoreInst::GetUses() {
    if(rhs->type == ItemType::kVariable) {
      return {(Variable*)rhs, lhs};
    } else {
      return {lhs};
    }
  }

  std::string StoreInst::ToString() {
    std::stringstream code;
    code << INDENT(1) << "store " << lhs->name << " <- " << rhs->name << std::endl;
    return code.str();
  }

  std::vector<Variable*> CallInst::GetDefs() {
    if(lhs != nullptr) {
      return {(Variable*)lhs};
    } else {
      return {};
    }
  }

  std::vector<Variable*> CallInst::GetUses() {
    std::vector<Variable*> uses;
    for(auto arg : args) {
      if(arg->type == ItemType::kVariable) {
        uses.emplace_back((Variable*)arg);
      }
    }
    if(callee->type == ItemType::kVariable) {
      uses.push_back((Variable*)callee);
    }
    return uses;
  }

  std::string CallInst::ToString() {
    std::stringstream code;
    code << INDENT(1);
    if(lhs != nullptr) {
      code << lhs->name << " <- ";
    }
    code << "call " << callee->name << "(";
    for(auto i = args.begin(); i != args.end(); ++i) {
      code << (i == args.begin() ? "" : ", ") << (*i)->name;
    }
    code << ")" << std::endl;
    return code.str();
  }
}