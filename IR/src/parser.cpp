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
#include "IR.hpp"
#include "tao/pegtl/ascii.hpp"
#include "utils.hpp"



namespace IR {

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

  struct variable_declare_rule : variable {};

  struct variable_use_rule : variable {};

  struct label : 
    pegtl::seq<
      pegtl::one<':'>,
      name
    > {};

  struct label_rule : label {};

  struct basic_block_name : label {};

  struct f_name :
    pegtl::seq<
      pegtl::one<'@'>,
      name
    > {};

  struct f_name_rule : f_name {};

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
      TAOCPP_PEGTL_STRING("call"),
      seps,
      pegtl::sor<
        r_name_rule,
        f_name_rule,
        variable_use_rule
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
      TAOCPP_PEGTL_STRING("call"),
      seps,
      pegtl::sor<
        r_name_rule,
        f_name_rule,
        variable_use_rule
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
      pegtl::seq<pegtl::at<array_declare_rule>, array_declare_rule>,
      pegtl::seq<pegtl::at<int64_declare_rule>, int64_declare_rule>,
      pegtl::seq<pegtl::at<tuple_declare_rule>, tuple_declare_rule>,
      pegtl::seq<pegtl::at<code_declare_rule>, code_declare_rule>, 
      pegtl::seq<pegtl::at<store_container_rule>, store_container_rule>,
      pegtl::seq<pegtl::at<load_container_rule>, load_container_rule>,
      pegtl::seq<pegtl::at<binary_op_rule>, binary_op_rule>,
      pegtl::seq<pegtl::at<assign_inst_rule>, assign_inst_rule>,
      pegtl::seq<pegtl::at<array_length_rule>, array_length_rule>,
      pegtl::seq<pegtl::at<tuple_length_rule>, tuple_length_rule>,
      pegtl::seq<pegtl::at<call_with_ret>, call_with_ret>,
      pegtl::seq<pegtl::at<call_without_ret>, call_without_ret>,
      pegtl::seq<pegtl::at<array_allocate_rule>, array_allocate_rule>,
      pegtl::seq<pegtl::at<tuple_allocate_rule>, tuple_allocate_rule>
    > {};

  struct instructions_rule :
    pegtl::star <
      pegtl::seq<
        seps,
        instruction_rule,
        seps
      >
    > {};

  struct terminator_rule :
    pegtl::sor<
      pegtl::seq<pegtl::at<branch_condition_rule>, branch_condition_rule>,
      pegtl::seq<pegtl::at<branch_goto_rule>, branch_goto_rule>,
      pegtl::seq<pegtl::at<return_with_var>, return_with_var>,
      pegtl::seq<pegtl::at<return_without_var>, return_without_var>
    > {};

  struct basic_block_rule :
    pegtl::seq<
      basic_block_name,
      seps,
      instructions_rule,
      seps,
      terminator_rule
    > {};

  struct basic_blocks_rule :
    pegtl::plus<
      seps,
      basic_block_rule,
      seps
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


  struct f_name_declare : f_name {};

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
      TAOCPP_PEGTL_STRING("define"),
      seps,
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
      basic_blocks_rule,
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
      parsed_items.push_back(f->variables_mapper.at(in.string()));
    }
  };

  template<> struct action <label_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Label> label(new Label(in.string()));
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

  template<> struct action <basic_block_name> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto f = p.functions.back();
      auto bb = std::make_shared<BasicBlock>();
      f->basic_blocks.push_back(bb);
      bb->parent = f;
      bb->name = std::make_shared<Label>(in.string());
      f->bbs_mapper[in.string()] = bb;
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
      auto bb = f->basic_blocks.back();
      if(f->basic_blocks.size() != 1) {
        std::cerr << "Must declare variables in first basic block" << std::endl;
        abort();
      }
      f->variables_mapper[var_name] = variable;
      auto inst = std::make_shared<DeclareInst>(variable);
      bb->instructions.push_back(inst);
      inst->SetParent(bb);
      variable->SetFunction(f);
    }
  };

  template<> struct action <code_declare_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto var_name = declared_vars.back();
      declared_vars.pop_back();
      auto variable = std::make_shared<Code>(var_name);
      auto f = p.functions.back();
      auto bb = f->basic_blocks.back();
      if(f->basic_blocks.size() != 1) {
        std::cerr << "Must declare variables in first basic block" << std::endl;
        abort();
      }
      f->variables_mapper[var_name] = variable;
      auto inst = std::make_shared<DeclareInst>(variable);
      bb->instructions.push_back(inst);
      inst->SetParent(bb);
      variable->SetFunction(f);
    }
  };

  template<> struct action <tuple_declare_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      auto var_name = declared_vars.back();
      declared_vars.pop_back();
      auto variable = std::make_shared<Tuple>(var_name);
      auto f = p.functions.back();
      auto bb = f->basic_blocks.back();
      if(f->basic_blocks.size() != 1) {
        std::cerr << "Must declare variables in first basic block" << std::endl;
        abort();
      }
      f->variables_mapper[var_name] = variable;
      auto inst = std::make_shared<DeclareInst>(variable);
      bb->instructions.push_back(inst);
      inst->SetParent(bb);
      variable->SetFunction(f);
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
      auto bb = f->basic_blocks.back();
      if(f->basic_blocks.size() != 1) {
        std::cerr << "Must declare variables in first basic block" << std::endl;
        abort();
      }
      f->variables_mapper[var_name] = variable;
      auto inst = std::make_shared<DeclareInst>(variable);
      bb->instructions.push_back(inst);
      inst->SetParent(bb);
      variable->SetFunction(f);
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
      auto bb = f->basic_blocks.back();
      auto inst = std::make_shared<AssignInst>(lhs, rhs);
      inst->SetParent(bb);
      bb->instructions.push_back(inst);
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
      auto bb = f->basic_blocks.back();
      auto inst = std::make_shared<BinaryOperator>(lhs, operand1, op, operand2);
      inst->SetParent(bb);
      bb->instructions.push_back(inst);
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
      auto bb = f->basic_blocks.back();
      //DEBUG_COUT << typeid(*rhs).name() << std::endl;
      if(typeid(*rhs) == typeid(Tuple)) {
        auto inst = std::make_shared<LoadTuple>(lhs, std::dynamic_pointer_cast<Tuple>(rhs), index[0]);
        inst->SetParent(bb);
        bb->instructions.push_back(inst);
      } else {
        auto inst = std::make_shared<LoadArray>(lhs, std::dynamic_pointer_cast<Array>(rhs), index);
        inst->SetParent(bb);
        bb->instructions.push_back(inst);
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
      auto bb = f->basic_blocks.back();
      //DEBUG_COUT << typeid(*lhs).name() << std::endl;
      if(typeid(*lhs) == typeid(Tuple)) {
        auto inst = std::make_shared<StoreTuple>(std::dynamic_pointer_cast<Tuple>(lhs), index[0], rhs);
        inst->SetParent(bb);
        bb->instructions.push_back(inst);
      } else {
        auto inst = std::make_shared<StoreArray>(std::dynamic_pointer_cast<Array>(lhs), index, rhs);
        inst->SetParent(bb);
        bb->instructions.push_back(inst);
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
      auto bb = f->basic_blocks.back();
      auto inst = std::make_shared<ArrayLength>(lhs, rhs, dim);
      inst->SetParent(bb);
      bb->instructions.push_back(inst);
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
      auto bb = f->basic_blocks.back();
      auto inst = std::make_shared<TupleLength>(lhs, rhs);
      inst->SetParent(bb);
      bb->instructions.push_back(inst);
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
      auto bb = f->basic_blocks.back();
      auto inst = std::make_shared<CallInst>(ret_value, f_name, operands);
      inst->SetParent(bb);
      bb->instructions.push_back(inst);
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
      auto bb = f->basic_blocks.back();
      auto inst = std::make_shared<CallInst>(ret_value, f_name, operands);
      inst->SetParent(bb);
      bb->instructions.push_back(inst);
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
      auto bb = f->basic_blocks.back();
      auto inst = std::make_shared<ArrayAllocate>(lhs, args);
      inst->SetParent(bb);
      bb->instructions.push_back(inst);
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
      auto bb = f->basic_blocks.back();
      auto inst = std::make_shared<TupleAllocate>(lhs, dim);
      inst->SetParent(bb);
      bb->instructions.push_back(inst);
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
      auto bb = f->basic_blocks.back();
      auto inst = std::make_shared<BranchInst>(mark, labels);
      inst->SetParent(bb);
      bb->terminator = inst;
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
      auto bb = f->basic_blocks.back();
      auto inst = std::make_shared<BranchInst>(mark, labels);
      inst->SetParent(bb);
      bb->terminator = inst;
    }
  };

  template<> struct action <return_with_var> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Item>ret_value = parsed_items.back();
      parsed_items.pop_back();
      auto f = p.functions.back();
      auto bb = f->basic_blocks.back();
      auto inst = std::make_shared<ReturnInst>(ret_value);
      inst->SetParent(bb);
      bb->terminator = inst;
    }
  };

  template<> struct action <return_without_var> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Item>ret_value = nullptr;
      auto f = p.functions.back();
      auto bb = f->basic_blocks.back();
      auto inst = std::make_shared<ReturnInst>(ret_value);
      inst->SetParent(bb);
      bb->terminator = inst;
    }
  };

  template<> struct action <return_type_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      std::shared_ptr<Function> f(new Function);
      f->parent = std::shared_ptr<Program>(&p);
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