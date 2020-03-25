#include "runtime.hpp"
#include <string>
#include <iostream>

RuntimeStack::RuntimeStack() {
    Context* c = new Context();
    c -> id = __context_last_id++;
    contexts.push_back(c);
}

void RuntimeStack::new_frame() {
    Context* c = new Context();
    c -> id = __context_last_id++;
    contexts.push_back(c);
}

void RuntimeStack::destroy_frame() {
    if (contexts.size() == 1) {
        throw runtime_error("Attempting to destroy last frame!");
    }
    int top_size = top() -> size;
    contexts.pop_back();
    for (Context* c : contexts) {
        c -> offset -= top_size;
    }
}

void RuntimeStack::push(string name, int size) {
    defined_symbols.insert(name);
    for (Context* c : contexts) {
        if (c != top()) c -> offset += size;
    }
    top() -> size += size;
    for (Symbol & s: top() -> symbols) {
        s.offset += size;
    }
    Symbol s {.name = name, .offset = 0, .is_return_address = false};
    top() -> symbols.push_back(s);
}

Context* RuntimeStack::top() {
    return contexts.back();
}

int RuntimeStack::offset_of(string name) const {
    for (Context* c : contexts) {
        if (c -> defined(name)) {
            if (c -> find(name).offset + c -> offset > 60) {
                cerr << "[Warn] Offset of " << name << " is over 60" << endl;
            }
            return c -> find(name).offset + c -> offset;
        }
    }
    throw runtime_error(name + ": this symbol is undefined");
}

bool RuntimeStack::defined_anywhere(string name) const {
    for (Context* c : contexts) {
        if (c -> defined(name)) return true;
    }
    return false;
}

bool Context::defined(string name) const {
    for (Symbol s : symbols) {
        if (s.name == name) return true;
    }
    return false;
}

Symbol Context::find(string name) const {
    for (Symbol s : symbols) {
        if (s.name == name) return s;
    }
    throw runtime_error(name + ": this symbol is undefined");
}

bool FunctionTable::exists(string name) const {
    for (auto f : functions) {
        if (f -> name == name) return true;
    }
    return false;
}

FunctionTable::Function* FunctionTable::find(string name) const {
    for (auto f : functions) {
        if (f -> name == name) return f;
    }
    throw runtime_error(name + ": this symbol is undefined");
}

FunctionTable::Function::Function(string name, vector<string> args) {
    this -> name = name;
    this -> args = args;
    argc = args.size();
}

void FunctionTable::define(string name, vector<string> args) {
    functions.push_back(new FunctionTable::Function(name, args));
}