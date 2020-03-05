#pragma once
#include "parser.hpp"
#include <string>

using namespace std;

class Compiler {
    public:
    static int registers[8];
    static void emit(string msg);
    static vector<string> define_static(string name, int len, int* init_val);
    static vector<string> store_constant(string name, int val);
    static vector<string> compile_program(Token* program);
};