#include "compiler.hpp"
#include "parser.hpp"
#include <iostream>

using namespace std;

void print_vector(vector<string> v) {
    for (string s : v) {
        cout << s << endl;
    }
}

Token* parseOne(string program) {
    queue<string> tokens = Parser::queue_tokens(program);
    return Parser::parse(tokens);
}

vector<Token*> parseMany(string program) {
    vector<string> exprs = Parser::split_expressions(program);
    vector<Token*> ret;
    for (string s : exprs) {
        ret.push_back(parseOne(s));
    }
    return ret;
}

string read_program() {
    string program, tmp;
    while (getline(cin, tmp)) {
        program += tmp;
    }
    return tmp;
}

int main() {
    string program = read_program();
    vector<Token*> exprs = parseMany(program);
    Compiler c;
    for (Token* t : exprs) {
        vector<string> v = c.compile_program(t);
        print_vector(v);
    }
}

