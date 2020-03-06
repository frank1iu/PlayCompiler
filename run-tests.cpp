#include "catch.hpp"
#include "parser.hpp"
#include "compiler.hpp"

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

    program = "(outer d (inner a b c))";
    tokens = Parser::queue_tokens(program);
    t = Parser::parse(tokens);
    REQUIRE( t -> type == IDENTIFIER );
    REQUIRE( t -> getName() == "outer" );
    REQUIRE( t -> children.at(0) -> getName() == "d" );
    REQUIRE( t -> children.at(0) -> children.size() == 0 );
    REQUIRE( t -> children.at(1) -> getName() == "inner" );
    REQUIRE( t -> children.size() == 2 );

    program = "(outer (inner a b c) d)";
    tokens = Parser::queue_tokens(program);
    t = Parser::parse(tokens);
    REQUIRE( t -> type == IDENTIFIER );
    REQUIRE( t -> getName() == "outer" );
    REQUIRE( t -> children.at(0) -> getName() == "inner" );
    REQUIRE( t -> children.at(0) -> children.size() == 3 );
    REQUIRE( t -> children.size() == 2 );
    REQUIRE( t -> children.at(1) -> getName() == "d" );
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

TEST_CASE( "Compiler::load", "[compile]") {
    Compiler c;
    Token *t = new Token("test");
    int i;
    vector<string> p = c.load_into_register(t, &i);
    REQUIRE( i == 0 );
    REQUIRE( p.at(0) == "\tld $test, r0" );
    REQUIRE( p.at(1) == "\tld (r0), r0" );
    REQUIRE( p.size() == 2 );

    c.rc_free_ref(i);
    delete t;
    t = new Token("123");
    p = c.load_into_register(t, &i);
    REQUIRE( i == 0 );
    REQUIRE( p.at(0) == "\tld $123, r0" );
    REQUIRE( p.size() == 1 );
    delete t;
}

TEST_CASE( "Compiler::eval", "[compile]") {
    Compiler c;
    Token *t = new Token("test");
    int i;
    vector<string> p = c.eval(t, &i);
    REQUIRE( i == 0 );
    REQUIRE( p.at(0) == "\tld $test, r0" );
    REQUIRE( p.at(1) == "\tld (r0), r0" );
    REQUIRE( p.size() == 2 );
    REQUIRE( c.registers[0] == 1 );

    c.rc_free_ref(i);
    REQUIRE( c.registers[0] == 0 );

    t = Parser::parse_expression("(+ 1 2)");
    p = c.eval(t, &i);
    REQUIRE( i == 1 );
    REQUIRE( p.at(0) == "\tld $1, r0" );
    REQUIRE( p.at(1) == "\tld $2, r1" );
    REQUIRE( p.at(2) == "\tadd r0, r1" );
    REQUIRE( p.size() == 3 );
}

TEST_CASE( "Compiler::ref/deref", "[compile]") {
    Compiler c;
    Token *t = Parser::parse_expression("(& name)");
    int dest;
    vector<string> p = c.eval(t, &dest);
    REQUIRE( p.at(0) == "\tld $name, r0" );
    REQUIRE( p.size() == 1 );
    REQUIRE( dest == 0 );
    c.rc_free_ref(dest);

    t = Parser::parse_expression("(* 100)");
    p = c.eval(t, &dest);
    REQUIRE( p.at(0) == "\tld $100, r0" );
    REQUIRE( p.at(1) == "\tld (r0), r1" );
    REQUIRE( p.size() == 2 );
    REQUIRE( dest == 1 );
    c.rc_free_ref(dest);
    REQUIRE( c.all_registers_free() );
}