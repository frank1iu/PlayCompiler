#include "compiler.hpp"
#include "parser.hpp"
#include <iostream>

using namespace std;

void print_vector(vector<string> v) {
    for (string s : v) {
        cout << s << endl;
    }
}

vector<Token*> parseMany(string program) {
    vector<string> exprs = Parser::split_expressions(program);
    vector<Token*> ret;
    for (string s : exprs) {
        ret.push_back(Parser::parse_expression(s));
    }
    return ret;
}

string read_program() {
    string program, tmp;
    while (getline(cin, tmp)) {
        program += tmp;
    }
    return program;
}

int main() {
    string program = read_program();
    Compiler c;
    c.compile_program(program);
    cout << c.toString();
    if (!c.all_registers_free()) {
        cerr << "[Warn] Register leak found" << endl;
        for (unsigned int i = 0; i < c.registers.size(); i++) {
            cerr << "r" << i << ": " << c.registers[i] << endl;
        }
    }
    for (Compiler::Symbol s: c.symbol_table) {
        cerr << s.name << " is at offset " << s.offset << endl;
    }
}

