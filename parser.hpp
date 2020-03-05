#pragma once
#include <vector>
#include <string>
#include <stack>
#include <queue>
#include <algorithm>
#include <cstring>
using namespace std;

enum Type {
    IDENTIFIER,
    VALUE
};

class Token {
    public:
        enum Type type;
        void* data;
        vector<Token*> children;

        Token(enum Type type, void* data);
        Token(string data);
        int getValue() const;
        string getName() const;
};

class Parser {
    public:
        // Parses a *single* s-expression into a Token
        // tokens_raw must represent a valid, single s-expression
        static Token* parse(queue<string> & tokens_raw);

        // Produces a queue of raw tokens from a single s-expression
        static queue<string> queue_tokens(string program);

        // Splits multiple s-expressions in a string into a vector of strings
        // where each string contains only one s-expression
        static vector<string> split_expressions(string program);

        // Checks if every opening paren has a matching closing paren.
        static bool check_valid(string program);
    private:
        enum Paren {OPEN, CLOSE};
};