LLVM_PREFIX = /opt/homebrew/opt/llvm
CXX = $(LLVM_PREFIX)/bin/clang++
CXXFLAGS = -std=c++17 -g -Wall -fexceptions -I$(LLVM_PREFIX)/include -I$(shell xcrun --show-sdk-path)/usr/include
LDFLAGS = -L$(LLVM_PREFIX)/lib $(shell $(LLVM_PREFIX)/bin/llvm-config --ldflags)
LIBS = $(shell $(LLVM_PREFIX)/bin/llvm-config --libs core irreader support)

SRC = main.cpp lexer.cpp parser.cpp codegen.cpp
OBJ = $(SRC:.cpp=.o)

compiler: $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f *.o compiler