

#include <sstream>
#include <string>
#include <vector>

#include "LB.hpp"


namespace LB {
  std::string Comparator::GetCode() {
    std::stringstream code;
    code << t1_->GetCode() << " " << cmp_->GetCode() << " " << t2_->GetCode();
    return code.str();
  }

  Code::Code(std::string code)  {
    SetCode(code);
    SetType("code");
  }

  Array::Array(std::string code, const unsigned dim) {
    SetCode(code);
    std::string type = "int64";
    for(auto i = 0U; i < dim; i++) {
      type += "[]";
    }
    SetType(type);
    indexes_.resize(dim);
  }

  Tuple::Tuple(std::string code) {
    SetCode(code);
    SetType("tuple");
  }

  std::string Scope::GenerateCode() {
    std::stringstream code;
    code << "\n";
    for(auto inst : instructions_) {
      code << "\t" << inst->GenerateCode() << '\n';
    }
    return code.str();
  }

  std::string Scope::ToString() {
    std::stringstream code;
    code << "{\n\t";
    for(auto inst : instructions_) {
      code << inst->ToString() << "\n\t";
    }
    code << "\n}\n";
    return code.str();
  }

  std::string DeclareInst::GenerateCode() {
    std::stringstream code;
    for(auto var : vars_) {
      code << var->GetType() << " " << var->GetCode() << "\t";
    }
    code << "\n";
    return code.str();
  }
  
  std::string DeclareInst::ToString() {
    std::stringstream code;
    auto type = vars_[0]->GetType();
    code << type;
    for(int i = 0; i < vars_.size(); i++) {
      code << (i == 0 ? "" : ", ") << vars_[i]->GetCode();
    }
    return code.str();
  }

  std::string AssignInst::GenerateCode() {
    return ToString();
  }

  std::string AssignInst::ToString() {
    std::stringstream code;
    code << lhs_->GetCode() << " <- " << rhs_->GetCode();
    return code.str();
  }

  std::string BinaryOperator::GenerateCode() {
    return ToString();
  }

  std::string BinaryOperator::ToString() {
    std::stringstream code;
    code << lhs_->GetCode() << " <- " << t1_->GetCode()
    << " " << op_->GetCode() << " " << t2_->GetCode();
    return code.str();
  }

  std::string LabelInst::GenerateCode() {
    return ToString();
  }

  std::string LabelInst::ToString() {
    return label_->GetCode();
  }


  std::string IfStatement::GenerateCode() {
    std::stringstream code;
    auto id = std::to_string((long long)this);
    auto var_name = "newV" + id;
    code << "\tint64 " << var_name << "\n";
    code << "\t" << var_name << " " <<cmp_->GetCode() <<"\n";
    code << "\tbr " << var_name << " " << true_label_->GetCode() << " " << false_label_->GetCode();
    return code.str();
  }

  std::string IfStatement::ToString() {
    std::stringstream code;
    code << "if (" << cmp_->GetCode() << ")" 
    << true_label_->GetCode() << " " << false_label_->GetCode();
    return code.str();
  }

  std::string JumpInst::GenerateCode() {
    return "br " + label_->GetCode();
  }

  std::string JumpInst::ToString() {
    return "goto" + label_->GetCode();
  }

  std::string ReturnInst::GenerateCode() {
    return ToString();
  }

  std::string ReturnInst::ToString() {
    return "return" + (ret_ ? ret_->GetCode() : "");
  }

  std::string WhileStatement::GenerateCode() {
    std::stringstream code;
    code << "while (" << cmp_->GetCode() << ")" 
    << true_label_->GetCode() << " " << false_label_->GetCode();
    return code.str();
  }

  std::string WhileStatement::ToString() {
    std::stringstream code;
    auto id = std::to_string((long long)this);
    auto var_name = "newV" + id;
    code << "\tint64 " << var_name << "\n";
    code << "\t" << var_name << " " <<cmp_->GetCode() <<"\n";
    code << "\tbr " << var_name << " " << true_label_->GetCode() << " " << false_label_->GetCode();
    return code.str();
  }

  std::string Continue::GenerateCode() {
    auto loop = GetLoop();
    auto l_cond = loop->GetCondLabel();
    return "br " + l_cond;
  }

  std::string Continue::ToString() {
    return "continue";
  }

  std::string Break::GenerateCode() {
    auto loop = GetLoop();
    auto l_exit = loop->GetFalseLabel();
    return "br " + l_exit;
  }

  std::string Break::ToString() {
    return "break";
  } 

  std::string LoadContainer::GenerateCode() {
    return ToString();
  }

  std::string LoadContainer::ToString() {
    std::stringstream code;
    code << lhs_->GetCode() << " <- " << rhs_->GetCode();
    for(auto index : indexes_) {
      code << '[' + index->GetCode() + ']';
    }
    return code.str();
  }

  std::string StoreContainer::GenerateCode() {
    return ToString();
  }

  std::string StoreContainer::ToString() {
    std::stringstream code;
    code << rhs_->GetCode();
    for(auto index : indexes_) {
      code << '[' + index->GetCode() + ']';
    }
    code << " <- " << rhs_->GetCode();
    return code.str();
  }

  std::string GetLength::GenerateCode() {
    return ToString();
  }

  std::string GetLength::ToString() {
    std::stringstream code;
    code << lhs_->GetCode() << " <- length "
    << rhs_->GetCode() << " " << index_->GetCode();
    return code.str();
  }

  std::string CallInst::GenerateCode() {
    return ToString();
  }

  std::string CallInst::ToString() {
    std::stringstream code;
    code << (ret_ ? ret_->GetCode() + " <- " : "");
    code << callee_->GetCode() << "(";
    for(int i = 0; i < args_.size(); i++) {
      code << (i == 0 ? "" : ", ") << args_[i]->GetCode();
    }
    code << ")";
    return code.str();
  }

  std::string AllocateTuple::GenerateCode() {
    return ToString();
  }

  std::string AllocateTuple::ToString() {
    std::stringstream code;
    code << lhs_->GetCode() << " <- Tuple(" << index_->GetCode() << ")";
    return code.str();
  }

  std::string AllocateArray::GenerateCode() {
    return ToString();
  }

  std::string AllocateArray::ToString() {
    std::stringstream code;
    code << lhs_->GetCode() << " <- Array(";
    for(int i = 0; i < args_.size(); i++) {
      code << (i == 0 ? "" : ", ") << args_[i]->GetCode();
    }
    code << ")";
    return code.str();
  }
}