#include "compiler.hpp"
#include "parser.hpp"
#include <iostream>
#include <string>
#include <boost/algorithm/hex.hpp>

using namespace std;

vector<string> Compiler::define_static(string name, int len, int* init_val) {
    vector<string> p;
    if (!static_data_phase) {
        static_data_phase = true;
        p.push_back("\thalt");
        p.push_back(".pos 0x4000");
    }
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
    if (program -> getName() != "define" && static_data_phase) {
        throw runtime_error("Non-define expression found in static data phase!");
    }
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
    } else if (program -> getName() == ";") {
        return p;
    } else {
        throw runtime_error("Unrecognized s-expression");
    }
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
        return expr_add(program, r_dest);
    } else if (op == "-") {
        return expr_sub(program, r_dest);
    } else if (op == "not") {
        return expr_not(program, r_dest);
    } else if (op == "add1") {
        return expr_add1(program, r_dest);
    } else if (op == "&") {
        p.push_back(load_addr(program -> children.at(0) -> getName(), r_dest).at(0));
    } else if (op == "*") {
        int r_temp;
        concat(p, eval(program -> children.at(0), &r_temp));
        *r_dest = rc_ralloc();
        p.push_back("\tld (r" + to_string(r_temp) + "), r" + to_string(*r_dest));
        rc_free_ref(r_temp);
    } else {
        throw runtime_error("Compiler::eval: unrecognized identifier" + program -> getName());
    }
    // delete program;
    return p;
}

vector<string> Compiler::expr_sub(Token* program, int* r_dest) {
    if (program -> children.size() < 2) {
        throw runtime_error("?");
    }
    Token *transformed = new Token("+");
    transformed -> children.push_back(program -> children.at(0));
    Token *not_child = new Token("not");
    Token *add1_child = new Token("add1");
    not_child -> children.push_back(program -> children.at(1));
    add1_child -> children.push_back(not_child);
    transformed -> children.push_back(add1_child);
    return eval(transformed, r_dest);
}

vector<string> Compiler::expr_add(Token* program, int* r_dest) {
    vector<string> p;
    int r_temp;
    concat(p, eval(program -> children.at(0), &r_temp));
    concat(p, eval(program -> children.at(1), r_dest));
    p.push_back("\tadd r" + to_string(r_temp) + ", r" + to_string(*r_dest));
    rc_free_ref(r_temp);
    return p;
}

vector<string> Compiler::expr_add1(Token* program, int* r_dest) {
    vector<string> p;
    concat(p, eval(program -> children.at(0), r_dest));
    p.push_back("\tinc r" + to_string(*r_dest));
    return p;
}

vector<string> Compiler::expr_not(Token* program, int* r_dest) {
    vector<string> p;
    concat(p, eval(program -> children.at(0), r_dest));
    p.push_back("\tnot r" + to_string(*r_dest));
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