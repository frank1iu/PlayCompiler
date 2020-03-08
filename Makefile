all: runTests main

.PHONY: all clean

FLAGS = -g -O0 -Wall

parser.o: parser.cpp parser.hpp
	g++ $(FLAGS) -o parser.o parser.cpp -c

test.o: test.cpp
	g++ $(FLAGS) -o test.o test.cpp -c

run-tests.o: run-tests.cpp
	g++ $(FLAGS) -o run-tests.o run-tests.cpp -c

compiler.o: compiler.cpp compiler.hpp
	g++ $(FLAGS) -o compiler.o compiler.cpp -c

runTests: test.o parser.o run-tests.o compiler.o
	g++ $(FLAGS) -o runTests test.o parser.o run-tests.o compiler.o

main.o: main.cpp
	g++ $(FLAGS) -o main.o main.cpp -c

main: main.o parser.o compiler.o
	g++ $(FLAGS) -o main main.o parser.o compiler.o

clean:
	rm *.o runTests main