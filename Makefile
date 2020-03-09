all: runTests main

.PHONY: all clean

FLAGS = -std=c++2a -stdlib=libc++ -c -g -O0 -Wall -Wextra -pedantic
LD_FLAGS = -std=c++2a -stdlib=libc++ -lc++abi -lpthread -lm

parser.o: parser.cpp parser.hpp
	clang++ $(FLAGS) -o parser.o parser.cpp

test.o: test.cpp
	clang++ $(FLAGS) -o test.o test.cpp

run-tests.o: run-tests.cpp
	clang++ $(FLAGS) -o run-tests.o run-tests.cpp

compiler.o: compiler.cpp compiler.hpp
	clang++ $(FLAGS) -o compiler.o compiler.cpp

runTests: test.o parser.o run-tests.o compiler.o
	clang++ $(LD_FLAGS) -o runTests test.o parser.o run-tests.o compiler.o

main.o: main.cpp
	clang++ $(FLAGS) -o main.o main.cpp

main: main.o parser.o compiler.o
	clang++ $(LD_FLAGS) -o main main.o parser.o compiler.o

clean:
	rm *.o runTests main