all: runTests main

.PHONY: all clean

CXX = clang++

FLAGS = -std=c++2a -stdlib=libc++ -c -g -O0 -Wall -Wextra -pedantic
LD_FLAGS = -std=c++2a -stdlib=libc++ -lc++abi -lpthread -lm

parser.o: parser.cpp parser.hpp
	$(CXX) $(FLAGS) -o parser.o parser.cpp

test.o: test.cpp
	$(CXX) $(FLAGS) -o test.o test.cpp

run-tests.o: run-tests.cpp
	$(CXX) $(FLAGS) -o run-tests.o run-tests.cpp

compiler.o: compiler.cpp compiler.hpp
	$(CXX) $(FLAGS) -o compiler.o compiler.cpp

runtime.o: runtime.cpp runtime.hpp
	$(CXX) $(FLAGS) -o runtime.o runtime.cpp

runTests: test.o parser.o run-tests.o compiler.o runtime.o
	$(CXX) $(LD_FLAGS) -o runTests runtime.o test.o parser.o run-tests.o compiler.o
	#./runTests

main.o: main.cpp
	$(CXX) $(FLAGS) -o main.o main.cpp

main: main.o parser.o compiler.o runtime.o
	$(CXX) $(LD_FLAGS) -o main runtime.o main.o parser.o compiler.o

clean:
	rm *.o runTests main
