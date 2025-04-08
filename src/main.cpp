#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include <iostream>

int main(int argc, char* argv[]) {
    std::string source = argv[1];  // Changed this line only
    
    try {
        Lexer lexer(source);
        Parser parser(lexer);
        auto ast = parser.parseProgram();
        
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