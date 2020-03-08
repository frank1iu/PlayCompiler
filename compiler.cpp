#include "compiler.hpp"
#include "parser.hpp"
#include <iostream>
#include <string>

using namespace std;

void Compiler::compile_program(string program) {
    vector<string> exprs = Parser::split_expressions(program);
    for (unsigned int i = 0; i < exprs.size(); i++) {
        string expr_raw = exprs.at(i);
        Token* expr = Parser::parse_expression(expr_raw);
        compile_one(expr);
    }
}

int Compiler::compile_one(Token* program) {
    if (program -> type == VALUE || program -> children.size() == 0) {
        return load_value(program);
    }
    string op = program -> getName();
    if (op == "define") {
        return expr_define(program);
    } else if (op == "set!") {
        return expr_set(program);
    } else if (op == "+") {
        return expr_add(program);
    } else if (op == "add1") {
        return expr_add1(program);
    } else if (op == "not") {
        return expr_not(program);
    } else if (op == "-") {
        return expr_sub(program);
    } else if (op == "sub1") {
        return expr_sub1(program);
    } else if (op == "begin") {
        return expr_begin(program);
    } else {
        throw runtime_error("Unrecognized expression in " + program -> toString());
    }
}

int Compiler::load_value(Token* program) {
    int r = rc_ralloc();
    if (program -> type == VALUE) {
        emit("ld $" + program -> toString() + ", " + rtos(r), program);
    } else {
        emit("ld $" + program -> toString() + ", " + rtos(r), program);
        emit("ld (" + rtos(r) + "), " + rtos(r), program);
    }
    return r;
}

int Compiler::expr_begin(Token* program) {
    for (Token* t : program -> children) {
        compile_one(t);
    }
    return -1;
}

int Compiler::expr_add(Token* program) {
    int return_register = compile_one(program -> children.at(0));
    int r_dest = compile_one(program -> children.at(1));
    emit("add " + rtos(return_register) + ", " + rtos(r_dest), program);
    rc_free_ref(return_register);
    return r_dest;
}

int Compiler::expr_add1(Token* program) {
    int return_register = compile_one(program -> children.at(0));
    emit("inc " + rtos(return_register), program);
    return return_register;
}

int Compiler::expr_sub1(Token* program) {
    int return_register = compile_one(program -> children.at(0));
    emit("dec " + rtos(return_register), program);
    return return_register;
}

int Compiler::expr_not(Token* program) {
    int return_register = compile_one(program -> children.at(0));
    emit("not " + rtos(return_register), program);
    return return_register;
}

int Compiler::expr_sub(Token* program) {
    Token* parent = new Token("+");
    parent -> children.push_back(program -> children.at(0));
    Token* child_not = new Token("not");
    Token* child_add1 = new Token("add1");
    child_add1 -> children.push_back(program -> children.at(1));
    child_not -> children.push_back(child_add1);
    parent -> children.push_back(child_not);
    return compile_one(parent);
}

int Compiler::expr_define(Token* program) {
    if (program -> children.at(1) -> children.size() != 0) {
        throw runtime_error("define: defining a constant from an expression is not supported");
    }
    string name = program -> children.at(0) -> getName();
    int val = program -> children.at(1) -> getValue();
    asm_data.push_back(name + ":");
    asm_data.push_back(".long " + to_string(val));
    return -1;
}

int Compiler::expr_set(Token* program) {
    int return_register = compile_one(program -> children.at(1));
    string name = program -> children.at(0) -> getName();
    int temp = rc_ralloc();
    emit("ld $" + name + ", " + rtos(temp), program);
    emit("st " + rtos(return_register) + ", (" + rtos(temp) + ")", program);
    rc_free_ref(return_register);
    rc_free_ref(temp);
    return -1;
}

/* vector<string> Compiler::define_static(string name, int len, int* init_val) {
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
} */

int Compiler::rc_ralloc() {
    for (unsigned int i = 0; i < registers.size(); i++) {
        if (registers[i] == 0) {
            registers[i]++;
            return i;
        }
    }
    throw runtime_error("rc_ralloc cannot find a free register!");
}
void Compiler::rc_keep_ref(int r_dest) {
    registers[r_dest]++;
}
void Compiler::rc_free_ref(int r_dest) {
    if (registers[r_dest] == 0) throw runtime_error("rc_free_ref: double free");
    registers[r_dest]--;
}

Compiler::Compiler() {
    registers = {0, 0, 0, 0, 0, -1, -1, 0};
    asm_data.push_back(".pos 0x1000\t# Data Region");
};

bool Compiler::all_registers_free() const {
    for (unsigned int i = 0; i < registers.size(); i++) {
        if (registers[i] > 0) {
            return false;
        }
    }
    return true;
}

string Compiler::rtos(int r) const {
    return "r" + to_string(r);
}

void Compiler::emit(string code, Token* program) {
    asm_code.push_back(code + "\t# " + program -> toString());
}

string Compiler::toString() const {
    string ret;
    for (string s : asm_code) {
        ret += s + "\n";
    }
    for (string s : asm_data) {
        ret += s + "\n";
    }
    return ret;
}

string Compiler::next_label() {
    return "L" + to_string(last_label++);
}