#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include <iostream>

int main() {
    std::string source = "int a = 2; a = 4; String asd = \"asd\"; asd = \"ewd\";";
    
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