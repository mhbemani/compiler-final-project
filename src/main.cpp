#include "lexer.h"
#include "parser.h"
#include "optimizer.h"
#include "codegen.h"
#include <iostream>
/*
    to run it in lli:
       copy llvm ir output in main.ll
       llc -filetype=obj main.ll -o main.o
       clang main.o -o main
       ./main 
*/ 

// no support for "bool a = true, f, s = false;" structure
// no minus before ()
// for and foreach (mostly for) has some problems, check its abilities and fix them
// try-catch might have problems on hadeling exceptions
// equality for array elements
int main(int argc, char* argv[]) {
    std::string source = argv[1];  // Changed this line only
    
    try {
        Lexer lexer(source);
        Parser parser(lexer);
        auto ast = parser.parseProgram();
        
    // std::cout << "Before Optimization:\n" << ast->toString() << "\n";
    // std::cout << "\nBefore Optimization: " << optimizer.printNode(*ast) << "\n\n";
    Optimizer optimizer;
    optimizer.optimize(*ast);
    // optimizer.printModifiedNodes();
    // std::cout << "\nAfter Optimization: " << optimizer.printNode(*ast) << "\n\n";

    // std::cout << "After Optimization:\n" << ast->toString() << "\n";

        CodeGen codegen;
        codegen.generate(*ast);
        codegen.dump();
    } 
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}