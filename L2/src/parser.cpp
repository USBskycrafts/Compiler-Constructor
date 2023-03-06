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

#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include "parser.hpp"
#include "printer.hpp"
#include "tao/pegtl/internal/pegtl_string.hpp"
#include "utils.hpp"
#include "L2.hpp"
#include "utils.hpp"

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;

namespace L2 {
  std::vector<Item*> parsed_items;
  std::string spill_prefix;
  Variable* spill_variable;

  /*
   * Keywords
  */
  struct str_return : TAOCPP_PEGTL_STRING("return"){};
  struct str_arrow : pegtl::seq<TAOCPP_PEGTL_STRING("<-")>{};
  struct str_call : TAOCPP_PEGTL_STRING("call"){};
  struct str_mem : TAOCPP_PEGTL_STRING("mem"){};
  struct str_cjump : TAOCPP_PEGTL_STRING("cjump"){};
  struct str_goto : TAOCPP_PEGTL_STRING("goto"){};
  struct str_stack_arg : TAOCPP_PEGTL_STRING("stack-arg"){};
  /*
   * Runtime functions
  */
  struct str_print : TAOCPP_PEGTL_STRING("print"){};
  struct str_input : TAOCPP_PEGTL_STRING("input"){};
  struct str_allocate : TAOCPP_PEGTL_STRING("allocate"){};
  struct str_tensor_error : TAOCPP_PEGTL_STRING("tensor-error"){};

  /*
   * Registers
  */

  struct str_rdi : TAOCPP_PEGTL_STRING("rdi"){};
  struct str_rsi : TAOCPP_PEGTL_STRING("rsi"){};
  struct str_rdx : TAOCPP_PEGTL_STRING("rdx"){};
  struct str_rcx : TAOCPP_PEGTL_STRING("rcx"){};
  struct str_r8 : TAOCPP_PEGTL_STRING("r8"){};
  struct str_r9 : TAOCPP_PEGTL_STRING("r9"){};
  struct str_rax : TAOCPP_PEGTL_STRING("rax"){};
  struct str_rbx : TAOCPP_PEGTL_STRING("rbx"){};
  struct str_rbp : TAOCPP_PEGTL_STRING("rbp"){};
  struct str_r10 : TAOCPP_PEGTL_STRING("r10"){};
  struct str_r11 : TAOCPP_PEGTL_STRING("r11"){};
  struct str_r12 : TAOCPP_PEGTL_STRING("r12"){};
  struct str_r13 : TAOCPP_PEGTL_STRING("r13"){};
  struct str_r14 : TAOCPP_PEGTL_STRING("r14"){};
  struct str_r15 : TAOCPP_PEGTL_STRING("r15"){};
  struct str_rsp : TAOCPP_PEGTL_STRING("rsp"){};

  /*
   * Operators
  */

  struct str_add : TAOCPP_PEGTL_STRING("+="){};
  struct str_sub : TAOCPP_PEGTL_STRING("-="){};
  struct str_mul : TAOCPP_PEGTL_STRING("*="){};
  struct str_and : TAOCPP_PEGTL_STRING("&="){};
  struct str_lshift : pegtl::seq<TAOCPP_PEGTL_STRING("<<=")>{};
  struct str_rshift : pegtl::seq<TAOCPP_PEGTL_STRING(">>=")>{};
  struct str_inc : TAOCPP_PEGTL_STRING("++"){};
  struct str_dec : TAOCPP_PEGTL_STRING("--"){};
  struct str_eq : TAOCPP_PEGTL_STRING("="){};
  struct str_le : pegtl::seq<TAOCPP_PEGTL_STRING("<=")>{};
  struct str_l : pegtl::seq<TAOCPP_PEGTL_STRING("<")>{};
  /*
   * Modules
  */

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

  struct label:
    pegtl::seq<
      pegtl::one<':'>,
      name
    > {};
  struct function_name:
    pegtl::seq<
      pegtl::one<'@'>,
      name
    > {};

  struct runtime_function_name:
    pegtl::sor<
      str_input,
      str_print,
      str_allocate,
      str_tensor_error
    > {};

  struct register_name:
    pegtl::sor<
      str_rdi, str_rsi, str_rdx, str_rcx, str_r8, str_r9,
      str_rax, str_rbx, str_rbp, str_r10, str_r11, str_r12, str_r13, str_r14, str_r15, str_rsp
    > {};

  struct var_name: 
    pegtl::seq<
      pegtl::one<'%'>,
      name
    > {};

  struct number:
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

  struct operator_expression:
    pegtl::sor<
      str_add, str_sub, str_mul, str_and,
      str_lshift, str_rshift,
      str_eq, str_le, str_l
    > {};
  
  struct self_operator_expression:
    pegtl::sor<
      str_inc, str_dec
    > {};

  /*
   * Item rules
  */

  struct register_rule:
    pegtl::seq<
      register_name
    > {};

  struct var_rule:
    pegtl::seq<
      var_name
    > {};

  struct number_rule:
    pegtl::seq<
      number
    > {};

  struct argument_number_rule:
    pegtl::seq<
      number
    > {};

  struct local_number_rule:
    pegtl::seq<
      number
    > {};

  struct memory_address_rule:
    pegtl::seq<
      str_mem,
      seps,
      pegtl::sor<
        register_rule,
        var_rule
      >,
      seps,
      number_rule
    > {};
  
  struct operator_rule:
    pegtl::seq<
      operator_expression
    > {};

  struct self_operator_rule: self_operator_expression{};

  struct compare_rule:
    pegtl::seq<
      pegtl::sor<
        register_rule,
        var_rule,
        number_rule
      >,
      seps,
      operator_rule,
      seps,
      pegtl::sor<
        register_rule,
        var_rule,
        number_rule
      >
    > {};

  struct function_name_rule:
    pegtl::seq<
      pegtl::sor<
        function_name,
        runtime_function_name
      >
    > {};

  struct label_rule:
    pegtl::seq<
      label
    > {};

  struct function_call_rule:
    pegtl::seq<
      pegtl::sor<
        function_name_rule,
        register_rule,
        var_rule
      >
    > {};

  struct stack_arg_rule:
    pegtl::seq<
      str_stack_arg,
      seps,
      number_rule
    > {};

  struct function_declare_rule:
    pegtl::seq<
      function_name
    > {};


  /*
   * Instruction rules
  */
  
  struct instruction_return_rule:
    pegtl::seq<
      str_return
    > {};

  struct instruction_call_rule:
    pegtl::seq<
      str_call,
      seps,
      function_call_rule,
      seps,
      number_rule
    > {};

  struct instruction_assignment_rule:
    pegtl::seq<
      pegtl::sor<
        pegtl::seq<pegtl::at<register_rule>, register_rule>,
        pegtl::seq<pegtl::at<var_rule>, var_rule>,
        pegtl::seq<pegtl::at<memory_address_rule>, memory_address_rule>
      >,
      seps,
      str_arrow,
      seps,
      pegtl::sor<
        pegtl::seq<pegtl::at<compare_rule>, compare_rule>,
        pegtl::seq<pegtl::at<register_rule>, register_rule>,
        pegtl::seq<pegtl::at<var_rule>, var_rule>,
        pegtl::seq<pegtl::at<number_rule>, number_rule>,
        pegtl::seq<pegtl::at<memory_address_rule>, memory_address_rule>,
        pegtl::seq<pegtl::at<label_rule>, label_rule>,
        pegtl::seq<pegtl::at<function_name_rule>, function_name_rule>
      >
    > {};

  struct instruction_label_rule:
    pegtl::seq<
      label_rule
    > {};

  struct instruction_arithmetic_rule:
    pegtl::seq<
      pegtl::sor<
        register_rule,
        var_rule,
        memory_address_rule,
        number_rule
      >,
      seps,
      operator_rule,
      seps,
      pegtl::sor<
        register_rule,
        var_rule,
        memory_address_rule,
        number_rule
      >
    > {};

  struct instruction_self_operation_rule:
    pegtl::seq<
      pegtl::sor<
        register_rule,
        var_rule
      >,
      seps,
      self_operator_rule
    > {};

  struct instruction_conditional_jump_rule:
    pegtl::seq<
      str_cjump,
      seps,
      compare_rule,
      seps,
      label_rule
    > {};

  struct instruction_jump_rule:
    pegtl::seq<
      str_goto,
      seps,
      label_rule
    > {};

  struct instruction_leaq_rule:
    pegtl::seq<
      pegtl::sor<
        register_rule,
        var_rule
      >,
      seps,
      pegtl::one<'@'>,
      seps,
      pegtl::sor<
        register_rule,
        var_rule
      >,
      seps,
      pegtl::sor<
        register_rule,
        var_rule
      >,
      seps,
      number_rule
    > {};

  struct instruction_stack_arg_rule:
    pegtl::seq<
      pegtl::sor<
        register_rule,
        var_rule
      >,
      seps,
      str_arrow,
      seps,
      stack_arg_rule
    > {};

  /*
   * Instruction collection
  */

  struct instruction_rule:
    pegtl::sor<
      pegtl::seq<pegtl::at<instruction_return_rule>, instruction_return_rule>,
      pegtl::seq<pegtl::at<instruction_call_rule>, instruction_call_rule>,
      pegtl::seq<pegtl::at<instruction_assignment_rule>, instruction_assignment_rule>,
      pegtl::seq<pegtl::at<instruction_label_rule>, instruction_label_rule>,
      pegtl::seq<pegtl::at<instruction_arithmetic_rule>, instruction_arithmetic_rule>,
      pegtl::seq<pegtl::at<instruction_self_operation_rule>, instruction_self_operation_rule>,
      pegtl::seq<pegtl::at<instruction_conditional_jump_rule>, instruction_conditional_jump_rule>,
      pegtl::seq<pegtl::at<instruction_jump_rule>, instruction_jump_rule>,
      pegtl::seq<pegtl::at<instruction_leaq_rule>, instruction_leaq_rule>,
      pegtl::seq<pegtl::at<instruction_stack_arg_rule>, instruction_stack_arg_rule>
    > {};

  struct instructions_rule:
    pegtl::plus<
      pegtl::seq<
        seps,
        instruction_rule,
        seps
      >
    > {};

  /*
  * function rules
  */

  struct function_rule:
    pegtl::seq<
      seps,
      pegtl::one<'('>,
      seps,
      function_declare_rule,
      seps,
      argument_number_rule,
      seps,
      pegtl::opt<
        local_number_rule
      >,
      instructions_rule,
      seps,
      pegtl::one<')'>
    > {};

  struct functions_rule:
    pegtl::plus<
      pegtl::seq<
        seps,
        function_rule,
        seps
      >
    > {};

  struct entry_point_rule : function_name {};

  struct program_entry_rule:
    pegtl::seq<
      seps,
      pegtl::one<'('>,
      seps,
      entry_point_rule,
      seps,
      functions_rule,
      seps,
      pegtl::one<')'>,
      seps
    > {};

  struct function_entry_rule : function_rule {};

  struct prefix_rule : 
  pegtl::seq<
    var_name
  > {};

  struct spill_variable_rule : 
  pegtl::seq<
    var_name
  > {};

  struct spill_entry_rule : 
    pegtl::seq<
      function_entry_rule,
      seps,
      spill_variable_rule,
      seps,
      prefix_rule
    > {};

  struct program_grammar :
    pegtl::must<
      program_entry_rule
    > {};

  struct function_grammar :
    pegtl::must<
      function_entry_rule
    > {};
  
  struct spill_grammar :
    pegtl::must<
      spill_entry_rule
    > {};
  

  /*
  * variable rules
  */

  template<typename Rule>
  struct action : pegtl::nothing<Rule> {};


  /*
  * DEBUG rules
  */

  template<> struct action<program_entry_rule> {
    template<typename Input>
    static void apply(const Input&in, Program& p) {
      DEBUG_COUT << in.string() << std::endl;
    }
  };
  /*
   * Item rules
  */
  
  
  template<> struct action<register_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      DEBUG_PRINT("register: %s", in.string().c_str());
      auto f = p.functions.back();
      auto item = Register::GetInstance(in.string());
      parsed_items.push_back(item);
      DEBUG_PRINT("register is added");
    }
  };

  template<> struct action<var_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      DEBUG_PRINT("var: %s", in.string().c_str());
      auto f = p.functions.back();
      auto item = Variable::GetInstance(in.string());
      parsed_items.push_back(item);
    }
  };

  template<> struct action<number_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      DEBUG_PRINT("number: %s", in.string().c_str());
      auto f = p.functions.back();
      auto n = new Number(in.string());
      parsed_items.push_back(n);
      DEBUG_PRINT("number is added");
    }
  };

  template<> struct action<memory_address_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
    
      auto n = (Number*)parsed_items.back();
      parsed_items.pop_back();
      auto r = (Register*)parsed_items.back();
      parsed_items.pop_back();

      auto m = new MemoryAddress(r, n);
      parsed_items.push_back(m);
    }
  };

  template<> struct action<operator_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
      auto o = new Operator(in.string());
      parsed_items.push_back(o);
    }
  };

  template<> struct action<self_operator_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
      auto o = new Operator(in.string());
      parsed_items.push_back(o);
    }
  };

  template<> struct action<compare_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
      

      auto i2 = parsed_items.back();
      parsed_items.pop_back();
      auto o = (Operator*)parsed_items.back();
      parsed_items.pop_back();
      auto i1 = parsed_items.back();
      parsed_items.pop_back();

      DEBUG_PRINT("%s %s %s", i2->get_code().c_str(), o->get_code().c_str(), i1->get_code().c_str());
      auto c = new Comparison(i1, o, i2);
      parsed_items.push_back(c);
    }
  
  };

  template<> struct action<label_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
      auto l = new Label(in.string());
      parsed_items.push_back(l);
    }
  };

  template<> struct action <function_name_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
      auto l = new FunctionName(in.string());
      parsed_items.push_back(l);
    }
  };



  
  /*
   * Instruction rules
  */

  template<> struct action <instruction_return_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
      f->instructions.push_back(new InstructionReturn());
    }
  };

  template<> struct action <instruction_call_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
      auto n = (Number*)parsed_items.back();
      parsed_items.pop_back();
      auto i = parsed_items.back();
      parsed_items.pop_back();
      f->instructions.push_back(new InstructionCall(i, n));
    }
  };

  template<> struct action <instruction_assignment_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f =p.functions.back();
      auto src = parsed_items.back();
      parsed_items.pop_back();
      auto dst = parsed_items.back();
      parsed_items.pop_back();
      f->instructions.push_back(new InstructionAssignment(dst, src));
    }
  };

  template<> struct action <instruction_label_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
      auto l = (Label*)parsed_items.back();
      parsed_items.pop_back();
      f->instructions.push_back(new InstructionLabel(l));
    }
  };

  template<> struct action <instruction_arithmetic_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
      auto i2 = parsed_items.back();
      parsed_items.pop_back();
      auto o = (Operator*)parsed_items.back();
      parsed_items.pop_back();
      auto i1 = parsed_items.back();
      parsed_items.pop_back();
      f->instructions.push_back(new InstructionArithmetic(i1, o, i2));
    }
  };

  template<> struct action <instruction_self_operation_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
      auto o = (Operator*)parsed_items.back();
      parsed_items.pop_back();
      auto i = parsed_items.back();
      parsed_items.pop_back();
      f->instructions.push_back(new InstructionSelfOperation(i, o));
    }
  };

  template<> struct action <instruction_conditional_jump_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
      auto l = (Label*)parsed_items.back();
      parsed_items.pop_back();
      auto c = (Comparison*)parsed_items.back();
      parsed_items.pop_back();
      f->instructions.push_back(new InstructionConditionalJump(c, l));
    }
  };

  template<> struct action <instruction_jump_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
      auto l = (Label*)parsed_items.back();
      parsed_items.pop_back();
      f->instructions.push_back(new InstructionJump(l));
    }
  };

  template<> struct action <instruction_leaq_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
      auto n = (Number*)parsed_items.back();
      parsed_items.pop_back();
      auto i3 = parsed_items.back();
      parsed_items.pop_back();
      auto i2 = parsed_items.back();
      parsed_items.pop_back();
      auto i1 = parsed_items.back();
      parsed_items.pop_back();
      f->instructions.push_back(new InstructionLeaq(i1, i2, i3, n));
    }
  };


  template<> struct action <instruction_stack_arg_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
      auto n = (Number*)parsed_items.back();
      parsed_items.pop_back();
      auto i = parsed_items.back();
      parsed_items.pop_back();
      f->instructions.push_back(new InstructionStackArg(i, n));
    }
  };

  


  /*
   * Function and entry point rules
  */


  template<> struct action <function_declare_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = new Function();
      f->name = in.string();
      p.functions.push_back(f);
    }
  };

  template<> struct action<argument_number_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
      f->arguments = std::stoll(in.string());
      f->locals = 0;
    }
  };

  
  template<> struct action<local_number_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      auto f = p.functions.back();
      f->arguments = std::stoll(in.string());
    }
  };


  template<> struct action<entry_point_rule> {
    template<typename Input>
    static void apply(const Input& in, Program &p) {
      p.entryPointLabel = in.string();
    }
  };

  /*
  * test rules
  */

  template<> struct action<prefix_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      spill_prefix = in.string();
    }
  };

  template<> struct action<spill_variable_rule> {
    template<typename Input>
    static void apply(const Input& in, Program& p) {
      spill_variable = Variable::GetInstance(in.string());
    }
  };




  Program parse_file(char *fileName) {
    pegtl::analyze<program_entry_rule>();
    file_input<> fileInput(fileName);
    Program p;
    parse<program_grammar, action>(fileInput, p);
    DEBUG_PRINT("file parsed\n");
    return p;
  }

  Program parse_function_file(char *fileName) {
    DEBUG_PRINT("parsing the function");
    pegtl::analyze<function_entry_rule>();
    file_input<> fileInput(fileName);
    Program p;
    parse<function_grammar, action>(fileInput, p);
    return p;
  }

  

  SpillInput parse_spill_file(char *fileName) {
    pegtl::analyze<spill_entry_rule>();
    file_input<> fileInput(fileName);
    Program p;
    parse<spill_grammar, action>(fileInput, p);
    SpillInput input {p.functions.back(), spill_prefix, spill_variable};
    return input;
  }

}