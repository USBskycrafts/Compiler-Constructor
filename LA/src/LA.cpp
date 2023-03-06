
#include <cstddef>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <string>

#include "LA.hpp"
#include "utils.hpp"


namespace LA {

  Label::Label(std::string code) : code_(code) {}

  std::string Label::ToString() {
    return code_;
  }

  std::string Label::GetType() {
    return "code";
  }

  std::shared_ptr<Function> Var::GetFunction() {
    return parent_;
  }

  void Var::SetFunction(std::shared_ptr<Function> parent) {
    parent_ = parent;
  }

  Code::Code(std::string code) : code_(code) {}

  std::shared_ptr<Var> Code::Copy(std::string code) {
    auto copy = std::make_shared<Code>(code);
    copy->parent_ = parent_;
    auto f = this->parent_;
    f->variables_mapper[code] = copy;
    return std::dynamic_pointer_cast<Var>(copy);
  }

  std::string Code::ToString() {
    return "%" + code_;
  }

  std::string Code::GetType() {
    return "code";
  }

  Int64::Int64(std::string code) : code_(code) {}

  std::shared_ptr<Var> Int64::Copy(std::string code) {
    auto copy = std::make_shared<Int64>(code);
    copy->parent_ = parent_;
    auto f = this->parent_;
    //DEBUG_COUT << code << " " << copy->ToString() << std::endl;
    f->variables_mapper[code] = copy;
    return std::dynamic_pointer_cast<Var>(copy);
  }

  std::string Int64::ToString() {
    return "%" + code_;
  }

  std::string Int64::GetType() {
    return "int64";
  }

  Tuple::Tuple(std::string code) : code_(code) {}

  std::shared_ptr<Var> Tuple::Copy(std::string code) {
    auto copy = std::make_shared<Tuple>(code);
    copy->length_ = length_;
    copy->parent_ = parent_;
    auto f = this->parent_;
    f->variables_mapper[code] = copy;
    return std::dynamic_pointer_cast<Var>(copy);
  }

  std::string Tuple::ToString() {
    return "%" + code_;
  }

  std::string Tuple::GetType() {
    return "tuple";
  }

  void Tuple::SetLength(std::shared_ptr<Item> length) {
    length_ = length;
  }

  Array::Array(std::string code, unsigned dims) : code_(code), dimensions_(dims, 0) {}

  std::shared_ptr<Var> Array::Copy(std::string code) {
    auto copy = std::make_shared<Array>(code, 0);
    copy->dimensions_ = dimensions_;
    copy->parent_ = parent_;
    auto f = this->parent_;
    f->variables_mapper[code] = copy;
    return std::dynamic_pointer_cast<Var>(copy);
  }

  std::string Array::ToString() {
    return "%" + code_;
  }
  
  std::string Array::GetType() {
    std::stringstream code;
    code << "int64";
    for(size_t i = 0; i < dimensions_.size(); i++) {
      code << "[]";
    }
    return code.str();
  }

  void Array::SetDim(unsigned int index, std::shared_ptr<Item> dim) {
    dimensions_[index] = dim;
  }


  Operator::Operator(std::string code) :code_(code) {}

  std::string Operator::ToString() {
    return code_;
  }
  
  std::string Operator::GetType() {
    return "operator";
  }

  Number::Number(std::string code) : code_(code) {}

  std::string Number::ToString() {
    return code_;
  }

  std::string Number::GetType() {
    return "const int";
  }

  FunctionName::FunctionName(std::string code) : code_(code) {}

  std::string FunctionName::ToString() {
    if(code_ != "print" && code_ != "allocate" && code_ != "tensor-error" && code_ != "input") {
      return "@" + code_;
    } else {
      return code_;
    }
    
  }

  std::string FunctionName::GetType() {
    return "function name";
  }

  std::shared_ptr<Function> Instruction::GetParent() {
    return parent_;
  }

  void Instruction::SetParent(std::shared_ptr<Function> parent) {
    parent_ = parent;
  }

  std::string Instruction::GetLine() {
    return line;
  }

  void Instruction::SetLine(std::string line) {
    this->line = line;
  }

  DeclareInst::DeclareInst(std::shared_ptr<Var> variable) : variable_(variable) {}

  std::string DeclareInst::ToString() {
    return variable_->GetType() + " " + variable_->ToString();
  }
  
  std::string DeclareInst::GenerateCode() {
    return "";
  }

  AssignInst::AssignInst(std::shared_ptr<Var> lhs, 
    std::shared_ptr<Item> rhs) : lhs_(lhs), rhs_(rhs) {}

  std::string AssignInst::ToString() {
    std::stringstream code;
    code << lhs_->ToString() << " <- " << rhs_->ToString();
    return code.str();
  }
  
  std::string AssignInst::GenerateCode() {
    return ToString();
  }

  BinaryOperator::BinaryOperator(std::shared_ptr<Var> lhs,
    std::shared_ptr<Item> operand1,
    std::shared_ptr<Operator> op,
    std::shared_ptr<Item> operand2) : lhs_(lhs), operand1_(operand1), op_(op), operand2_(operand2) {}

  std::string BinaryOperator::GenerateCode() {
    return ToString();
  }
  
  std::string BinaryOperator::ToString() {
    std::stringstream code;
    code << lhs_->ToString() << " <- "  
    << operand1_->ToString() << " " << op_->ToString() << " " << operand2_->ToString();
    return code.str();
  }

  LoadArray::LoadArray(std::shared_ptr<Var> lhs,
    std::shared_ptr<Array> rhs,
    std::vector<std::shared_ptr<Item>> index) : lhs_(lhs), rhs_(rhs), index_(index) {}

  std::string LoadArray::GenerateCode()  {
    return ToString();
  }

  std::string LoadArray::ToString() {
    std::stringstream code;
    code << lhs_->ToString() << " <- " << rhs_->ToString();
    for(int i = 0; i < index_.size(); i++) {
      code << "[" + index_[i]->ToString() << "]";
    }
    return code.str();
  }

  StoreArray::StoreArray(std::shared_ptr<Array> lhs,
    std::vector<std::shared_ptr<Item>> index,
    std::shared_ptr<Item> rhs) : lhs_(lhs), index_(index), rhs_(rhs) {}

  std::string StoreArray::ToString() {
    std::stringstream code;
    code << lhs_->ToString();
    for(auto index : index_) {
      code << "[" << index->ToString() << "]";
    }
    code << " <- " << rhs_->ToString();
    return code.str();
  }
  
  std::string StoreArray::GenerateCode() {
    return ToString();
  }

  LoadTuple::LoadTuple(std::shared_ptr<Var> lhs,
    std::shared_ptr<Tuple> rhs,
    std::shared_ptr<Item> index) : lhs_(lhs), rhs_(rhs), index_(index) {}

  std::string LoadTuple::GenerateCode() {
    return ToString();
  }

  std::string LoadTuple::ToString() {
    std::stringstream code;
    code << lhs_->ToString() << " <- " << rhs_->ToString() << "[" << index_->ToString() << "]";
    return code.str();
  }

  StoreTuple::StoreTuple(std::shared_ptr<Tuple> lhs,
    std::shared_ptr<Item> index,
    std::shared_ptr<Item> rhs) : lhs_(lhs), index_(index), rhs_(rhs) {}

  std::string StoreTuple::GenerateCode() {
    return ToString();
  }

  std::string StoreTuple::ToString(){
    std::stringstream code;
    code << lhs_->ToString() << "[" << index_->ToString() << "] <- " << rhs_->ToString();
    return code.str();
  }


  ArrayLength::ArrayLength(std::shared_ptr<Var> lhs,
    std::shared_ptr<Array> rhs,
    std::shared_ptr<Item> dim) : lhs_(lhs), rhs_(rhs), dim_(dim) {}
  
  std::string ArrayLength::GenerateCode() {
    return ToString();
  }
  
  std::string ArrayLength::ToString() {
    std::stringstream code;
    code << lhs_->ToString() << " <- length " << rhs_->ToString() << " " << dim_->ToString(); 
    return code.str();
  }

  TupleLength::TupleLength(std::shared_ptr<Var> lhs,
    std::shared_ptr<Tuple> rhs) : lhs_(lhs), rhs_(rhs) {}

  std::string TupleLength::GenerateCode() {
    return ToString();
  }
  
  std::string TupleLength::ToString() {
    std::stringstream code;
    code << lhs_->ToString() << " <- length " << rhs_->ToString();
    return code.str();
  }

  CallInst::CallInst(std::shared_ptr<Var> ret_value,
    std::shared_ptr<Item> f_name,
    std::vector<std::shared_ptr<Item>> operands) : ret_value_(ret_value), f_name_(f_name), operands_(operands){}


  std::string CallInst::GenerateCode() {
    return ToString();
  }
  
  
  std::string CallInst::ToString() {
    std::stringstream code;
    if(ret_value_ != nullptr) {
      code << ret_value_->ToString() << " <- ";
    }
    code << "call " << this->f_name_->ToString() << " (";
    for(size_t i = 0; i < operands_.size(); i++) {
      code << (i == 0 ? "" : ", ") << operands_[i]->ToString();
    }
    code << ")";
    return code.str();
  }


  ArrayAllocate::ArrayAllocate(std::shared_ptr<Array> lhs,
    std::vector<std::shared_ptr<Item>> args) : lhs_(lhs), args_(args) {}

  std::string ArrayAllocate::ToString() {
    std::stringstream code;
    code << lhs_->ToString() << " <- new Array(";
    for(size_t i = 0; i < args_.size(); i++) {
      code << (i == 0 ? "" : ", ") << args_[i]->ToString();
    }
    code << ")";
    return code.str();
  }

  std::string ArrayAllocate::GenerateCode() {
    return ToString();
  }


  TupleAllocate::TupleAllocate(std::shared_ptr<Tuple> lhs, 
    std::shared_ptr<Item> dim) : lhs_(lhs), dim_(dim) {}

  std::string TupleAllocate::ToString() {
    std::stringstream code;
    code << lhs_->ToString() << " <- new Tuple(" << dim_->ToString() << ")";
    return code.str();
  }

  std::string TupleAllocate::GenerateCode() {
    return ToString();
  }

  ReturnInst::ReturnInst(std::shared_ptr<Item>ret_value) : ret_value_(ret_value) {}

  std::string ReturnInst::GenerateCode() {
    return this->ToString();
  }

  std::string ReturnInst::ToString() {
    std::stringstream code;
    code << "return";
    if(this->ret_value_ != nullptr) {
      code << " " << ret_value_->ToString();
    }
    return code.str();
  }


  BranchInst::BranchInst(
    std::shared_ptr<Item> mark, 
    std::vector<std::shared_ptr<Label>> labels) : mark_(mark), labels_(labels) {}

  std::string BranchInst::ToString() {
    std::stringstream code;
    if(this->mark_ == nullptr) {
      code << "br " << labels_[0]->ToString();
    } else {
      code << "br " << mark_->ToString() << " " 
      << labels_[0]->ToString() << " " << labels_[1]->ToString();
    }
    return code.str();
  }

  std::string BranchInst::GenerateCode() {
    return ToString();
  }

  std::vector<std::shared_ptr<Label>> BranchInst::GetSuccessors() {
    return this->labels_;
  }



  LabelInst::LabelInst(std::shared_ptr<Label> label) :label_(label) {}
  
  std::string LabelInst::GenerateCode() {
    return ToString();
  }
  
  std::string LabelInst::ToString() {
    std::stringstream code;
    code << label_->ToString() << "\n\t";
    return code.str();
  }


  std::string Function::ToString() {
    std::stringstream code;
    code << ret
    << " " << this->function_name->ToString() << " (";
    for(int i = 0; i < args.size(); i++) {
      code << (i == 0 ? "" : ", ") << args[i]->ToString();
    }
    code << "){\n";
    for(auto [var_name, var] : this->variables_mapper) {
      code << var->GetType() << " " << var->ToString() << "\n\t";
    }
    for(auto inst : this->instructions) {
      code << inst->ToString() << std::endl << "\n\t";
    }
    code << "}\n";
    return code.str();
  }

  std::string Function::GenerateCode() {
    std::stringstream code;
    static int label_counter = 0;
    code << "define " << ret
    << " " << this->function_name->ToString() << " (";
    for(int i = 0; i < args.size(); i++) {
      code << (i == 0 ? "" : ", ") << args[i]->GetType() << " " << args[i]->ToString();
    }
    code << ") {\n";
    //generating entry basic block where declare the variable
    std::string label_name = ":entry_point";
    for(auto [name, _] : this->labels_mapper) {
      if(label_name.size() < name.size()) {
        label_name = name;
      }
    }
    code << label_name + std::to_string(label_counter++) << "\n\t";
    for(auto [var_name, var] : this->variables_mapper) {
      try {
        for(int i = 0; i < args.size(); i++) {
          if(args[i]->ToString() == var->ToString()) {
            throw "";
          }
        }
      } catch(...) {
        continue;
      }
      code << var->GetType() << " " << var->ToString() << "\n\t";
    }

    if(!this->instructions.empty()) {
      auto label_inst = this->instructions[0];
      if(typeid(*label_inst) != typeid(LabelInst)) {
        auto label = std::make_shared<Label>(label_name + std::to_string(label_counter));
        label_inst = std::make_shared<LabelInst>(label);
      }
      code << "br " << label_inst->ToString() << "\n";
    }  
    //generating normal basic block
    bool is_started = true;
    for(auto inst : this->instructions) {
      if(is_started) {
        is_started = false;
        if(typeid(*inst) != typeid(LabelInst)) {
          auto label = std::make_shared<Label>(label_name + std::to_string(label_counter++));
          auto label_inst = std::make_shared<LabelInst>(label);
          code << label_inst->GenerateCode() << "\n\t";
        }
      } else if(typeid(*inst) == typeid(LabelInst)) {
        code << "br " << inst->ToString() << "\n";
      }
      code << inst->GenerateCode() << "\n";
      if(typeid(*inst) != typeid(BranchInst) && typeid(*inst) != typeid(ReturnInst)) {
        code << "\t";
      }
      if(typeid(*inst) == typeid(BranchInst) || typeid(*inst) == typeid(ReturnInst)) {
        is_started = true;
      }
    }
    if(!is_started || instructions.empty()) {
      if(ret == "void") {
        code << "return\n";
      } else {
        code << "return 0\n";
      }
    }
    code << "}\n";
    return code.str();
  }

  std::shared_ptr<Program> Function::GetParent() {
    return parent;
  }

  std::string Program::ToString() {
    std::stringstream code;
    for(auto f : this->functions) {
      code << f->ToString();
    }
    return code.str();
  }
  
  
  std::string Program::GenerateCode() {
    std::stringstream code;
    for(auto f : this->functions) {
      code << f->GenerateCode() << "\n";
    }
    return code.str();
  }
}