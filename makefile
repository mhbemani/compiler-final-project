CC = clang
CXX = clang++
LLVM_PREFIX = /opt/homebrew/opt/llvm

CFLAGS = -std=c11 -g -O2 -I$(LLVM_PREFIX)/include
CXXFLAGS = -g -O2 -I$(LLVM_PREFIX)/include
LDFLAGS = -L$(LLVM_PREFIX)/lib
LIBS = -lLLVMCore -lLLVMSupport -lLLVMBitstreamReader -lLLVMRemarks -lLLVMDemangle -lz -lcurses -lm

SRCS = main.c lexer.c parser.c codegen.c ast.c
OBJS = $(SRCS:.c=.o)

compiler: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f compiler *.o