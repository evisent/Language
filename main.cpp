#include "lexer.h"
#include "Parser.h"
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
    if (!lexer.loadTokensFromFile("../tokens.txt")) {
        std::cerr << "Failed to load token definitions!" << std::endl;
        return 1;
    }
    std::string filename = "../test.txt";
    std::string sourceCode = readFile(filename);
    if (sourceCode.empty()) {
        std::cerr << "No source code found in " << filename << "!" << std::endl;
        return 1;
    }
    std::cout << "Loaded source code from " << filename << std::endl;
    auto tokens = lexer.tokenize(sourceCode);
    writeTokensToFile(tokens, lexer, "../output.txt");
    std::cout << "\n=====================================" << std::endl;
    std::cout << "    SYNTAX ANALYSIS (PARSING)" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    // Создаем парсер с лексером
    Parser parser(lexer, tokens);
    
    std::cout << "\nStarting syntax analysis..." << std::endl;
    
    bool parseSuccess = false;
    try {
        // Если у Parser есть метод, принимающий исходный код:
        parseSuccess = parser.program();
        
        // ИЛИ если нужно сначала установить токены:
        // parser.setTokens(tokens); // если такой метод существует
        // parseSuccess = parser.program();
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR during parsing: " << e.what() << std::endl;
        parseSuccess = false;
    }
    
    // Вывод результата
    if (parseSuccess) {
        std::cout << "✓ Syntax analysis: SUCCESS" << std::endl;
        std::cout << "The program is syntactically correct!" << std::endl;
    } else {
        std::cout << "✗ Syntax analysis: FAILED" << std::endl;
        std::cout << "The program contains syntax errors!" << std::endl;
    }
    std::string outputFile = "syntax.txt";
    // Добавляем результат в файл
    std::ofstream outFile(outputFile, std::ios::app);
    if (outFile.is_open()) {
        outFile << "\n\n=== PARSING RESULT ===" << std::endl;
        outFile << (parseSuccess ? "SUCCESS" : "FAILED") << std::endl;
        outFile.close();
    }
    
    return parseSuccess ? 0 : 1;
}