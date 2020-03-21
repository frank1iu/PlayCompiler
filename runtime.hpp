#pragma once
#include <string>
#include <vector>
#include <set>

using namespace std;

struct Symbol {
    string name;
    int offset;
    bool is_return_address;
};

static int __context_last_id = 0;

class Context {
    public:
    bool defined(string name) const;
    Symbol find(string name) const;
    vector<Symbol> symbols;
    int offset;
    int size;
    int id; // for debug only
};

class RuntimeStack {
    public:
    void new_frame();
    void destroy_frame();
    void push(string name, int size);
    int offset_of(string name) const;
    bool defined_anywhere(string name) const;
    vector<Context*> contexts;
    set<string> defined_symbols;
    Context* top();
    RuntimeStack();
};

class FunctionTable {
    public:
    class Function {
        public:
        string name;
        int argc;
        vector<string> args;
        Function(string name, vector<string> args);
    };
    void define(string name, vector<string> args);
    bool exists(string name) const;
    Function* find(string name) const;
    private:
    vector<Function*> functions;
};