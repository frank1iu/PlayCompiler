FLAGS = -std=c++2a -c -Wall -Wextra -pedantic
LD_FLAGS = -std=c++2a -lpthread -lm
ifdef WASM
# Compilation to WASM results in slow execution
# Added -O3 flag to hopefully improve performance
FLAGS += -fexceptions -O3
LD_FLAGS += -fexceptions -O3
CXX = em++
OBJ = main.html
else
FLAGS += -g -O0
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

ifdef WASM
$(OBJ): main.o parser.o compiler.o runtime.o
	$(CXX) $(LD_FLAGS) -o $(OBJ) runtime.o main.o parser.o compiler.o
	cp main_patched.html main.html
else
$(OBJ): main.o parser.o compiler.o runtime.o
	$(CXX) $(LD_FLAGS) -o $(OBJ) runtime.o main.o parser.o compiler.o
endif

clean:
	rm *.o runTests main main.html main.wasm main.js || true
