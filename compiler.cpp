#include "compiler.hpp"
#include "parser.hpp"
#include <iostream>
#include <string>
#include <boost/algorithm/hex.hpp>

using namespace std;

void Compiler::emit(string msg) {
    cout << msg << endl;
}

vector<string> Compiler::define_static(string name, int len, int* init_val) {
    vector<string> p;
    p.push_back(name + ":");
    for (int i = 0; i < len; i++) {
        string out = "\t.long\t";
        if (init_val != nullptr) {
            out += to_string(init_val[i]);
        } else {
            out += "0x0";
        }
        p.push_back(out);
    }
    return p;
}

vector<string> Compiler::store_constant(string name, int val) {
    vector<string> p;
    p.push_back("\tld $" + name + ", r0");
    p.push_back("\tld $" + to_string(val) + ", r1");
    p.push_back("\tst r1, (r0)");
    return p;
}

vector<string> Compiler::compile_program(Token* program) {
    if (program -> getName() == "set!") {
        return store_constant(program -> children.at(0) -> getName(),
        program -> children.at(1) -> getValue());
    } else if (program -> getName() == "define") {
        int *v = new int;
        *v = program -> children.at(1) -> getValue();
        return define_static(program -> children.at(0) -> getName(), 1, v);
    } else throw "Unrecognized s-expression";
}