
#include <sstream>
#include <string>

#include "liveness_analysis.hpp"
#include "tree_generator.hpp"
#include "L3.hpp"
#include "tree.hpp"
#include "utils.hpp"

namespace L3 {
  TreeGenerator::TreeGenerator(std::vector<Instruction*> context, 
  std::map<Instruction*, std::set<Variable*>> out_set_mapper) {
    this->context_ = context;
    this->out_set_mapper_ = out_set_mapper;
    for(auto inst : context_) {
      auto defs = inst->get_defs();
      auto uses = inst->get_uses();
      for(auto def : defs) {
        if(auto variable = dynamic_cast<Variable*>(def)) {
          gen_set_mapper[inst].insert(variable);
        }
      }
      for(auto use : uses) {
        if(auto variable = dynamic_cast<Variable*>(use)) {
          kill_set_mapper_[inst].insert(variable);
        }
      }
    }
    GenerateTrees();
    MergeTrees();
  }

  std::vector<Node*> TreeGenerator::GetTrees() {
    return this->trees_;
  }

  void TreeGenerator::GenerateTrees() {
    for(auto inst : context_) {
      inst->accept(this);

      auto node = trees_.back();
      node_defs_mapper[node] = kill_set_mapper_[inst];
      node_uses_mapper[node] = gen_set_mapper[inst];
    }
  }

  std::string TreeGenerator::CodeGenerate() {
    std::stringstream code;
    for(auto node : trees_) {
      if(node == nullptr) {
        continue;
      }
      node->parse();
      for(auto inst : node->GetGeneratedCode()) {
        code << inst << std::endl;
      }
    }
    return code.str();
  }

  std::vector<Node*> GetAllChildren(Node* node) {
    if(node->GetChildren().empty()) {
      return {node};
    }
    std::vector<Node*> leaves;
    for(auto child : node->GetChildren()) {
      auto l = GetAllChildren(child);
      leaves.insert(leaves.end(), l.begin(), l.end());
    }
    return leaves;
  }

  void TreeGenerator::MergeTrees() {
#ifdef MERGE
    int n = trees_.size();
    for(int i = 0; i < n; i++) {
      if(trees_[i] == nullptr) {
        continue;
      }
      std::set<Variable*> all_uses, all_defs, all_out;
      auto instructions = instructions_mapper_[trees_[i]];
      for(auto inst : instructions) {
        all_out.insert(out_set_mapper_[inst].begin(), out_set_mapper_[inst].end());
      }
      for(int j = i - 1; j >= 0; j--) {
        try {
          if(trees_[j] == nullptr) {
            throw "";
          }
          for(auto variable : all_out) {
          if(trees_[j]->GetCode() == variable->to_string()) {
            throw "";
          }
        }
        } catch(...) {
          DEBUG_COUT << "continue for not dead in out set" << std::endl;
          continue;
        }
        instructions = instructions_mapper_[trees_[j]];
        for(auto inst : instructions) {
          auto gen_set = node_uses_mapper[trees_[j]];
          auto kill_set = node_defs_mapper[trees_[j]];
          all_uses.insert(gen_set.begin(), gen_set.end());
          all_defs.insert(kill_set.begin(), kill_set.end());
        }
        auto children = GetAllChildren(trees_[j]);
        try {
          for(auto use : all_uses) {
            if(use->to_string() == trees_[j]->GetCode()) {
              throw "";
            }
          }
          for(auto def : all_defs) {  
            for(auto child : children) {
              if(def->to_string() == child->GetCode()) {
                throw "";
              }
            }
          }
        } catch(...) {
          DEBUG_COUT << "continue for there is dependencies in middle" << std::endl;
          continue;
        }
        children = GetAllChildren(trees_[i]);
        for(auto child : children) {
          if(child->GetCode() == trees_[j]->GetCode()) {
            DEBUG_COUT << "able to merge" << std::endl;
            if(trees_[j]->GetChildren().size() != 1) {
              break;
            }
            child->AddChild(trees_[j]->GetChildren()[0]);
            trees_[j] = nullptr;
            node_defs_mapper[trees_[i]].insert(node_defs_mapper[trees_[j]].begin(), node_defs_mapper[trees_[j]].end());
            node_uses_mapper[trees_[i]].insert(node_uses_mapper[trees_[j]].begin(), node_uses_mapper[trees_[j]].end());
            instructions_mapper_[trees_[i]].insert(instructions_mapper_[trees_[j]].begin(), instructions_mapper_[trees_[j]].end());
            node_defs_mapper.erase(trees_[j]);
            node_uses_mapper.erase(trees_[j]);
            instructions_mapper_.erase(trees_[j]);
            DEBUG_COUT << "merge finished" << std::endl;
            break;
          }
        }
      }
    }
#endif
  }


  void TreeGenerator::visit(Item* item) {
    item->accept(this);
  }

  void TreeGenerator::visit(Variable* var) {
    auto variable_node = new VariableNode(var->to_string());
    parsed_nodes.push(variable_node);
  }

  void TreeGenerator::visit(Number* num) {
    auto number_node = new NumberNode(num->to_string());
    parsed_nodes.push(number_node);
  }

  void TreeGenerator::visit(Operator* op) {
    auto op_node = new OperatorNode(op->to_string());
    parsed_nodes.push(op_node);
  }

  void TreeGenerator::visit(Label* label) {
    auto value_node = new ValueNode(label->to_string());
    parsed_nodes.push(value_node);
  }

  void TreeGenerator::visit(FunctionName* name) {
    auto value_node = new ValueNode(name->to_string());
    parsed_nodes.push(value_node);
  }

  void TreeGenerator::visit(Instruction* inst) {
    inst->accept(this);
  }

  void TreeGenerator::visit(AssignInst* inst) {
    DEBUG_COUT << "tree generating" << std::endl;
    auto l_value = (Variable*)inst->get_items()[0];
    auto r_value = inst->get_items()[1];
    l_value->accept(this);
    r_value->accept(this);

    auto op = new OperatorNode("<-");
    auto leaf = parsed_nodes.top();
    parsed_nodes.pop();
    auto root = parsed_nodes.top();
    parsed_nodes.pop();

    root->AddChild(op);
    op->AddChild(leaf);
    trees_.push_back(root);
  }

  void TreeGenerator::visit(StoreInst* inst) {
    DEBUG_COUT << "tree generating" << std::endl;
    auto l_value = (Variable*)inst->get_items()[0];
    auto r_value = inst->get_items()[1];
    l_value->accept(this);
    r_value->accept(this);

    auto op = new OperatorNode("store");
    auto leaf = parsed_nodes.top();
    parsed_nodes.pop();
    auto root = parsed_nodes.top();
    parsed_nodes.pop();

    op->AddChild(root);
    op->AddChild(leaf);
    trees_.push_back(op);
  }

  void TreeGenerator::visit(LoadInst* inst) {
    DEBUG_COUT << "tree generating" << std::endl;
    auto l_value = (Variable*)inst->get_items()[0];
    auto r_value = (Variable*)inst->get_items()[1];
    l_value->accept(this);
    r_value->accept(this);

    auto op = new OperatorNode("load");
    auto leaf = parsed_nodes.top();
    parsed_nodes.pop();
    auto root = parsed_nodes.top();
    parsed_nodes.pop();

    root->AddChild(op);
    op->AddChild(leaf);
    trees_.push_back(root);
  }

  void TreeGenerator::visit(BinaryOperator* inst) {
    DEBUG_COUT << "tree generating" << std::endl;
    auto l_value = (Variable*)inst->get_items()[0];
    auto op = (Operator*)inst->get_items()[2];
    auto t1 = inst->get_items()[1], t2 = inst->get_items()[3];

    t2->accept(this);
    t1->accept(this);
    l_value->accept(this);
    auto assign_op = new OperatorNode("<-");
    auto op_node = new OperatorNode(op->to_string());
    auto root = parsed_nodes.top();
    parsed_nodes.pop();
    root->AddChild(assign_op);
    auto t1_node = parsed_nodes.top();
    parsed_nodes.pop();
    auto t2_node = parsed_nodes.top();
    parsed_nodes.pop();
    DEBUG_COUT << t1_node->GetCode() << " " << op_node->GetCode() << " " << t2_node->GetCode() << std::endl;
    assign_op->AddChild(t1_node);
    assign_op->AddChild(op_node);
    assign_op->AddChild(t2_node);
    trees_.push_back(root);
  }

  void TreeGenerator::visit(CmpInst* inst) {
    DEBUG_COUT << "tree generating" << std::endl;
    auto l_value = (Variable*)inst->get_items()[0];
    auto op = (Operator*)inst->get_items()[2];
    auto t1 = inst->get_items()[1], t2 = inst->get_items()[3];

    t2->accept(this);
    t1->accept(this);
    l_value->accept(this);
    auto assign_op = new OperatorNode("<-");
    auto op_node = new OperatorNode(op->to_string());
    auto root = parsed_nodes.top();
    parsed_nodes.pop();
    root->AddChild(assign_op);
    auto t1_node = parsed_nodes.top();
    parsed_nodes.pop();
    auto t2_node = parsed_nodes.top();
    parsed_nodes.pop();
    assign_op->AddChild(t1_node);
    assign_op->AddChild(op_node);
    assign_op->AddChild(t2_node);
    trees_.push_back(root);
  }

  void TreeGenerator::visit(LabelInst* inst) {
    DEBUG_COUT << "tree generating" << inst->to_string() << std::endl;
    auto op_node = new OperatorNode("label");
    auto label = inst->get_items()[0];
    auto label_node = new ValueNode(label->to_string());
    op_node->AddChild(label_node);
    trees_.push_back(op_node);
  }

  void TreeGenerator::visit(ReturnInst* inst) {
    DEBUG_COUT << "tree generating" << std::endl;
    auto op_node = new OperatorNode("return");
    auto ret_value = inst->get_items()[0];
    if(ret_value == nullptr) {
      auto ret_node = new ValueNode("nullptr");
      op_node->AddChild(ret_node);
    } else {
      ret_value->accept(this);
      auto ret_node = parsed_nodes.top();
      parsed_nodes.pop();
      op_node->AddChild(ret_node);
    }
    trees_.push_back(op_node);
  }

  void TreeGenerator::visit(BranchInst* inst) {
    DEBUG_COUT << "tree generating" << std::endl;
    auto op_node = new OperatorNode("br");
    auto flag = inst->get_items()[0];
    auto label = inst->get_items()[1];
    if(flag == nullptr) {
      op_node->AddChild(new ValueNode("nullptr"));
      label->accept(this);
      auto label_node = parsed_nodes.top();
      parsed_nodes.pop();
      op_node->AddChild(label_node);
    } else {
      label->accept(this);
      flag->accept(this);
      auto flag_node = parsed_nodes.top();
      parsed_nodes.pop();
      auto label_node = parsed_nodes.top();
      parsed_nodes.pop();
      op_node->AddChild(flag_node);
      op_node->AddChild(label_node);
    }
    trees_.push_back(op_node);
  }

  void TreeGenerator::visit(CallInst* inst) {
    DEBUG_COUT << "tree generating" << std::endl;
    auto ret_value = inst->get_ret_value();
    Node* ret_node;
    if(ret_value == nullptr) {
      ret_node = new ValueNode("nullptr");
    } else {
      ret_value->accept(this);
      ret_node = parsed_nodes.top();
      parsed_nodes.pop();
    }
    auto op_node = new OperatorNode("call");
    ret_node->AddChild(op_node);
    auto callee = inst->get_callee();
    callee->accept(this);
    auto callee_node = parsed_nodes.top();
    parsed_nodes.pop();
    op_node->AddChild(callee_node);
    for(auto arg : inst->get_arguments()) {
      arg->accept(this);
      auto arg_node = parsed_nodes.top();
      parsed_nodes.pop();
      callee_node->AddChild(arg_node);
    }
    trees_.push_back(ret_node);
  }

  void TreeGenerator::visit(Function* f) {
    DEBUG_COUT << "check the code" << std::endl;
    abort();
  }

}