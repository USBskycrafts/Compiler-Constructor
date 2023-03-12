#include "tiling.hpp"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <queue>
#include "liveness-analysis.hpp"
#include "utils.hpp"

namespace Tiling {

  LivenessAnalysis::AnalysisResult result;

  void PrintTree(Tree* tree) {
    DEBUG_COUT << "{\n";
    std::queue<Node*> nodes;
    nodes.push(tree->root);
    while(!nodes.empty()) {
      auto node = nodes.front();
      nodes.pop();
      log_open && std::cerr << "parent: " << node->code << " ";
      int counter = 0;
      for(auto child : node->children) {
        log_open && std::cerr << "child " << std::to_string(counter++) << ": " << child->code << " ";
        nodes.push(child);
      }
      log_open && std::cerr << "\n";
    }
    log_open && std::cerr << "}\n";
  }

  std::vector<Context> GenerateContext(L3::Function* function) {
    std::vector<Context> contexts;
    result = LivenessAnalysis::Analyze(function);
    auto context = Context();
    for(auto inst : function->instructions) {
      if(inst->type == "CallInst" || inst->type == "LabelInst") {
        contexts.push_back(context);
        context = Context();
      }
      context.instructions.emplace_back(inst);
      if(inst->type == "CallInst" || inst->type == "LabelInst" 
          || inst->type == "BranchInst" || inst->type == "ReturnInst") {
        contexts.push_back(context);
        context = Context();
      }
#ifdef DEBUG
      //DEBUG_COUT << "solving: " << inst->ToString();
#endif
      
    }
    auto iter = std::remove_if(contexts.begin(), contexts.end(), 
        [&](Context context){ return context.instructions.empty(); });
    contexts.erase(iter, contexts.end());
    return contexts;
  }

  std::vector<Tree*> GenerateTree(Context context) {
    std::vector<Tree*> trees;
    for(auto inst : context.instructions) {
      auto tree = GenerateTree(inst);
      trees.emplace_back(tree);
    }
    return trees;
  }

  Tree* GenerateTree(L3::Instruction* inst) {
    TreeGenerator generator;
#ifdef DEBUG
    DEBUG_COUT << "generate tree of" << inst->ToString();
#endif
    generator.visit(inst);
    return generator.tree;
  }

  std::vector<Node*> GetLeaves(Tree* tree) {
    auto root = tree->root;
    std::vector<Node*> leaves;
    std::queue<Node*> nodes;
    nodes.push(root);
    while(!nodes.empty()) {
      auto node = nodes.front();
      nodes.pop();
      if(node->children.empty()) {
        leaves.push_back(node);
      }
      for(auto child : node->children) {
        nodes.push(child);
      }
    }
    return leaves;
  }

  std::vector<Tree*> MergingTree(std::vector<Tree*> trees) {
#ifdef DEBUG
    DEBUG_COUT << "before merging\n";
    for(auto tree : trees) {
      PrintTree(tree);
    }
#endif
    for(auto i = trees.begin(); i != trees.end(); ++i) {
      for(auto j = i + 1; j != trees.end(); ++j) {
        if(*i == nullptr || *j == nullptr) {
          continue;
        }
        auto v = (*i)->root;
        try{
          // check if v is leaves of j
          auto leaves = GetLeaves(*j);
          for(auto n : leaves) {
            if(v->code == n->code) {
              goto success;
            }
          }
          continue;
success:
#ifdef DEBUG
          DEBUG_COUT << "probably able to merge\n";
#endif
          //check if v is dead
          for(auto n : (*j)->outs) {
            if(n->name == v->code) {
              throw "";
            }
          }
#ifdef DEBUG
          DEBUG_COUT << v->code << " is dead\n";
#endif
          //no other uses in [T2, T1)
          VariableSet var_set;
          for(auto k = i; k < j; ++k) {
            var_set.insert((*k)->uses.begin(), (*k)->uses.end());
          }
          for(auto n : var_set) {
            if(n->name == v->code) {
              throw "";
            }
          }
#ifdef DEBUG
          DEBUG_COUT << "no other uses in [T2, T1)\n";
#endif
          //no defs of variables used by T2 in [T2, T1)
          var_set.clear();
          for(auto k = i; k < j; ++k) {
            var_set.insert((*k)->defs.begin(), (*k)->defs.end());
          }
          for(auto n : (*i)->uses) {
            for(auto p : var_set) {
              if(n->name == p->name) {
                throw "";
              }
            }
          }
#ifdef DEBUG
          DEBUG_COUT << "no defs of variables used by T2 in [T2, T1)\n";
#endif
          //no load or store
          for(auto k = i; k < j; ++k) {
            if((*k)->root->code == "store") {
              throw "";
            }
            auto children = (*k)->root->children;
            for(auto child : children) {
              if(child->code == "load") {
                throw "";
              }
            }
          }
#ifdef DEBUG
          DEBUG_COUT << "able to merge\n";
#endif
          //then it's able to merge
          for(auto n : leaves) {
            if(v->code == n->code) {
              std::copy(v->children.begin(), v->children.end(), std::back_inserter(n->children));
              auto iter = std::find_if((*i)->defs.begin(), (*i)->defs.end(),
                  [&](L3::Variable* var){ return var->name == v->code; });
              (*i)->defs.erase(iter);
              (*j)->defs.insert((*i)->defs.begin(), (*i)->defs.end());
              (*j)->uses.insert((*i)->uses.begin(), (*i)->uses.end());
              (*j)->ins.insert((*i)->ins.begin(), (*i)->ins.end());
              (*j)->outs.insert((*i)->outs.begin(), (*i)->outs.end());
              delete *i;
              (*i) = nullptr;
              break;
            }
          }
        } catch(...) {
          continue;
        }
      }
    }
    trees.erase(std::remove_if(trees.begin(), trees.end(), 
        [&](Tree* tree){ return tree == nullptr; }), trees.end());
#ifdef DEBUG
    DEBUG_COUT << "after merging\n";
    for(auto tree : trees) {
      PrintTree(tree);
    }
#endif
    return trees;
  }

  void TreeGenerator::visit(L3::Item* item) {
    abort();
  }

  void TreeGenerator::visit(L3::Variable* var) {
    abort();
  }

  void TreeGenerator::visit(L3::Number* num) {
    abort();
  }

  void TreeGenerator::visit(L3::Operator* op) {
    abort();
  }

  void TreeGenerator::visit(L3::Label* label) {
    abort();
  }

  void TreeGenerator::visit(L3::FunctionName* name) {
    abort();
  }

  void TreeGenerator::visit(L3::Instruction* inst) {
    inst->accept(this);
  }

  void TreeGenerator::visit(L3::AssignInst* inst) {
    auto lhs = inst->lhs;
    auto rhs = inst->rhs;
    Node* l_node = new Node;
    l_node->code = lhs->name;
    Node* op = new Node;
    op->code = "<-";
    Node* r_node = new Node;
    r_node->code = rhs->name;
    l_node->children.push_back(op);
    op->children.push_back(r_node);
    tree = new Tree;
    tree->root = l_node;
    auto defs = inst->GetDefs(), uses = inst->GetUses();
    std::copy(defs.begin(), defs.end(), std::inserter(tree->defs, tree->defs.begin()));
    std::copy(uses.begin(), uses.end(), std::inserter(tree->uses, tree->uses.begin()));
    std::copy(result.in_set[inst].begin(), result.in_set[inst].end(), std::inserter(tree->ins, tree->ins.begin()));
    std::copy(result.out_set[inst].begin(), result.out_set[inst].end(), std::inserter(tree->outs, tree->outs.begin()));
  }

  void TreeGenerator::visit(L3::BinaryOperator* inst) {
    auto lhs = inst->lhs;
    auto rhs = inst->rhs;
    auto l_node = new Node;
    l_node->code = lhs->name;
    auto op = std::get<1>(rhs);
    auto op_node = new Node;
    op_node->code = op->name;
    auto t1 = std::get<0>(rhs), t2 = std::get<2>(rhs);
    auto t1_node = new Node;
    t1_node->code = t1->name;
    auto t2_node = new Node;
    t2_node->code = t2->name;
    l_node->children.push_back(op_node);
    op_node->children.push_back(t1_node);
    op_node->children.push_back(t2_node);
    tree = new Tree;
    tree->root = l_node;
    auto defs = inst->GetDefs(), uses = inst->GetUses();
    std::copy(defs.begin(), defs.end(), std::inserter(tree->defs, tree->defs.begin()));
    std::copy(uses.begin(), uses.end(), std::inserter(tree->uses, tree->uses.begin()));
    std::copy(result.in_set[inst].begin(), result.in_set[inst].end(), std::inserter(tree->ins, tree->ins.begin()));
    std::copy(result.out_set[inst].begin(), result.out_set[inst].end(), std::inserter(tree->outs, tree->outs.begin()));
  }

  void TreeGenerator::visit(L3::ReturnInst* inst) {
    auto op_node = new Node;
    op_node->code = "return";
    if(inst->ret_val) {
      auto ret_node = new Node;
      ret_node->code = inst->ret_val->name;
      op_node->children.push_back(ret_node);
    }
    tree = new Tree;
    tree->root = op_node;
    auto defs = inst->GetDefs(), uses = inst->GetUses();
    std::copy(defs.begin(), defs.end(), std::inserter(tree->defs, tree->defs.begin()));
    std::copy(uses.begin(), uses.end(), std::inserter(tree->uses, tree->uses.begin()));
    std::copy(result.in_set[inst].begin(), result.in_set[inst].end(), std::inserter(tree->ins, tree->ins.begin()));
    std::copy(result.out_set[inst].begin(), result.out_set[inst].end(), std::inserter(tree->outs, tree->outs.begin()));
  }

  void TreeGenerator::visit(L3::LabelInst* inst) {
    auto label_node = new Node;
    label_node->code = inst->label->name;
    tree = new Tree;
    tree->root = label_node;
    auto defs = inst->GetDefs(), uses = inst->GetUses();
    std::copy(defs.begin(), defs.end(), std::inserter(tree->defs, tree->defs.begin()));
    std::copy(uses.begin(), uses.end(), std::inserter(tree->uses, tree->uses.begin()));
    std::copy(result.in_set[inst].begin(), result.in_set[inst].end(), std::inserter(tree->ins, tree->ins.begin()));
    std::copy(result.out_set[inst].begin(), result.out_set[inst].end(), std::inserter(tree->outs, tree->outs.begin()));
  }

  void TreeGenerator::visit(L3::BranchInst* inst) {
    auto op_node = new Node;
    op_node->code = "br";
    auto label_node = new Node;
    label_node->code = inst->label->name;
    op_node->children.push_back(label_node);
    if(inst->t) {
      auto t_node = new Node;
      t_node->code = inst->t->name;
      op_node->children.push_back(t_node);
    }
    tree = new Tree;
    tree->root = op_node;
    auto defs = inst->GetDefs(), uses = inst->GetUses();
    std::copy(defs.begin(), defs.end(), std::inserter(tree->defs, tree->defs.begin()));
    std::copy(uses.begin(), uses.end(), std::inserter(tree->uses, tree->uses.begin()));
    std::copy(result.in_set[inst].begin(), result.in_set[inst].end(), std::inserter(tree->ins, tree->ins.begin()));
    std::copy(result.out_set[inst].begin(), result.out_set[inst].end(), std::inserter(tree->outs, tree->outs.begin()));
  }

  void TreeGenerator::visit(L3::LoadInst* inst) {
    auto lhs_node = new Node;
    lhs_node->code = inst->lhs->name;
    auto rhs_node = new Node;
    auto op_node = new Node;
    op_node->code = "load";
    rhs_node->code = inst->rhs->name;
    lhs_node->children.push_back(op_node);
    op_node->children.push_back(rhs_node);
    tree = new Tree;
    tree->root = lhs_node;
    auto defs = inst->GetDefs(), uses = inst->GetUses();
    std::copy(defs.begin(), defs.end(), std::inserter(tree->defs, tree->defs.begin()));
    std::copy(uses.begin(), uses.end(), std::inserter(tree->uses, tree->uses.begin()));
    std::copy(result.in_set[inst].begin(), result.in_set[inst].end(), std::inserter(tree->ins, tree->ins.begin()));
    std::copy(result.out_set[inst].begin(), result.out_set[inst].end(), std::inserter(tree->outs, tree->outs.begin()));
  }

  void TreeGenerator::visit(L3::StoreInst* inst) {
    auto op_node = new Node;
    op_node->code = "store";
    auto addr_node = new Node;
    addr_node->code = inst->lhs->name;
    auto rhs = new Node;
    rhs->code = inst->rhs->name;
    op_node->children.push_back(addr_node);
    op_node->children.push_back(rhs);
    tree = new Tree;
    tree->root = op_node;
    auto defs = inst->GetDefs(), uses = inst->GetUses();
    std::copy(defs.begin(), defs.end(), std::inserter(tree->defs, tree->defs.begin()));
    std::copy(uses.begin(), uses.end(), std::inserter(tree->uses, tree->uses.begin()));
    std::copy(result.in_set[inst].begin(), result.in_set[inst].end(), std::inserter(tree->ins, tree->ins.begin()));
    std::copy(result.out_set[inst].begin(), result.out_set[inst].end(), std::inserter(tree->outs, tree->outs.begin()));
  }

  void TreeGenerator::visit(L3::CallInst* inst) {
    tree = new Tree;
    Node* op_node;
    if(inst->lhs) {
      auto lhs_node = new Node;
      lhs_node->code = inst->lhs->name;
      tree->root = lhs_node;
      op_node = new Node;
      op_node->code = "call";
      tree->root = lhs_node;
      lhs_node->children.push_back(op_node);
    } else {
      op_node = new Node;
      op_node->code = "call";
      tree->root = op_node;
    }
    auto callee_node = new Node;
    callee_node->code = inst->callee->name;
    op_node->children.push_back(callee_node);
    for(auto arg : inst->args) {
      auto arg_node = new Node;
      arg_node->code = arg->name;
      callee_node->children.push_back(arg_node);
    }
    auto defs = inst->GetDefs(), uses = inst->GetUses();
    std::copy(defs.begin(), defs.end(), std::inserter(tree->defs, tree->defs.begin()));
    std::copy(uses.begin(), uses.end(), std::inserter(tree->uses, tree->uses.begin()));
    std::copy(result.in_set[inst].begin(), result.in_set[inst].end(), std::inserter(tree->ins, tree->ins.begin()));
    std::copy(result.out_set[inst].begin(), result.out_set[inst].end(), std::inserter(tree->outs, tree->outs.begin()));
  }

}