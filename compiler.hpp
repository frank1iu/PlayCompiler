#pragma once
#include "parser.hpp"
#include <string>
#include <array>

using namespace std;

class Compiler {
    public:
    array<int, 8> registers;
    vector<string> define_static(string name, int len, int* init_val);
    vector<string> store_constant(string name, int val);
    vector<string> compile_program(Token* program);
    Compiler();
};