#pragma once
#include "parser.hpp"
#include "runtime.hpp"
#include <string>
#include <array>

using namespace std;

static int last_label = 0;

class Compiler {
    public:
    Compiler();
    // reference counts of each register
    // -1 means reserved
    // r5, r6, r7 start as reserved, so there are actually
    // 8 - 3 = 5 available registers to use.
    array<int, 8> registers;

    // compiles a program represented as a string
    // multiple s-expressions are ok
    void compile_program(string program);

    // if any register has a positive reference count
    // this function will return true.
    bool all_registers_free() const;

    // exports all emitted assembly code as a string
    string toString() const;

    private:
    vector<string> asm_code;
    vector<string> asm_fn;
    vector<string> asm_data;
    int rc_ralloc();
    void rc_keep_ref(int r_dest);
    void rc_free_ref(int r_dest);
    int load_value(Token* program);
    int load_value_stack(Token* program);
    int compile_one(Token* program);
    int expr_define(Token* program);
    int expr_define_stack(Token* program);
    int expr_define_fn(Token* program);
    int expr_set(Token* program);
    int expr_void(Token* program);
    int expr_write(Token* program);
    int expr_call(Token* program);
    int expr_not(Token* program);
    int expr_sub(Token* program);
    int expr_sub1(Token* program);
    int expr_add(Token* program);
    int expr_add1(Token* program);
    int expr_begin(Token* program);
    int expr_addr(Token* program);
    int expr_deref(Token* program);
    int expr_equals(Token* program);
    int expr_less_than(Token* program);
    int expr_greater_than(Token* program);
    int expr_if(Token* program);
    int expr_while(Token* program);
    int expr_for(Token* program);
    void stack_define(string name, int size);
    void stack_shrink(int size);
    string next_label();
    string rtos(int r) const;
    void emit(string code, Token* program);
    void emit(string code, string debug);
    int _ralloc_return_val = -1;
    void ralloc_force_return(int r_dest);
    RuntimeStack stack;
    FunctionTable ft;
};