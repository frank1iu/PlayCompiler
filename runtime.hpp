#pragma once
#include <string>
#include <vector>
#include <set>

using namespace std;

struct Symbol {
    // Symbol represents a defined symbol.
    
    // the name of the symbol. "__COMPILER_RETURN_ADDRESS" is reserved
    // for return addresses
    string name;

    // the offset of the symbol from the top of its frame.
    // to get the offset of the symbol from r5, use RuntimeStack::offset_of
    int offset;
};

static int __context_last_id = 0;

class Context {
    // Context represents a single frame on the stack.
    public:
    // returns whether a symbol identified by name exists in the current frame
    bool defined(string name) const;
    // returns the symbol identified by name.
    // throws runtime_error if the symbol cannot be found.
    Symbol find(string name) const;
    int id; // for debug only
    // a vector of symbols in the symbol table.
    vector<Symbol> symbols;
    // the offset of the top of the frame, from r5
    int offset;
    // the size of the frame
    int size;
};

class RuntimeStack {
    public:
    // pushes a new frame on the top of the stack
    void new_frame();
    // pops the top frame off the stack
    void destroy_frame();
    // pushes a symbol identified by name and size onto the top of the
    // topmost frame.
    void push(string name, int size);
    // gives the offset of a symbol identified by name, from r5
    // throws runtime_error if such a symbol cannot be found
    // the "load offset" instruction in SM213 can take a maximum of 60 bytes as offset
    // this function prints a warning to stderr if it finds an offset over 60
    int offset_of(string name) const;
    // returns true if a symbol identified by name is defined anywhere on the stack
    bool defined_anywhere(string name) const;
    // a list of frames
    vector<Context*> contexts;
    // a set of defined symbol names
    set<string> defined_symbols;
    // returns a pointer to the top of the runtime stack
    Context* top();
    RuntimeStack();
};

class FunctionTable {
    // FunctionTable is an index of defined functions
    // and their arguments.
    public:
    class Function {
        public:
        // the name of the function
        string name;
        // the number of arguments the function takes
        int argc;
        // names of each argument, left to right order
        // for example, for (define (example a b) (+ a b))
        // args will be {"a", "b"}
        vector<string> args;
        // creates a function with name and args
        Function(string name, vector<string> args);
    };
    // defines a function with name and args
    void define(string name, vector<string> args);
    // returns true if a function identified by name exists in this function table.
    bool exists(string name) const;
    // returns a pointer to a Function identified by name
    // throws runtime_error if no such function exists in the table
    Function* find(string name) const;
    
    private:
    vector<Function*> functions;
};