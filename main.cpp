#include "lexer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void writeTokensToFile(const std::vector<std::pair<int, std::string>>& tokens, Lexer& lexer, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot create output file: " << filename << std::endl;
        return;
    }
    
    // Записываем только таблицу токенов (без summary)
    file << std::left << std::setw(20) << "TYPE" << "VALUE" << std::endl;
    file << std::string(50, '-') << std::endl;
    
    for (const auto& token : tokens) {
        std::string typeName = lexer.getTokenTypeName(token.first);
        file << std::left << std::setw(20) << typeName << "'" << token.second << "'" << std::endl;
    }
    
    file.close();
}

int main() {
    Lexer lexer;
    
    // Load token definitions
    if (!lexer.loadTokensFromFile("../tokens.txt")) {
        std::cerr << "Failed to load token definitions!" << std::endl;
        return 1;
    }
    
    // Read source code from file
    std::string filename = "../test.txt";
    std::string sourceCode = readFile(filename);
    
    if (sourceCode.empty()) {
        std::cerr << "No source code found in " << filename << "!" << std::endl;
        return 1;
    }
    
    std::cout << "Loaded source code from " << filename << std::endl;
    
    // Tokenize
    std::cout << "Tokenizing source code..." << std::endl;
    auto tokens = lexer.tokenize(sourceCode);
    
    // Write results to output.txt (только таблица токенов)
    writeTokensToFile(tokens, lexer, "../output.txt");
    
    std::cout << "Tokenization completed! Results written to output.txt" << std::endl;
    std::cout << "Total tokens: " << tokens.size() << std::endl;
    
    return 0;
}