#include "tile.hpp"

#include <map>
#include <queue>
#include "liveness_analysis.hpp"

namespace Tiling {
  std::map<L3::Function*, std::vector<Context*>> contexts;
  std::map<std::string, Tile*> tile_mapper;

  void GenerateContext(L3::Function* function) {
    auto context = new Context;
    context->function = function;
    contexts[function].push_back(context);
    for(auto inst : function->instructions) {  
      if(typeid(*inst) == typeid(L3::Label) ||
          typeid(*inst) == typeid(L3::BranchInst) ||
          typeid(*inst) == typeid(L3::CallInst) ||
          typeid(*inst) == typeid(L3::ReturnInst)) {
        contexts[function].push_back(context);
        context = new Context;
        context->function = function;
        context->tree_mapper[inst.get()] = {};
        contexts[function].push_back(context);
        context = new Context;
        context->function = function;
      }
      context->tree_mapper[inst.get()] = {};
    }
    auto is_empty = [&](Context* context) { return context->tree_mapper.empty(); };
    context->tree_mapper.erase(std::remove_if(context->tree_mapper.begin(), 
        context->tree_mapper.end(), is_empty), context->tree_mapper.end());
  }

  std::vector<Node*> GetAllNode(Node* root) {
    std::vector<Node*> nodes;
    if(root->GetChildren().empty()) {
      return {};
    }
    for(auto node : root->GetChildren()) {
      nodes.emplace_back(GetAllNode(node));
    }
    return nodes;
  }

  void MergingTree(Context* context) {
    auto result_mapper = LivenessAnalysis::GenerateInAndOut(context->function);
    for(auto i = context->trees.begin(); i != context->trees.end(); ++i) {
      for(auto j = i + 1; j != context->trees.end(); ++j) {
        if(*i == nullptr || *j == nullptr) {
          continue;
        }
        auto v = (*i)->GetRoot();
        auto leaves = (*j)->GetLeaves();
        bool equal = false;
        //check if v equals to one leaf
        for(auto leaf : leaves) {
          if(v->GetCode() == leaf->GetCode()) {
            equal = true;
            break;
          }
        }
        if(!equal) {
          continue;
        }
        //checking if able to merge
        try {
          //if v is dead after *j
          for(auto [inst, tree] : context->tree_mapper) {
            if(tree == (*j)) {
              //get inst attached to tree i
              auto result = result_mapper[inst];
              auto out = result->outs;
              auto is_equal = [&](L3::Variable var) { var.GetCode() == v->GetCode(); };
              auto iter = std::find_if(out.begin(), out.end(), is_equal);
              if(iter != out.end()) {
                throw "";
              }
            }
          }
          //no other uses of %v in [*i, *j)
          //get all instructions in [*i, *j)
          std::vector<L3::Instruction*> instructions;
          for(auto k = i; k != j; ++k) {
            for(auto [inst, tree] : context->tree_mapper) {
              if(tree == *k) {
                instructions.push_back(inst);
              }
            }
          }
          //get all uses
          std::vector<L3::Variable> vars;
          for(auto inst : instructions) {
            vars.emplace_back(inst->GetUses());
          }
          for(auto var : vars) {
            if(var.GetCode() == v->GetCode()) {
              throw "";
            }
          }
          //no defs of vars used by *i
          std::vector<Node*> roots;
          for(auto inst : instructions) {
            roots.emplace_back(context->tree_mapper[inst]->GetRoot());
          }
          auto nodes = GetAllNode((*i)->GetRoot());
          for(auto root : roots) {
            for(auto node : nodes) {
              if(root->GetCode() == node->GetCode()) {
                throw "";
              }
            }
          }
          for(auto inst : instructions) {
            if(typeid(*inst) == typeid(L3::LoadInst) || typeid(*inst) == typeid(L3::StoreInst)) {
              throw "";
            }
          }
        } catch (...) {
          continue;
        }
        //merging
        for(auto leaf : leaves) {
          if(v->GetCode() == leaf->GetCode()) {
            leaf->InsertChildren(v);
            return;
          }
        }
      }
    }
  }

  std::vector<Node*> Tree::GetLeaves() {
    std::queue<Node*> nodes;
    std::vector<Node*> children;
    nodes.push(root_.get());
    while(!nodes.empty()) {
      auto node = nodes.front();
      nodes.pop();
      if(node->GetChildren().empty()) {
        children.push_back(node);
      }
      for(auto child : node->GetChildren()) {
        nodes.push(child);
      }
    }
  }

  void MatchesTile(Node* root, Tree* tree) {
    if(root == nullptr) {
      return;
    }
    for(auto child : root->GetChildren()) {
      MatchesTile(child, tree);
    }
    for(auto [name, tile] : tile_mapper) {
      auto leaves = tile->Matches(root);
      if(leaves.empty()) {
        continue;
      }
      std::vector<std::string> codes;
      for(auto leave : leaves) {
        auto code = tree->GetNodeCode(leave);
        codes.emplace_back(code);
      }
      auto cur_code = tree->GetNodeCode(root);
      if(codes.size() + tile->GenerateCode(root).size() < cur_code.size()) {
        tree->SetGeneratedCode(root, tile->GenerateCode(root));
      }
    }
  }

  void MatchesTile(Tree* tree) {
    MatchesTile(tree->GetRoot(), tree);
  }

  void MatchesTile(Context* context) {
    for(auto tree : context->trees) {
      MatchesTile(tree);
      context->generated_codes[tree] = tree->GetRootCode();
    }
  }
}