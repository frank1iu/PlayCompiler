# Uncomment next line to compile to web (main.html, main.js, main.wasm)
# Compiling to web requires Emscripten. https://emscripten.org/
# WASM = true

FLAGS = -std=c++2a -c -g -O0 -Wall -Wextra -pedantic
LD_FLAGS = -std=c++2a -lpthread -lm
ifdef WASM
CXX = em++
OBJ = main.html
else
CXX = clang++
OBJ = main
endif

all: runTests $(OBJ)

.PHONY: all clean

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

main.o: main.cpp
	$(CXX) $(FLAGS) -o main.o main.cpp

$(OBJ): main.o parser.o compiler.o runtime.o
	$(CXX) $(LD_FLAGS) -o $(OBJ) runtime.o main.o parser.o compiler.o

clean:
	rm *.o runTests $(OBJ)
