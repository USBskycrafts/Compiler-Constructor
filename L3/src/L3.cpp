#include "L3.hpp"
#include "visitor.hpp"

namespace L3 {

  void Item::accept(Visitor* visitor) {
      visitor->visit(this);
  }

  void Instruction::accept(Visitor* visitor) {
      visitor->visit(this);
  }

  std::vector<Variable> AssignInst::GetDefs() {
    return {*dst_};
  }

  std::vector<Variable> AssignInst::GetUses() {
    if(auto src = std::dynamic_pointer_cast<Variable>(src_)) {
      return {*src};
    } else {
      return {};
    }
  }

  std::vector<Variable> BinaryOperator::GetDefs() {
    return {*dst_};
  }

  std::vector<Variable> BinaryOperator::GetUses() {
    std::vector<Variable> uses;
    if(auto operand1 = std::dynamic_pointer_cast<Variable>(operand1_)) {
      uses.push_back(*operand1);
    }
    if(auto operand2 = std::dynamic_pointer_cast<Variable>(operand2_)) {
      uses.push_back(*operand2);
    }
    return uses;
  }

  std::vector<Variable> LoadInst::GetDefs() {
    return {*dst_};
  }

  std::vector<Variable> LoadInst::GetUses() {
    return {*src_};
  }

  std::vector<Variable> StoreInst::GetDefs() {
    return {};
  }
  
  std::vector<Variable> StoreInst::GetUses() {
    std::vector<Variable> uses;
    if(auto src = std::dynamic_pointer_cast<Variable>(src_)) {
      uses.push_back(*src);
    }
    uses.push_back(*dst_);
    return uses;
  }

  std::vector<Variable> ReturnInst::GetDefs() {
    return {};
  }

  std::vector<Variable> ReturnInst::GetUses() {
    if(auto ret = std::dynamic_pointer_cast<Variable>(ret_)) {
      return {*ret};
    } else {
      return {};
    }
  }

  std::vector<Variable> BranchInst::GetDefs() {
    return {};
  }

  std::vector<Variable> BranchInst::GetUses() {
    if(auto mark = std::dynamic_pointer_cast<Variable>(mark_)) {
      return {*mark};
    } else {
      return {};
    }
  }

  std::vector<Variable> CallInst::GetDefs() {
    return {*ret_};
  }

  std::vector<Variable> CallInst::GetUses() {
    std::vector<Variable> uses;
    for(auto arg : args_) {
      if(auto var = std::dynamic_pointer_cast<Variable>(arg)) {
        uses.push_back(*var);
      }
    }
    return uses;
  }
}