#pragma once
#include "parser.hpp"
#include <string>
#include <array>

using namespace std;

class Compiler {
    public:
    // reference counts of each register
    array<int, 8> registers;
    vector<string> asm_code;
    vector<string> asm_data;
    void compile_program(string program);
    // allocates or frees registers with reference counting
    // throws std::runtime_error if all registers are in use
    int rc_ralloc();
    void rc_keep_ref(int r_dest);
    void rc_free_ref(int r_dest);
    bool all_registers_free() const;
    Compiler();
    //private:
    int load_value(Token* program);
    int compile_one(Token* program);
    int expr_define(Token* program);
    int expr_set(Token* program);
    int expr_not(Token* program);
    int expr_sub(Token* program);
    int expr_sub1(Token* program);
    int expr_add(Token* program);
    int expr_add1(Token* program);
    int expr_begin(Token* program);
    int expr_addr(Token* program);
    int expr_deref(Token* program);
    int expr_equals(Token* program);
    int last_label = 0;
    string next_label();
    string rtos(int r) const;
    void emit(string code, Token* program);
    void concat(vector<string> & a, vector<string> c) {
        vector<string> b = c;
        a.insert(a.end(), b.begin(), b.end());
    }
    string toString() const;
};