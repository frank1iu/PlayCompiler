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
        delete expr;
    }
    asm_code.push_back("halt");
}

int Compiler::compile_one(Token* program) {
    if (program -> which_register != -1) {
        return program -> which_register;
    }
    if (program -> type == VALUE || program -> children.size() == 0) {
        return load_value(program);
    }
    string op = program -> getName();
    if (op == ";") {
        return -1;
    } else if (op == "define") {
        return expr_define(program);
    } else if (op == "define!") {
        return expr_define_stack(program);
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
    } else if (op == "*") {
        if (program -> children.size() == 2)
            return expr_mult(program);
        return expr_deref(program);
    } else if (op == "&") {
        return expr_addr(program);
    } else if (op == "=") {
        return expr_equals(program);
    } else if (op == "<") {
        return expr_less_than(program);
    } else if (op == ">") {
        return expr_greater_than(program);
    } else if (op == "if") {
        return expr_if(program);
    } else if (op == "while") {
        return expr_while(program);
    } else if (op == "for") {
        return expr_for(program);
    } else {
        throw runtime_error("Unrecognized expression in " + program -> toString());
    }
}

int Compiler::load_value_stack(Token* program) {
    string name = program -> getName();
    int offset;
    for (Symbol s : symbol_table) {
        if (s.name == name) offset = s.offset;
    }
    int r = rc_ralloc();
    emit("ld " + to_string(offset) + "(r5), " + rtos(r), program);
    return r;
}
int Compiler::load_value(Token* program) {
    int r;
    if (program -> type == VALUE) {
        r = rc_ralloc();
        emit("ld $" + program -> toString() + ", " + rtos(r), program);
    } else {
        if (program -> getName() == "#true") {
            Token* tmp = new Token("1");
            int rval =  load_value(tmp);
            delete tmp;
            return rval;
        } else if (program -> getName() == "#false") {
            Token* tmp = new Token("0");
            int rval =  load_value(tmp);
            delete tmp;
            return rval;
        }
        for (Symbol s: symbol_table) {
            if (s.name == program -> getName()) return load_value_stack(program);
        }
        r = rc_ralloc();
        emit("ld $" + program -> toString() + ", " + rtos(r), program);
        emit("ld (" + rtos(r) + "), " + rtos(r), program);
    }
    return r;
}

int Compiler::expr_addr(Token* program) {
    if (program -> children.at(0) -> children.size() != 0) {
        throw runtime_error("Getting address of an expression is not supported");
    }
    int r = rc_ralloc();
    string name = program -> children.at(0) -> getName();
    if (defined(name)) {
        emit("mov r5, " + rtos(r), program);
        int tmp = rc_ralloc();
        emit("ld $" + to_string(offset_of(name)) + ", " + rtos(tmp), program);
        emit("add " + rtos(tmp) + ", " + rtos(r), program);
        rc_free_ref(tmp);
    } else {
        emit("ld $" + name + ", " + rtos(r), program);
    }
    return r;
}

int Compiler::expr_define_stack(Token* program) {
    string name = program -> children.at(0) -> getName();
    stack_alloc(4, name);
    int return_register = compile_one(program -> children.at(1));
    struct Symbol s;
    s.offset = 0;
    s.name = name;
    symbol_table.push_back(s);
    emit("st " + rtos(return_register) + ", (r5)", program);
    rc_free_ref(return_register);
    for (struct Symbol s: symbol_table) {
        s.offset += 4;
    }
    return -1;
}

int Compiler::expr_deref(Token* program) {
    int return_register = compile_one(program -> children.at(0));
    emit("ld (" + rtos(return_register) + "), " + rtos(return_register), program);
    return return_register;
}

int Compiler::expr_less_than(Token* program) {
    Token* parent = new Token("-");
    parent -> children.push_back(program -> children.at(1));
    parent -> children.push_back(program -> children.at(0));
    int difference = compile_one(parent);
    delete parent;
    // difference > 0 -> return true
    int r_dest = rc_ralloc();
    string label = next_label();
    string label2 = next_label();
    emit("bgt " + rtos(difference) + ", " + label, program);
    emit("ld $0, " + rtos(r_dest), program);
    emit("br " + label2, program);
    emit(label + ":", program);
    emit("ld $1, " + rtos(r_dest), program);
    emit(label2 + ":", program);
    rc_free_ref(difference);
    return r_dest;
}

void Compiler::stack_alloc(int size, string name) {
    Token* t = new Token("[stack allocation for " + name + "]");
    int temp = rc_ralloc();
    emit("ld $-" + to_string(size) + ", " + rtos(temp), t);
    emit("add " + rtos(temp) + ", r5", t);
    delete t;
    for (unsigned int i = 0; i < symbol_table.size(); i++) {
        symbol_table.at(i).offset += 4;
    }
    rc_free_ref(temp);
}

int Compiler::expr_while(Token* program) {
    Token* cond_child = program -> children.at(0);
    Token* body_child = program -> children.at(1);
    string label_loop_start = next_label();
    string label_loop_continue = next_label();
    string label_loop_end = next_label();
    emit(label_loop_start + ":", program);
    int cond_result = compile_one(cond_child);
    emit("bgt " + rtos(cond_result) + ", " + label_loop_continue, program);
    emit("br " + label_loop_end, program);
    emit(label_loop_continue + ":", program);
    rc_free_ref(cond_result);
    int body_result = compile_one(body_child);
    rc_free_ref(body_result);
    emit("br " + label_loop_start, program);
    emit(label_loop_end + ":", program);
    return -1;
}

int Compiler::expr_greater_than(Token* program) {
    Token* parent = new Token("-");
    parent -> children.push_back(program -> children.at(0));
    parent -> children.push_back(program -> children.at(1));
    int difference = compile_one(parent);
    delete parent;
    int r_dest = rc_ralloc();
    string label = next_label();
    string label2 = next_label();
    emit("bgt " + rtos(difference) + ", " + label, program);
    emit("ld $0, " + rtos(r_dest), program);
    emit("br " + label2, program);
    emit(label + ":", program);
    emit("ld $1, " + rtos(r_dest), program);
    emit(label2 + ":", program);
    rc_free_ref(difference);
    return r_dest;
}

int Compiler::expr_if(Token* program) {
    int return_register = compile_one(program -> children.at(0));
    string label_true_ans = next_label();
    string label_end = next_label();
    emit("bgt " + rtos(return_register) + ", " + label_true_ans, program);
    rc_free_ref(return_register);
    int shared_register = compile_one(program -> children.at(2));
    emit("br " + label_end, program);
    emit(label_true_ans + ":", program);
    ralloc_force_return(shared_register);
    compile_one(program -> children.at(1));
    emit(label_end + ":", program);
    return shared_register;
}

int Compiler::expr_begin(Token* program) {
    for (Token* t : program -> children) {
        compile_one(t);
    }
    return -1;
}

int Compiler::expr_mult(Token* program) {
    
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
    child_not -> children.push_back(program -> children.at(1));
    child_add1 -> children.push_back(child_not);
    parent -> children.push_back(child_add1);
    int rval = compile_one(parent);
    delete parent;
    return rval;
}

int Compiler::expr_define(Token* program) {
    if (program -> children.at(1) -> children.size() != 0) {
        throw runtime_error("define: defining from expression is not supported; use set!");
    }
    string name = program -> children.at(0) -> getName();
    int val = program -> children.at(1) -> getValue();
    asm_data.push_back(name + ":");
    asm_data.push_back(".long " + to_string(val));
    return -1;
}

int Compiler::expr_for(Token* program) {
    compile_one(program -> children.at(0));
    Token* parent = new Token("while");
    Token* cond = program -> children.at(1);
    parent -> children.push_back(cond);
    Token* subroutine = new Token("begin");
    subroutine -> children.push_back(program -> children.at(3));
    subroutine -> children.push_back(program -> children.at(2));
    parent -> children.push_back(subroutine);
    int rval = compile_one(parent);
    delete parent;
    return rval;
}

int Compiler::expr_set(Token* program) {
    int return_register = compile_one(program -> children.at(1));
    string name = program -> children.at(0) -> getName();
    if (defined(name)) {
        int offset = offset_of(name);
        emit("st " + rtos(return_register) + ", " + to_string(offset) + "(r5)", program);
        rc_free_ref(return_register);
        return -1;
    }
    int temp = rc_ralloc();
    emit("ld $" + name + ", " + rtos(temp), program);
    emit("st " + rtos(return_register) + ", (" + rtos(temp) + ")", program);
    rc_free_ref(return_register);
    rc_free_ref(temp);
    return -1;
}

int Compiler::expr_equals(Token* program) {
    Token* parent = new Token("-");
    parent -> children.push_back(program -> children.at(0));
    parent -> children.push_back(program -> children.at(1));
    int difference = compile_one(parent);
    delete parent;
    int r_dest = rc_ralloc();
    string label = next_label();
    string label2 = next_label();
    emit("beq " + rtos(difference) + ", " + label, program);
    emit("ld $0, " + rtos(r_dest), program);
    emit("br " + label2, program);
    emit(label + ":", program);
    emit("ld $1, " + rtos(r_dest), program);
    emit(label2 + ":", program);
    rc_free_ref(difference);
    return r_dest;
}

int Compiler::rc_ralloc() {
    if (_ralloc_return_val != -1) {
        int tmp = _ralloc_return_val;
        _ralloc_return_val = -1;
        return tmp;
    }
    for (unsigned int i = 0; i < registers.size(); i++) {
        if (registers[i] == 0) {
            registers[i]++;
            return i;
        }
    }
    cerr << toString() << endl;
    throw runtime_error("rc_ralloc cannot find a free register!");
}
void Compiler::rc_keep_ref(int r_dest) {
    registers[r_dest]++;
}
void Compiler::rc_free_ref(int r_dest) {
    if (r_dest == -1) return;
    if (registers[r_dest] == 0) throw runtime_error("rc_free_ref: double free on r" + to_string(r_dest));
    registers[r_dest]--;
}
void Compiler::ralloc_force_return(int r_dest) {
    _ralloc_return_val = r_dest;
}

Compiler::Compiler() {
    registers = {0, 0, 0, 0, 0, -1, -1, 0};
    asm_data.push_back(".pos 0x4000\t# Data Region");
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
    string ret = "ld $stacktop, r5\n";
    for (string s : asm_code) {
        ret += s + "\n";
    }
    for (string s : asm_data) {
        ret += s + "\n";
    }
    ret += ".pos 0xfe00\nstack:\n";
    for (int i = 0; i < 512; i++) {
        ret += ".long 0\n";
    }
    ret += ".pos 0x10000\nstacktop:\n.long 0\n";
    return ret;
}

string Compiler::next_label() {
    return "L" + to_string(last_label++);
}

bool Compiler::defined(string name) const {
    for (Symbol s : symbol_table) {
        if (s.name == name) return true;
    }
    return false;
}

int Compiler::offset_of(string name) const {
    for (Symbol s : symbol_table) {
        if (s.name == name) return s.offset;
    }
    throw runtime_error(name + ": this symbol is undefined");
}