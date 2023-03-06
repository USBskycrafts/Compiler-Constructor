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

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>
#include "parser.hpp"
#include "LA.hpp"
#include "tao/pegtl/ascii.hpp"
#include "utils.hpp"



namespace LA {

  namespace pegtl = tao::TAO_PEGTL_NAMESPACE;
  using namespace pegtl;

  std::vector<std::shared_ptr<Item>> parsed_items;
  std::vector<std::string> declared_vars;



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

  struct variable : 
    pegtl::seq<
      pegtl::one<'%'>,
      name
    > {};

  struct variable_declare_rule : name {};

  struct variable_use_rule : name {};

  struct label : 
    pegtl::seq<
      pegtl::one<':'>,
      name
    > {};

  struct label_rule : label {};

  struct f_name_rule : name {};

  struct r_name :
    pegtl::sor<
      TAOCPP_PEGTL_STRING("print"),
      TAOCPP_PEGTL_STRING("allocate"),
      TAOCPP_PEGTL_STRING("input"),
      TAOCPP_PEGTL_STRING("tensor-error")
    > {};

  struct r_name_rule : r_name {};
  
  struct int64_declare_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("int64"),
      seps,
      variable_declare_rule
    > {};

  struct tuple_declare_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("tuple"),
      seps,
      variable_declare_rule
    > {};

  struct code_declare_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("code"),
      seps,
      variable_declare_rule
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
      variable_declare_rule
    > {};

  struct assign_inst_rule :
    pegtl::seq<
      variable_use_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      pegtl::sor<
        variable_use_rule,
        number_rule,
        label_rule,
        f_name_rule,
        r_name_rule
      >
    > {};

  struct binary_op_rule :
    pegtl::seq<
      variable_use_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      pegtl::sor<
        variable_use_rule,
        number_rule
      >,
      seps,
      op_rule,
      seps,
      pegtl::sor<
        variable_use_rule,
        number_rule
      >
    > {};

  struct load_container_rule : 
    pegtl::seq<
      variable_use_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      variable_use_rule,
      seps,
      pegtl::plus<
        pegtl::seq<
          seps,
          pegtl::one<'['>,
          seps,
          pegtl::sor<
            variable_use_rule,
            number_rule
          >,
          seps,
          pegtl::one<']'>,
          seps
        >
      >
    > {};

  struct store_container_rule :
    pegtl::seq<
      variable_use_rule,
      seps,
      pegtl::plus<
        seps,
        pegtl::one<'['>,
        seps,
        pegtl::sor<
          variable_use_rule,
          number_rule
        >,
        seps,
        pegtl::one<']'>,
        seps
      >,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      pegtl::sor<
        variable_use_rule,
        number_rule,
        label_rule,
        f_name_rule,
        r_name_rule
      >
    > {};

  // struct load_tuple_rule :
  //   pegtl::seq<
  //     variable_use_rule,
  //     seps,
  //     TAOCPP_PEGTL_STRING("<-"),
  //     seps,
  //     variable_use_rule,
  //     pegtl::one<'['>,
  //     seps,
  //     pegtl::sor<
  //       variable_use_rule,
  //       number_rule
  //     >,
  //     seps,
  //     pegtl::one<']'>
  //   > {};

  // struct store_tuple_rule :
  //   pegtl::seq<
  //     variable_use_rule,
  //     seps,
  //     pegtl::one<'['>,
  //     seps,
  //     pegtl::sor<
  //       variable_use_rule,
  //       number_rule
  //     >,
  //     seps,
  //     pegtl::one<']'>,
  //     seps,
  //     TAOCPP_PEGTL_STRING("<-"),
  //     seps,
  //     pegtl::sor<
  //       variable_use_rule,
  //       number_rule
  //     >
  //   > {};

  struct array_length_rule :
    pegtl::seq<
      variable_use_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      TAOCPP_PEGTL_STRING("length"),
      seps,
      variable_use_rule,
      seps,
      pegtl::sor<
        variable_use_rule,
        number_rule
      >
    > {};

  struct tuple_length_rule :
    pegtl::seq<
      variable_use_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      TAOCPP_PEGTL_STRING("length"),
      seps,
      variable_use_rule
    > {};

  struct args_rule : 
    pegtl::seq<
      pegtl::sor<
        variable_use_rule,
        number_rule
      >,
      seps,
      pegtl::star<
        pegtl::seq<
          pegtl::one<','>,
          seps,
          pegtl::sor<
            variable_use_rule,
            number_rule
          >
        >
      >
    > {};


  struct call_without_ret : 
    pegtl::seq<
      pegtl::sor<
        variable_use_rule,
        r_name_rule,
        f_name_rule
        
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

  struct call_with_ret : 
    pegtl::seq<
      variable_use_rule,
      seps,
      TAOCPP_PEGTL_STRING("<-"),
      seps,
      pegtl::sor<
        variable_use_rule,
        r_name_rule,
        f_name_rule
        
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

  struct array_allocate_rule :
    pegtl::seq<
      variable_use_rule,
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
      variable_use_rule,
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
        variable_use_rule
      >,
      seps,
      pegtl::one<')'>
    > {};

  struct label_inst_rule :
    pegtl::seq<
      label_rule
    > {};

  struct branch_condition_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("br"),
      seps,
      pegtl::sor<
        variable_use_rule,
        number_rule
      >,
      seps,
      label_rule,
      seps,
      label_rule
    > {};

    struct branch_goto_rule :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("br"),
      seps,
      label_rule
    > {};
  
  struct return_with_var :  
    pegtl::seq<
      TAOCPP_PEGTL_STRING("return"),
      seps,
      pegtl::sor<
        variable_use_rule,
        number_rule
      >
    > {};

  struct return_without_var :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("return")
    > {};

  struct instruction_rule :
    pegtl::sor<
    pegtl::seq<pegtl::at<label_inst_rule>, label_inst_rule>,
      pegtl::seq<pegtl::at<call_with_ret>, call_with_ret>,
      pegtl::seq<pegtl::at<array_length_rule>, array_length_rule>,
      pegtl::seq<pegtl::at<tuple_length_rule>, tuple_length_rule>,
      pegtl::seq<pegtl::at<array_allocate_rule>, array_allocate_rule>,
      pegtl::seq<pegtl::at<tuple_allocate_rule>, tuple_allocate_rule>,
      pegtl::seq<pegtl::at<array_declare_rule>, array_declare_rule>,
      pegtl::seq<pegtl::at<int64_declare_rule>, int64_declare_rule>,
      pegtl::seq<pegtl::at<tuple_declare_rule>, tuple_declare_rule>,
      pegtl::seq<pegtl::at<code_declare_rule>, code_declare_rule>, 
      pegtl::seq<pegtl::at<store_container_rule>, store_container_rule>,
      pegtl::seq<pegtl::at<load_container_rule>, load_container_rule>,
      pegtl::seq<pegtl::at<binary_op_rule>, binary_op_rule>,
      pegtl::seq<pegtl::at<assign_inst_rule>, assign_inst_rule>,
      pegtl::seq<pegtl::at<call_without_ret>, call_without_ret>,   
      pegtl::seq<pegtl::at<branch_condition_rule>, branch_condition_rule>,
      pegtl::seq<pegtl::at<branch_goto_rule>, branch_goto_rule>,
      pegtl::seq<pegtl::at<return_with_var>, return_with_var>,
      pegtl::seq<pegtl::at<return_without_var>, return_without_var>   
    > {};

  struct instructions_rule :
    pegtl::plus <
      pegtl::seq<
        seps,
        instruction_rule,
        seps
      >
    > {};

  struct terminator_rule :
    pegtl::sor<
     
    > {};


  struct return_type_rule :
    pegtl::sor< 
      TAOCPP_PEGTL_STRING("void"),
      TAOCPP_PEGTL_STRING("tuple"),
      TAOCPP_PEGTL_STRING("code"),
      pegtl::seq<
        TAOCPP_PEGTL_STRING("int64"),
        seps,
        pegtl::star<
          pegtl::seq<
            pegtl::one<'['>,
            seps,
            pegtl::one<']'>
          >
        >
      >,
      TAOCPP_PEGTL_STRING("int64")
    > {};


  struct f_name_declare : name {};

  struct int64_arg_delcare : 
    pegtl::seq<
      TAOCPP_PEGTL_STRING("int64"),
      seps,
      variable_declare_rule
    > {};

  struct code_arg_declare :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("code"),
      seps,
      variable_declare_rule
    > {};

  struct tuple_arg_declare :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("tuple"),
      seps,
      variable_declare_rule
    > {};

  struct array_arg_declare :
    pegtl::seq<
      TAOCPP_PEGTL_STRING("int64"),
      seps,
      pegtl::plus<
        square_bracket_rule
      >,
      seps,
      variable_declare_rule
    > {};

  struct args_declare :
    pegtl::seq<
      pegtl::sor<
          int64_arg_delcare,
          code_arg_declare,
          tuple_arg_declare,
          array_arg_declare
      >,
      pegtl::star<
        pegtl::seq<
          pegtl::one<','>,
          seps,
          pegtl::sor<
            int64_arg_delcare,
            code_arg_declare,
            tuple_arg_declare,
            array_arg_declare
          >
        >
      > 
   >{};


  struct function_rule :
    pegtl::seq<
      return_type_rule,
      seps,
      f_name_declare,
      seps,
      pegtl::one<'('>,
      seps,
      pegtl::opt<
        args_declare
      >,
      seps,
      pegtl::one<')'>,
      seps,
      pegtl::one<'{'>,
      seps,
      pegtl::opt<
        instructions_rule
      >,
      seps,
      pegtl::one<'}'>
    > {};

  struct functions_rule :
    pegtl::plus<
      pegtl::seq<
        seps,
        function_rule,
        seps
      >
    >{};

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
      std::shared_ptr<Number> number(new Number(in.string()));
      parsed_items.push_back(number);
    }
  };


  template<> struct action <variable_declare_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      declared_vars.push_back(in.string());
    }
  };


  template<> struct action <variable_use_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto f = p.functions.back();
      if(f->variables_mapper.count(in.string()) == 0) {
        auto f_name = std::make_shared<FunctionName>(in.string());
        parsed_items.push_back(f_name);
      } else {
        parsed_items.push_back(f->variables_mapper.at(in.string()));
      }
    }
  };

  template<> struct action <label_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Label> label(new Label(in.string()));
      auto f = p.functions.back();
      if(f->labels_mapper.count(in.string()) == 0) {
        f->labels_mapper[in.string()] = label;
      }
      f->labels_mapper[in.string()] = label;
      parsed_items.push_back(label);
    }
  };

  template<> struct action <op_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Operator> op(new Operator(in.string()));
      parsed_items.push_back(op);
    }
  };


  template<> struct action <f_name_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<FunctionName> func_name(new FunctionName(in.string()));
      parsed_items.push_back(func_name);
    }
  };

  template<> struct action <r_name_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<FunctionName> func_name(new FunctionName(in.string()));
      parsed_items.push_back(func_name);
    }
  };


  template<> struct action <int64_declare_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto var_name = declared_vars.back();
      declared_vars.pop_back();
      auto variable = std::make_shared<Int64>(var_name);
      auto f = p.functions.back(); 
      f->variables_mapper[var_name] = variable;
      auto inst = std::make_shared<DeclareInst>(variable);
      f->instructions.push_back(inst);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
      variable->SetFunction(f);
      auto one = std::make_shared<Number>("1");
      auto init_inst = std::make_shared<AssignInst>(variable, one);
      f->instructions.push_back(init_inst);
      init_inst->SetParent(f);
      init_inst->SetLine(std::to_string(in.position().line));
    }
  };

  template<> struct action <code_declare_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto var_name = declared_vars.back();
      declared_vars.pop_back();
      auto variable = std::make_shared<Code>(var_name);
      auto f = p.functions.back();
      f->variables_mapper[var_name] = variable;
      auto inst = std::make_shared<DeclareInst>(variable);
      f->instructions.push_back(inst);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
      variable->SetFunction(f);
      auto one = std::make_shared<Number>("1");
      auto init_inst = std::make_shared<AssignInst>(variable, one);
      f->instructions.push_back(init_inst);
      init_inst->SetParent(f);
      init_inst->SetLine(std::to_string(in.position().line));
    }
  };

  template<> struct action <tuple_declare_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto var_name = declared_vars.back();
      declared_vars.pop_back();
      auto variable = std::make_shared<Tuple>(var_name);
      auto f = p.functions.back();
      f->variables_mapper[var_name] = variable;
      auto inst = std::make_shared<DeclareInst>(variable);
      f->instructions.push_back(inst);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
      variable->SetFunction(f);
      auto zero = std::make_shared<Number>("0");
      auto init_inst = std::make_shared<AssignInst>(variable, zero);
      f->instructions.push_back(init_inst);
      init_inst->SetParent(f);
      init_inst->SetLine(std::to_string(in.position().line));
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
      auto var_name = declared_vars.back();
      declared_vars.pop_back();
      auto dim = square_bracket_rule::pair_number;
      square_bracket_rule::pair_number = 0;
      auto variable = std::make_shared<Array>(var_name, dim);
      auto f = p.functions.back();
      f->variables_mapper[var_name] = variable;
      auto inst = std::make_shared<DeclareInst>(variable);
      f->instructions.push_back(inst);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
      variable->SetFunction(f);
      auto zero = std::make_shared<Number>("0");
      auto init_inst = std::make_shared<AssignInst>(variable, zero);
      f->instructions.push_back(init_inst);
      init_inst->SetParent(f);
      init_inst->SetLine(std::to_string(in.position().line));
    }
  };

  template<> struct action <label_inst_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto label = parsed_items.back();
      parsed_items.pop_back();
      auto inst = std::make_shared<LabelInst>(std::dynamic_pointer_cast<Label>(label));
      auto f = p.functions.back();
      f->instructions.push_back(inst);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
    }
  };


  template<> struct action <assign_inst_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto rhs = parsed_items.back();
      parsed_items.pop_back();
      auto lhs = std::dynamic_pointer_cast<Var>(parsed_items.back());
      parsed_items.pop_back();
      auto f = p.functions.back();
      if(typeid(*rhs) == typeid(Number)) {
        int num = std::stoi(rhs->ToString());
        num = num * 2 + 1;
        rhs = std::make_shared<Number>(std::to_string(num));
      }
      auto inst = std::make_shared<AssignInst>(lhs, rhs);
      f->instructions.push_back(inst);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
    }
  };

  template<> struct action <binary_op_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto operand2 = parsed_items.back();
      parsed_items.pop_back();
      auto op = std::dynamic_pointer_cast<Operator>(parsed_items.back());
      parsed_items.pop_back();
      auto operand1 = parsed_items.back();
      parsed_items.pop_back();
      auto lhs = std::dynamic_pointer_cast<Var>(parsed_items.back());
      parsed_items.pop_back();
      auto f = p.functions.back();

      //decoding
      std::string temp_name = "p";
      static int var_counter = 0;
      for(auto [name, _] : f->variables_mapper) {
        if(name.size() > temp_name.size()) {
          temp_name = name;
        }
      }
      if(typeid(*operand1) != typeid(Number)) {
        auto ori_operand1 = operand1;
        operand1 = std::dynamic_pointer_cast<Var>(operand1)->Copy(
          temp_name + std::to_string(var_counter++)
        );
        auto rshift = std::make_shared<Operator>(">>");
        auto one = std::make_shared<Number>("1");
        auto inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(operand1), 
            ori_operand1, rshift, one);
        f->instructions.push_back(inst);
        inst->SetParent(f);
        inst->SetLine(std::to_string(in.position().line));
      }
      if(typeid(*operand2) != typeid(Number)) {
        auto ori_operand2 = operand2;
        operand2 = std::dynamic_pointer_cast<Var>(operand2)->Copy(
          temp_name + std::to_string(var_counter++)
        );
        auto rshift = std::make_shared<Operator>(">>");
        auto one = std::make_shared<Number>("1");
        auto inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(operand2), 
            ori_operand2, rshift, one);
        f->instructions.push_back(inst);
        inst->SetParent(f);
        inst->SetLine(std::to_string(in.position().line));
      } 
      //operating
      auto inst = std::make_shared<BinaryOperator>(lhs, operand1, op, operand2);
      f->instructions.push_back(inst);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));


      //encoding
      auto lshift = std::make_shared<Operator>("<<");
      auto add = std::make_shared<Operator>("+");
      auto one = std::make_shared<Number>("1");
      inst = std::make_shared<BinaryOperator>(lhs, lhs, lshift, one);
      f->instructions.push_back(inst);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
      inst = std::make_shared<BinaryOperator>(lhs, lhs, add, one);
      f->instructions.push_back(inst);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
    }
  };

  template<> struct action <load_container_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Var> lhs;
      std::shared_ptr<Var> rhs;
      std::vector<std::shared_ptr<Item>> index;
      for(size_t i = 0; i < parsed_items.size(); i++) {
        if(i == 0) {
          lhs = std::dynamic_pointer_cast<Var>(parsed_items[i]);
        } else if(i == 1) {
          rhs = std::dynamic_pointer_cast<Var>(parsed_items[i]);
        } else {
          index.push_back(parsed_items[i]);
        }
      }
      parsed_items.clear();
      auto f = p.functions.back();
      //generating temp vars
      std::string temp_name = "p";
      static int var_counter = 0;
      for(auto [name, _] : f->variables_mapper) {
        if(name.size() > temp_name.size()) {
          temp_name = name;
        }
      }

      //checking allocating
      auto variable_name = temp_name + std::to_string(var_counter++);
      auto line_number_var = std::make_shared<Int64>(variable_name);
      f->variables_mapper[variable_name] = line_number_var;
      line_number_var->SetFunction(f);
      auto line_number = std::make_shared<Number>(std::to_string(in.position().line * 2 + 1));
      auto store_line = std::make_shared<AssignInst>(line_number_var, line_number);
      f->instructions.push_back(store_line);
      store_line->SetParent(f);
      store_line->SetLine(std::to_string(in.position().line));
      //generate compare instruction
      variable_name = temp_name + std::to_string(var_counter++);
      auto mark = std::make_shared<Int64>(variable_name);
      f->variables_mapper[variable_name] = mark;
      mark->SetFunction(f);
      auto equal = std::make_shared<Operator>("=");
      auto zero = std::make_shared<Number>("0");
      auto cmp_inst = std::make_shared<BinaryOperator>(mark, rhs, equal, zero);
      f->instructions.push_back(cmp_inst);
      cmp_inst->SetParent(f);
      cmp_inst->SetLine(std::to_string(in.position().line));
      //generate branch inst
      std::string label_name = ":init_check_out_of_bound_";
      for(auto [name, _] : f->labels_mapper) {
        if(name.size() > temp_name.size()) {
          label_name = name;
        }
      }
      auto correct = label_name + "load_container_" + std::to_string(var_counter++);
      auto error = label_name + "load_container_" + std::to_string(var_counter++);
      auto correct_label = std::make_shared<Label>(correct);
      auto error_label = std::make_shared<Label>(error);
      std::shared_ptr<BranchInst> br_inst(new BranchInst(mark, {error_label, correct_label}));
      f->instructions.push_back(br_inst);
      br_inst->SetParent(f);
      br_inst->SetLine(std::to_string(in.position().line));

      //error label inst
      auto error_inst = std::make_shared<LabelInst>(error_label);
      f->instructions.push_back(error_inst);
      error_inst->SetParent(f);
      error_inst->SetLine(std::to_string(in.position().line));

      //call tensor error
      auto f_name = std::make_shared<FunctionName>("tensor-error");
      std::shared_ptr<CallInst> call_inst(new CallInst(nullptr, f_name, {line_number}));
      f->instructions.push_back(call_inst);
      call_inst->SetParent(f);
      call_inst->SetLine(std::to_string(in.position().line));

      //correct label inst
      auto correct_inst = std::make_shared<LabelInst>(correct_label);
      f->instructions.push_back(correct_inst);
      correct_inst->SetParent(f);
      correct_inst->SetLine(std::to_string(in.position().line));


      if(typeid(*rhs) == typeid(Tuple)) {
        //decoding
        if(typeid(*index[0]) != typeid(Number)) {
          auto orig_var = index[0];
          index[0] = std::dynamic_pointer_cast<Var>(orig_var)->Copy(
                temp_name + std::to_string(var_counter++)
              );
          auto rshift = std::make_shared<Operator>(">>");
          auto one = std::make_shared<Number>("1");
          auto inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(index[0]), 
              orig_var, rshift, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));
        }
        //loading
        auto inst = std::make_shared<LoadTuple>(lhs, std::dynamic_pointer_cast<Tuple>(rhs), index[0]);
        f->instructions.push_back(inst);
        inst->SetParent(f);
        inst->SetLine(std::to_string(in.position().line));
      } else {
        //decoding
        for(int i = 0; i < index.size(); i++) {
          if(typeid(*index[i]) != typeid(Number)) {
            auto orig_var = index[i];
            index[i] = std::dynamic_pointer_cast<Var>(orig_var)->Copy(
                  temp_name + std::to_string(var_counter++)
                );
            auto rshift = std::make_shared<Operator>(">>");
            auto one = std::make_shared<Number>("1");
            auto inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(index[i]), orig_var, rshift, one);
            f->instructions.push_back(inst);
            inst->SetParent(f);
            inst->SetLine(std::to_string(in.position().line));
          }
        }
        if(index.size() == 1) {
          //checking out of bound
          auto array = std::dynamic_pointer_cast<Array>(rhs);
          variable_name = temp_name + std::to_string(var_counter++);
          auto length = std::make_shared<Int64>(variable_name);
          f->variables_mapper[variable_name] = length;
          length->SetFunction(f);
          auto get_length = std::make_shared<ArrayLength>(length, array, zero);
          f->instructions.push_back(get_length);
          get_length->SetParent(f);
          get_length->SetLine(std::to_string(in.position().line));
          //decoding length
          auto rshift = std::make_shared<Operator>(">>");
          auto one = std::make_shared<Number>("1");
          auto inst = std::make_shared<BinaryOperator>(length, length, rshift, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));
            // generating cmp inst
          variable_name = temp_name + std::to_string(var_counter++);
          auto mark = std::make_shared<Int64>(variable_name);
          f->variables_mapper[variable_name] = mark;
          mark->SetFunction(f);
          auto ge = std::make_shared<Operator>(">=");
          auto cmp_inst = std::make_shared<BinaryOperator>(mark, index[0], ge, length);
          f->instructions.push_back(cmp_inst);
          cmp_inst->SetParent(f);
          cmp_inst->SetLine(std::to_string(in.position().line));

          auto correct = label_name + "load_container_____" + std::to_string(var_counter++);
          auto error = label_name + "load_container_____" + std::to_string(var_counter++);
          auto correct_label = std::make_shared<Label>(correct);
          auto error_label = std::make_shared<Label>(error);
          std::shared_ptr<BranchInst> br_inst(new BranchInst(mark, {error_label, correct_label}));
          f->instructions.push_back(br_inst);
          br_inst->SetParent(f);
          br_inst->SetLine(std::to_string(in.position().line));

          //error label inst
          auto error_inst = std::make_shared<LabelInst>(error_label);
          f->instructions.push_back(error_inst);
          error_inst->SetParent(f);
          error_inst->SetLine(std::to_string(in.position().line));

          //call tensor error
          auto f_name = std::make_shared<FunctionName>("tensor-error");
          //encoding length
          auto lshift = std::make_shared<Operator>("<<");
          inst = std::make_shared<BinaryOperator>(length, length, lshift, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));
          auto add = std::make_shared<Operator>("+");
          inst = std::make_shared<BinaryOperator>(length, length, add, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));
          variable_name = temp_name + std::to_string(var_counter++);
          //encoding index
          auto id = std::make_shared<Int64>(variable_name);
          f->variables_mapper[variable_name] = id;
          inst = std::make_shared<BinaryOperator>(id, index[0], lshift, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));
          inst = std::make_shared<BinaryOperator>(id, id, add, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));
          std::shared_ptr<CallInst> call_inst(new CallInst(nullptr, f_name, {line_number, length, id}));
          f->instructions.push_back(call_inst);
          call_inst->SetParent(f);
          call_inst->SetLine(std::to_string(in.position().line));

          //correct label inst
          auto correct_inst = std::make_shared<LabelInst>(correct_label);
          f->instructions.push_back(correct_inst);
          correct_inst->SetParent(f);
          correct_inst->SetLine(std::to_string(in.position().line));
        } else {
          for(int i = 0; i < index.size(); i++) {
            auto array = std::dynamic_pointer_cast<Array>(rhs);
            variable_name = temp_name + std::to_string(var_counter++);
            auto length = std::make_shared<Int64>(variable_name);
            f->variables_mapper[variable_name] = length;
            length->SetFunction(f);
            auto index_number = std::make_shared<Number>(std::to_string(i));
            auto get_length = std::make_shared<ArrayLength>(length, array, index_number);
            f->instructions.push_back(get_length);
            get_length->SetParent(f);
            get_length->SetLine(std::to_string(in.position().line));
            //decoding length
            auto rshift = std::make_shared<Operator>(">>");
            auto one = std::make_shared<Number>("1");
            auto inst = std::make_shared<BinaryOperator>(length, length, rshift, one);
            f->instructions.push_back(inst);
            inst->SetParent(f);
            inst->SetLine(std::to_string(in.position().line));
              // generating cmp inst
            variable_name = temp_name + std::to_string(var_counter++);
            auto mark = std::make_shared<Int64>(variable_name);
            f->variables_mapper[variable_name] = mark;
            mark->SetFunction(f);
            auto ge = std::make_shared<Operator>(">=");
            auto cmp_inst = std::make_shared<BinaryOperator>(mark, index[i], ge, length);
            f->instructions.push_back(cmp_inst);
            cmp_inst->SetParent(f);
            cmp_inst->SetLine(std::to_string(in.position().line));

            auto correct = label_name + "load_container__" + std::to_string(var_counter++);
            auto error = label_name + "load_container__" + std::to_string(var_counter++);
            auto correct_label = std::make_shared<Label>(correct);
            auto error_label = std::make_shared<Label>(error);
            std::shared_ptr<BranchInst> br_inst(new BranchInst(mark, {error_label, correct_label}));
            f->instructions.push_back(br_inst);
            br_inst->SetParent(f);
            br_inst->SetLine(std::to_string(in.position().line));

            //error label inst
            auto error_inst = std::make_shared<LabelInst>(error_label);
            f->instructions.push_back(error_inst);
            error_inst->SetParent(f);
            error_inst->SetLine(std::to_string(in.position().line));

            //call tensor error
            auto f_name = std::make_shared<FunctionName>("tensor-error");
            //encoding length
            auto lshift = std::make_shared<Operator>("<<");
            inst = std::make_shared<BinaryOperator>(length, length, lshift, one);
            f->instructions.push_back(inst);
            inst->SetParent(f);
            inst->SetLine(std::to_string(in.position().line));
            auto add = std::make_shared<Operator>("+");
            inst = std::make_shared<BinaryOperator>(length, length, add, one);
            f->instructions.push_back(inst);
            inst->SetParent(f);
            inst->SetLine(std::to_string(in.position().line));
            //encoding index
            auto id = std::make_shared<Int64>(variable_name);
            f->variables_mapper[variable_name] = id;
            inst = std::make_shared<BinaryOperator>(id, index[i], lshift, one);
            f->instructions.push_back(inst);
            inst->SetParent(f);
            inst->SetLine(std::to_string(in.position().line));
            inst = std::make_shared<BinaryOperator>(id, id, add, one);
            f->instructions.push_back(inst);
            inst->SetParent(f);
            inst->SetLine(std::to_string(in.position().line));
            index_number = std::make_shared<Number>(std::to_string(i * 2 + 1));
            std::shared_ptr<CallInst> call_inst(new CallInst(nullptr, f_name, {line_number, index_number, length, id}));
            f->instructions.push_back(call_inst);
            call_inst->SetParent(f);
            call_inst->SetLine(std::to_string(in.position().line));

            //correct label inst
            auto correct_inst = std::make_shared<LabelInst>(correct_label);
            f->instructions.push_back(correct_inst);
            correct_inst->SetParent(f);
            correct_inst->SetLine(std::to_string(in.position().line));
          }
        }
        //loading
        auto inst = std::make_shared<LoadArray>(lhs, std::dynamic_pointer_cast<Array>(rhs), index);
        f->instructions.push_back(inst);
        inst->SetParent(f);
        inst->SetLine(std::to_string(in.position().line));
      }
    }
  };

  template<> struct action <store_container_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Var> lhs;
      std::vector<std::shared_ptr<Item>> index;
      std::shared_ptr<Item> rhs;
      for(size_t i = 0; i < parsed_items.size(); i++) {
        if(i == 0) {
          lhs = std::dynamic_pointer_cast<Var>(parsed_items[i]);
        } else if(i == parsed_items.size() - 1) {
          rhs = parsed_items[i];
        } else {
          index.push_back(parsed_items[i]);
        }
      }
      parsed_items.clear();
      auto f = p.functions.back();
      std::string temp_name = "p";
      static int var_counter = 0;
      for(auto [name, _] : f->variables_mapper) {
        if(name.size() > temp_name.size()) {
          temp_name = name;
        }
      }
      //encoding const
      if(typeid(*rhs) == typeid(Number)) {
        auto val = std::stoi(rhs->ToString());
        val = val * 2 + 1;
        rhs = std::make_shared<Number>(std::to_string(val));
      }

      //checking allocating
      auto variable_name = temp_name + std::to_string(var_counter++);
      auto line_number_var = std::make_shared<Int64>(variable_name);
      f->variables_mapper[variable_name] = line_number_var;
      auto line_number = std::make_shared<Number>(std::to_string(in.position().line * 2 + 1));
      auto store_line = std::make_shared<AssignInst>(line_number_var, line_number);
      f->instructions.push_back(store_line);
      store_line->SetParent(f);
      store_line->SetLine(std::to_string(in.position().line));
      //generate compare instruction
      variable_name = temp_name + std::to_string(var_counter++);
      auto mark = std::make_shared<Int64>(variable_name);
      f->variables_mapper[variable_name] = mark;
      mark->SetFunction(f);
      auto equal = std::make_shared<Operator>("=");
      auto zero = std::make_shared<Number>("0");
      auto cmp_inst = std::make_shared<BinaryOperator>(mark, lhs, equal, zero);
      f->instructions.push_back(cmp_inst);
      cmp_inst->SetParent(f);
      cmp_inst->SetLine(std::to_string(in.position().line));
      //generate branch inst
      std::string label_name = ":init_check_out_of_bound_";
      for(auto [name, _] : f->labels_mapper) {
        if(name.size() > temp_name.size()) {
          label_name = name;
        }
      }
      auto correct = label_name + "store_container_" + std::to_string(var_counter++);
      auto error = label_name + "store_container_" + std::to_string(var_counter++);
      auto correct_label = std::make_shared<Label>(correct);
      auto error_label = std::make_shared<Label>(error);
      std::shared_ptr<BranchInst> br_inst(new BranchInst(mark, {error_label, correct_label}));
      f->instructions.push_back(br_inst);
      br_inst->SetParent(f);
      br_inst->SetLine(std::to_string(in.position().line));

      //error label inst
      auto error_inst = std::make_shared<LabelInst>(error_label);
      f->instructions.push_back(error_inst);
      error_inst->SetParent(f);
      error_inst->SetLine(std::to_string(in.position().line));

      //call tensor error
      auto f_name = std::make_shared<FunctionName>("tensor-error");
      std::shared_ptr<CallInst> call_inst(new CallInst(nullptr, f_name, {line_number}));
      f->instructions.push_back(call_inst);
      call_inst->SetParent(f);
      call_inst->SetLine(std::to_string(in.position().line));

      //correct label inst
      auto correct_inst = std::make_shared<LabelInst>(correct_label);
      f->instructions.push_back(correct_inst);
      correct_inst->SetParent(f);
      correct_inst->SetLine(std::to_string(in.position().line));


      if(typeid(*lhs) == typeid(Tuple)) {
        //decoding 
        if(typeid(*index[0]) != typeid(Number)) {
          auto orig_var = index[0];
          index[0] = std::dynamic_pointer_cast<Var>(orig_var)->Copy(
                temp_name + std::to_string(var_counter++)
              );
          auto rshift = std::make_shared<Operator>(">>");
          auto one = std::make_shared<Number>("1");
          auto inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(index[0]), orig_var, rshift, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));
        }
        auto inst = std::make_shared<StoreTuple>(std::dynamic_pointer_cast<Tuple>(lhs), index[0], rhs);
        f->instructions.push_back(inst);
        inst->SetParent(f);
        inst->SetLine(std::to_string(in.position().line));
      } else {
        //decoding
        for(int i = 0; i < index.size(); i++) {
          if(typeid(*index[i]) != typeid(Number)) {
            auto orig_var = index[i];
            index[i] = std::dynamic_pointer_cast<Var>(orig_var)->Copy(
                  temp_name + std::to_string(var_counter++)
                );
            auto rshift = std::make_shared<Operator>(">>");
            auto one = std::make_shared<Number>("1");
            auto inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(index[i]), orig_var, rshift, one);
            f->instructions.push_back(inst);
            inst->SetParent(f);
            inst->SetLine(std::to_string(in.position().line));
          }
        }
        if(index.size() == 1) {
          //checking out of bound
          auto array = std::dynamic_pointer_cast<Array>(lhs);
          variable_name = temp_name + std::to_string(var_counter++);
          auto length = std::make_shared<Int64>(variable_name);
          f->variables_mapper[variable_name] = length;
          length->SetFunction(f);
          // DEBUG_COUT << length->ToString() << std::endl;
          // DEBUG_COUT << array->ToString() << std::endl;
          auto get_length = std::make_shared<ArrayLength>(length, array, zero);       
          f->instructions.push_back(get_length);
          get_length->SetParent(f);
          get_length->SetLine(std::to_string(in.position().line));
          //decoding length
          auto rshift = std::make_shared<Operator>(">>");
          auto one = std::make_shared<Number>("1");
          auto inst = std::make_shared<BinaryOperator>(length, length, rshift, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));
            // generating cmp inst
          variable_name = temp_name + std::to_string(var_counter++);
          auto mark = std::make_shared<Int64>(variable_name);
          f->variables_mapper[variable_name] = mark;
          mark->SetFunction(f);
          auto ge = std::make_shared<Operator>(">=");
          auto cmp_inst = std::make_shared<BinaryOperator>(mark, index[0], ge, length);
          f->instructions.push_back(cmp_inst);
          cmp_inst->SetParent(f);
          cmp_inst->SetLine(std::to_string(in.position().line));

          auto correct = label_name + "store_container_____" + std::to_string(var_counter++);
          auto error = label_name + "store_container_____" + std::to_string(var_counter++);
          auto correct_label = std::make_shared<Label>(correct);
          auto error_label = std::make_shared<Label>(error);
          std::shared_ptr<BranchInst> br_inst(new BranchInst(mark, {error_label, correct_label}));
          f->instructions.push_back(br_inst);
          br_inst->SetParent(f);
          br_inst->SetLine(std::to_string(in.position().line));

          //error label inst
          auto error_inst = std::make_shared<LabelInst>(error_label);
          f->instructions.push_back(error_inst);
          error_inst->SetParent(f);
          error_inst->SetLine(std::to_string(in.position().line));

          //call tensor error
          auto f_name = std::make_shared<FunctionName>("tensor-error");
          //encoding length
          auto lshift = std::make_shared<Operator>("<<");
          inst = std::make_shared<BinaryOperator>(length, length, lshift, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));
          auto add = std::make_shared<Operator>("+");
          inst = std::make_shared<BinaryOperator>(length, length, add, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));
          //encoding index
          auto id = std::make_shared<Int64>(variable_name);
          f->variables_mapper[variable_name] = id;
          inst = std::make_shared<BinaryOperator>(id, index[0], lshift, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));
          inst = std::make_shared<BinaryOperator>(id, id, add, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));
          std::shared_ptr<CallInst> call_inst(new CallInst(nullptr, f_name, {line_number, length, id}));
          f->instructions.push_back(call_inst);
          call_inst->SetParent(f);
          call_inst->SetLine(std::to_string(in.position().line));

          //correct label inst
          auto correct_inst = std::make_shared<LabelInst>(correct_label);
          f->instructions.push_back(correct_inst);
          correct_inst->SetParent(f);
          correct_inst->SetLine(std::to_string(in.position().line));
        } else {
          for(int i = 0; i < index.size(); i++) {
            auto array = std::dynamic_pointer_cast<Array>(lhs);
            variable_name = temp_name + std::to_string(var_counter++);
            auto length = std::make_shared<Int64>(variable_name);
            f->variables_mapper[variable_name] = length;
            length->SetFunction(f);
            auto index_number = std::make_shared<Number>(std::to_string(i));
            auto get_length = std::make_shared<ArrayLength>(length, array, index_number);
            f->instructions.push_back(get_length);
            get_length->SetParent(f);
            get_length->SetLine(std::to_string(in.position().line));
            //decoding length
            auto rshift = std::make_shared<Operator>(">>");
            auto one = std::make_shared<Number>("1");
            auto inst = std::make_shared<BinaryOperator>(length, length, rshift, one);
            f->instructions.push_back(inst);
            inst->SetParent(f);
            inst->SetLine(std::to_string(in.position().line));
              // generating cmp inst
            variable_name = temp_name + std::to_string(var_counter++);
            auto mark = std::make_shared<Int64>(variable_name);
            f->variables_mapper[variable_name] = mark;
            mark->SetFunction(f);
            auto ge = std::make_shared<Operator>(">=");
            auto cmp_inst = std::make_shared<BinaryOperator>(mark, index[i], ge, length);
            f->instructions.push_back(cmp_inst);
            cmp_inst->SetParent(f);
            cmp_inst->SetLine(std::to_string(in.position().line));

            auto correct = label_name + "store_container___" + std::to_string(var_counter++);
            auto error = label_name + "store_container___" + std::to_string(var_counter++);
            auto correct_label = std::make_shared<Label>(correct);
            auto error_label = std::make_shared<Label>(error);
            std::shared_ptr<BranchInst> br_inst(new BranchInst(mark, {error_label, correct_label}));
            f->instructions.push_back(br_inst);
            br_inst->SetParent(f);
            br_inst->SetLine(std::to_string(in.position().line));

            //error label inst
            auto error_inst = std::make_shared<LabelInst>(error_label);
            f->instructions.push_back(error_inst);
            error_inst->SetParent(f);
            error_inst->SetLine(std::to_string(in.position().line));

            //call tensor error
            auto f_name = std::make_shared<FunctionName>("tensor-error");
            //encoding length
            auto lshift = std::make_shared<Operator>("<<");
            inst = std::make_shared<BinaryOperator>(length, length, lshift, one);
            f->instructions.push_back(inst);
            inst->SetParent(f);
            inst->SetLine(std::to_string(in.position().line));
            auto add = std::make_shared<Operator>("+");
            inst = std::make_shared<BinaryOperator>(length, length, add, one);
            f->instructions.push_back(inst);
            inst->SetParent(f);
            inst->SetLine(std::to_string(in.position().line));
            //encoding index
            auto id = std::make_shared<Int64>(variable_name);
            f->variables_mapper[variable_name] = id;
            inst = std::make_shared<BinaryOperator>(id, index[i], lshift, one);
            f->instructions.push_back(inst);
            inst->SetParent(f);
            inst->SetLine(std::to_string(in.position().line));
            inst = std::make_shared<BinaryOperator>(id, id, add, one);
            f->instructions.push_back(inst);
            inst->SetParent(f);
            inst->SetLine(std::to_string(in.position().line));
            index_number = std::make_shared<Number>(std::to_string(i * 2 + 1));
            std::shared_ptr<CallInst> call_inst(new CallInst(nullptr, f_name, {line_number, index_number, length, id}));
            f->instructions.push_back(call_inst);
            call_inst->SetParent(f);
            call_inst->SetLine(std::to_string(in.position().line));

            //correct label inst
            auto correct_inst = std::make_shared<LabelInst>(correct_label);
            f->instructions.push_back(correct_inst);
            correct_inst->SetParent(f);
            correct_inst->SetLine(std::to_string(in.position().line));
          }
        }
        auto inst = std::make_shared<StoreArray>(std::dynamic_pointer_cast<Array>(lhs), index, rhs);
        f->instructions.push_back(inst);
        inst->SetParent(f);
        inst->SetLine(std::to_string(in.position().line));
      }
    }
  };

  // template<> struct action <store_tuple_rule> {
  //   template<typename Input>
  //   static void apply(const Input& in, Program& p) {
  //     std::shared_ptr<Tuple> lhs;
  //     std::shared_ptr<Item> index;
  //     std::shared_ptr<Item> rhs;

  //     rhs = parsed_items.back();
  //     parsed_items.pop_back();
  //     index = parsed_items.back();
  //     parsed_items.pop_back();
  //     lhs = std::dynamic_pointer_cast<Tuple>(parsed_items.back());
  //     parsed_items.pop_back();

  //     auto f = p.functions.back();
  //     auto bb = f->basic_blocks.back();
  //     auto inst = std::make_shared<StoreTuple>(lhs, index, rhs);
  //     inst->SetParent(bb);
  //     bb->instructions.push_back(inst);
  //   }
  // };

  // template<> struct action <load_tuple_rule> {
  //   template<typename Input>
  //   static void apply(const Input& in, Program& p) {
  //     std::shared_ptr<Var> lhs;
  //     std::shared_ptr<Tuple> rhs;
  //     std::shared_ptr<Item> index;

  //     index = parsed_items.back();
  //     parsed_items.pop_back();
  //     rhs = std::dynamic_pointer_cast<Tuple>(parsed_items.back());
  //     parsed_items.pop_back();
  //     lhs = std::dynamic_pointer_cast<Var>(parsed_items.back());
  //     parsed_items.pop_back();

  //     auto f = p.functions.back();
  //     auto bb = f->basic_blocks.back();
  //     auto inst = std::make_shared<LoadTuple>(lhs, rhs, index);
  //     inst->SetParent(bb);
  //     bb->instructions.push_back(inst);
  //   }
  // };

  template<> struct action <array_length_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Var> lhs;
      std::shared_ptr<Array> rhs;
      std::shared_ptr<Item> dim;
      dim = parsed_items.back();
      parsed_items.pop_back();
      rhs = std::dynamic_pointer_cast<Array>(parsed_items.back());
      parsed_items.pop_back();
      lhs = std::dynamic_pointer_cast<Var>(parsed_items.back());
      parsed_items.pop_back();

      auto f = p.functions.back();
      std::string temp_name = "p";
      static int var_counter = 0;
      for(auto [name, _] : f->variables_mapper) {
        if(name.size() > temp_name.size()) {
          temp_name = name;
        }
      }
      //checking allocating
      auto variable_name = temp_name + std::to_string(var_counter++);
      auto line_number_var = std::make_shared<Int64>(variable_name);
      f->variables_mapper[variable_name] = line_number_var;
      auto line_number = std::make_shared<Number>(std::to_string(in.position().line));
      auto store_line = std::make_shared<AssignInst>(line_number_var, line_number);
      f->instructions.push_back(store_line);
      store_line->SetParent(f);
      store_line->SetLine(std::to_string(in.position().line));
      //generate compare instruction
      variable_name = temp_name + std::to_string(var_counter++);
      auto mark = std::make_shared<Int64>(variable_name);
      f->variables_mapper[variable_name] = mark;
      mark->SetFunction(f);
      auto equal = std::make_shared<Operator>("=");
      auto zero = std::make_shared<Number>("0");
      auto cmp_inst = std::make_shared<BinaryOperator>(mark, rhs, equal, zero);
      f->instructions.push_back(cmp_inst);
      cmp_inst->SetParent(f);
      cmp_inst->SetLine(std::to_string(in.position().line));
      //generate branch inst
      std::string label_name = ":init_check_out_of_bound_";
      for(auto [name, _] : f->labels_mapper) {
        if(name.size() > temp_name.size()) {
          label_name = name;
        }
      }
      auto correct = label_name + "array_length_" + std::to_string(var_counter++);
      auto error = label_name + "array_length_" + std::to_string(var_counter++);
      auto correct_label = std::make_shared<Label>(correct);
      auto error_label = std::make_shared<Label>(error);
      std::shared_ptr<BranchInst> br_inst(new BranchInst(mark, {error_label, correct_label}));
      f->instructions.push_back(br_inst);
      br_inst->SetParent(f);
      br_inst->SetLine(std::to_string(in.position().line));

      //error label inst
      auto error_inst = std::make_shared<LabelInst>(error_label);
      f->instructions.push_back(error_inst);
      error_inst->SetParent(f);
      error_inst->SetLine(std::to_string(in.position().line));

      //call tensor error
      auto f_name = std::make_shared<FunctionName>("tensor-error");
      std::shared_ptr<CallInst> call_inst(new CallInst(nullptr, f_name, {line_number}));
      f->instructions.push_back(call_inst);
      call_inst->SetParent(f);
      call_inst->SetLine(std::to_string(in.position().line));

      //correct label inst
      auto correct_inst = std::make_shared<LabelInst>(correct_label);
      f->instructions.push_back(correct_inst);
      correct_inst->SetParent(f);
      correct_inst->SetLine(std::to_string(in.position().line));



      if(typeid(*dim) != typeid(Number)) {
        //decoding
        auto orig_var = dim;
        dim = std::dynamic_pointer_cast<Var>(orig_var)->Copy(
              temp_name + std::to_string(var_counter++)
            );
        auto rshift = std::make_shared<Operator>(">>");
        auto one = std::make_shared<Number>("1");
        auto inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(dim), orig_var, rshift, one);
        f->instructions.push_back(inst);
        inst->SetParent(f);
        inst->SetLine(std::to_string(in.position().line));
      }
      auto inst = std::make_shared<ArrayLength>(lhs, rhs, dim);
      f->instructions.push_back(inst);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
    }
  };

  template<> struct action <tuple_length_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Var> lhs;
      std::shared_ptr<Tuple> rhs;
      rhs = std::dynamic_pointer_cast<Tuple>(parsed_items.back());
      parsed_items.pop_back();
      lhs = std::dynamic_pointer_cast<Var>(parsed_items.back());
      parsed_items.pop_back();
      auto f = p.functions.back();
      std::string temp_name = "p";
      static int var_counter = 0;
      for(auto [name, _] : f->variables_mapper) {
        if(name.size() > temp_name.size()) {
          temp_name = name;
        }
      }
      //checking allocating
      auto variable_name = temp_name + std::to_string(var_counter++);
      auto line_number_var = std::make_shared<Int64>(variable_name);
      f->variables_mapper[variable_name] = line_number_var;
      auto line_number = std::make_shared<Number>(std::to_string(in.position().line));
      auto store_line = std::make_shared<AssignInst>(line_number_var, line_number);
      f->instructions.push_back(store_line);
      store_line->SetParent(f);
      store_line->SetLine(std::to_string(in.position().line));
      //generate compare instruction
      variable_name = temp_name + std::to_string(var_counter++);
      auto mark = std::make_shared<Int64>(variable_name);
      f->variables_mapper[variable_name] = mark;
      mark->SetFunction(f);
      auto equal = std::make_shared<Operator>("=");
      auto zero = std::make_shared<Number>("0");
      auto cmp_inst = std::make_shared<BinaryOperator>(mark, rhs, equal, zero);
      f->instructions.push_back(cmp_inst);
      cmp_inst->SetParent(f);
      cmp_inst->SetLine(std::to_string(in.position().line));
      //generate branch inst
      std::string label_name = ":init_check_out_of_bound_";
      for(auto [name, _] : f->labels_mapper) {
        if(name.size() > temp_name.size()) {
          label_name = name;
        }
      }
      auto correct = label_name + "tuple_length" + std::to_string(var_counter++);
      auto error = label_name + "tuple_length" + std::to_string(var_counter++);
      auto correct_label = std::make_shared<Label>(correct);
      auto error_label = std::make_shared<Label>(error);
      std::shared_ptr<BranchInst> br_inst(new BranchInst(mark, {error_label, correct_label}));
      f->instructions.push_back(br_inst);
      br_inst->SetParent(f);
      br_inst->SetLine(std::to_string(in.position().line));

      //error label inst
      auto error_inst = std::make_shared<LabelInst>(error_label);
      f->instructions.push_back(error_inst);
      error_inst->SetParent(f);
      error_inst->SetLine(std::to_string(in.position().line));

      //call tensor error
      auto f_name = std::make_shared<FunctionName>("tensor-error");
      std::shared_ptr<CallInst> call_inst(new CallInst(nullptr, f_name, {line_number}));
      f->instructions.push_back(call_inst);
      call_inst->SetParent(f);
      call_inst->SetLine(std::to_string(in.position().line));

      //correct label inst
      auto correct_inst = std::make_shared<LabelInst>(correct_label);
      f->instructions.push_back(correct_inst);
      correct_inst->SetParent(f);
      correct_inst->SetLine(std::to_string(in.position().line));


      auto inst = std::make_shared<TupleLength>(lhs, rhs);
      f->instructions.push_back(inst);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
    }
  };


  template<> struct action <call_without_ret> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Var> ret_value = nullptr;
      std::shared_ptr<Item> f_name;
      std::vector<std::shared_ptr<Item>> operands;
      for(size_t i = 0; i < parsed_items.size(); i++) {
        if(i == 0) {
          f_name = parsed_items[i];
        } else {
          operands.push_back(parsed_items[i]);
        }
      }
      parsed_items.clear();
      auto f = p.functions.back();
      auto f_name_temp = f_name->ToString();
      f_name_temp = f_name_temp.substr(1);
      if(f->variables_mapper.count(f_name_temp)) {
        f_name = std::make_shared<Code>(f_name_temp);
        f->variables_mapper[f_name_temp] = f_name;
      }
      std::string temp_name = "p";
      static int var_counter = 0;
      for(auto [name, _] : f->variables_mapper) {
        if(name.size() > temp_name.size()) {
          temp_name = name;
        }
      }

      for(int i = 0; i < operands.size(); i++) {
        //encoding const number
        if(typeid(*operands[i]) == typeid(Number)) {
          auto orig_var = operands[i];
          auto variable_name = temp_name + std::to_string(var_counter++);
          operands[i] = std::make_shared<Int64>(variable_name);
          f->variables_mapper[variable_name] = operands[i];
          std::dynamic_pointer_cast<Var>(operands[i])->SetFunction(f);
          auto lshift = std::make_shared<Operator>("<<");
          auto add = std::make_shared<Operator>("+");
          auto one = std::make_shared<Number>("1");
          auto inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(operands[i]), orig_var, lshift, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));

          inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(operands[i]), operands[i], add, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));
        }
      }


      auto inst = std::make_shared<CallInst>(ret_value, f_name, operands);
      f->instructions.push_back(inst);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
    }
  };

  template<> struct action <call_with_ret> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Var> ret_value;
      std::shared_ptr<Item> f_name;
      std::vector<std::shared_ptr<Item>> operands;
      for(size_t i = 0; i < parsed_items.size(); i++) {
        if(i == 0) {
          ret_value = std::dynamic_pointer_cast<Var>(parsed_items[i]);
        } else if(i == 1) {
          f_name = parsed_items[i];
        } else {
          operands.push_back(parsed_items[i]);
        }
      }
      parsed_items.clear();
      auto f = p.functions.back();
      auto f_name_temp = f_name->ToString();
      f_name_temp = f_name_temp.substr(1);
      if(f->variables_mapper.count(f_name_temp)) {
        f_name = std::make_shared<Code>(f_name_temp);
        f->variables_mapper[f_name_temp] = f_name;
      }
      std::string temp_name = "p";
      static int var_counter = 0;
      for(auto [name, _] : f->variables_mapper) {
        if(name.size() > temp_name.size()) {
          temp_name = name;
        }
      }

      for(int i = 0; i < operands.size(); i++) {
        //encoding const number
        if(typeid(*operands[i]) == typeid(Number)) {
          auto orig_var = operands[i];
          auto variable_name = temp_name + std::to_string(var_counter++);
          operands[i] = std::make_shared<Int64>(variable_name);
          f->variables_mapper[variable_name] = std::dynamic_pointer_cast<Var>(operands[i]);
          std::dynamic_pointer_cast<Var>(operands[i])->SetFunction(f);  
          auto lshift = std::make_shared<Operator>("<<");
          auto add = std::make_shared<Operator>("+");
          auto one = std::make_shared<Number>("1");
          auto inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(operands[i]), orig_var, lshift, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));

          inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(operands[i]), operands[i], add, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));
        }
      }

      auto inst = std::make_shared<CallInst>(ret_value, f_name, operands);
      f->instructions.push_back(inst);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
    }
  };

  template<> struct action <array_allocate_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Array> lhs;
      std::vector<std::shared_ptr<Item>> args;
      for(size_t i = 0; i < parsed_items.size(); i++) {
        if(i == 0) {
          lhs = std::dynamic_pointer_cast<Array>(parsed_items[i]);
        } else {
          args.push_back(parsed_items[i]);
          lhs->SetDim(args.size() - 1, parsed_items[i]);
        }
      }
      parsed_items.clear();
      auto f = p.functions.back();
      std::string temp_name = "p";
      static int var_counter = 0;
      for(auto [name, _] : f->variables_mapper) {
        if(name.size() > temp_name.size()) {
          temp_name = name;
        }
      }

      for(int i = 0; i < args.size(); i++) {
        //encoding const number
        if(typeid(*args[i]) == typeid(Number)) {
          auto orig_var = args[i];
          auto variable_name = temp_name + std::to_string(var_counter++);
          args[i] = std::make_shared<Int64>(variable_name);
          f->variables_mapper[variable_name] = std::dynamic_pointer_cast<Var>(args[i]);
          std::dynamic_pointer_cast<Var>(args[i])->SetFunction(f);
          auto lshift = std::make_shared<Operator>("<<");
          auto add = std::make_shared<Operator>("+");
          auto one = std::make_shared<Number>("1");
          auto inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(args[i]), orig_var, lshift, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));

          inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(args[i]), args[i], add, one);
          f->instructions.push_back(inst);
          inst->SetParent(f);
          inst->SetLine(std::to_string(in.position().line));
        }
      }


      auto inst = std::make_shared<ArrayAllocate>(lhs, args);
      f->instructions.push_back(inst);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
    }
  };

  template<> struct action <tuple_allocate_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Tuple> lhs;
      std::shared_ptr<Item> dim;

      dim = parsed_items.back();
      parsed_items.pop_back();
      lhs = std::dynamic_pointer_cast<Tuple>(parsed_items.back());
      parsed_items.pop_back();
      lhs->SetLength(dim);

      auto f = p.functions.back();
      std::string temp_name = "p";
      static int var_counter = 0;
      for(auto [name, _] : f->variables_mapper) {
        if(name.size() > temp_name.size()) {
          temp_name = name;
        }
      }
      //encoding
      if(typeid(*dim) == typeid(Number)) {
        auto orig_var = dim;
        auto variable_name = temp_name + std::to_string(var_counter++);
        dim = std::make_shared<Int64>(variable_name);
        f->variables_mapper[variable_name] = dim;
        std::dynamic_pointer_cast<Var>(dim)->SetFunction(f);
        auto lshift = std::make_shared<Operator>("<<");
        auto add = std::make_shared<Operator>("+");
        auto one = std::make_shared<Number>("1");
        auto inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(dim), orig_var, lshift, one);
        f->instructions.push_back(inst);
        inst->SetParent(f);
        inst->SetLine(std::to_string(in.position().line));

        inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(dim), dim, add, one);
        f->instructions.push_back(inst);
        inst->SetParent(f);
        inst->SetLine(std::to_string(in.position().line));
      }
      
      auto inst = std::make_shared<TupleAllocate>(lhs, dim);
      f->instructions.push_back(inst);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
    }
  };

  template<> struct action <branch_condition_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Item> mark;
      std::vector<std::shared_ptr<Label>> labels;

      labels.push_back(std::dynamic_pointer_cast<Label>(parsed_items.back()));
      parsed_items.pop_back();
      labels.push_back(std::dynamic_pointer_cast<Label>(parsed_items.back()));
      parsed_items.pop_back();
      std::reverse(labels.begin(), labels.end());
      mark = parsed_items.back();
      parsed_items.pop_back();

      auto f = p.functions.back();
      std::string temp_name = "p";
      static int var_counter = 0;
      for(auto [name, _] : f->variables_mapper) {
        if(name.size() > temp_name.size()) {
          temp_name = name;
        }
      }
      if(typeid(*mark) != typeid(Number)) {
        auto ori_operand1 = mark;
        mark = std::dynamic_pointer_cast<Var>(mark)->Copy(
          temp_name + std::to_string(var_counter++)
        );
        auto rshift = std::make_shared<Operator>(">>");
        auto one = std::make_shared<Number>("1");
        auto inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(mark), ori_operand1, rshift, one);
        f->instructions.push_back(inst);
        inst->SetParent(f);
        inst->SetLine(std::to_string(in.position().line));
      }

      auto inst = std::make_shared<BranchInst>(mark, labels);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
      f->instructions.push_back(inst);
    }
  };

  template<> struct action <branch_goto_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Item> mark;
      std::vector<std::shared_ptr<Label>> labels;
      labels.push_back(std::dynamic_pointer_cast<Label>(parsed_items.back()));
      parsed_items.pop_back();
      mark = nullptr;

      auto f = p.functions.back();      
      auto inst = std::make_shared<BranchInst>(mark, labels);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
      f->instructions.push_back(inst);
    }
  };

  template<> struct action <return_with_var> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Item>ret_value = parsed_items.back();
      parsed_items.pop_back();
      auto f = p.functions.back();
      std::string temp_name = "p";
      static int var_counter = 0;
      for(auto [name, _] : f->variables_mapper) {
        if(name.size() > temp_name.size()) {
          temp_name = name;
        }
      }
      //encoding
      if(typeid(*ret_value) == typeid(Number)) {
        auto orig_var = ret_value;
        auto variable_name = temp_name + std::to_string(var_counter++);
        ret_value = std::make_shared<Int64>(variable_name);
        f->variables_mapper[variable_name] = std::dynamic_pointer_cast<Var>(ret_value);
        std::dynamic_pointer_cast<Var>(ret_value)->SetFunction(f);
        auto lshift = std::make_shared<Operator>("<<");
        auto add = std::make_shared<Operator>("+");
        auto one = std::make_shared<Number>("1");
        auto inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(ret_value), orig_var, lshift, one);
        f->instructions.push_back(inst);
        inst->SetParent(f);
        inst->SetLine(std::to_string(in.position().line));

        inst = std::make_shared<BinaryOperator>(std::dynamic_pointer_cast<Var>(ret_value), ret_value, add, one);
        f->instructions.push_back(inst);
        inst->SetParent(f);
        inst->SetLine(std::to_string(in.position().line));
      }
      auto inst = std::make_shared<ReturnInst>(ret_value);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
      f->instructions.push_back(inst);
    }
  };

  template<> struct action <return_without_var> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Item>ret_value = nullptr;
      auto f = p.functions.back();
      auto inst = std::make_shared<ReturnInst>(ret_value);
      inst->SetParent(f);
      inst->SetLine(std::to_string(in.position().line));
      f->instructions.push_back(inst);
    }
  };

  template<> struct action <return_type_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Function> f(new Function);
      //f->parent = std::shared_ptr<Program>(&p);
      f->ret = in.string();
      p.functions.push_back(f);
    }
  };

  template<> struct action <f_name_declare> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto f = p.functions.back();
      f->function_name = std::make_shared<FunctionName>(in.string());
    }
  };

  template<> struct action <int64_arg_delcare> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto variable_name = declared_vars.back();
      declared_vars.pop_back();
      auto var = std::make_shared<Int64>(variable_name);
      auto f = p.functions.back();
      f->args.push_back(var);
      f->variables_mapper[variable_name] = var;
      var->SetFunction(f);
    }
  };

  template<> struct action <code_arg_declare> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto variable_name = declared_vars.back();
      declared_vars.pop_back();
      auto var = std::make_shared<Code>(variable_name);
      auto f = p.functions.back();
      f->args.push_back(var);
      f->variables_mapper[variable_name] = var;
      var->SetFunction(f);
    }
  };

   template<> struct action <tuple_arg_declare> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto variable_name = declared_vars.back();
      declared_vars.pop_back();
      auto var = std::make_shared<Tuple>(variable_name);
      auto f = p.functions.back();
      f->args.push_back(var);
      f->variables_mapper[variable_name] = var;
      var->SetFunction(f);
    }
  };

  template<> struct action <array_arg_declare> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto variable_name = declared_vars.back();
      declared_vars.pop_back();
      auto dim = square_bracket_rule::pair_number;
      square_bracket_rule::pair_number = 0;
      auto var = std::make_shared<Array>(variable_name, dim);
      auto f = p.functions.back();
      f->args.push_back(var);
      f->variables_mapper[variable_name] = var;
      var->SetFunction(f);
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