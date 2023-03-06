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

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>
#include "parser.hpp"
#include "utils.hpp"
#include "LB.hpp"
#include "tao/pegtl/ascii.hpp"
#include "tao/pegtl/nothing.hpp"
#include "tao/pegtl/rules.hpp"




namespace LB {

  namespace pegtl = tao::TAO_PEGTL_NAMESPACE;
  using namespace pegtl;


  std::vector<Item*> parsed_items;
  std::vector<std::string> declared_vars;
  std::vector<Scope*> parsed_scopes;
  std::vector<WhileStatement*> parsed_loops;
  std::map<WhileStatement*, std::string> begin_while, end_while, cond_labels;


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

  struct label_rule :
    pegtl::seq<
      pegtl::one<':'>,
      name
    > {};

  struct name_declare_rule : name {};

  struct name_use_rule : name {};

  struct function_name_rule : name {};

  struct names_rule :
    pegtl::seq<
      name_declare_rule,
      pegtl::star<
        seps,
        pegtl::one<','>,
        seps,
        name_declare_rule
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
    >{};

  struct number_rule : number {};

  struct args_rule :
    pegtl::seq<
      pegtl::sor<
        number_rule,
        name_use_rule
      >,
      pegtl::star<
        seps,
        pegtl::one<','>,
        seps,
        pegtl::sor<
          number_rule,
          name_use_rule
        >
      > 
    > {};

  struct int64_declare_rule : 
    pegtl::seq<
      TAOCPP_PEGTL_STRING("int64"),
      seps,
      names_rule
    > {};

  struct code_declare_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("code"),
      seps,
      names_rule
    > {};

  struct tuple_declare_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("tuple"),
      seps,
      names_rule
    > {};

  struct square_bracket_rule :
    pegtl::seq<
      pegtl::one<'['>,
      seps,
      pegtl::one<']'>
    > {
      static int pair_number;
    };
  
  int square_bracket_rule::pair_number = 0;

  struct array_declare_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("int64"),
      seps,
      pegtl::plus<
        square_bracket_rule
      >,
      seps,
      names_rule
    > {};



  struct op :
    pegtl::sor<
      TAOCPP_PEGTL_STRING("+"),
      TAOCPP_PEGTL_STRING("-"),
      TAOCPP_PEGTL_STRING("*"),
      TAOCPP_PEGTL_STRING("&"),
      TAOCPP_PEGTL_STRING("<<"),
      TAOCPP_PEGTL_STRING(">>"),
      TAOCPP_PEGTL_STRING("<="),
      TAOCPP_PEGTL_STRING("<"),
      TAOCPP_PEGTL_STRING("="),
      TAOCPP_PEGTL_STRING(">="),
      TAOCPP_PEGTL_STRING(">")
    > {};

  struct op_rule : op {};


  struct scope_rule;

  struct assign_inst_rule :
    pegtl::seq<
      name_use_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      pegtl::sor<
        number_rule,
        name_use_rule
      >
    > {};

  struct binary_op_rule :
    pegtl::seq<
      name_use_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      pegtl::sor<
        number_rule,
        name_use_rule
      >,
      seps,
      op_rule,
      seps,
      pegtl::sor<
        number_rule,
        name_use_rule
      >
    > {};
  
  struct label_inst_rule :
    pegtl::seq<
      label_rule
    > {};

  struct if_state_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("if"),
      seps,
      pegtl::one<'('>,
      seps,
      pegtl::sor<
        number_rule,
        name_use_rule
      >,
      seps,
      pegtl::sor<
        number_rule,
        name_use_rule
      >,
      seps,
      pegtl::one<')'>,
      seps,
      label_rule,
      seps,
      label_rule
    > {};

  struct goto_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("goto"),
      seps,
      label_rule
    > {};

  struct return_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("return"),
      seps,
      pegtl::opt<
        pegtl::sor<
          number_rule,
          name_use_rule
        > 
      >
    > {};
  
  struct while_state_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("while"),
      seps,
      pegtl::one<'('>,
      seps,
      pegtl::sor<
        number_rule,
        name_use_rule
      >,
      seps,
      pegtl::sor<
        number_rule,
        name_use_rule
      >,
      seps,
      pegtl::one<')'>,
      seps,
      label_rule,
      seps,
      label_rule
    > {};

  struct continue_state_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("continue")
    > {};

  struct break_state_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("break")
    > {};

  struct load_container_rule :
    pegtl::seq<
      name_use_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      name_use_rule,
      pegtl::plus<
        pegtl::seq<
          seps,
          pegtl::one<'['>,
          seps,
          pegtl::sor<
            number_rule,
            name_use_rule
          >,
          seps,
          pegtl::one<']'>,
          seps
        >
      >
    > {};

  struct store_container_rule :
    pegtl::seq<
      name_use_rule,
      pegtl::plus<
        pegtl::seq<
          seps,
          pegtl::one<'['>,
          seps,
          pegtl::sor<
            number_rule,
            name_use_rule
          >,
          seps,
          pegtl::one<']'>,
          seps
        >
      >,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      pegtl::sor<
        number_rule,
        name_use_rule
      >
    > {};

  struct get_length_rule :
    pegtl::seq<
      name_use_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      TAOCPP_PEGTL_STRING("length"),
      seps,
      name_use_rule,
      seps,
      pegtl::sor<
        number_rule,
        name_use_rule
      >
    > {};

  struct call_without_ret :
    pegtl::seq<
      name_use_rule,
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
      name_use_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      name_use_rule,
      seps,
      pegtl::one<'('>,
      seps,
      pegtl::opt<
        args_rule
      >,
      seps,
      pegtl::one<')'>
    > {};

  struct array_allocate_rule :  
    pegtl::seq<
      name_use_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      TAOCPP_PEGTL_STRING("new"),
      seps,
      TAOCPP_PEGTL_STRING("Array"),
      seps,
      pegtl::one<'('>,
      seps,
      args_rule,
      seps,
      pegtl::one<')'>
    > {};

    struct tuple_allocate_rule :  
      pegtl::seq<
        name_use_rule,
        seps,
        TAOCPP_PEGTL_STRING("<-"),
        seps,
        TAOCPP_PEGTL_STRING("new"),
        seps,
        TAOCPP_PEGTL_STRING("Tuple"),
        seps,
        pegtl::one<'('>,
        seps,
        pegtl::sor<
          number_rule,
          name_use_rule
        >,
        seps,
        pegtl::one<')'>
      > {};

  struct instruction_rule :
    pegtl::sor<
      pegtl::seq<pegtl::at<binary_op_rule>, binary_op_rule>,
      pegtl::seq<pegtl::at<load_container_rule>, load_container_rule>,
      pegtl::seq<pegtl::at<store_container_rule>, store_container_rule>,
      pegtl::seq<pegtl::at<array_allocate_rule>, array_allocate_rule>,
      pegtl::seq<pegtl::at<tuple_allocate_rule>, tuple_allocate_rule>,
      pegtl::seq<pegtl::at<call_with_ret>, call_with_ret>,
      pegtl::seq<pegtl::at<call_without_ret>, call_without_ret>,
      pegtl::seq<pegtl::at<get_length_rule>, get_length_rule>,
      pegtl::seq<pegtl::at<assign_inst_rule>, assign_inst_rule>,
      pegtl::seq<pegtl::at<label_inst_rule>, label_inst_rule>,
      pegtl::seq<pegtl::at<if_state_rule>, if_state_rule>,
      pegtl::seq<pegtl::at<goto_rule>, goto_rule>,
      pegtl::seq<pegtl::at<return_rule>, return_rule>,
      pegtl::seq<pegtl::at<while_state_rule>, while_state_rule>,
      pegtl::seq<pegtl::at<continue_state_rule>, continue_state_rule>,
      pegtl::seq<pegtl::at<break_state_rule>, break_state_rule>,
      pegtl::seq<pegtl::at<scope_rule>, scope_rule>
    > {};
    


  struct instructions_rule :
    pegtl::star<
      seps,
      instruction_rule,
      seps
    > {};

  struct scope_begin : pegtl::one<'{'> {};

  struct scope_end : pegtl::one<'}'> {};

  struct scope_rule :
    pegtl::seq<
      scope_begin,
      seps,
      instructions_rule,
      seps,
      scope_end
    > {};


  struct function_type :
    pegtl::sor<
      TAOCPP_PEGTL_STRING("int64"),
      TAOCPP_PEGTL_STRING("code"),
      TAOCPP_PEGTL_STRING("void"),
      TAOCPP_PEGTL_STRING("tuple"),
      pegtl::seq<
        TAOCPP_PEGTL_STRING("int64"),
        seps,
        pegtl::plus<
          square_bracket_rule
        >
      >
    > {};

  struct function_name_declare :  name {};

  struct int64_arg_declare : 
    pegtl::seq<
      TAOCPP_PEGTL_STRING("int64"),
      seps,
      names_rule
    > {};

   struct code_arg_declare :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("code"),
      seps,
      names_rule
    > {};

  struct tuple_arg_declare :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("tuple"),
      seps,
      names_rule
    > {};

  struct array_arg_declare :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("int64"),
      seps,
      pegtl::plus<
        square_bracket_rule
      >,
      seps,
      names_rule
    > {};

  struct arg_declare : 
    pegtl::sor<
      int64_arg_declare,
      code_arg_declare,
      tuple_arg_declare,
      array_arg_declare
    > {};

  struct args_declare :
    pegtl::seq<
      arg_declare,
      pegtl::star<
        seps,
        pegtl::one<','>,
        seps,
        arg_declare
      >
    > {};

  
  struct function_scope_rule : scope_rule {};

  struct function_rule :
    pegtl::seq<
      function_type,
      seps,
      function_name_declare,
      seps,
      pegtl::one<'('>,
      seps,
      pegtl::opt<
        args_declare
      >,
      seps,
      pegtl::one<')'>,
      seps,
      function_scope_rule
    > {};

  struct functions_rule :
    pegtl::plus <
      pegtl::seq<
        seps,
        function_rule,
        seps
      >
    > {};

  struct program : functions_rule {};
    

  struct grammar : 
    pegtl::must<
      program
    > {};

  template<typename Rule>
  struct action : pegtl::nothing<Rule> {};

  template<> struct action <number_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      parsed_items.push_back(new Number(in.string()));
    }
  };

  template<> struct action <op_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      parsed_items.push_back(new Operator(in.string()));
    }
  };

  template<> struct action <label_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      parsed_items.push_back(new Label(in.string()));
    }
  };
  

  template<> struct action <name_declare_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      static int counter = 0;
      declared_vars.push_back(in.string());
      auto scope = parsed_scopes.back();
      scope->SetUniqueName(in.string(), in.string() + std::to_string(counter++));
    }
  };

  template<> struct action <function_name_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      parsed_items.push_back(new FunctionName(in.string()));
    }
  };

  

  template<> struct action <int64_declare_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::vector<Variable*> vars;
      for(auto var_name : declared_vars) {
        for(auto i = parsed_scopes.rbegin(); i != parsed_scopes.rend(); i++) {
          auto scope = *i;
          try {
            auto unique_name = scope->GetUniqueName(var_name);
            auto var = new Int64(unique_name);
            vars.push_back(var);
            scope->SetVarName(unique_name, var);
            break;
          } catch(...) {
            continue;
          }
        }
      }
      auto scope = parsed_scopes.back();
      auto inst = new DeclareInst(vars);
      inst->SetScope(scope);
      scope->InsertInstructions(inst);
      declared_vars.clear();
    }
  };

  template<> struct action <code_declare_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::vector<Variable*> vars;
      for(auto var_name : declared_vars) {
        for(auto i = parsed_scopes.rbegin(); i != parsed_scopes.rend(); i++) {
          auto scope = *i;
          try {
            auto unique_name = scope->GetUniqueName(var_name);
            auto var = new Code(unique_name);
            vars.push_back(var);
            scope->SetVarName(unique_name, var);
            break;
          } catch(...) {
            continue;
          }
        }
      }
      auto scope = parsed_scopes.back();
      auto inst = new DeclareInst(vars);
      inst->SetScope(scope);
      scope->InsertInstructions(inst);
      declared_vars.clear();
    }
  };

  template<> struct action <tuple_declare_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::vector<Variable*> vars;
      for(auto var_name : declared_vars) {
        for(auto i = parsed_scopes.rbegin(); i != parsed_scopes.rend(); i++) {
          auto scope = *i;
          try {
            auto unique_name = scope->GetUniqueName(var_name);
            auto var = new Tuple(unique_name);
            vars.push_back(var);
            scope->SetVarName(unique_name, var);
            break;
          } catch(...) {
            continue;
          }
        }
      }
      auto scope = parsed_scopes.back();
      auto inst = new DeclareInst(vars);
      inst->SetScope(scope);
      scope->InsertInstructions(inst);
      declared_vars.clear();
    }
  };


  template<> struct action <square_bracket_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      square_bracket_rule::pair_number++;
    }
  };
  
  template<> struct action <array_declare_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::vector<Variable*> vars;
      for(auto var_name : declared_vars) {
        for(auto i = parsed_scopes.rbegin(); i != parsed_scopes.rend(); i++) {
          auto scope = *i;
          try {
            auto unique_name = scope->GetUniqueName(var_name);
            auto var = new Array(unique_name, square_bracket_rule::pair_number);
            vars.push_back(var);
            scope->SetVarName(unique_name, var);
            break;
          } catch(...) {
            continue;
          }
        }
      }
      auto scope = parsed_scopes.back();
      auto inst = new DeclareInst(vars);
      inst->SetScope(scope);
      scope->InsertInstructions(inst);
      declared_vars.clear();
      square_bracket_rule::pair_number = 0;
    }
  };
  

  template<> struct action <name_use_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      Variable* var;
      for(auto i = parsed_scopes.rbegin(); i != parsed_scopes.rend(); i++) {
        auto scope = *i;
        try {
          auto unique_name = scope->GetUniqueName(in.string());
          var = scope->GetVar(unique_name);
          parsed_items.push_back(var);
          break;
        } catch(...) {
          continue;
        }
      }
      if(var == nullptr) {
        parsed_items.push_back(new FunctionName(in.string()));
      }
    }
  };

  template<> struct action <function_type> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto f = new Function();
      p.functions.push_back(f);
      square_bracket_rule::pair_number = 0;
      f->type = in.string();
    }
  };

  template<> struct action <function_name_declare> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto f = p.functions.back();
      f->callee = in.string();
    }
  };

  template<> struct action <int64_arg_declare> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto f = p.functions.back();
      f->args.push_back(new Int64(in.string()));
    }
  };

  template<> struct action <array_arg_declare> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto f = p.functions.back();
      f->args.push_back(new Array(in.string(), square_bracket_rule::pair_number));
      square_bracket_rule::pair_number = 0;
    }
  };

  template<> struct action <tuple_arg_declare> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto f = p.functions.back();
      f->args.push_back(new Tuple(in.string()));
    }
  };

  template<> struct action <code_arg_declare> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto f = p.functions.back();
      f->args.push_back(new Code(in.string()));
    }
  };

  template<> struct action <scope_begin> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto f = p.functions.back();
      auto scope = new Scope;
      if(!parsed_scopes.empty()) {
        scope->SetScope(parsed_scopes.back());
      }
      parsed_scopes.push_back(scope);
    }
  };

  template<> struct action <scope_end> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      parsed_scopes.pop_back();
    }
  };

  template<> struct action <function_scope_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto f = p.functions.back();
      auto scope = parsed_scopes.back();
      f->scope = scope;
      for(auto arg : f->args) {
        f->scope->SetUniqueName(arg->GetCode(), arg->GetCode());
        f->scope->SetVarName(arg->GetCode(), arg);
      }
    }
  };


  template<> struct action <assign_inst_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto scope = parsed_scopes.back();
      auto rhs = parsed_items.back();
      parsed_items.pop_back();
      auto lhs = static_cast<Variable*>(parsed_items.back());
      parsed_items.pop_back();
      auto inst = new AssignInst(lhs, rhs);
      scope->InsertInstructions(inst);
      inst->SetScope(scope);
    }
  };

  template<> struct action <binary_op_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto scope = parsed_scopes.back();
      auto t2 = parsed_items.back();
      parsed_items.pop_back();
      auto op = static_cast<Operator*>(parsed_items.back());
      parsed_items.pop_back();
      auto t1 = parsed_items.back();
      parsed_items.pop_back();
      auto lhs = static_cast<Variable*>(parsed_items.back());
      parsed_items.pop_back();
      auto inst = new BinaryOperator(lhs, t1, op, t2);
      scope->InsertInstructions(inst);
      inst->SetScope(scope);
    }
  };

  template<> struct action <label_inst_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto scope = parsed_scopes.back();
      auto label = static_cast<Label*>(parsed_items.back());
      auto inst = new LabelInst(label);
      scope->InsertInstructions(inst);
      inst->SetScope(scope);
    }
  };

  template<> struct action <if_state_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto scope = parsed_scopes.back();
      auto f_label = static_cast<Label*>(parsed_items.back());
      parsed_items.pop_back();
      auto t_label = static_cast<Label*>(parsed_items.back());
      parsed_items.pop_back();
      auto t2 = parsed_items.back();
      parsed_items.pop_back();
      auto cmp = static_cast<Operator*>(parsed_items.back());
      parsed_items.pop_back();
      auto t1 = parsed_items.back();
      parsed_items.pop_back();
      auto comparator = new Comparator(t1, cmp, t2);
      auto inst = new IfStatement(comparator, t_label, f_label);
      scope->InsertInstructions(inst);
      inst->SetScope(scope);
    }
  };
  
  template<> struct action <goto_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto scope = parsed_scopes.back();
      auto label = static_cast<Label*>(parsed_items.back());
      auto inst = new JumpInst(label);
      scope->InsertInstructions(inst);
      inst->SetScope(scope);
    }
  };

  template<> struct action <return_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto scope = parsed_scopes.back();
      Item* t = nullptr;
      if(!parsed_items.empty()) {
        t = parsed_items.back();
        parsed_items.pop_back();
      }
      auto inst = new ReturnInst(t);
      scope->InsertInstructions(inst);
      inst->SetScope(scope);
    }
  };
  
  template<> struct action <while_state_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      static int counter = 0;
      auto scope = parsed_scopes.back();
      auto f_label = static_cast<Label*>(parsed_items.back());
      parsed_items.pop_back();
      auto t_label = static_cast<Label*>(parsed_items.back());
      parsed_items.pop_back();
      auto t2 = parsed_items.back();
      parsed_items.pop_back();
      auto cmp = static_cast<Operator*>(parsed_items.back());
      parsed_items.pop_back();
      auto t1 = parsed_items.back();
      parsed_items.pop_back();
      auto comparator = new Comparator(t1, cmp, t2);
      auto cond_label = new Label(":cond_label" + std::to_string(counter++));
      auto inst = new WhileStatement(comparator, t_label, f_label, cond_label);
      cond_labels[inst] = cond_label->GetCode();
      begin_while[inst] = t_label->GetCode();
      end_while[inst] = f_label->GetCode();
      auto label_inst = new LabelInst(cond_label);
      scope->InsertInstructions(label_inst);
      label_inst->SetScope(scope);
      scope->InsertInstructions(inst);
      inst->SetScope(scope);
    }
  };

  template<> struct action <continue_state_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto scope = parsed_scopes.back();
      auto inst = new Continue;
      scope->InsertInstructions(inst);
      inst->SetScope(scope);
    }
  };

  template<> struct action <break_state_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto scope = parsed_scopes.back();
      auto inst = new Break;
      scope->InsertInstructions(inst);
      inst->SetScope(scope);
    }
  };

  template<> struct action <load_container_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto scope = parsed_scopes.back();
      Variable* lhs;
      Variable* rhs;
      std::vector<Item*> args; 
      for(auto i  = parsed_items.begin(); i != parsed_items.end(); i++) {
        if(i == parsed_items.begin()) {
          lhs = static_cast<Variable*>(*i);
        } else if (i == parsed_items.begin() + 1) {
          rhs = static_cast<Variable*>(*i);
        } else {
          args.push_back(*i);
        }
      }
      parsed_items.clear();
      auto inst = new LoadContainer(lhs, rhs, args);
      scope->InsertInstructions(inst);
      inst->SetScope(scope);
    }
  };

  template<> struct action <store_container_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto scope = parsed_scopes.back();
      Variable* lhs;
      Item* rhs;
      std::vector<Item*> args; 
      for(auto i  = parsed_items.begin(); i != parsed_items.end(); i++) {
        if(i == parsed_items.begin()) {
          lhs = static_cast<Variable*>(*i);
        } else if (i == parsed_items.end() - 1) {
          rhs = *i;
        } else {
          args.push_back(*i);
        }
      }
      parsed_items.clear();
      auto inst = new StoreContainer(lhs, args, rhs);
      scope->InsertInstructions(inst);
      inst->SetScope(scope);
    }
  };

  template<> struct action <get_length_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto scope = parsed_scopes.back();
      auto t = parsed_items.back();
      parsed_items.pop_back();
      auto rhs = static_cast<Variable*>(parsed_items.back());
      parsed_items.pop_back();
      auto lhs = static_cast<Variable*>(parsed_items.back());
      parsed_items.pop_back();
      auto inst = new GetLength(lhs, rhs, t);
      scope->InsertInstructions(inst);
      inst->SetScope(scope);
    }
  };

  template<> struct action <call_with_ret> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto scope = parsed_scopes.back();
      Variable* lhs;
      FunctionName* callee;
      std::vector<Item*> args;
      for(auto i = parsed_items.begin(); i != parsed_items.end(); i++) {
        if(i == parsed_items.begin()) {
          lhs = static_cast<Variable*>(*i);
        } else if(i == parsed_items.begin() + 1) {
          callee = static_cast<FunctionName*>(*i);
        } else {
          args.push_back(*i);
        }
      }
      parsed_items.clear();
      auto inst = new CallInst(lhs, callee, args);
      scope->InsertInstructions(inst);
      inst->SetScope(scope);
    }
  };

  template<> struct action <call_without_ret> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto scope = parsed_scopes.back();
      FunctionName* callee;
      std::vector<Item*> args;
      for(auto i = parsed_items.begin(); i != parsed_items.end(); i++) {
        if(i == parsed_items.begin()) {
          callee = static_cast<FunctionName*>(*i);
        } else {
          args.push_back(*i);
        }
      }
      parsed_items.clear();
      auto inst = new CallInst(nullptr, callee, args);
      scope->InsertInstructions(inst);
      inst->SetScope(scope);
    }
  };

  template<> struct action <array_allocate_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto scope = parsed_scopes.back();
      Array* lhs;
      std::vector<Item*> args;
      for(auto i = parsed_items.begin(); i != parsed_items.end(); i++) {
        if(i == parsed_items.begin()) {
          lhs = static_cast<Array*>(*i);
        } else {
          args.push_back(*i);
        }
      }
      auto inst = new AllocateArray(lhs, args);
      scope->InsertInstructions(inst);
      inst->SetScope(scope);
    }
  };

  template<> struct action <tuple_allocate_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto scope = parsed_scopes.back();
      auto t = parsed_items.back();
      parsed_items.pop_back();
      auto lhs = static_cast<Tuple*>(parsed_items.back());
      parsed_items.pop_back();
      auto inst = new AllocateTuple(lhs, t);
      scope->InsertInstructions(inst);
      inst->SetScope(scope);
    }
  };
    

  template<> struct action <function_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      // mapping loop
      auto f = p.functions.back();
      std::stack<WhileStatement*> loop_stack;
      std::stack<Scope*> scope_stack;
      scope_stack.push(f->scope);
      while(!scope_stack.empty()) {
        auto scope = scope_stack.top();
        scope_stack.pop();
        for(auto inst : scope->GetInstructions()) {
          if(auto scope = dynamic_cast<Scope*>(inst)) {
            scope_stack.push(scope);
          } 
          if(!loop_stack.empty()) {
            auto w = loop_stack.top();
            inst->SetLoop(w);
          }
          if(auto label_inst = dynamic_cast<LabelInst*>(inst)) {
            for(auto [w, label] : begin_while) {
              if(label == label_inst->ToString()) {
                loop_stack.push(w);
                break;
              }
            }
            for(auto [w, label] : end_while) {
              if(label == label_inst->ToString()) {
                loop_stack.pop();
                break;
              }
            }
          }
        }
      }
    }
  };
  
  
  

  Program parse_file(char* fileName) {
    pegtl::analyze<grammar>();

    /*
     * Parse.
     */
    file_input< > fileInput(fileName);
    Program p;
    parse<grammar, action>(fileInput, p);

    return p;
  }
}