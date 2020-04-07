#include "compiler.hpp"
#include "parser.hpp"
#include <iostream>
#include <string>

#define ALIGN 4

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
    if (op == "ignored") {
        return -1;
    } else if (op == "define") {
        return expr_define(program);
    } else if (op == "define!") {
        return expr_define_stack(program);
    } else if (op == "set!") {
        return expr_set(program);
    } else if (op == "write!") {
        return expr_write(program);
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
    } else if (op == "call") {
        return expr_call(program);
    } else if (op == "void") {
        return expr_void(program);
    } else {
        throw runtime_error("Unrecognized expression in " + program -> toString());
    }
}

int Compiler::load_value_stack(Token* program) {
    string name = program -> getName();
    int offset = stack.offset_of(name);
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
        if (stack.defined_anywhere(program -> getName())) {
            return load_value_stack(program);
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
    if (stack.defined_anywhere(name)) {
        emit("mov r5, " + rtos(r), program);
        int tmp = rc_ralloc();
        emit("ld $" + to_string(stack.offset_of(name)) + ", " + rtos(tmp), program);
        emit("add " + rtos(tmp) + ", " + rtos(r), program);
        rc_free_ref(tmp);
    } else {
        emit("ld $" + name + ", " + rtos(r), program);
    }
    return r;
}

int Compiler::expr_define_stack(Token* program) {
    string name = program -> children.at(0) -> getName();
    stack_define(name, ALIGN);
    int return_register = compile_one(program -> children.at(1));
    emit("st " + rtos(return_register) + ", (r5)", program);
    rc_free_ref(return_register);
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
    parent -> children.clear();
    delete parent;
    return r_dest;
}

void Compiler::stack_define(string name, int size) {
    int temp = rc_ralloc();
    emit("ld $-" + to_string(size) + ", " + rtos(temp), "[stack allocation for " + name + "]");
    emit("add " + rtos(temp) + ", r5", "[stack allocation for " + name + "]");
    rc_free_ref(temp);
    stack.push(name, size);
}

void Compiler::stack_shrink(int size) {
    int temp = rc_ralloc();
    emit("ld $" + to_string(size) + ", " + rtos(temp), "[stack shrink]");
    emit("add " + rtos(temp) + ", r5", "[stack shrink]");
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
    emit("j " + label_loop_end, program);
    emit(label_loop_continue + ":", program);
    rc_free_ref(cond_result);
    int body_result = compile_one(body_child);
    rc_free_ref(body_result);
    emit("j " + label_loop_start, program);
    emit(label_loop_end + ":", program);
    return -1;
}

int Compiler::expr_greater_than(Token* program) {
    Token* parent = new Token("-");
    parent -> children.push_back(program -> children.at(0));
    parent -> children.push_back(program -> children.at(1));
    int difference = compile_one(parent);
    parent -> children.clear();
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

int Compiler::expr_void(Token* program) {
    int ignored = compile_one(program -> children.at(0));
    rc_free_ref(ignored);
    return -1;
}

int Compiler::expr_begin(Token* program) {
    stack.new_frame();
    emit("# Frame created. ID: " + to_string(stack.top() -> id), program);
    int r = 0;
    for (Token* t : program -> children) {
        r = compile_one(t);
        rc_free_ref(r);
    }
    rc_keep_ref(r);
    emit("# Frame info at time of destruction: ", "");
    emit("# ID: " + to_string(stack.top() -> id), "");
    for (Symbol s : stack.top() -> symbols) {
        emit("# Offset " + to_string(s.offset) + ": " + s.name , "");
    }
    stack_shrink(stack.top() -> size);
    stack.destroy_frame();
    return r;
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
    return rval;
}

int Compiler::expr_define(Token* program) {
    if (program -> children.at(0) -> children.size() != 0) {
        return expr_define_fn(program);
    }
    if (program -> children.at(1) -> children.size() != 0) {
        throw runtime_error("define: defining from expression is not supported; use set!");
    }
    string name = program -> children.at(0) -> getName();
    int val = program -> children.at(1) -> getValue();
    asm_data.push_back(name + ":");
    asm_data.push_back(".long " + to_string(val));
    return -1;
}

int Compiler::expr_define_fn(Token* program) {
    string name = program -> children.at(0) -> getName();

    Token* r6 = new Token("");
    r6 -> which_register = 6;
    Token* parent = new Token("define!");
    Token* ra_name = new Token("__COMPILER_RETURN_ADDRESS");
    parent -> children.push_back(ra_name);
    parent -> children.push_back(r6);

    Compiler c;
    vector<string> args;
    for (Token* t : program -> children.at(0) -> children) {
        args.push_back(t -> getName());
        c.stack.push(t -> getName(), ALIGN);
    }
    ft.define(name, args);
    c.ft = ft;
    c.compile_one(parent);
    delete parent;
    int r_loc = c.compile_one(program -> children.at(1));
    asm_fn.push_back(name + ": ");
    asm_fn.insert(asm_fn.end(), c.asm_code.begin(), c.asm_code.end());
    if (r_loc != -1) {
        asm_fn.push_back("mov " + rtos(r_loc) + ", r7 # [copying return value to r7 for " + name + "]");
    }
    int offset = c.stack.offset_of("__COMPILER_RETURN_ADDRESS");
    asm_fn.push_back("ld " + to_string(offset) + "(r5), r6 # [load return address for " + name + "]");
    asm_fn.push_back("j (r6) # [function return for " + name + "]");
    return -1;
}

int Compiler::expr_call(Token* program) {
    string name = program -> children.at(0) -> getName();
    if (!ft.exists(name))
        throw runtime_error(name + ": this function is undefined");
    auto func = ft.find(name);
    if (func -> argc != program -> children.size() - 1)
        throw runtime_error(name + ": expects " + to_string(func -> argc) + " arguments, but given " + to_string(program -> children.size() - 1));
    stack.new_frame(); // saved registers
    vector<int> register_cache;
    for (int i = 0; i < registers.size(); i++) {
        if (registers.at(i) > 0) {
            register_cache.push_back(i);
        }
    }
    for (int i: register_cache) {
        stack_define(rtos(i), ALIGN);
        emit("st " + rtos(i) + ", (r5)", "[function call: save registers] [call to " + name + "]");
    }
    stack.new_frame(); // arguments
    emit("nop", "[begin function call: " + name + "]");
    for (int i = 1; i < program -> children.size(); i++) {
        Token* parent = new Token("define!");
        string arg_name = func -> args.at(i - 1);
        Token* c1 = new Token(arg_name);
        Token* c2 = program -> children.at(i);
        parent -> children.push_back(c1);
        parent -> children.push_back(c2);
        compile_one(parent);
        parent -> children.pop_back();
        delete parent;
    }
    emit("gpc $6, r6", program);
    emit("j " + program -> children.at(0) -> getName(), program);
    stack_shrink(stack.top() -> size + ALIGN);
    stack.destroy_frame(); // arguments
    for (int i: register_cache) {
        string offset = to_string(stack.offset_of(rtos(i)));
        emit("ld " + offset + "(r5), " + rtos(i), "[function call: load saved registers] [call to " + name + "]");
    }
    stack_shrink(ALIGN * register_cache.size());
    stack.destroy_frame(); // saved registers
    int ret = rc_ralloc();
    emit("mov r7, " + rtos(ret), program);
    return ret;
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
    // delete parent;
    return rval;
}

int Compiler::expr_set(Token* program) {
    int return_register = compile_one(program -> children.at(1));
    string name = program -> children.at(0) -> getName();
    if (stack.defined_anywhere(name)) {
        int offset = stack.offset_of(name);
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

int Compiler::expr_write(Token* program) {
    int r_dest = compile_one(program -> children.at(0));
    int r_val = compile_one(program -> children.at(1));
    emit("st " + rtos(r_val) + ", (" + rtos(r_dest) + ")", program);
    rc_free_ref(r_dest);
    rc_free_ref(r_val);
    return -1;
}

int Compiler::expr_equals(Token* program) {
    Token* parent = new Token("-");
    parent -> children.push_back(program -> children.at(0));
    parent -> children.push_back(program -> children.at(1));
    int difference = compile_one(parent);
    // delete parent;
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
    registers = {0, 0, 0, 0, 0, -1, -1, -1};
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

void Compiler::emit(string code, string debug) {
    asm_code.push_back(code + "\t# " + debug);
}

string Compiler::toString() const {
    string ret = "ld $stacktop, r5\n";
    for (string s : asm_code) {
        ret += s + "\n";
    }
    for (string s : asm_data) {
        ret += s + "\n";
    }
    ret += ".pos 0xc000\n";
    for (string s : asm_fn) {
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