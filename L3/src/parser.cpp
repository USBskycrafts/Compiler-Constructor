#include "parser.hpp"

#include <cstddef>
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
#include <memory>
#include <stack>
#include <algorithm>
#include <functional>

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include "utils.hpp"

namespace L3 {

  namespace pegtl = tao::TAO_PEGTL_NAMESPACE;
  using namespace pegtl;

  std::map<Function*, LabelSet> labels;

  std::vector<Item*> parsed_items;

  struct comment : 
    pegtl::disable< 
      TAOCPP_PEGTL_STRING("//"), 
      pegtl::until<pegtl::eolf> 
    > {};

  struct seps : 
    pegtl::star< 
      pegtl::sor< 
        pegtl::ascii::space, 
        comment 
      > 
    > {};

  struct number :
    pegtl::seq<
      pegtl::opt<
        pegtl::sor<
          pegtl::one< '-' >,
          pegtl::one< '+' >
        >
      >,
      pegtl::plus< 
        pegtl::digit
      >
    > {};

  struct number_rule : number {};

  struct name :
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

  struct var_name :
    pegtl::seq<
      pegtl::one<'%'>,
      seps,
      name
    > {};

  struct var_rule : var_name {};

  struct function_name :
    pegtl::sor<
      pegtl::seq<
        pegtl::one<'@'>,
        seps,
        name
      >,
      TAOCPP_PEGTL_STRING("allocate"),
      TAOCPP_PEGTL_STRING("tensor-error"),
      TAOCPP_PEGTL_STRING("input"),
      TAOCPP_PEGTL_STRING("print")
    > {};

  struct function_name_rule : function_name {};

  struct label_name : 
    pegtl::seq<
      pegtl::one<':'>,
      seps,
      name
    > {};

  struct label_rule : label_name {};

  struct operator_name :
    pegtl::sor<
      TAOCPP_PEGTL_STRING("+"),
      TAOCPP_PEGTL_STRING("-"),
      TAOCPP_PEGTL_STRING("*"),
      TAOCPP_PEGTL_STRING("&"),
      TAOCPP_PEGTL_STRING("<<"),
      TAOCPP_PEGTL_STRING(">>"),
      TAOCPP_PEGTL_STRING(">="),
      TAOCPP_PEGTL_STRING("<="),
      TAOCPP_PEGTL_STRING("<"),
      TAOCPP_PEGTL_STRING(">"),
      TAOCPP_PEGTL_STRING("=")
    > {};

  struct operator_rule : operator_name {};

  struct instruction_assign_rule :
    pegtl::seq<
      var_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      pegtl::sor<
        var_rule,
        number_rule,
        label_rule,
        function_name_rule
      >
    > {};

  struct binary_operator_rule : 
    pegtl::seq<
      var_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      pegtl::sor<
        var_rule,
        number_rule
      >,
      seps,
      operator_rule,
      seps,
      pegtl::sor<
        var_rule,
        number_rule
      >
    > {};

  struct load_inst_rule :
    pegtl::seq<
      var_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      TAOCPP_PEGTL_STRING("load"),
      seps,
      var_rule
    > {};

  struct store_inst_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("store"),
      seps,
      var_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      pegtl::sor<
        var_rule,
        number_rule,
        label_rule,
        function_name_rule
      >
     > {};

  struct return_without_val :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("return")
    > {};

  struct return_with_val :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("return"),
      seps,
       pegtl::sor<
        var_rule,
        number_rule
      >
    > {};

  struct label_inst_rule :
    pegtl::seq<
      label_rule
    > {};

  struct goto_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("br"),
      seps,
      label_rule
    > {};

  struct conditional_jump_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("br"),
      seps,
      pegtl::sor<
        var_rule,
        number_rule
      >,
      seps,
      label_rule
    > {};

  struct args_rule :
    pegtl::seq<
      pegtl::sor<
        var_rule,
        number_rule
      >,
      pegtl::star<
        pegtl::seq<
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

  struct call_without_ret :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("call"),
      seps,
      function_name_rule,
      seps,
      pegtl::one<'('>,
      seps,
      pegtl::opt<
        args_rule
      >,
      seps,
      pegtl::one<')'>
    > {};
  
  struct call_with_ret :
    pegtl::seq<
      var_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      TAOCPP_PEGTL_STRING("call"),
      seps,
      pegtl::sor<
        function_name_rule,
        var_rule
      >,
      seps,
      pegtl::one<'('>,
      seps,
      pegtl::opt<
        args_rule
      >,
      seps,
      pegtl::one<')'>
    > {};

  struct instruction_rule :
    pegtl::sor<
      pegtl::seq<pegtl::at<binary_operator_rule>, binary_operator_rule>,
      pegtl::seq<pegtl::at<instruction_assign_rule>, instruction_assign_rule>,
      pegtl::seq<pegtl::at<load_inst_rule>, load_inst_rule>,
      pegtl::seq<pegtl::at<store_inst_rule>, store_inst_rule>,
      pegtl::seq<pegtl::at<return_with_val>, return_with_val>,
      pegtl::seq<pegtl::at<return_without_val>, return_without_val>,
      pegtl::seq<pegtl::at<label_inst_rule>, label_inst_rule>,
      pegtl::seq<pegtl::at<goto_rule>, goto_rule>,
      pegtl::seq<pegtl::at<conditional_jump_rule>, conditional_jump_rule>,
      pegtl::seq<pegtl::at<call_without_ret>, call_without_ret>,
      pegtl::seq<pegtl::at<call_with_ret>, call_with_ret>
    > {};

  struct instructions_rule :
    pegtl::plus<
      seps,
      instruction_rule,
      seps
    > {};

  struct function_define_rule :
    pegtl::seq<
      function_name
    > {};

  struct function_args_rule :
    pegtl::seq<
      var_rule,
      pegtl::star<
        pegtl::seq<
          seps,
          pegtl::one<','>,
          seps,
          var_rule
        >
      >
    > {};

  struct function_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("define"),
      seps,
      function_define_rule,
      seps,
      TAOCPP_PEGTL_STRING("("),
      seps,
      pegtl::opt<
        function_args_rule
      >,
      seps,
      TAOCPP_PEGTL_STRING(")"),
      seps,
      TAOCPP_PEGTL_STRING("{"),
      seps,
      instructions_rule,
      seps,
      TAOCPP_PEGTL_STRING("}")
    > {};

  struct functions_rule :
    pegtl::plus<
      pegtl::seq<
        seps,
        function_rule,
        seps
      >
    > {};
  

  struct grammar :
    pegtl::must<
      pegtl::seq<
        functions_rule
      >
    > {};

  template<typename Rule>
  struct action : pegtl::nothing<Rule> {};

  template<> struct action <var_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      parsed_items.push_back(new Variable(in.string()));
    }
  };

  template<> struct action <number_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      parsed_items.push_back(new Number(in.string()));
    }
  };

  template<> struct action <function_name_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      parsed_items.push_back(new FunctionName(in.string()));
    }
  };

  template<> struct action <label_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto label = new Label(in.string());
      parsed_items.push_back(label);
      labels[p.functions.back()].insert(label);
    }
  };

  template<> struct action <operator_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      parsed_items.push_back(new Operator(in.string()));
    }
  };

  template<> struct action <instruction_assign_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto s = parsed_items.back();
      parsed_items.pop_back();
      auto var = parsed_items.back();
      parsed_items.pop_back();
      auto inst = new AssignInst((Variable*)var, s);
      auto f = p.functions.back();
      f->instructions.emplace_back(inst);
    }
  };

  template<> struct action <binary_operator_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto t2 = parsed_items.back();
      parsed_items.pop_back();
      auto op = (Operator*)parsed_items.back();  
      parsed_items.pop_back();
      auto t1 =  parsed_items.back();
      parsed_items.pop_back();
      auto var = (Variable*)parsed_items.back();
      parsed_items.pop_back();
      auto inst = new BinaryOperator(var, t1, op, t2);
      auto f = p.functions.back();
      f->instructions.emplace_back(inst);
    }
  };

  template<> struct action <load_inst_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto rhs = (Variable*)parsed_items.back();
      parsed_items.pop_back();
      auto lhs = (Variable*)parsed_items.back();
      parsed_items.pop_back();
      auto inst = new LoadInst(lhs, rhs);
      auto f = p.functions.back();
      f->instructions.push_back(inst);
    }
  };

  template<> struct action <store_inst_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto rhs = parsed_items.back();
      parsed_items.pop_back();
      auto lhs = (Variable*)parsed_items.back();
      parsed_items.pop_back();
      auto inst = new StoreInst(lhs, rhs);
      auto f = p.functions.back();
      f->instructions.push_back(inst);
    }
  };

  template<> struct action <return_without_val> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto inst = new ReturnInst(nullptr);
      auto f = p.functions.back();
      f->instructions.push_back(inst);
    }
  };

  template<> struct action <return_with_val> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto ret_val = parsed_items.back();
      parsed_items.pop_back();
      auto inst = new ReturnInst(ret_val);
      auto f = p.functions.back();
      f->instructions.push_back(inst);
    }
  };

  template<> struct action <label_inst_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto label = (Label*)parsed_items.back();
      parsed_items.pop_back();
      auto inst = new LabelInst(label);
      auto f = p.functions.back();
      f->instructions.push_back(inst);
    }
  };

   template<> struct action <goto_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto label = (Label*)parsed_items.back();
      parsed_items.pop_back();
      auto inst = new BranchInst(nullptr, label);
      auto f = p.functions.back();
      f->instructions.push_back(inst);
    }
  };

   template<> struct action <conditional_jump_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto label = (Label*)parsed_items.back();
      parsed_items.pop_back();
      auto t = parsed_items.back();
      parsed_items.pop_back();
      auto inst = new BranchInst(t, label);
      auto f = p.functions.back();
      f->instructions.push_back(inst);
    }
  };

  template<> struct action <call_without_ret> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      Item* callee;
      std::vector<Item*> args;
      for(auto i = parsed_items.begin(); i != parsed_items.end(); ++i) {
        if(i == parsed_items.begin()) {
          callee = *i;
        } else {
          args.emplace_back(*i);
        }
      }
      parsed_items.clear();
      auto inst = new CallInst(nullptr, callee, args);
      auto f = p.functions.back();
      f->instructions.push_back(inst);
    }
  };

  template<> struct action <call_with_ret> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      Variable* lhs;
      Item* callee;
      std::vector<Item*> args;
      for(auto i = parsed_items.begin(); i != parsed_items.end(); ++i) {
        if(i == parsed_items.begin()) {
          lhs = (Variable*)(*i);
        } else if(i == parsed_items.begin() + 1) {
          callee = *i;
        } else {
          args.emplace_back(*i);
        }
      }
      parsed_items.clear();
      auto inst = new CallInst(lhs, callee, args);
      auto f = p.functions.back();
      f->instructions.push_back(inst);
    }
  };

  template<> struct action <function_define_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto f = new Function;
      f->name = new FunctionName(in.string());
      p.functions.push_back(f);
    }
  };

  template<> struct action <function_args_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto f = p.functions.back();
      std::vector<Item*> args;
      for(auto item : parsed_items) {
        args.emplace_back(item);
      }
      parsed_items.clear();
      f->args = args;
    }
  };

  

  

  Program parse_file(char* fileName) {
    pegtl::analyze<grammar>();

    /*
     * Parse.
     */
    file_input<> fileInput(fileName);
    Program p;
    parse<grammar, action>(fileInput, p);

    return p;
  }
}