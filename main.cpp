#include "compiler.hpp"
#include "parser.hpp"
#include <iostream>

using namespace std;

Token* parseOne(string program) {
    queue<string> tokens = Parser::queue_tokens(program);
    return Parser::parse(tokens);
}

void print_vector(vector<string> v) {
    for (string s : v) {
        cout << s << endl;
    }
}

int main() {
    Compiler c;
    vector<string> program = c.compile_program(parseOne("(set! x (+ 1 1))"));
    int q;
    print_vector(program);
    //print_vector(c.compile_program(parseOne("(set! x 1)")));
    //print_vector(c.compile_program(parseOne("(define x 5)")));
}

