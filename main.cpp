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
    vector<Token*> exprs = parseMany(program);
    Compiler c;
    for (Token* t : exprs) {
        vector<string> v = c.compile_program(t);
        if (t -> getName() != ";") {
            cout << "\t\t\t# " << t -> toString() << endl;
        }
        print_vector(v);
    }
    if (!c.all_registers_free())
        cerr << "[Warn] Register leak found" << endl;
}

