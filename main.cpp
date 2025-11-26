#include "lexer.h"
#include <iostream>
#include <fstream>
#include <sstream>

bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main() {
    Lexer lexer;
    
    // Пробуем разные пути к tokens.txt
    std::string tokensPath;
    const char* possiblePaths[] = {
        "tokens.txt",
        "../tokens.txt",
        "../../tokens.txt",
        "bin/tokens.txt"
    };
    
    for (const char* path : possiblePaths) {
        if (fileExists(path)) {
            tokensPath = path;
            break;
        }
    }
    
    if (tokensPath.empty()) {
        std::cerr << "Cannot find tokens.txt file!" << std::endl;
        std::cerr << "Please ensure tokens.txt is in the same directory as the executable." << std::endl;
        return 1;
    }
    
    std::cout << "Found tokens file: " << tokensPath << std::endl;
    
    // Load token definitions
    if (!lexer.loadTokensFromFile(tokensPath)) {
        std::cerr << "Failed to load token definitions!" << std::endl;
        return 1;
    }
    
    // Остальной код без изменений...
    std::string sourceCode = R"(
        class MyClass {
            int x = 10;
            constructor(int value) {
                this.x = value;
            }
        }
    )";
    
    std::cout << "\nTokenizing source code...\n" << std::endl;
    auto tokens = lexer.tokenize(sourceCode);
    lexer.printTokens(tokens);
    
    return 0;
}