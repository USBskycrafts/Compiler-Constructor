
#include <unordered_map>
#include <vector>

#include "L3.hpp"

namespace LivenessAnalysis {

  using InstMapper = std::unordered_map<L3::Instruction*, std::vector<L3::Variable*>>;
  
  struct AnalysisResult {
    std::unordered_map<L3::Instruction*, std::vector<L3::Variable*>> in_set;
    std::unordered_map<L3::Instruction*, std::vector<L3::Variable*>> out_set;
  };

  AnalysisResult Analyze(L3::Function* function);

}