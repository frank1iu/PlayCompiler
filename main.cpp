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
    /*
    int i[3] = {5, 6, 12};
    cout << Compiler::store_constant("i", 70);
    cout << Compiler::define_static("i", 3, i);*/

    print_vector(Compiler::compile_program(parseOne("(set! x 10)")));
    print_vector(Compiler::compile_program(parseOne("(set! x 1)")));
    print_vector(Compiler::compile_program(parseOne("(define x 5)")));
}

