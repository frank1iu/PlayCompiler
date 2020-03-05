all: runTests main

.PHONY: clean

parser.o: parser.cpp parser.hpp
	g++ -g -o parser.o parser.cpp -c

test.o: test.cpp
	g++ -g -o test.o test.cpp -c

run-tests.o: run-tests.cpp
	g++ -g -o run-tests.o run-tests.cpp -c

compiler.o: compiler.cpp compiler.hpp
	g++ -g -o compiler.o compiler.cpp -c

runTests: test.o parser.o run-tests.o compiler.o
	g++ -g -o runTests test.o parser.o run-tests.o compiler.o

main.o: main.cpp
	g++ -g -o main.o main.cpp -c

main: main.o parser.o compiler.o
	g++ -g -o main main.o parser.o compiler.o

clean:
	rm *.o runTests