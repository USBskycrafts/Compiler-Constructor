

#include "L2.hpp"
#include "utils.hpp"
#include "visitor.hpp"
#include "control_flow_graph.hpp"


namespace L2 {
  
  std::set<Register*> Register::registers_;

  std::set<Variable*> Variable::variables_;

  Item::Item(const std::string name) : code_{name} {}

  void Item::accept(Visitor *v) {
    v->visit(this);
  }

  Item::ItemType Item::get_type() const {
    return type_;
  }

  std::string Item::get_code() const {
    return code_;
  }

  std::ostream& operator<<(std::ostream& stream, const Item *item) {
    stream << item->get_code();
    return stream;
  }

  bool Item::operator==(const Item& other) {
    return code_ == other.code_;
  }

  bool Item::operator<(const Item &other) {
    return code_ < other.code_;
  }

  Value::Value(std::string name) : Item(name) {}


  void Value::accept(Visitor *v) {
    v->visit(this);
  }

  Register::Register(const std::string name) : Value(name) {
    this->register_type_ = reg_name_mapper.at(name);
    Item::type_ = kRegister;
    //DEBUG_PRINT("register: %s", this->get_code().c_str());
  }

  Register* Register::GetInstance(const std::string& name) {
    for(auto r : registers_) {
      if(r->code_ == name) {
        return r;
      }
    }
    auto r = new Register(name);
    registers_.insert(r);
    return r;
  }

  void Register::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  int Register::countRegister() {
    return registers_.size();
  }


  std::vector<Register*> Register::getAllCallerSavedRegisters() {
    std::vector<Register*> registers;
    registers.push_back(Register::GetInstance("rdi"));
    registers.push_back(Register::GetInstance("rsi"));
    registers.push_back(Register::GetInstance("rdx"));
    registers.push_back(Register::GetInstance("rcx"));
    registers.push_back(Register::GetInstance("r8"));
    registers.push_back(Register::GetInstance("r9"));
    registers.push_back(Register::GetInstance("r10"));
    registers.push_back(Register::GetInstance("r11"));
    registers.push_back(Register::GetInstance("rax"));
    return registers;
  }

  std::vector<Register*> Register::getAllCalleeSavedRegisters() {
    std::vector<Register*> registers;
    registers.push_back(Register::GetInstance("r12"));
    registers.push_back(Register::GetInstance("r13"));
    registers.push_back(Register::GetInstance("r14"));
    registers.push_back(Register::GetInstance("r15"));
    registers.push_back(Register::GetInstance("rbp"));
    registers.push_back(Register::GetInstance("rbx"));
    return registers;
  }

  std::vector<Register*> Register::getGeneralPurposeRegisters() {
    std::vector<Register*> registers;
    registers.push_back(Register::GetInstance("rdi"));
    registers.push_back(Register::GetInstance("rsi"));
    registers.push_back(Register::GetInstance("rdx"));
    registers.push_back(Register::GetInstance("rcx"));
    registers.push_back(Register::GetInstance("r8"));
    registers.push_back(Register::GetInstance("r9"));
    registers.push_back(Register::GetInstance("r10"));
    registers.push_back(Register::GetInstance("rax"));
    registers.push_back(Register::GetInstance("r11"));
    registers.push_back(Register::GetInstance("r12"));
    registers.push_back(Register::GetInstance("r13"));
    registers.push_back(Register::GetInstance("r14"));
    registers.push_back(Register::GetInstance("r15"));
    registers.push_back(Register::GetInstance("rbp"));
    registers.push_back(Register::GetInstance("rbx"));
    return registers;
  }


  Variable::Variable(const std::string name) : Value(name) {
    this->type_ = kVariable;
  }

  void Variable::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  int Variable::countVariable() {
    return variables_.size();
  }

  Variable* Variable::GetInstance(const std::string &name) {
    for(auto v : variables_) {
      if(v->get_code() == name) {
        return v;
      }
    }
    auto v = new Variable(name);
    //DEBUG_COUT << variables_.size() << std::endl;
    variables_.insert(v);
    return v;
  }

  std::set<Variable*> Variable::getAll() {
    return variables_;
  }

  Number::Number(const std::string value) : Item(value) {
    value_ = std::stoll(value);
    this->type_ = kNumber;
  }

  void Number::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  int64_t Number::get_value() const {
    return value_;
  }

  MemoryAddress::MemoryAddress(Value* v, Number* n)
    : Item("mem " + v->get_code() + " " + n->get_code()), 
    v_(v), n_(n) {
    this->type_ = kMemoryAddress;
  }

  Value* MemoryAddress::get_value() const {
    return v_;
  }

  Number* MemoryAddress::get_number() const {
    return n_;
  }

  void MemoryAddress::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  Operator::Operator(const std::string symbol) : Item(symbol) {
    this->type_ = kOperator;
  }

  void Operator::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  Label::Label(const std::string name) : Item(name) {
    this->type_ = kLabel;
  }

  void Label::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  FunctionName::FunctionName(const std::string name) : Item(name) {
    this->type_ = kFunctionName;
  }

  void FunctionName::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  Comparison::Comparison(Item *v1, Operator *o, Item *v2) 
    : Item(v1->get_code() + " " + o->get_code() + " " + v2->get_code()), 
    v1_(v1), v2_(v2), o_(o){
    this->type_ = kComparison;
  }

  void Comparison::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  std::pair<Item*, Item*> Comparison::get_items() const {
    return {v1_, v2_};
  }

  Operator* Comparison::get_operator() const {
    return o_;
  }


  Instruction::Instruction() {}

  Instruction::InstructionType Instruction::get_type() const {
    return type_;
  }

  void Instruction::accept(Visitor* visitor) {
    visitor->visit(this);
  }


  InstructionReturn::InstructionReturn() {
    type_ = kReturn;
  }

  void InstructionReturn::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  InstructionCall::InstructionCall(Item *i, Number *n) 
    : i_(i), n_(n) {
    type_ = kCall;
  }

  Item* InstructionCall::get_item() const {
    return i_;
  }

  Number* InstructionCall::get_number() const {
    return n_;
  }

  void InstructionCall::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  InstructionAssignment::InstructionAssignment(Item *d, Item *s)
    : d_(d), s_(s) {
      type_ = kAssignment;
  }

  std::pair<Item*, Item*> InstructionAssignment::get_items() const {
    return {d_, s_};
  }

  void InstructionAssignment::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  InstructionLabel::InstructionLabel(Label* l) : l_(l) {
    type_ = kLabel;
  }

  Label* InstructionLabel::get_label() const {
    return l_;
  }

  void InstructionLabel::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  InstructionArithmetic::InstructionArithmetic(Item *i1, Operator *o, Item *i2)
    : i1_(i1), i2_(i2), o_(o) {
      type_ = kArithmetic;
  }

  std::pair<Item*, Item*> InstructionArithmetic::get_items() const {
    return {i1_, i2_};
  }

  Operator* InstructionArithmetic::get_operator() const {
    return o_;
  }

  void InstructionArithmetic::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  InstructionSelfOperation::InstructionSelfOperation(Item *i, Operator *o)
  : i_(i), o_{o} {
    type_ = kSelfOperation;
  }

  Item* InstructionSelfOperation::get_item() const {
    return i_;
  }

  Operator* InstructionSelfOperation::get_operator() const {
    return o_;
  }

  void InstructionSelfOperation::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  InstructionConditionalJump::InstructionConditionalJump(Comparison *c, Label* l)
    : c_(c), l_(l) {
      type_ = kConditionalJump;
  }

  Comparison* InstructionConditionalJump::get_comparison() const {
    return c_;
  }

  Label* InstructionConditionalJump::get_label() const {
    return l_;
  }

  void InstructionConditionalJump::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  InstructionJump::InstructionJump(Label* l) : l_{l} {
    type_ = kJump;
  }

  Label* InstructionJump::get_label() const {
    return l_;
  }

  void InstructionJump::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  InstructionLeaq::InstructionLeaq(Item* i1, Item* i2, Item* i3, Number* n)
    : i1_(i1), i2_(i2), i3_(i3), n_(n) {
    type_ = kLeaq;
  }

  std::vector<Item*> InstructionLeaq::get_items() const {
    return {i1_, i2_, i3_};
  }

  Number* InstructionLeaq::get_number() const {
    return n_;
  }

  void InstructionLeaq::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  InstructionStackArg::InstructionStackArg(Item* i, Number* n) 
    : n_(n), i_{i} {
    type_ = kStackArg;
  }

  void InstructionStackArg::accept(Visitor *visitor) {
    visitor->visit(this);
  } 

  Number* InstructionStackArg::get_number() const {
    return n_;
  }

  Item* InstructionStackArg::get_item() const {
    return i_;
  }

  void Function::accept(Visitor *visitor) {
    visitor->visit(this);
  }

  Function::Function() {}

  std::map<Instruction*, std::set<Instruction*>> Function::getSuccessors() {
    this->cfg = new ControlFlowGraph(this);
    return cfg->getSuccessors();
  }

  std::map<Instruction*, std::set<Instruction*>> Function::getPredecessors() {
    this->cfg = new ControlFlowGraph(this);
    return cfg->getPredecessors();
  }

  std::ostream& InstructionArithmetic::print(std::ostream& stream) {
    return stream << this->get_items().first->get_code() 
                  << " <- "
                  << this->get_operator()->get_code()
                  << " "
                  << this->get_items().second->get_code();
  }

  std::ostream& InstructionAssignment::print(std::ostream& stream) {
    return stream << this->get_items().first->get_code()
                  << " <- "
                  << this->get_items().second->get_code();
  }

  std::ostream& InstructionCall::print(std::ostream& stream) {
   return stream << "call "
                  << this->get_item()->get_code()
                  << " "
                  << this->get_number()->get_code();
  }
  

  std::ostream& InstructionConditionalJump::print(std::ostream& stream) {
   return stream << "cjump "
                  << this->get_comparison()->get_code()
                  << " "
                  << this->get_label()->get_code();
  }

  std::ostream& InstructionJump::print(std::ostream& stream) {
   return stream << "goto "
                  << this->get_label()->get_code();
  }

  std::ostream& InstructionLabel::print(std::ostream& stream) {
   return stream << this->get_label()->get_code();
  }

  std::ostream& InstructionLeaq::print(std::ostream& stream) {
   return stream << this->get_items()[0]->get_code()
                  << " @ "
                  << this->get_items()[1]->get_code()
                  << " "
                  << this->get_items()[2]->get_code()
                  << " "
                  << this->get_number()->get_code();
  }

  std::ostream& InstructionReturn::print(std::ostream& stream) {
   return stream << "return";
  }

  std::ostream& InstructionSelfOperation::print(std::ostream& stream) {
   return stream << this->get_item()->get_code()
                  << this->get_operator();
  }

  std::ostream& InstructionStackArg::print(std::ostream& stream) {
   return stream << this->get_item()->get_code()
                  << " <- stack-arg"
                  << this->get_number()->get_code();
  }
}