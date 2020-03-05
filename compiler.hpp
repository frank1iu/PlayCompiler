#pragma once
#include "parser.hpp"
#include <string>
#include <array>

using namespace std;

class Compiler {
    public:
    array<int, 8> registers;
    vector<string> define_static(string name, int len, int* init_val);
    vector<string> store_value(string name, int r);
    vector<string> compile_program(Token* program);
    vector<string> load_into_register(Token* program, int* r_dest);
    vector<string> load_addr(string name, int* r_dest);
    vector<string> eval(Token* program, int* r_dest);
    int rc_ralloc();
    int rc_keep_ref(int r_dest);
    int rc_free_ref(int r_dest);
    Compiler();
    private:
    void concat(vector<string> & a, vector<string> c) {
        vector<string> b = c;
        a.insert(a.end(), b.begin(), b.end());
    }
};