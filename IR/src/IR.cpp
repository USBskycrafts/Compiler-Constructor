
#include <cstddef>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <string>

#include "IR.hpp"
#include "utils.hpp"


namespace IR {

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

  std::string Code::ToString() {
    return code_;
  }

  std::string Code::GetType() {
    return "code";
  }

  Int64::Int64(std::string code) : code_(code) {}

  std::string Int64::ToString() {
    return code_;
  }

  std::string Int64::GetType() {
    return "int64";
  }

  Tuple::Tuple(std::string code) : code_(code) {}

  std::string Tuple::ToString() {
    return code_;
  }

  std::string Tuple::GetType() {
    return "tuple";
  }

  void Tuple::SetLength(std::shared_ptr<Item> length) {
    length_ = length;
  }

  Array::Array(std::string code, unsigned dims) : code_(code), dimensions_(dims, 0) {}

  std::string Array::ToString() {
    return code_;
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
    return code_;
  }

  std::string FunctionName::GetType() {
    return "function name";
  }

  std::shared_ptr<BasicBlock> Instruction::GetParent() {
    return parent_;
  }

  void Instruction::SetParent(std::shared_ptr<BasicBlock> parent) {
    parent_ = parent;
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
    std::stringstream code;
    auto dim =  index_.size();
    std::vector<std::string> dims;
    static unsigned var_counter = 0;
    std::string temp_name;
    auto bb = this->GetParent();
    auto f = bb->GetParent();
    for(auto [name, variable] : f->variables_mapper) {
      if(temp_name.size() < name.size()) {
        temp_name = name;
      }
    }
    /*
    * To fetch M, N ...
    */
    for(size_t i = 1; i < dim; i++) {
      auto size = temp_name + std::to_string(var_counter++);
      dims.push_back(size);
      code << size << " <- " << rhs_->ToString() << " + " << std::to_string((i + 1) * 8) << "\n\t";
      code << size << " <- load " << size << "\n\t";
      code << size << " <- " << size << " >> 1\n\t";
    }

    /*
    * calculate index/offset
    * ((k * M + i) * N + j)...
    */
    auto offset = temp_name + std::to_string(var_counter++);
    code << offset << " <- " << index_[0]->ToString() << "\n\t";
    for(size_t i = 0; i < index_.size() - 1; i++) {
      auto temp = temp_name + std::to_string(var_counter++);
      code << temp << " <- " << dims[i] << " * " << offset << "\n\t";
      code << temp << " <- " << temp << " + " << index_[i + 1]->ToString() << "\n\t";
      code << offset << " <- " << temp << "\n\t";
    }
    if(index_.size() == 1) {
      code << offset << " <- " << index_[0]->ToString() << "\n\t";
    }
    code << offset << " <- " << offset << " * 8\n\t";
    code << offset << " <- " << offset << " + " << std::to_string(8 + dim * 8) << "\n\t";


    /*
    * load var
    */
    auto addr = temp_name + std::to_string(var_counter++);
    code << addr << " <- " << rhs_->ToString() << " + " << offset << "\n\t";
    code << lhs_->ToString() << " <- load " << addr; 
    return code.str();
  }
  
  std::string LoadArray::ToString() {
    std::stringstream code;
    code << lhs_->ToString() << " <- ";
    for(auto index : index_) {
      code << "[" << index->ToString() << "]";
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
    std::stringstream code;
    auto dim =  index_.size();
    std::vector<std::string> dims;
    static unsigned var_counter = 0;
    std::string temp_name;
    auto bb = this->GetParent();
    auto f = bb->GetParent();
    for(auto [name, variable] : f->variables_mapper) {
      if(temp_name.size() < name.size()) {
        temp_name = name;
      }
    }
    /*
    * To fetch M, N ...
    */
    for(size_t i = 1; i < dim; i++) {
      auto size = temp_name + std::to_string(var_counter++);
      dims.push_back(size);
      code << size << " <- " << lhs_->ToString() << " + " << std::to_string((i + 1) * 8) << "\n\t";
      code << size << " <- load " << size << "\n\t";
      code << size << " <- " << size << " >> 1\n\t";
    }

    /*
    * calculate index/offset
    * ((k * M + i) * N + j)...
    */
    auto offset = temp_name + std::to_string(var_counter++);
    code << offset << " <- " << index_[0]->ToString() << "\n\t";
    for(size_t i = 0; i < index_.size() - 1; i++) {
      auto temp = temp_name + std::to_string(var_counter++);
      code << temp << " <- " << dims[i] << " * " << offset << "\n\t";
      code << temp << " <- " << temp << " + " << index_[i + 1]->ToString() << "\n\t";
      code << offset << " <- " << temp << "\n\t";
    }
    
    code << offset << " <- " << offset << " * 8\n\t";
    code << offset << " <- " << offset << " + " << std::to_string(8 + dim * 8) << "\n\t";

    /*
    * store var
    */
    auto addr = temp_name + std::to_string(var_counter++);
    code << addr << " <- " << lhs_->ToString() << " + " << offset << "\n\t";
    code << "store " << addr << " <- " << rhs_->ToString();
    return code.str();
  }

  LoadTuple::LoadTuple(std::shared_ptr<Var> lhs,
    std::shared_ptr<Tuple> rhs,
    std::shared_ptr<Item> index) : lhs_(lhs), rhs_(rhs), index_(index) {}

  std::string LoadTuple::GenerateCode() {
    std::stringstream code;
    static unsigned var_counter = 0;
    std::string temp_name;
    auto bb = this->GetParent();
    auto f = bb->GetParent();
    for(auto [name, variable] : f->variables_mapper) {
      if(temp_name.size() < name.size()) {
        temp_name = name;
      }
    }
    auto temp = temp_name + std::to_string(var_counter++);
    code << temp << " <- " << index_->ToString() << " + 1\n\t";
    code << temp << " <- " << temp << " * " << "8\n\t";
    code << temp << " <- " << temp << " + " << rhs_->ToString() << "\n\t";
    code << lhs_->ToString() << " <- load " << temp;
    return code.str();
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
    std::stringstream code;
    static unsigned var_counter = 0;
    std::string temp_name;
    auto bb = this->GetParent();
    auto f = bb->GetParent();
    for(auto [name, variable] : f->variables_mapper) {
      if(temp_name.size() < name.size()) {
        temp_name = name;
      }
    }
    auto temp = temp_name + std::to_string(var_counter++);
    code << temp << " <- " << index_->ToString() << " + 1\n\t";
    code << temp << " <- " << temp << " * " << "8\n\t";
    code << temp << " <- " << temp << " + " << lhs_->ToString() << "\n\t";
    code << "store " << temp << " <- " << rhs_->ToString();
    return code.str();
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
    std::stringstream code;
    std::string temp_name;
    static unsigned var_counter = 0;
    auto bb = this->GetParent();
    auto f = bb->GetParent();
    for(auto [name, variable] : f->variables_mapper) {
      if(temp_name.size() < name.size()) {
        temp_name = name;
      }
    }
    auto offset = temp_name + std::to_string(var_counter++);
    code << offset << " <- 8 * " << dim_->ToString() << "\n\t";
    code << offset << " <- " << offset << " + 8\n\t";
    auto address = temp_name + std::to_string(var_counter++);
    code << address << " <- " << rhs_->ToString() << " + " << offset << "\n\t";
    code << lhs_->ToString() << " <- load " << address;
    return code.str();
  }
  
  std::string ArrayLength::ToString() {
    std::stringstream code;
    code << lhs_->ToString() << " <- length " << rhs_->ToString() << " " << dim_->ToString(); 
    return code.str();
  }

  TupleLength::TupleLength(std::shared_ptr<Var> lhs,
    std::shared_ptr<Tuple> rhs) : lhs_(lhs), rhs_(rhs) {}

  std::string TupleLength::GenerateCode() {
    std::stringstream code;
    code << lhs_->ToString() << " <- load " << rhs_->ToString() << "\n\t";
    code << lhs_->ToString() << " <- " << lhs_->ToString() << " << 1\n\t";
    code << lhs_->ToString() << " <- " << lhs_->ToString() << " + 1"; 
    return code.str();
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
    std::stringstream code;
    std::string temp_name;
    static unsigned var_counter = 0;
    auto bb = this->GetParent();
    auto f = bb->GetParent();
    for(auto [name, variable] : f->variables_mapper) {
      if(temp_name.size() < name.size()) {
        temp_name = name;
      }
    }
    auto allocate_size = temp_name + std::to_string(var_counter++);
    code << allocate_size + " <- 1\n\t";
    for(auto arg : args_) {
      auto temp = temp_name + std::to_string(var_counter++);
      code << temp << " <- " << arg->ToString() << " >> 1\n\t";
      code << allocate_size << " <- " << allocate_size << " * " << temp << "\n\t";
    }
    /*
    * Encoding
    */
    code << allocate_size << " <- " << allocate_size << " + " << args_.size() << "\n\t";
    code << allocate_size << " <- " << allocate_size << " << 1\n\t";
    code << allocate_size << " <- " << allocate_size << " + 1\n\t";
    /*
    * Call allocate
    */
    code << lhs_->ToString() << " <- call allocate(" << allocate_size << ", 1)\n\t";
    /*
    * store length in header
    */
    for(size_t i = 0; i < args_.size(); i++) {
      auto temp = temp_name + std::to_string(var_counter++);
      code << temp << " <- " << lhs_->ToString() << " + " << std::to_string((i + 1) * 8) << "\n\t";
      code << "store " << temp << " <- " << args_[i]->ToString() << "\n\t";
    }
    code << std::endl;
    return code.str();
  }


  TupleAllocate::TupleAllocate(std::shared_ptr<Tuple> lhs, 
    std::shared_ptr<Item> dim) : lhs_(lhs), dim_(dim) {}

  std::string TupleAllocate::ToString() {
    std::stringstream code;
    code << lhs_->ToString() << " <- new Array(" << dim_->ToString() << ")";
    return code.str();
  }

  std::string TupleAllocate::GenerateCode() {
    std::stringstream code;
    code << lhs_->ToString() << " <- call allocate(" << dim_->ToString() << ", 1)";
    return code.str();
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
    std::stringstream code;
    if(this->mark_ == nullptr) {
      code << this->ToString();
    } else {
      code << "br " << mark_->ToString() << " " << labels_[0]->ToString() << "\n\t";
      code << "br " << labels_[1]->ToString();
    }
    return code.str();
  }

  std::vector<std::shared_ptr<Label>> BranchInst::GetSuccessors() {
    return this->labels_;
  }


  std::string BasicBlock::GenerateCode() {
    std::stringstream code;
    code << this->name->ToString() << "\n";
    for(auto inst : this->instructions) {
      code << "\t" << inst->GenerateCode() << std::endl;
    }
    code << "\t" << this->terminator->GenerateCode() << std::endl;
    return code.str();
  }
  
  
  std::string BasicBlock::ToString() {
    std::stringstream code;
    code << this->name->ToString() << "\n";
    for(auto inst : this->instructions) {
      code << "\t" << inst->ToString() << std::endl;
    }
    code << "\t" << this->terminator->ToString() << std::endl;
    return code.str();
  }

  std::shared_ptr<Function> BasicBlock::GetParent() {
    return parent;
  }
  

  std::string Function::ToString() {
    std::stringstream code;
    code << "define " << ret
    << " " << this->function_name->ToString() << " (";
    for(int i = 0; i < args.size(); i++) {
      code << (i == 0 ? "" : ", ") << args[i]->ToString();
    }
    code << "){\n";
    for(auto bb : this->basic_blocks) {
      code << bb->ToString() << std::endl;
    }
    code << "}\n";
    return code.str();
  }

  std::string Function::GenerateCode() {
    std::stringstream code;
    code << "define " << this->function_name->ToString() << " (";
    for(int i = 0; i < args.size(); i++) {
      code << (i == 0 ? "" : ", ") << args[i]->ToString();
    }
    code << "){\n";
    std::string traces;
    std::queue<std::shared_ptr<BasicBlock>> list;
    std::map<std::shared_ptr<BasicBlock>, bool> color_mapper;
    for(auto bb : basic_blocks) {
      list.push(bb);
      color_mapper[bb] = true;
    }
    do {
      std::string trace;
      auto bb = list.front();
      list.pop();
      while(color_mapper[bb]) {
        color_mapper[bb] = false;
        auto terminator = bb->terminator;
        trace += bb->GenerateCode() + "\n";
        if(auto br = std::dynamic_pointer_cast<BranchInst>(terminator)) {
          auto labels = br->GetSuccessors();
          if(labels.size() == 1) {
            auto suc = this->bbs_mapper[labels[0]->ToString()];
            if(color_mapper[suc]) {
              bb = suc;
            }
          } else {
            auto suc = this->bbs_mapper[labels[1]->ToString()];
            if(color_mapper[suc]) {
              bb = suc;
            }
          }
        }
      }
      traces += trace;
    }while(!list.empty());
    code << traces << "\n}";
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