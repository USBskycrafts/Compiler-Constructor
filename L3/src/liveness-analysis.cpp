#include "liveness-analysis.hpp"

#include <unordered_map>
#include <set>
#include <ostream>
#include <algorithm>
#include <queue>
#include "utils.hpp"

namespace LivenessAnalysis {

  std::unordered_map<L3::Instruction*, std::vector<L3::Instruction*>> predecessors, successors;

  struct cmp {
    bool operator()(const L3::Variable* v1, const L3::Variable* v2) const {
      return v1->name < v2->name;
    }
  };

  std::unordered_map<L3::Instruction*, std::set<L3::Variable*, cmp>> in_set;
  std::unordered_map<L3::Instruction*, std::set<L3::Variable*, cmp>> out_set;
  std::unordered_map<L3::Instruction*, std::set<L3::Variable*, cmp>> gen_set;
  std::unordered_map<L3::Instruction*, std::set<L3::Variable*, cmp>> kill_set;

  std::ostream& operator<<(std::ostream& out, std::set<L3::Variable*, cmp> s) {
    out << "\n{";
    for(auto i = s.begin(); i != s.end(); i++) {
      out << (i == s.begin() ? "" : ", ") << (*i)->name;
    }
    out << "}\n";
    return out;
  }

  std::ostream& operator<<(std::ostream& out, std::vector<L3::Variable*> v) {
    out << "\n{";
    for(auto i = v.begin(); i != v.end(); i++) {
      out << (i == v.begin() ? "" : ", ") << (*i)->name;
    }
    out << "}\n";
    return out;
  }

  void Init() {
    predecessors.clear();
    successors.clear();
    in_set.clear();
    out_set.clear();
    gen_set.clear();
    kill_set.clear();
  }

  void GenerateControlFlow(L3::Function* function) {
    auto instructions = function->instructions;
    for(auto i = instructions.begin(); i < instructions.end() - 1; ++i) {
      if((*i)->type == "BranchInst") {
        auto inst = (L3::BranchInst*)(*i);
        if(inst->t != nullptr) {
          successors[inst].emplace_back(*(i + 1));
        }
        for(auto l : instructions) {
          if(l->type == "LabelInst") {
            auto label = (L3::LabelInst*)l;
            if(label->label->name == inst->label->name) {
              successors[inst].emplace_back(l);
              break;
            }
          }
        }
      } else if((*i)->type == "CallInst") {
        auto inst = (L3::CallInst*)(*i);
        if(inst->callee->name == "tensor-error") {
          successors[inst] = {};
        } else {
          successors[inst].emplace_back(*(i + 1));
        }
      } else if((*i)->type == "ReturnInst") {
        successors[(L3::ReturnInst*)(*i)] = {};
      } else {
        successors[(*i)].emplace_back(*(i + 1));
      }
    }
    for(auto i = instructions.begin(); i != instructions.end(); ++i) {
      auto suc = successors[*i];
      for(auto inst : suc) {
        predecessors[inst].emplace_back(*i);
      }
    }
#ifdef DEBUG
    for(auto inst : instructions) {
      DEBUG_COUT << "current: " << inst->ToString();
      DEBUG_COUT << "successors are: \n";
      for(auto suc : successors[inst]) {
        DEBUG_COUT << suc->ToString();
      }
    }
#endif
  }

  void GenerateInAndOut(L3::Function* function) {
    std::queue<L3::Instruction*> work_list;
    auto instructions = function->instructions;
    for(auto i = instructions.rbegin(); i != instructions.rend(); ++i) {
      work_list.push(*i);
    }
#ifdef DEBUG
    static unsigned long long counter = 0;
#endif
    while (!work_list.empty()) {
      auto inst = work_list.front();
      work_list.pop();
      std::set<L3::Variable*, cmp> in, out;
#ifdef DEBUG
      DEBUG_COUT << inst->ToString() << in << " " << out << std::endl;
      ++counter;
#endif
      for(auto suc : successors[inst]) {
        out.insert(in_set[suc].begin(), in_set[suc].end());
      }
      in.insert(out.begin(), out.end());
      for(auto var : inst->GetDefs()) {
        in.erase(var);
      }
      for(auto var : inst->GetUses()) {
        in.insert(var);
      }
      if(in.size() != in_set[inst].size() or out.size() != out_set[inst].size()) {
        in_set[inst] = in;
        out_set[inst] = out;
        for(auto pre : predecessors[inst]) {
          work_list.push(pre);
        }
      }
    }
#ifdef DEBUG
    for(auto inst : function->instructions) {
      DEBUG_COUT << inst->ToString() << in_set[inst] << out_set[inst];
    }
    DEBUG_COUT << counter << " times\n";
#endif
  }

  AnalysisResult Analyze(L3::Function* function) {
    Init();
    AnalysisResult result;
    GenerateControlFlow(function);
    GenerateInAndOut(function);
    for(auto [inst, set] : in_set) {
      std::copy(set.begin(), set.end(), std::back_inserter(result.in_set[inst]));
    }
    for(auto [inst, set] : out_set) {
      std::copy(set.begin(), set.end(), std::back_inserter(result.out_set[inst]));
    }
    return result;
  }

}