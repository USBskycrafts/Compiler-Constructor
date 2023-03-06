#pragma once

#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <map>
#include <boost/dynamic_bitset.hpp>


#include "L2.hpp"
#include "control_flow_graph.hpp"
#include "visitor.hpp"


namespace L2 {

  class LivenessAnalysis : Visitor {
    using Bitset = boost::dynamic_bitset<>;
    using Set = std::map<Instruction*, Bitset>;
    public:   
      LivenessAnalysis(Function* f);
      std::map<Instruction*, std::set<Value*>> GetInSet();
      std::map<Instruction*, std::set<Value*>> GetOutSet();
      std::map<Instruction*, std::set<Value*>> GetKillSet();
      std::map<Instruction*, std::set<Value*>> GetGenSet();
      

    private:    
      void Analysis(Function* f);
      Bitset GenerateBitSet(Value* item);
      void setInOutSet();
      std::vector<Value*> parsed_values;
      Set gen, kill, in, out;
      Function* f_;
      std::map<Value*, Bitset> value_mapper;
      const int value_counter;
      int encoded_value_counter = 0;
      ControlFlowGraph* cfg;

      void visit(Item* item) override;
      void visit(Value* value) override;
      void visit(Register* reg) override;
      void visit(Variable* var) override;
      void visit(MemoryAddress* address) override;
      void visit(Comparison* cmp) override;
      void visit(Number* num) override;
      void visit(Operator* op ) override;
      void visit(Label* label) override;
      void visit(FunctionName* name) override;

      void visit(Instruction* inst) override;
      void visit(InstructionArithmetic* inst) override;
      void visit(InstructionCall* inst) override;
      void visit(InstructionConditionalJump* inst) override;
      void visit(InstructionJump* inst) override;
      void visit(InstructionLeaq* inst) override;
      void visit(InstructionSelfOperation* inst) override;
      void visit(InstructionAssignment* inst) override;
      void visit(InstructionLabel* inst) override;
      void visit(InstructionReturn* inst) override;
      void visit(InstructionStackArg* inst) override;
      void visit(Function* f) override;
  };
}

