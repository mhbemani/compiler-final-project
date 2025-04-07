#include "../src/lexer.h"
#include <cassert>
#include <iostream>

void test_lexer() {
    std::string code = "int x = 42;";
    Lexer lexer(code);
    
    auto tokens = lexer.tokenize();
    assert(tokens[0].type == TokenType::INT);
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].lexeme == "x");
    // ... more assertions
}

int main() {
    test_lexer();
    std::cout << "Lexer tests passed!\n";
    return 0;
}