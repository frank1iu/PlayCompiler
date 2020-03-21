#include "catch.hpp"
#include "parser.hpp"
#include "compiler.hpp"
#include "runtime.hpp"
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

TEST_CASE( "Test files integrety", "[file]") {
    system("cd tests && make");
}

#define ALIGN 4
TEST_CASE( "RuntimeStack", "[stack]") {
    RuntimeStack rs;
    rs.push("test", ALIGN);
    REQUIRE(rs.offset_of("test") == 0);
    rs.push("test2", ALIGN);
    REQUIRE(rs.offset_of("test") == 4);
    REQUIRE(rs.offset_of("test2") == 0);
    rs.new_frame();
    rs.push("test3", ALIGN);
    REQUIRE(rs.offset_of("test") == 8);
    REQUIRE(rs.offset_of("test2") == 4);
    REQUIRE(rs.offset_of("test3") == 0);
    rs.destroy_frame();
    REQUIRE(rs.offset_of("test") == 4);
    REQUIRE(rs.offset_of("test2") == 0);
}

TEST_CASE( "FunctionTable", "[fn]") {
    FunctionTable ft;
    vector<string> args1 = {"a", "b", "c"};
    ft.define("fn1", args1);
    REQUIRE(!ft.exists("asdf"));
    REQUIRE(ft.exists("fn1"));
    auto f = ft.find("fn1");
    REQUIRE(f -> argc == 3);
    REQUIRE(f -> name == "fn1");
    REQUIRE(f -> args.at(0) == "a");
    REQUIRE(f -> args.at(1) == "b");
    REQUIRE(f -> args.at(2) == "c");
}