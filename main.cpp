#include "lexer.h"
#include "Parser.h"
#include "ASTnode.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <functional>  // Добавляем этот заголовок
#include <unordered_map>  // Для exportASTToDOT

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

// Функция для вывода дерева в текстовом виде
void printAST(const std::shared_ptr<ASTNode>& node, const std::string& prefix = "", bool isLast = true) {
    if (!node) return;
    
    std::cout << prefix;
    std::cout << (isLast ? "└── " : "├── ");
    std::cout << node->toString() << std::endl;
    
    // Рекурсивно выводим детей
    const auto& children = node->getChildren();
    for (size_t i = 0; i < children.size(); ++i) {
        printAST(children[i], prefix + (isLast ? "    " : "│   "), i == children.size() - 1);
    }
}

// Функция для записи дерева в файл
void writeASTToFile(const std::shared_ptr<ASTNode>& root, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot create AST output file: " << filename << std::endl;
        return;
    }
    
    // Рекурсивная функция для записи в файл
    std::function<void(const std::shared_ptr<ASTNode>&, const std::string&, bool)> 
    writeNode = [&](const std::shared_ptr<ASTNode>& node, const std::string& prefix, bool isLast) {
        if (!node) return;
        
        file << prefix;
        file << (isLast ? "└── " : "├── ");
        file << node->toString() << "\n";
        
        const auto& children = node->getChildren();
        for (size_t i = 0; i < children.size(); ++i) {
            writeNode(children[i], prefix + (isLast ? "    " : "│   "), i == children.size() - 1);
        }
    };
    
    file << "SYNTAX TREE:\n";
    file << "============\n\n";
    writeNode(root, "", true);
    file.close();
}

// Функция для экспорта в DOT формат (для Graphviz)
void exportASTToDOT(const std::shared_ptr<ASTNode>& root, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot create DOT file: " << filename << std::endl;
        return;
    }
    
    file << "digraph AST {\n";
    file << "  node [shape=box, style=filled, fillcolor=lightblue];\n";
    file << "  rankdir=TB;\n\n";
    
    int nodeId = 0;
    std::unordered_map<void*, int> nodeIds;
    
    // Рекурсивная функция для обхода дерева
    std::function<void(const std::shared_ptr<ASTNode>&)> traverse = 
    [&](const std::shared_ptr<ASTNode>& node) {
        if (!node) return;
        
        int id = nodeId++;
        nodeIds[node.get()] = id;
        file << "  node" << id << " [label=\"" << node->toString() << "\"];\n";
        
        for (const auto& child : node->getChildren()) {
            if (child) {
                traverse(child);
            }
        }
    };
    
    // Создаем связи между узлами
    std::function<void(const std::shared_ptr<ASTNode>&)> createEdges = 
    [&](const std::shared_ptr<ASTNode>& node) {
        if (!node) return;
        
        int parentId = nodeIds[node.get()];
        for (const auto& child : node->getChildren()) {
            if (child && nodeIds.find(child.get()) != nodeIds.end()) {
                int childId = nodeIds[child.get()];
                file << "  node" << parentId << " -> node" << childId << ";\n";
                createEdges(child);
            }
        }
    };
    
    traverse(root);
    file << "\n";
    createEdges(root);
    file << "}\n";
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
    
    std::cout << "\nStarting syntax analysis and AST construction..." << std::endl;
    
    bool parseSuccess = false;
    std::shared_ptr<ASTNode> astRoot = nullptr;
    
    try {
        // Парсим программу и получаем корень AST
        astRoot = parser.program();
        parseSuccess = (astRoot != nullptr);
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR during parsing: " << e.what() << std::endl;
        parseSuccess = false;
    }
    
    // Вывод результата
    if (parseSuccess && astRoot) {
        std::cout << "✓ Syntax analysis: SUCCESS" << std::endl;
        std::cout << "The program is syntactically correct!" << std::endl;
        
        // Выводим синтаксическое дерево
        std::cout << "\n=====================================" << std::endl;
        std::cout << "    ABSTRACT SYNTAX TREE (AST)" << std::endl;
        std::cout << "=====================================" << std::endl;
        printAST(astRoot);
        
        // Сохраняем AST в файл
        std::string astTextFile = "ast.txt";
        writeASTToFile(astRoot, astTextFile);
        std::cout << "\nAST saved to: " << astTextFile << std::endl;
        
        // Экспортируем в DOT формат для визуализации
        std::string astDotFile = "ast.dot";
        exportASTToDOT(astRoot, astDotFile);
        std::cout << "DOT format saved to: " << astDotFile << std::endl;
        std::cout << "To visualize: dot -Tpng " << astDotFile << " -o ast.png" << std::endl;
        
    } else {
        std::cout << "✗ Syntax analysis: FAILED" << std::endl;
        std::cout << "The program contains syntax errors!" << std::endl;
        if (!astRoot) {
            std::cout << "AST root is null!" << std::endl;
        }
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