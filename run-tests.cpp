#include "catch.hpp"
#include "parser.hpp"
#include "compiler.hpp"
#include <iostream>

TEST_CASE( "Parser::check_valid", "[parse]" ) {
    string program = "";
    string program2 = "(+ 1 1)";
    string program3 = "(define (identity x) x)";
    string program4 = "(define A 1) (+ A 1)";
    string program_bad = "(define";
    string program_bad2 = "(define A 1) (add1";
    REQUIRE( Parser::check_valid(program) == true );
    REQUIRE( Parser::check_valid(program2) == true );
    REQUIRE( Parser::check_valid(program3) == true );
    REQUIRE( Parser::check_valid(program4) == true );
    REQUIRE( Parser::check_valid(program_bad) == false );
    REQUIRE( Parser::check_valid(program_bad2) == false );
}

TEST_CASE( "Parser::split_expressions", "[parse]" ) {
    string program = "(+ 1 1)";
    vector<string> program_split = Parser::split_expressions(program);
    REQUIRE( program_split.size() == 1 );
    REQUIRE( program_split.at(0) == "(+ 1 1)" );

    string program2 = "(define A 1) (add1 A)";
    vector<string> program2_split = Parser::split_expressions(program2);
    REQUIRE( program2_split.size() == 2 );
    REQUIRE( program2_split.at(0) == "(define A 1)" );
    REQUIRE( program2_split.at(1) == "(add1 A)" );

    string program3 = "(define (t a) (add1 a)) \n(add1 s)   (set! a 1)";
    vector<string> program3_split = Parser::split_expressions(program3);
    REQUIRE( program3_split.size() == 3 );
    REQUIRE( program3_split.at(0) == "(define (t a) (add1 a))" );
    REQUIRE( program3_split.at(1) == "(add1 s)" );
    REQUIRE( program3_split.at(2) == "(set! a 1)" );
}

TEST_CASE( "Parser::queue_tokens ", "[parse]" ) {
    queue<string> q = Parser::queue_tokens("(identity x)");
    REQUIRE( q.front() == "(" );
    q.pop();
    REQUIRE( q.front() == "identity" );
    q.pop();
    REQUIRE( q.front() == "x" );
    q.pop();
    REQUIRE( q.front() == ")" );

    q = Parser::queue_tokens("(outer (inner y) x)");
    REQUIRE( q.front() == "(" );
    q.pop();
    REQUIRE( q.front() == "outer" );
    q.pop();
    REQUIRE( q.front() == "(" );
    q.pop();
    REQUIRE( q.front() == "inner" );
    q.pop();
    REQUIRE( q.front() == "y" );
    q.pop();
    REQUIRE( q.front() == ")" );
    q.pop();
    REQUIRE( q.front() == "x" );
    q.pop();
    REQUIRE( q.front() == ")" );
}

TEST_CASE( "Parser::parse", "[parse]" ) {
    string program = "(add1 4)";
    queue<string> tokens = Parser::queue_tokens(program);
    Token* t = Parser::parse(tokens);
    REQUIRE( t -> type == IDENTIFIER );
    REQUIRE( t -> getName() == "add1" );
    REQUIRE( t -> children.size() == 1 );
    REQUIRE( t -> children.at(0) -> getValue() == 4 );
    REQUIRE( t -> toString() == program );

    program = "(outer d (inner a b c))";
    tokens = Parser::queue_tokens(program);
    t = Parser::parse(tokens);
    REQUIRE( t -> type == IDENTIFIER );
    REQUIRE( t -> getName() == "outer" );
    REQUIRE( t -> children.at(0) -> getName() == "d" );
    REQUIRE( t -> children.at(0) -> children.size() == 0 );
    REQUIRE( t -> children.at(1) -> getName() == "inner" );
    REQUIRE( t -> children.size() == 2 );
    REQUIRE( t -> toString() == program );

    program = "(outer (inner a b c) d)";
    tokens = Parser::queue_tokens(program);
    t = Parser::parse(tokens);
    REQUIRE( t -> type == IDENTIFIER );
    REQUIRE( t -> getName() == "outer" );
    REQUIRE( t -> children.at(0) -> getName() == "inner" );
    REQUIRE( t -> children.at(0) -> children.size() == 3 );
    REQUIRE( t -> children.size() == 2 );
    REQUIRE( t -> children.at(1) -> getName() == "d" );
    REQUIRE( t -> toString() == program );
}

TEST_CASE( "Compiler ralloc reference counting", "[compile]" ) {
    Compiler c;
    int first = c.rc_ralloc();
    REQUIRE( first == 0 );
    c.rc_keep_ref(first);
    REQUIRE( c.registers[first] == 2);

    int second = c.rc_ralloc();
    REQUIRE( second == 1 );

    c.rc_free_ref(first);
    c.rc_free_ref(first);
    int third = c.rc_ralloc();
    REQUIRE( third == 0 );
}
/*

TEST_CASE( "Compiler ", "[compile]") {

}

*/

TEST_CASE( "Compiler define", "[compile]") {
    Compiler c;
    REQUIRE(c.asm_data.size() == 1);
    c.compile_program("(define asdf 1)");
    REQUIRE(c.asm_data.at(1) == "asdf:");
    REQUIRE(c.asm_data.at(2) == ".long 1");
    REQUIRE(c.asm_data.size() == 3);
}

TEST_CASE( "Compiler set", "[compile]") {
    Compiler c;
    c.compile_program("(set! asdf 1)");
    REQUIRE(c.asm_code.at(0) == "ld $1, r0\t# 1");
    REQUIRE(c.asm_code.at(1) == "ld $asdf, r1\t# (set! asdf 1)");
    REQUIRE(c.asm_code.at(2) == "st r0, (r1)\t# (set! asdf 1)");
    REQUIRE(c.asm_code.size() == 4);
    REQUIRE(c.all_registers_free());
}

TEST_CASE( "Compiler add", "[compile]") {
    Compiler c;
    c.compile_program("(set! test (+ 2 3))");
    REQUIRE(c.asm_code.at(0) == "ld $2, r0\t# 2");
    REQUIRE(c.asm_code.at(1) == "ld $3, r1\t# 3");
    REQUIRE(c.asm_code.at(2) == "add r0, r1\t# (+ 2 3)");
    REQUIRE(c.asm_code.at(3) == "ld $test, r0\t# (set! test (+ 2 3))");
    REQUIRE(c.asm_code.at(4) == "st r1, (r0)\t# (set! test (+ 2 3))");
    REQUIRE(c.asm_code.size() == 6);
    REQUIRE(c.all_registers_free());

    Compiler c2;
    c2.compile_program("(add1 2)");
    REQUIRE(c2.asm_code.at(0) == "ld $2, r0\t# 2");
    REQUIRE(c2.asm_code.at(1) == "inc r0\t# (add1 2)");

    Compiler c3;
    c3.compile_program("(add1 #true)");
    REQUIRE(c3.asm_code.at(0) == "ld $1, r0\t# 1");
    REQUIRE(c3.asm_code.at(1) == "inc r0\t# (add1 #true)");
}

TEST_CASE( "Compiler sub", "[compile]") {
    Compiler c;
    c.compile_program("(- 1 2)");
    REQUIRE(c.asm_code.at(0) == "ld $1, r0\t# 1");
    REQUIRE(c.asm_code.at(1) == "ld $2, r1\t# 2");
    REQUIRE(c.asm_code.at(2) == "inc r1\t# (add1 2)");
    REQUIRE(c.asm_code.at(3) == "not r1\t# (not (add1 2))");
    REQUIRE(c.asm_code.at(4) == "add r0, r1\t# (+ 1 (not (add1 2)))");
    REQUIRE(c.asm_code.size() == 6);
}