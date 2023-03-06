

#include "tree.hpp"
#include "L3.hpp"
#include "utils.hpp"
#include <cctype>
#include <sstream>
#include <string>

namespace L3 {
  //add new derived Tile at here
  std::map<std::string, Tile*> Tile::tile_mapper = {
    {"assign_tile", new AssignTile},
    {"load_tile", new LoadTile},
    {"store_tile", new StoreTile},
    {"load_with_offset_tile", new LoadWithOffsetTile},
    {"store_with_offset_tile", new StoreWithOffsetTile},
    {"return_tile", new ReturnTile},
    {"branch_tile", new BranchTile},
    {"label_tile", new LabelTile},
    {"cmp_tile", new CmpTile},
    {"binary_op_tile", new BinaryOpTile},
    {"call_tile", new CallTile},
    {"lea_left_tile", new LEALeftTile},
    {"lea_right_tile", new LEARightTile},
  };
  
  unsigned Node::GetCost() {
    return cost;
  }


  std::string Node::GetCode() {
    return code_;
  }

  std::vector<Node*> Node::GetChildren() {
    return children;
  }

  void Node::parse() {
    if(children.empty()) {
      cost = 1;
      return;
    }

    for(auto tile : Tile::getAllTile()) {
      auto [mark, next_nodes] = tile->accept(this);
      if(mark) {
        DEBUG_COUT << "node parsing matches" << std::endl;
        for(auto next : next_nodes) {
          DEBUG_COUT << "parsing next" << std::endl;
          next->parse();
          DEBUG_COUT << "next cost: " << next->GetCost() << std::endl;
          DEBUG_COUT << "tile cost: " << tile->GetCost() << std::endl;
          DEBUG_COUT << "cost: " << cost << std::endl;
          if(next->GetCost() + tile->GetCost() < cost) {
            generated_code = next->GetGeneratedCode();
            generated_code.push_back(tile->GenerateCode(this));
            cost = next->GetCost() + tile->GetCost();
            DEBUG_COUT << "cost now: " << cost << std::endl;
          }
        }
        DEBUG_COUT << "recurse finished" << std::endl;
      }
    }
  }

  std::vector<std::string> Node::GetGeneratedCode() {
    for(auto code : generated_code) {
      DEBUG_COUT << code << std::endl;
    }
    return generated_code;
  }

  void Node::AddChild(Node* child) {
    children.push_back(child);
  }

  void Node::ReplaceChild(Node* old_node, Node* new_node) {
    for(auto& node : children) {
      if(node == old_node) {
        node = new_node;
      }
    }
  }

  VariableNode::VariableNode(std::string variable) {
    code_ = variable;
  }

  ValueNode::ValueNode(std::string item) {
    code_ = item;
  }

  NumberNode::NumberNode(std::string number) {
    code_ = number;
  }

  OperatorNode::OperatorNode(std::string op) {
    code_ = op;
  }

  Tile* Tile::GetTile(const std::string& name) {
    return tile_mapper.at(name);
  }

  std::vector<Tile*> Tile::getAllTile() {
    std::vector<Tile*> tiles;
    for(auto [name, tile] : tile_mapper) {
      tiles.push_back(tile);
    }
    return tiles;
  }

  unsigned Tile::GetCost() {
    return cost;
  }

  AssignTile::AssignTile() {
    cost = 1;
  }

  std::pair<bool, std::vector<Node*>> AssignTile::accept(Node* root) {
    try {
      DEBUG_COUT << "parse assign tile" << std::endl;
      DEBUG_COUT << "root: " << root->GetCode() << std::endl;
      auto variable_node  = (VariableNode*)root;
      if(root->GetCode() == "nullptr" || root->GetChildren().size() == 0) {
        throw "";
      }

      auto op_node = (OperatorNode*)root->GetChildren()[0];
      DEBUG_COUT << "op: " <<  op_node->GetCode() << std::endl;
      if(op_node->GetCode() == "nullptr" || op_node->GetCode() != "<-") {
        throw "";
      }
     
      if(op_node->GetChildren().size() != 1) {
        throw "";
      }
      auto r_value = op_node->GetChildren()[0];
      DEBUG_COUT << "r_value: " << r_value->GetCode() << std::endl;
      // if(!r_value->GetChildren().empty()) {
      //   throw "";
      // }
      return {true, {r_value}};
    } catch(...) {
      DEBUG_COUT << "does not match" << std::endl;
      return {false, {}};
    }
    
  }

  std::string AssignTile::GenerateCode(Node* root) {
    std::stringstream code;
    auto variable_node  = (VariableNode*)root;
    auto op_node = (OperatorNode*)root->GetChildren()[0];
    auto r_value = op_node->GetChildren()[0];
    code << root->GetCode() << " <- " << r_value->GetCode() << std::endl;
    DEBUG_COUT << code.str() << std::endl;
    return code.str();
  }

  LoadTile::LoadTile() {
    cost = 1;
  }

  std::pair<bool, std::vector<Node*>> LoadTile::accept(Node* root) {
    try {
      DEBUG_COUT << "in load tile" << std::endl;
      auto variable_node  = (VariableNode*)root;
      DEBUG_COUT << root->GetCode() << std::endl;
      if(root->GetCode() == "nullptr" || root->GetChildren().size() == 0) {
        throw "";
      }

      auto op_node = (OperatorNode*)root->GetChildren()[0];
      if(op_node->GetCode() == "nullptr" || op_node->GetCode() != "load") {
        throw "";
      }
     
      if(op_node->GetChildren().size() != 1) {
        throw "";
      }
      auto r_value = op_node->GetChildren()[0];
      DEBUG_COUT << "right value: " << r_value->GetCode() << std::endl;
      // if(!r_value->GetChildren().empty()) {
      //   throw "";
      // }
      DEBUG_COUT << "load tile matches" << std::endl;
      return {true, {r_value}};
    } catch(...) {
      DEBUG_COUT << "load does not match" << std::endl;
      return {false, {}};
    }
  }
  
  std::string LoadTile::GenerateCode(Node* root) {
    std::stringstream code;
    auto variable_root = (VariableNode*)root;
    auto op_node = root->GetChildren()[0];
    auto r_value = op_node->GetChildren()[0];
    code << variable_root->GetCode() << " <- mem "
    << r_value->GetCode() << " 0" << std::endl;
    DEBUG_COUT << code.str() << std::endl;
    return code.str();
  }

  StoreTile::StoreTile() {
    cost = 1;
  }

  std::pair<bool, std::vector<Node*>> StoreTile::accept(Node* root) {
    try {
      auto op_node  = (VariableNode*)root;
      if(op_node->GetCode() != "store") {
        throw "";
      }
      auto l_value = op_node->GetChildren()[0];
      auto r_value = op_node->GetChildren()[1];
      return {true, {r_value}};
    } catch(...) {
      DEBUG_COUT << "does not match" << std::endl;
      return {false, {}};
    }
  }

  std::string StoreTile::GenerateCode(Node* root) {
    std::stringstream code;
    auto l_value = root->GetChildren()[0];
    auto r_value = root->GetChildren()[1];
    code << "mem " << l_value->GetCode() << " 0 <- " << r_value->GetCode() << std::endl;
    DEBUG_COUT << code.str() << std::endl;
    return code.str();
  }

  LoadWithOffsetTile::LoadWithOffsetTile() {
    cost = 1;
  }

  std::pair<bool, std::vector<Node*>> LoadWithOffsetTile::accept(Node* root) {
    try {
      if(root->GetCode()[0] != '%' || root->GetChildren().empty()) {
        throw "";
      }
      auto load_node = root->GetChildren()[0];
      if(load_node->GetCode() != "load") {
        throw "";
      }
      auto tmp_node = load_node->GetChildren()[0];
      if(tmp_node->GetChildren().empty()) {
        throw "";
      }
      auto assign = tmp_node->GetChildren()[0];
      if(assign->GetCode() != "<-" || assign->GetChildren().size() != 3) {
        throw "";
      }
      auto t1_node = assign->GetChildren()[0];
      auto op_node = assign->GetChildren()[1];
      auto t2_node = assign->GetChildren()[2];
      if(t1_node->GetCode()[0] == '%') {
        if(typeid(t2_node) == typeid(NumberNode)) {
          return {true, {t1_node}};
        } else {
          throw "";
        }
      } else if(t2_node->GetCode()[0] == '%') {
        if(typeid(t1_node) == typeid(NumberNode)) {
          return {true, {t2_node}};
        } else {
          throw "";
        }
      } else {
        throw "";
      }
    } catch(...) {
      return {false, {}};
    }
  }

  std::string LoadWithOffsetTile::GenerateCode(Node* root) {
    std::stringstream code;
    auto load_node = root->GetChildren()[0];
    auto tmp_node = load_node->GetChildren()[0];
    auto assign = tmp_node->GetChildren()[0];
    auto t1_node = assign->GetChildren()[0];
    auto op_node = assign->GetChildren()[1];
    auto t2_node = assign->GetChildren()[2];
    if(typeid(t1_node) == typeid(VariableNode)) {
      code << root->GetCode() << " <- mem " << t1_node->GetCode() << " " << t2_node->GetCode() << std::endl;
    } else {
      code << root->GetCode() << " <- mem " << t2_node->GetCode() << " " << t1_node->GetCode() << std::endl;
    }
    return code.str();
  }

  StoreWithOffsetTile::StoreWithOffsetTile() {
    cost = 1;
  }

  std::pair<bool, std::vector<Node*>> StoreWithOffsetTile::accept(Node* root) {
    try {
      if(root->GetCode() != "store") {
        throw "";
      }
      auto l_value = root->GetChildren()[0];
      auto r_value = root->GetChildren()[1];
      if(r_value->GetChildren().empty() || r_value->GetChildren()[0]->GetCode() != "<-") {
        throw "";
      }
      auto assign = r_value->GetChildren()[0];
      auto t1_node = assign->GetChildren()[0];
      auto op_node = assign->GetChildren()[1];
      auto t2_node = assign->GetChildren()[2];
      if(typeid(t1_node) == typeid(NumberNode) && typeid(t2_node) == typeid(VariableNode)) {
        return {true, {t2_node}};
      } else if(typeid(t2_node) == typeid(NumberNode) && typeid(t1_node) == typeid(VariableNode)) {
        return {true, {t1_node}};
      } else {
        throw "";
      }
    } catch(...) {
      return {false, {}};
    }
  }

  std::string StoreWithOffsetTile::GenerateCode(Node* root) {
    std::stringstream code;
    auto l_value = root->GetChildren()[0];
    auto r_value = root->GetChildren()[1];
    auto assign = r_value->GetChildren()[0];
    auto t1_node = assign->GetChildren()[0];
    auto op_node = assign->GetChildren()[1];
    auto t2_node = assign->GetChildren()[2];
    code << "mem " << l_value->GetCode();
    if(typeid(t1_node) == typeid(NumberNode)) {
      code << " " << t1_node->GetCode();
      code << " <- " << t2_node->GetCode() << std::endl;
    } else {
      code << " " << t2_node->GetCode();
      code << " <- " << t1_node->GetCode() << std::endl;
    }
    DEBUG_COUT << code.str() << std::endl;
    return code.str();
  }
  

  ReturnTile::ReturnTile() {
    cost = 1;
  }

  std::pair<bool, std::vector<Node*>> ReturnTile::accept(Node* root) {
    try {
      if(root->GetCode() == "nullptr") {
        throw "";
      }
      if(root->GetCode() != "return") {
        throw "";
      }
      auto t = root->GetChildren();
      if(t[0]->GetCode() != "nullptr") {
        cost = 2;
      }
      return {true, {t[0]}};
    } catch(...) {
      DEBUG_COUT << "does not match" << std::endl;
      return {false, {}};
    }
  }

  std::string ReturnTile::GenerateCode(Node* root) {
    auto op_node = (OperatorNode*)root;
    auto t = root->GetChildren()[0];
    std::stringstream code;
    if(t->GetCode() != "nullptr") {
      code << "rax <- " << t->GetCode() << std::endl;
    }
    code << "return";
    return code.str();
  }


  BranchTile::BranchTile() {
    cost = 1;
  }

  std::pair<bool, std::vector<Node*>> BranchTile::accept(Node* root) {
    try {
      if(root->GetCode() != "br") {
        throw "";
      }
      auto t = root->GetChildren()[0], label = root->GetChildren()[1];
      DEBUG_COUT << "br matches: " << t->GetCode() << " " << label->GetCode() << std::endl;
      return {true, {t, label}};
    }catch(...) {
      DEBUG_COUT << "does not match" << std::endl;
      return {false, {}};
    }
  }

  std::string BranchTile::GenerateCode(Node* root) {
    auto t = root->GetChildren()[0], label = root->GetChildren()[1];
    std::stringstream code;
    if(t->GetCode() == "nullptr") {
      code << "goto " << label->GetCode() << std::endl;
    } else {
      code << "cjump 1 = " << t->GetCode() << " " << label->GetCode() << std::endl;
    }
    return code.str();
  }

  LabelTile::LabelTile() {
    cost = 1;
  }

  std::pair<bool, std::vector<Node*>> LabelTile::accept(Node* root) {
    try {    
      if(root->GetCode() != "label") {
        throw "";
      }
      DEBUG_COUT << "label: " << root->GetChildren()[0] << std::endl;
      return {true, {root->GetChildren()[0]}};
    } catch(...) {
      DEBUG_COUT << "does not match" << std::endl;
      return {false, {}};
    }
  }

  std::string LabelTile::GenerateCode(Node* root) {
    std::stringstream code;
    code << root->GetChildren()[0]->GetCode() << std::endl;
    return code.str();
  }

  CmpTile::CmpTile() {
    cost = 1;
  }

  std::pair<bool, std::vector<Node*>> CmpTile::accept(Node* root) {
    try {
      if(root->GetCode()[0] != '%') {
        throw "";
      }
      if(root->GetChildren().empty()) {
        throw "";
      }
      auto op_node = root->GetChildren()[0];
      if(op_node->GetCode() != "<-" || op_node->GetChildren().size() != 3) {
        throw "";
      }
      auto t1 = op_node->GetChildren()[0], op = op_node->GetChildren()[1], t2 = op_node->GetChildren()[2];
      auto symbols = {"<", "<=", "=", ">=", ">"};
      int flag = 0;
      for(auto symbol : symbols) {
        if(op->GetCode() == symbol) {
          flag = 1;
        }
      }
      if(flag == 0) {
        throw "";
      }
      return {true, {t1, t2}};
    } catch(...) {
      DEBUG_COUT << "does not match" << std::endl;
      return {false, {}};
    }
  }

  std::string CmpTile::GenerateCode(Node* root) {
    auto assign = root->GetChildren()[0];
    auto t1 = assign->GetChildren()[0];
    auto op_node = assign->GetChildren()[1];
    auto t2 = assign->GetChildren()[2];
    std::stringstream code;
    if(op_node->GetCode()[0] == '>') {
      auto cmp = op_node->GetCode();
      cmp[0] = '<';
      code << root->GetCode() << " <- " << t2->GetCode()
      << " " + cmp + " " << t1->GetCode();
    } else {
      code << root->GetCode() << " <- " << t1->GetCode()
      << " " + op_node->GetCode() + " " << t2->GetCode();
    }
    code << std::endl;
    return code.str();
  }

  BinaryOpTile::BinaryOpTile() {
    cost = 3;
  }

  std::pair<bool, std::vector<Node*>> BinaryOpTile::accept(Node* root) {
    try {
      DEBUG_COUT << "root: " << root->GetCode() << std::endl;
      if(root->GetCode()[0] != '%' || root->GetChildren().empty()) {
        throw "";
      }
      auto assign = root->GetChildren()[0];
      DEBUG_COUT << "assign node: " << assign->GetCode() << std::endl;
      if(assign->GetCode() != "<-" || assign->GetChildren().size() != 3) {
        throw "";
      }
      auto t1 = assign->GetChildren()[0], op = assign->GetChildren()[1], t2 = assign->GetChildren()[2];
      DEBUG_COUT << "t1: " << t1->GetCode() << " t2: " << t2->GetCode() << std::endl;
      return {true, {t1, t2}};
    } catch(...) {
      DEBUG_COUT << "does not match" << std::endl;
      return {false, {}};
    }
  }

  std::string BinaryOpTile::GenerateCode(Node* root) {
    auto assign = root->GetChildren()[0];
    auto t1 = assign->GetChildren()[0];
    auto op_node = assign->GetChildren()[1];
    auto t2 = assign->GetChildren()[2];
    std::stringstream code;
    code << root->GetCode() + "__S" << " <- " << t1->GetCode() << std::endl;
    code << root->GetCode() + "__S" << " " + op_node->GetCode() + "= " << t2->GetCode() << std::endl;
    code << root->GetCode() << " <- " << root->GetCode() + "__S";
    return code.str();
  }

  CallTile::CallTile() {
    cost = 1;
  }

  unsigned CallTile::count = 0;

  std::pair<bool, std::vector<Node*>> CallTile::accept(Node* root) {
    try {
      DEBUG_COUT << "in call tile" << std::endl;
      if(root->GetChildren().empty()) {
        throw "";
      }
      auto call_node = root->GetChildren()[0];
      if(call_node->GetCode() != "call") {
        throw "";
      }
      auto callee = call_node->GetChildren()[0];
      std::vector<Node*> next_node;
      for(int i = 0; i < callee->GetChildren().size(); i++) {
        next_node.push_back(callee->GetChildren()[i]);
      }
      if(next_node.empty()) {
        return {true, {new ValueNode("nullptr")}};
      } else {
        return {true, next_node};
      }
    } catch(...) {
      DEBUG_COUT << "does not match" << std::endl;
      return {false, {}};
    }
  }

  std::string CallTile::GenerateCode(Node* root) {
    std::vector<std::string> regs = {
          "rdi", "rsi", "rdx", "rcx", "r8", "r9"
          };
    auto call_node = root->GetChildren()[0];
    auto callee = call_node->GetChildren()[0];
    std::stringstream code;
    for(int i = 0; i < callee->GetChildren().size(); i++) {
        if(i < 6) {
          code << regs.at(i) << " <- " << callee->GetChildren()[i]->GetCode() << std::endl;
        } else {
          code << "mem rsp " << std::to_string(32 - 8 * i) << " <- " << callee->GetChildren()[i]->GetCode() << std::endl;
        }
    }
    if(callee->GetCode()[0] != '@' && callee->GetCode()[0] != '%') {
       code << "call " << callee->GetCode() << " "
       << std::to_string(callee->GetChildren().size()) << std::endl;
    } else {
      auto call_label = callee->GetCode();
      call_label[0] = ':';
      call_label += "_ret" + std::to_string(count++);
      code << "mem rsp -8 <- " << call_label << std::endl;
      

      code << "call " << callee->GetCode()
       << " " << std::to_string(callee->GetChildren().size()) << std::endl;
      code << call_label << std::endl;
    }
    if(root->GetCode() != "nullptr") {
      code << root->GetCode() << " <- rax";
    }
    return code.str();
  }

  LEALeftTile::LEALeftTile() {
    cost = 1;
  }

  std::pair<bool, std::vector<Node*>> LEALeftTile::accept(Node* root) {
    try {
      if(typeid(root) != typeid(VariableNode) || root->GetChildren().empty()) {
        throw "";
      }
      auto assign = root->GetChildren()[0];
      if(assign->GetChildren().size() != 3) {
        throw "";
      }
      auto t1 = assign->GetChildren()[0];
      auto op = assign->GetChildren()[1];
      auto t2 = assign->GetChildren()[2];
      if(t1->GetChildren().empty() || op->GetCode() != "+") {
        throw "";
      }
      assign = t1->GetChildren()[0];
      if(assign->GetChildren().size() != 3) {
        throw "";
      }
      auto p1 = assign->GetChildren()[0];
      auto op_n = assign->GetChildren()[1];
      auto p2 = assign->GetChildren()[2];
      if(op_n->GetCode() != "*") {
        throw "";
      }
      if(typeid(p1) == typeid(NumberNode) && typeid(p2) == typeid(VariableNode)) {
        auto num = std::stoll(p1->GetCode());
        if(num == 1 || num == 2 || num == 4) {
          return {true, {t2, p2, p1}};
        } else {
          throw "";
        }
      } else if(typeid(p2) == typeid(NumberNode) && typeid(p1) == typeid(VariableNode)) {
        auto num = std::stoll(p2->GetCode());
        if(num == 1 || num == 2 || num == 4) {
          return {true, {t2, p1, p2}};
        } else {
          throw "";
        }
      }
      else {
        throw "";
      }
    } catch (...) {
      return {false, {}};
    }
  }

  std::string LEALeftTile::GenerateCode(Node* root) {
    auto assign = root->GetChildren()[0];
    auto t1 = assign->GetChildren()[0], t2 = assign->GetChildren()[2];
    auto p1 = t1->GetChildren()[0], p2 = t1->GetChildren()[2];
    std::stringstream code;
    if(typeid(p1) == typeid(NumberNode)) {
      code << root->GetCode() << " @ " << t2->GetCode() 
      << " " << p2->GetCode() << " " << p1->GetCode() << std::endl;
    }
    return code.str();
  }

  LEARightTile::LEARightTile() {
    cost = 1;
  }

  std::pair<bool, std::vector<Node*>> LEARightTile::accept(Node* root) {
    try {
      if(typeid(root) != typeid(VariableNode) || root->GetChildren().empty()) {
        throw "";
      }
      auto assign = root->GetChildren()[0];
      if(assign->GetChildren().size() != 3) {
        throw "";
      }
      auto t1 = assign->GetChildren()[0];
      auto op = assign->GetChildren()[1];
      auto t2 = assign->GetChildren()[2];
      if(t2->GetChildren().empty() || op->GetCode() != "+") {
        throw "";
      }
      assign = t2->GetChildren()[0];
      if(assign->GetChildren().size() != 3) {
        throw "";
      }
      auto p1 = assign->GetChildren()[0];
      auto op_n = assign->GetChildren()[1];
      auto p2 = assign->GetChildren()[2];
      if(op_n->GetCode() != "*") {
        throw "";
      }
      if(typeid(p1) == typeid(NumberNode) && typeid(p2) == typeid(VariableNode)) {
        auto num = std::stoll(p1->GetCode());
        if(num == 1 || num == 2 || num == 4) {
          return {true, {t2, p2, p1}};
        } else {
          throw "";
        }
      } else if(typeid(p2) == typeid(NumberNode) && typeid(p1) == typeid(VariableNode)) {
        auto num = std::stoll(p2->GetCode());
        if(num == 1 || num == 2 || num == 4) {
          return {true, {t2, p1, p2}};
        } else {
          throw "";
        }
      }
      else {
        throw "";
      }
    } catch (...) {
      return {false, {}};
    }
  }

  std::string LEARightTile::GenerateCode(Node* root) {
    auto assign = root->GetChildren()[0];
    auto t1 = assign->GetChildren()[0], t2 = assign->GetChildren()[2];
    auto p1 = t1->GetChildren()[0], p2 = t1->GetChildren()[2];
    std::stringstream code;
    if(typeid(p1) == typeid(NumberNode)) {
      code << root->GetCode() << " @ " << t1->GetCode() 
      << " " << p2->GetCode() << " " << p1->GetCode() << std::endl;
    }
    return code.str();
  }
}