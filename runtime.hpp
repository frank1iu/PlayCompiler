#include <string>
#include <vector>
#include <set>

using namespace std;

struct Symbol {
    string name;
    int offset;
    bool is_return_address;
};

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