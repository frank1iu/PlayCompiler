#include "parser.hpp"
#include <iostream>
#include <boost/algorithm/string.hpp>

vector<string> Parser::split_expressions(string program) {
    vector<string> ret;
    stack<enum Paren> s;
    int last_index = 0;
    if (!check_valid(program)) throw "Syntax Error: Paren Mismatch";

    for (int i = 0; i < program.length(); i++) {
        if (s.empty() && (program[i] == ' ' || program[i] == '\n')) {
            last_index++;
            continue;
        }
        if (program[i] == '(') {
            s.push(OPEN);
        } else if (program[i] == ')') {
            s.pop();
            if (s.empty()) {
                ret.push_back(program.substr(last_index, i - last_index + 1));
                last_index = i + 1;
            }
        }
    }

    return ret;
}

bool Parser::check_valid(string program) {
    stack<enum Paren> s;
    for (int i = 0; i < program.length(); i++) {
        if (program[i] == '(') {
            s.push(OPEN);
        } else if (program[i] == ')') {
            if (s.empty()) return false;
            s.pop();
        }
    }
    return s.empty();
}

queue<string> Parser::queue_tokens(string program) {
    if (!check_valid(program)) throw "Syntax Error: Paren Mismatch";
    string program_formatted;
    for (const char c : program) {
        switch (c) {
            case '(':
                program_formatted += " ( ";
                break;
            case ')':
                program_formatted += " ) ";
                break;
            default:
                program_formatted += c;
        }
    }

    queue<string> tokens_raw;
    int last_index = 1;
    for (int i = 1; i < program_formatted.length(); i++) {
        if (program_formatted[i] == ' ') {
            string token = program_formatted.substr(last_index, i - last_index);
            boost::algorithm::trim(token);
            if (token == "") continue;
            tokens_raw.push(token);
            last_index = i + 1;
        }
    }

    return tokens_raw;
}

Token* Parser::parse(queue<string> & tokens_raw) {
    if (tokens_raw.empty()) throw "Parser::parse expected non-empty queue";
    if (tokens_raw.front() == "(") tokens_raw.pop(); // pop off the '(', if any
    string top = tokens_raw.front(); tokens_raw.pop();
    Token* token = new Token(top);
    while (!tokens_raw.empty()) {
        top = tokens_raw.front(); tokens_raw.pop();
        if (top == "(") {
            token -> children.push_back(Parser::parse(tokens_raw));
        } else if (top == ")") {
            return token;
        } else {
            token -> children.push_back(new Token(top));
        }
    }
}

Token::Token(enum Type type, void* data) {
    this -> type = type;
    this -> data = data;
}

Token::Token(string data) {
    try {
        int i = stoi(data);
        int* d = (int*) malloc(sizeof(int));
        *d = i;
        this -> type = VALUE;
        this -> data = d;
    } catch (invalid_argument ignored) {
        this -> type = IDENTIFIER;
        this -> data = new string(data);
    }
}

int Token::getValue() const {
    if (type != VALUE) throw "Type Error: Expected a value, found an identifier";
    return * (int*) data;
}

string Token::getName() const {
    if (type != IDENTIFIER) throw "Type Error: Expected an identifier, found a value";
    return * (string*) data;
}