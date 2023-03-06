#include <sched.h>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <assert.h>
#include <stack>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include "L3.hpp"
#include "parser.hpp"
#include "tao/pegtl/internal/pegtl_string.hpp"
#include "utils.hpp"

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;
using namespace pegtl;

namespace L3 {

  std::stack<Item*> parsed_items;

  struct name:
    pegtl::seq<
      pegtl::plus< 
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >
        >
      >,
      pegtl::star<
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >,
          pegtl::digit
        >
      >
    > {};

  struct var : 
    pegtl::seq<
      pegtl::one<'%'>,
      name
    > {};

  struct var_rule : var {};

  struct label :
    pegtl::seq<
      pegtl::one<':'>,
      name
    > {};

  struct label_rule : label {};

  struct function_name : 
    pegtl::seq<
      pegtl::one<'@'>,
      name
    > {};

  struct function_name_rule : 
    pegtl::sor<
      TAO_PEGTL_STRING("print"),
      TAO_PEGTL_STRING("allocate"),
      TAO_PEGTL_STRING("input"),
      TAO_PEGTL_STRING("tensor-error"),
      function_name
    > {};

  struct function_name_declare : function_name {};

  struct op :
    pegtl::sor<
      TAO_PEGTL_STRING("+"),
      TAO_PEGTL_STRING("-"),
      TAO_PEGTL_STRING("*"),
      TAO_PEGTL_STRING("&"),
      TAO_PEGTL_STRING("<<"),
      TAO_PEGTL_STRING(">>")
    > {};
  
  struct op_rule : op {};

  struct cmp :
    pegtl::sor<
      TAO_PEGTL_STRING("<="),
      TAO_PEGTL_STRING("<"), 
      TAO_PEGTL_STRING("="),
      TAO_PEGTL_STRING(">="),
      TAO_PEGTL_STRING(">")
    > {};

  struct cmp_rule : cmp {};

  struct number :
    pegtl::seq<
      pegtl::opt<
        pegtl::sor<
          pegtl::one<'-'>,
          pegtl::one<'+'>
        >
      >,
      pegtl::plus< 
        pegtl::digit
      >
    > {};

  struct number_rule : number {};

  struct comment: 
  pegtl::disable< 
    TAOCPP_PEGTL_STRING("//"), 
    pegtl::until<pegtl::eolf> 
  > {};

  struct seps: 
    pegtl::star< 
      pegtl::sor< 
        pegtl::ascii::space, 
        comment 
      > 
    > {};

  struct assign_inst_rule : 
    pegtl::seq<
      var_rule,
      seps,
      TAO_PEGTL_STRING("<-"),
      seps,
      pegtl::sor<
        var_rule,
        number_rule,
        label_rule,
        function_name_rule
      >
    > {};

  struct store_inst_rule :
    pegtl::seq<
      TAO_PEGTL_STRING("store"),
      seps,
      var_rule,
      seps,
      TAO_PEGTL_STRING("<-"),
      seps,
      pegtl::sor<
        var_rule,
        number_rule,
        label_rule,
        function_name_rule
      >
    > {};

  struct load_inst_rule : 
    pegtl::seq<
      var_rule,
      seps,
      TAO_PEGTL_STRING("<-"),
      seps,
      TAO_PEGTL_STRING("load"),
      seps,
      var_rule
    > {};

  struct binary_op_rule :
    pegtl::seq<
      var_rule,
      seps,
      TAO_PEGTL_STRING("<-"),
      seps,
      pegtl::sor<
        var_rule,
        number_rule
      >,
      seps,
      op_rule,
      seps,
      pegtl::sor<
        var_rule,
        number_rule
      >
    > {};

  struct cmp_inst_rule :
    pegtl::seq<
      var_rule,
      seps,
      TAO_PEGTL_STRING("<-"),
      seps,
      pegtl::sor<
        var_rule,
        number_rule
      >,
      seps,
      cmp_rule,
      seps,
      pegtl::sor<
        var_rule,
        number_rule
      >
    > {};

  struct return_inst_rule : 
    pegtl::seq<
      TAO_PEGTL_STRING("return"),
      pegtl::opt<
        pegtl::seq<
          seps,
          pegtl::sor<
            var_rule,
            number_rule
          >
        >
      >
    > {};

  struct label_inst_rule : 
    pegtl::seq<
      label_rule
    > {};

  struct branch_inst_rule : 
    pegtl::seq<
      TAO_PEGTL_STRING("br"),
      seps,
      pegtl::opt<
        pegtl::sor<
          var_rule,
          number_rule
        >
      >,
      seps,
      label_rule
    > {};

  struct call_key_word_rule : TAO_PEGTL_STRING("call") {};

  struct args_rule : 
    pegtl::opt<
      pegtl::seq<
        pegtl::sor<
          var_rule,
          number_rule
        >,
        pegtl::star<
          seps,
          pegtl::one<','>,
          seps,
          pegtl::sor<
            var_rule,
            number_rule
          >
        >
      >
    > {};

  struct callee_rule : 
    pegtl::sor<
      var_rule,
      function_name_rule
    > {};

  struct call_inst_rule :
    pegtl::seq<
      pegtl::opt<
        pegtl::seq<
          var_rule,
          seps,
          TAO_PEGTL_STRING("<-")
        >
      >,
      seps,
      call_key_word_rule,
      seps,
      callee_rule,
      seps,
      pegtl::one<'('>,
      seps,
      args_rule,
      seps,
      pegtl::one<')'>
    > {};

  struct instruction_rule : 
    pegtl::sor<
      pegtl::seq<pegtl::at<binary_op_rule>, binary_op_rule>,
      pegtl::seq<pegtl::at<cmp_inst_rule>, cmp_inst_rule>,
      pegtl::seq<pegtl::at<call_inst_rule>, call_inst_rule>,
      pegtl::seq<pegtl::at<load_inst_rule>, load_inst_rule>,
      pegtl::seq<pegtl::at<store_inst_rule>, store_inst_rule>,
      pegtl::seq<pegtl::at<assign_inst_rule>, assign_inst_rule>,
      pegtl::seq<pegtl::at<return_inst_rule>, return_inst_rule>,
      pegtl::seq<pegtl::at<label_inst_rule>, label_inst_rule>,
      pegtl::seq<pegtl::at<branch_inst_rule>, branch_inst_rule>
    > {};

  struct instructions_rule : 
    pegtl::plus<
      pegtl::seq<
        seps,
        instruction_rule,
        seps
      >
    > {};

  struct vars_rule :
    pegtl::opt<
      pegtl::seq<
        var_rule,
        seps,
        pegtl::star<
          pegtl::seq<
            seps,
            pegtl::one<','>,
            seps,
            var_rule,
            seps
          >
        >
      >
    > {};

  struct function_rule : 
    pegtl::seq<
      TAO_PEGTL_STRING("define"),
      seps,
      function_name_declare,
      seps,
      pegtl::one<'('>,
      seps,
      vars_rule,
      seps,
      pegtl::one<')'>,
      seps,
      pegtl::one<'{'>,
      seps,
      instructions_rule,
      seps,
      pegtl::one<'}'>
    > {};

  struct functions_rule : 
    pegtl::plus <
      pegtl::seq<
        seps,
        function_rule,
        seps
      >
    > {};

  struct program_rule : functions_rule {};

  struct program_grammar :
    pegtl::must<
      program_rule
    > {};

  template<typename Rule>
  struct action : pegtl::nothing<Rule> {};


  /**
   * @brief debug rules
   * 
   */

  //  template<> struct action<instructions_rule> {
  //     template<typename Input>
  //     static void apply(const Input& in, Program& p) {
  //       DEBUG_COUT << "hello?" << std::endl;
  //       DEBUG_COUT << in.string() << std::endl;
  //     }
  //  };



  /**
   * @brief item rules
   * 
   * @param  
   */

  template<> struct action<var_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      parsed_items.push(Variable::get_variable(in.string()));
    }
  };

  template<> struct action<label_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto f = p.get_functions().back();
      auto suffix = f->get_name();
      suffix[0] = '_';
      parsed_items.push(Label::get_label(in.string() + suffix));
    }
  };

  template<> struct action<function_name_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      parsed_items.push(FunctionName::get_name(in.string()));
    }
  };

  template<> struct action<function_name_declare> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      auto f = new Function;
      f->set_name(in.string());
      p.set_function(f);
    }
  };

  template<> struct action<op_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      parsed_items.push(Operator::get_operator(in.string()));
    }
  };

  template<> struct action<cmp_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      parsed_items.push(Operator::get_operator(in.string()));
    }
  };

  template<> struct action<number_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      parsed_items.push(Number::get_number(in.string()));
    }
  };

  /**
   * @brief instruction rules
   * 
   */

  template<> struct action<assign_inst_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      auto f = p.get_functions().back();
      auto r_value = parsed_items.top();
      parsed_items.pop();
      auto l_value = (Variable*)parsed_items.top();
      parsed_items.pop();
      auto inst = new AssignInst(l_value, r_value);
      f->add_instruction(inst);
    }
  };

  template<> struct action<store_inst_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      auto f = p.get_functions().back();
      auto r_value = parsed_items.top();
      parsed_items.pop();
      auto l_value = (Variable*)parsed_items.top();
      parsed_items.pop();
      auto inst = new StoreInst(l_value, r_value);
      f->add_instruction(inst);
    }
  };

  template<> struct action<load_inst_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      auto f = p.get_functions().back();
      auto r_value = (Variable*)parsed_items.top();
      parsed_items.pop();
      auto l_value = (Variable*)parsed_items.top();
      parsed_items.pop();
      auto inst = new LoadInst(l_value, r_value);
      f->add_instruction(inst);
    }
  };

  template<> struct action<binary_op_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      auto f = p.get_functions().back();
      auto i2 = (Variable*)parsed_items.top();
      parsed_items.pop();
      auto op =(Operator*)parsed_items.top();
      parsed_items.pop();
      auto i1 = (Variable*)parsed_items.top();
      parsed_items.pop();
      auto l_value = (Variable*)parsed_items.top();
      parsed_items.pop();
      auto inst = new BinaryOperator(l_value, i1, op, i2);
      f->add_instruction(inst);
    }
  };

  template<> struct action<cmp_inst_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      auto f = p.get_functions().back();
      auto i2 = (Variable*)parsed_items.top();
      parsed_items.pop();
      auto op =(Operator*)parsed_items.top();
      parsed_items.pop();
      auto i1 = (Variable*)parsed_items.top();
      parsed_items.pop();
      auto l_value = (Variable*)parsed_items.top();
      parsed_items.pop();
      auto inst = new CmpInst(l_value, i1, op, i2);
      f->add_instruction(inst);
    }
  };

  template<> struct action<return_inst_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      auto f = p.get_functions().back();
      if(parsed_items.size() == 1) {
        auto variable = (Variable*) parsed_items.top();
        parsed_items.pop();
        auto inst = new ReturnInst(variable);
        f->add_instruction(inst);
      } else if(parsed_items.size() == 0) {
        auto inst = new ReturnInst(nullptr);
        f->add_instruction(inst);
      } else {
        DEBUG_COUT << "out of stack, check the code" << std::endl;
        abort();
      }
    }
  };

  template<> struct action<label_inst_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      auto f = p.get_functions().back();
      auto label = (Label*)parsed_items.top();
      parsed_items.pop();
      auto inst = new LabelInst(label);
      f->add_instruction(inst);
    }
  };

  template<> struct action<branch_inst_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      auto f = p.get_functions().back();
      if(parsed_items.size() == 2) {
        auto label = (Label*)parsed_items.top();
        parsed_items.pop();
        auto flag = parsed_items.top();
        parsed_items.pop();
        auto inst = new BranchInst(flag, label);
        f->add_instruction(inst);
      } else if(parsed_items.size() == 1) {
        auto label = (Label*)parsed_items.top();
        parsed_items.pop();
        auto inst = new BranchInst(nullptr, label);
        f->add_instruction(inst);
      } else {
        DEBUG_COUT << "out of stack, check the code" << std::endl;
        abort();
      }
    }
  };

  template<> struct action<call_key_word_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      // DEBUG_COUT << "ciao!" << std::endl;
      Variable* variable = nullptr;
      if(!parsed_items.empty()) {
        variable = (Variable*)parsed_items.top();
        parsed_items.pop();
      }
      CallInst::parsed_ret_value = variable;
    }
  };

  template<> struct action<callee_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      auto callee = parsed_items.top();
      parsed_items.pop();
      CallInst::parsed_callee = callee;
    }
  };

  template<> struct action<args_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      while(!parsed_items.empty()) {
        auto arg = parsed_items.top();
        parsed_items.pop();
        CallInst::parsed_args.push_back(arg);
      }
      std::reverse(CallInst::parsed_args.begin(),
                  CallInst::parsed_args.end());
    }
  };

  template<> struct action<call_inst_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      auto f = p.get_functions().back();
      auto callee = CallInst::parsed_callee;
      auto args = CallInst::parsed_args;
      auto ret_value = CallInst::parsed_ret_value;
      auto inst = new CallInst(ret_value, callee, args);
      f->add_instruction(inst);
      CallInst::parsed_callee = nullptr;
      CallInst::parsed_args.clear();
      CallInst::parsed_ret_value = nullptr;
    }
  };

  template<> struct action<vars_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      auto f = p.get_functions().back();
      std::vector<Variable*> args;
      while(!parsed_items.empty()) {
        auto vars = (Variable*)parsed_items.top();
        parsed_items.pop();
        args.push_back(vars);
      }
      std::reverse(args.begin(), args.end());
      for(auto arg : args) {
        f->add_argument(arg);
      }
    }
  };

  template<> struct action<function_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      auto f = p.get_functions().back();
      f->add_local();
    }
  };


  Program parse_file(char *fileName) {
    pegtl::analyze<program_rule>();
    file_input<> fileInput(fileName);
    Program p;
    parse<program_grammar, action>(fileInput, p);
    return p;
  }

  
}