#include "compiler.hpp"
#include "parser.hpp"
#include <iostream>
#include <string>
#include <boost/algorithm/hex.hpp>

using namespace std;

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

vector<string> Compiler::store_value(string name, int r) {
    vector<string> p;
    int temp = rc_ralloc();
    p.push_back("\tld $" + name + ", r" + to_string(temp));
    p.push_back("\tst r" + to_string(r) + ", (r" + to_string(temp) + ")");
    rc_free_ref(temp);
    return p;
}

vector<string> Compiler::compile_program(Token* program) {
    vector<string> p;
    if (program -> getName() == "set!") {
        int r;
        concat(p, eval(program -> children.at(1), &r));
        concat(p, store_value(program -> children.at(0) -> getName(), r));
        rc_free_ref(r);
        return p;
    } else if (program -> getName() == "define") {
        int *v = new int;
        *v = program -> children.at(1) -> getValue();
        return define_static(program -> children.at(0) -> getName(), 1, v);
    } else throw runtime_error("Unrecognized s-expression");
}

vector<string> Compiler::load_into_register(Token* program, int* r_dest) {
    vector<string> p;
    if (program -> children.size() != 0) {
        throw runtime_error("Compiler::load_into_register cannot take an expression!");
    } else {
        if (program -> type == IDENTIFIER) {
            concat(p, load_addr(program -> getName(), r_dest));
            p.push_back("\tld (r" + to_string(*r_dest) + "), r" + to_string(*r_dest));
        } else if (program -> type == VALUE) {
            *r_dest = rc_ralloc();
            p.push_back("\tld $" + to_string(program -> getValue()) + ", r" + to_string(*r_dest));
        }
    }
    return p;
}

vector<string> Compiler::load_addr(string name, int* r_dest) {
    vector<string> p;
    *r_dest = rc_ralloc();
    p.push_back("\tld $" + name + ", r" + to_string(*r_dest));
    return p;
}

vector<string> Compiler::eval(Token* program, int* r_dest) {
    if (program -> type == VALUE || program -> children.size() == 0) {
        return load_into_register(program, r_dest);
    }
    vector<string> p;
    const string op = program -> getName();
    if (op == "+") {
        int r_temp;
        concat(p, eval(program -> children.at(0), &r_temp));
        concat(p, eval(program -> children.at(1), r_dest));
        p.push_back("\tadd r" + to_string(r_temp) + ", r" + to_string(*r_dest));
        rc_free_ref(r_temp);
    } else if (op == "not") {
        concat(p, eval(program -> children.at(0), r_dest));
        p.push_back("\tnot r" + to_string(*r_dest));
    } else if (op == "&") {
        p.push_back(load_addr(program -> children.at(0) -> getName(), r_dest).at(0));
    } else if (op == "*") {
        int r_temp;
        concat(p, eval(program -> children.at(0), &r_temp));
        *r_dest = rc_ralloc();
        p.push_back("\tld (r" + to_string(r_temp) + "), r" + to_string(*r_dest));
        rc_free_ref(r_temp);
    }
    return p;
}

int Compiler::rc_ralloc() {
    for (int i = 0; i < registers.size(); i++) {
        if (registers[i] == 0) {
            registers[i]++;
            return i;
        }
    }
    throw runtime_error("rc_ralloc cannot find a free register!");
}
int Compiler::rc_keep_ref(int r_dest) {
    registers[r_dest]++;
}
int Compiler::rc_free_ref(int r_dest) {
    if (registers[r_dest] == 0) throw runtime_error("rc_free_ref: double free");
    registers[r_dest]--;
}

Compiler::Compiler() {
    registers = {0, 0, 0, 0, 0, -1, -1, 0};
};

bool Compiler::all_registers_free() const {
    for (int i = 0; i < registers.size(); i++) {
        if (registers[i] > 0) {
            return false;
        }
    }
    return true;
}