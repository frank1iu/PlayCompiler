#pragma once
#include "parser.hpp"
#include <string>
#include <array>

using namespace std;

class Compiler {
    public:
    // reference counts of each register
    array<int, 8> registers;
    vector<string> compile_program(Token* program);
    // allocates or frees registers with reference counting
    // throws std::runtime_error if all registers are in use
    int rc_ralloc();
    int rc_keep_ref(int r_dest);
    int rc_free_ref(int r_dest);
    bool all_registers_free() const;
    Compiler();
    //private:
    bool static_data_phase = false;
    vector<string> eval(Token* program, int* r_dest);
    vector<string> load_into_register(Token* program, int* r_dest);
    vector<string> load_addr(string name, int* r_dest);
    vector<string> define_static(string name, int len, int* init_val);
    vector<string> store_value(string name, int r);
    vector<string> expr_not(Token* program, int* r_dest);
    vector<string> expr_sub(Token* program, int* r_dest);
    vector<string> expr_add(Token* program, int* r_dest);
    vector<string> expr_add1(Token* program, int* r_dest);
    //vector<string> expr_while(Token* program);
    int last_label = 0;
    string next_label() {
        return "L" + to_string(last_label++);
    }
    void concat(vector<string> & a, vector<string> c) {
        vector<string> b = c;
        a.insert(a.end(), b.begin(), b.end());
    }
};