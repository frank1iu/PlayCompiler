#include "compiler.hpp"
#include "parser.hpp"
#include <iostream>

#define DEBUG 1

using namespace std;

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
    try {
        c.compile_program(program);
        cout << c.toString();
    } catch (runtime_error error) {
        cerr << c.toString();
        cerr << "=== Compilation aborted ===" << endl << error.what() << endl;
    }
    if (DEBUG) {
        if (!c.all_registers_free()) {
            cerr << "[Warn] Register leak found" << endl;
            for (unsigned int i = 0; i < c.registers.size(); i++) {
                cerr << "r" << i << "'s refcount is: " << c.registers[i] << endl;
            }
        }
    }
}