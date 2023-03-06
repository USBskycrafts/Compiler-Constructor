/*
 * SUGGESTION FROM THE CC TEAM:
 * double check the order of actions that are fired.
 * You can do this in (at least) two ways:
 * 1) by using gdb adding breakpoints to actions
 * 2) by adding printing statements in each action
 *
 * For 2), we suggest writing the code to make it straightforward to enable/disable all of them 
 * (e.g., assuming shouldIPrint is a global variable
 *    if (shouldIPrint) std::cerr << "MY OUTPUT" << std::endl;
 * )
 */
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

#include "L1.h"
#include "parser.h"

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;

namespace L1 {

  /* 
   * Tokens parsed
   */ 
  std::vector<Item *> parsed_items;

  /* 
   * Grammar rules from now on.
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

  /* 
   * Keywords.
   */
  struct str_return : TAOCPP_PEGTL_STRING("return") {};
  struct str_arrow : TAOCPP_PEGTL_STRING("<-") {};
  struct str_call : TAOCPP_PEGTL_STRING("call") {};
  struct str_mem : TAOCPP_PEGTL_STRING("mem") {};
  struct str_cjump : TAOCPP_PEGTL_STRING("cjump") {};
  struct str_goto : TAOCPP_PEGTL_STRING("goto") {};
  struct str_at : TAOCPP_PEGTL_STRING("@") {};

  struct str_print : TAOCPP_PEGTL_STRING("print") {};
  struct str_input : TAOCPP_PEGTL_STRING("input") {};
  struct str_allocate : TAOCPP_PEGTL_STRING("allocate") {};
  struct str_tensor_error : TAOCPP_PEGTL_STRING("tensor-error") {};

  struct str_rdi : TAOCPP_PEGTL_STRING("rdi") {};
  struct str_rsi : TAOCPP_PEGTL_STRING("rsi") {};
  struct str_rdx : TAOCPP_PEGTL_STRING("rdx") {};
  struct str_rcx : TAOCPP_PEGTL_STRING("rcx") {};
  struct str_r8 : TAOCPP_PEGTL_STRING("r8") {};
  struct str_r9 : TAOCPP_PEGTL_STRING("r9") {};
  struct str_rax : TAOCPP_PEGTL_STRING("rax") {};
  struct str_rbx : TAOCPP_PEGTL_STRING("rbx") {};
  struct str_rbp : TAOCPP_PEGTL_STRING("rbp") {};
  struct str_r10 : TAOCPP_PEGTL_STRING("r10") {};
  struct str_r11 : TAOCPP_PEGTL_STRING("r11") {};
  struct str_r12 : TAOCPP_PEGTL_STRING("r12") {};
  struct str_r13 : TAOCPP_PEGTL_STRING("r13") {};
  struct str_r14 : TAOCPP_PEGTL_STRING("r14") {};
  struct str_r15 : TAOCPP_PEGTL_STRING("r15") {};
  struct str_rsp : TAOCPP_PEGTL_STRING("rsp") {};


  struct str_add : TAOCPP_PEGTL_STRING("+=") {};
  struct str_sub : TAOCPP_PEGTL_STRING("-=") {};
  struct str_mul : TAOCPP_PEGTL_STRING("*=") {};
  struct str_and : TAOCPP_PEGTL_STRING("&=") {};
  struct str_lshift : TAOCPP_PEGTL_STRING("<<=") {};
  struct str_rshift : TAOCPP_PEGTL_STRING(">>=") {};
  struct str_inc : TAOCPP_PEGTL_STRING("++") {};
  struct str_dec : TAOCPP_PEGTL_STRING("--") {};
  struct str_le : TAOCPP_PEGTL_STRING("<=") {};
  struct str_less : TAOCPP_PEGTL_STRING("<") {};
  struct str_eq : TAOCPP_PEGTL_STRING("=") {};


  struct operator_rule :
    pegtl::sor<
      str_add,
      str_sub,
      str_mul,
      str_and,
      str_lshift,
      str_rshift
    > {};

  struct cmp_operator_rule :
    pegtl::sor<
      str_le,
      str_less,
      str_eq
    > {};

  struct self_operator_rule :
    pegtl::sor<
      str_inc,
      str_dec
    > {};

  struct label:
    pegtl::seq<
      pegtl::one<':'>,
      name
    > {};

  struct function_name_rule:
    pegtl::seq<
      pegtl::one<'@'>,
      name
    > {};

  struct lib_function_name_rule:
    pegtl::sor<
      str_print,
      str_input,
      str_allocate,
      str_tensor_error
    > {};
  struct register_rule:
    pegtl::sor<
      str_rdi,
      str_rsi,
      str_rdx,
      str_rcx,
      str_r8,
      str_r9,
      str_rax,
      str_rbx,
      str_rbp,
      str_r10,
      str_r11,
      str_r12,
      str_r13,
      str_r14,
      str_r15,
      str_rsp
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
    >{};

  struct comment: 
    pegtl::disable< 
      TAOCPP_PEGTL_STRING( "//" ), 
      pegtl::until< pegtl::eolf > 
    > {};

  struct seps: 
    pegtl::star< 
      pegtl::sor< 
        pegtl::ascii::space, 
        comment 
      > 
    > {};

  struct function_label_rule:
    pegtl::seq <
      pegtl::one<'@'>,
      name
    > {};

  struct argument_number:
    number {};

  struct local_number:
    number {} ;

  struct const_number_rule:
    number {};

  struct label_rule:
    label {};

  struct memory_address_rule:
    pegtl::seq<
      str_mem,
      seps,
      register_rule,
      seps,
      const_number_rule
    > {};

  struct cmp_rule:
    pegtl::seq<
      pegtl::sor<
        register_rule,
        const_number_rule
      >,
      seps,
      cmp_operator_rule,
      seps,
      pegtl::sor<
        register_rule,
        const_number_rule
      >
    > {};

  struct Instruction_arithmetic_operation_rule:
    pegtl::seq<
      pegtl::sor<
        register_rule,
        memory_address_rule
      >, 
      seps,
      operator_rule,
      seps,
      pegtl::sor<
        register_rule,
        memory_address_rule,
        const_number_rule
      >
    > {};

  struct Instruction_return_rule:
    pegtl::seq<
      str_return
    > {};

  struct Instruction_label_rule:
    pegtl::seq<
      seps,
      label_rule,
      seps
    > {};

  struct Instruction_self_operation_rule:
    pegtl::seq<
      register_rule,
      seps,
      self_operator_rule
    > {};

  struct Instruction_assignment_rule:
    pegtl::seq<
      pegtl::sor<
        memory_address_rule,
        register_rule
      >,
      seps,
      str_arrow,
      seps,
      pegtl::sor<
        memory_address_rule,
        pegtl::seq<pegtl::at<cmp_rule>, cmp_rule>,
        pegtl::seq<pegtl::at<register_rule>, register_rule>,
        pegtl::seq<pegtl::at<const_number_rule>, const_number_rule>,
        label_rule,
        function_label_rule
      >
    > {};
  
  struct Instruction_call_rule:
    pegtl::seq<
      str_call,
      seps,
      pegtl::sor<
        function_label_rule,
        lib_function_name_rule,
        register_rule
      >,
      seps,
      const_number_rule
    > {};

  struct Instruction_cjump_rule:
    pegtl::seq<
      str_cjump,
      seps,
      cmp_rule,
      seps,
      label_rule 
    > {};

  struct Instruction_jump_rule:
    pegtl::seq<
      str_goto,
      seps,
      label_rule
    > {};

  struct Instruction_leaq_rule:
    pegtl::seq<
      register_rule,
      seps,
      str_at,
      seps,
      register_rule,
      seps,
      register_rule,
      seps,
      const_number_rule
    > {};

  struct Instruction_rule:
    pegtl::sor<
      pegtl::seq<pegtl::at<Instruction_return_rule>, Instruction_return_rule>,
      pegtl::seq<pegtl::at<Instruction_assignment_rule>, Instruction_assignment_rule>,
      pegtl::seq<pegtl::at<Instruction_call_rule>, Instruction_call_rule>,
      pegtl::seq<pegtl::at<Instruction_arithmetic_operation_rule>, Instruction_arithmetic_operation_rule>,
      pegtl::seq<pegtl::at<Instruction_label_rule>, Instruction_label_rule>,
      pegtl::seq<pegtl::at<Instruction_self_operation_rule>, Instruction_self_operation_rule>,
      pegtl::seq<pegtl::at<Instruction_cjump_rule>, Instruction_cjump_rule>,
      pegtl::seq<pegtl::at<Instruction_jump_rule>, Instruction_jump_rule>,
      pegtl::seq<pegtl::at<Instruction_leaq_rule>, Instruction_leaq_rule>
    > {};

  struct Instructions_rule:
    pegtl::plus<
      pegtl::seq<
        seps,
        Instruction_rule,
        seps
      >
    > {};

  struct Function_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      seps,
      function_name_rule,
      seps,
      argument_number,
      seps,
      local_number,
      seps,
      Instructions_rule,
      seps,
      pegtl::one< ')' >
    > {};

  struct Functions_rule:
    pegtl::plus<
      seps,
      Function_rule,
      seps
    > {};

  struct entry_point_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      seps,
      function_name_rule,
      seps,
      Functions_rule,
      seps,
      pegtl::one< ')' >,
      seps
    > {};

  struct grammar : 
    pegtl::must< 
      entry_point_rule
    > {};

  /* 
   * Actions attached to grammar rules.
   */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < entry_point_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
    }
  };

  template<> struct action < Function_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
    }
  };

  template<> struct action < function_name_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      if (p.entryPointLabel.empty()){
        p.entryPointLabel = in.string();
      } else {
        auto newF = new Function();
        newF->name = in.string();
        p.functions.push_back(newF);
      }
    }
  };

  template<> struct action < function_label_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      parsed_items.push_back(new Function_name(in.string()));
    }
  };

  template<> struct action < lib_function_name_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      parsed_items.push_back(new Function_name(in.string()));
    }
  };

  template<> struct action < argument_number > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->arguments = std::stoll(in.string());
    }
  };

  template<> struct action < local_number > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      currentF->locals = std::stoll(in.string());
    }
  };


  template<> struct action < register_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      auto r = new Register(in.string());
      parsed_items.push_back(r);
    }
  };

  template<> struct action < const_number_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      auto n = new Number(stoll(in.string()));
      parsed_items.push_back(n);
    }
  };

  template<> struct action < operator_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {

      auto o = new Operator(in.string());
      parsed_items.push_back(o);
    }
  };

  template<> struct action < cmp_operator_rule > {
    template<typename Input >
    static void apply( const Input & in, Program & p) {
      auto o = new Operator(in.string());
      parsed_items.push_back(o);
    }
  };

  template<> struct action < self_operator_rule > {
    template< typename Input >
    static void apply( const Input & in, Program &p) {
      auto o = new Operator(in.string());
      parsed_items.push_back(o);
    }
  };

  template<> struct action < label_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      auto l = new Label(in.string());
      parsed_items.push_back(l);
    }
  };

  template<> struct action < memory_address_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      auto n = (Number*)parsed_items.back();
      parsed_items.pop_back();
      auto r = (Register*)parsed_items.back();
      parsed_items.pop_back();

      auto m = new Memory_address(r, n);
      parsed_items.push_back(m);
    }
  };

   
  template<> struct action < cmp_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      auto currentF = p.functions.back();

      auto t2 = parsed_items.back();
      parsed_items.pop_back();
      auto cmp = (Operator*)parsed_items.back();
      parsed_items.pop_back();
      auto t1 = parsed_items.back();
      parsed_items.pop_back();

      parsed_items.push_back(new Cmp_operation(t1, cmp ,t2));
    }
  };


  template<> struct action < Instruction_return_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto i = new Instruction_ret();
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_call_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      auto currentF = p.functions.back();
      auto num = (Number*)parsed_items.back();
      parsed_items.pop_back();
      auto name = (Function_name*)parsed_items.back();
      parsed_items.pop_back();
      auto i = new Instruction_call(name, num);
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_assignment_rule > {
    template< typename Input >
	  static void apply( const Input & in, Program & p){
      auto currentF = p.functions.back();
      auto src = parsed_items.back();
      parsed_items.pop_back();
      auto dst = parsed_items.back();
      parsed_items.pop_back();
      auto i = new Instruction_assignment(dst, src);
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_arithmetic_operation_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {

      auto currentF = p.functions.back();
      auto t2 = parsed_items.back();
      parsed_items.pop_back();
      auto op = (Operator*)parsed_items.back();
      parsed_items.pop_back();
      auto t1 = parsed_items.back();
      parsed_items.pop_back();

      auto i = new Instruction_arithmetic_operation(t1, op, t2);
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_self_operation_rule> {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      auto currentF = p.functions.back();

      auto op = (Operator*)parsed_items.back();
      parsed_items.pop_back();

      auto r = (Register*)parsed_items.back();
      parsed_items.pop_back();

      auto i = new Instruction_self_operation(r, op);
      currentF->instructions.push_back(i);

    }
  };


  template<> struct action < Instruction_label_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {

      auto currentF = p.functions.back();
      
      auto l = (Label*)parsed_items.back();
      parsed_items.pop_back(); 

      auto i = new Instruction_label(l);
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_cjump_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      auto currentF = p.functions.back();

      auto l = (Label*)parsed_items.back();
      parsed_items.pop_back();

      auto c = (Cmp_operation*)parsed_items.back();
      parsed_items.pop_back();

      auto i = new Instruction_conditional_jump_operation(c, l);
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_jump_rule > {
    template< typename Input >
    static void apply( const Input & in, Program & p) {
      auto currentF = p.functions.back();

      auto l = (Label*)parsed_items.back();
      parsed_items.pop_back();

      auto i = new Instruction_jump_operation(l);
      currentF->instructions.push_back(i);
    }
  };

  template<> struct action < Instruction_leaq_rule > {
    template< typename Input >
    static void apply(const Input & in, Program & p) {
      auto currentF = p.functions.back();

      auto n = (Number*)parsed_items.back();
      parsed_items.pop_back();
      auto r3 = (Register*)parsed_items.back();
      parsed_items.pop_back();
      auto r2 = (Register*)parsed_items.back();
      parsed_items.pop_back();
      auto r1 = (Register*)parsed_items.back();
      parsed_items.pop_back();

      auto i = new Instruction_leaq_operation(r1, r2, r3, n);
      currentF->instructions.push_back(i);
    }
  };

  Program parse_file (char *fileName){

    /* 
     * Check the grammar for some possible issues.
     */
    pegtl::analyze< grammar >();

    /*
     * Parse.
     */
    file_input< > fileInput(fileName);
    Program p;
    parse< grammar, action >(fileInput, p);

    return p;
  }

}
