#include "lexer.h"
#include "Parser.h"
#include "ASTnode.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <unordered_map>

void writePOLIZToFile(const std::shared_ptr<ASTNode>& root, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot create POLIZ output file: " << filename << std::endl;
        return;
    }
    
    file << "POLIZ (Reverse Polish Notation):\n";
    file << "================================\n\n";
    
    if (root) {
        std::string poliz = root->toPOLIZ();
        file << poliz;
    }
    
    file.close();
}

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

void printAST(const std::shared_ptr<ASTNode>& node, const std::string& prefix = "", bool isLast = true) {
    if (!node) return;    
    std::cout << prefix;
    std::cout << (isLast ? "└── " : "├── ");
    std::cout << node->toString() << std::endl;
    const auto& children = node->getChildren();
    for (size_t i = 0; i < children.size(); ++i) {
        printAST(children[i], prefix + (isLast ? "    " : "│   "), i == children.size() - 1);
    }
}

void writeASTNodeToFile(std::ofstream& file, const std::shared_ptr<ASTNode>& node, const std::string& prefix, bool isLast) {
    if (!node) return;
    file << prefix;
    file << (isLast ? "└── " : "├── ");
    file << node->toString() << "\n";
    const auto& children = node->getChildren();
    for (size_t i = 0; i < children.size(); ++i) {
        writeASTNodeToFile(file, children[i], prefix + (isLast ? "    " : "│   "), i == children.size() - 1);
    }
}

void writeASTToFile(const std::shared_ptr<ASTNode>& root, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot create AST output file: " << filename << std::endl;
        return;
    }
    file << "SYNTAX TREE:\n";
    file << "============\n\n";
    writeASTNodeToFile(file, root, "", true);
    file.close();
}

void traverseASTForDOT(std::ofstream& file, std::unordered_map<void*, int>& nodeIds, int& nodeId, const std::shared_ptr<ASTNode>& node) {
    if (!node) return;
    int id = nodeId++;
    nodeIds[node.get()] = id;
    file << "  node" << id << " [label=\"" << node->toString() << "\"];\n";
    for (const auto& child : node->getChildren()) {
        if (child) {
            traverseASTForDOT(file, nodeIds, nodeId, child);
        }
    }
}

void createEdgesForDOT(std::ofstream& file, std::unordered_map<void*, int>& nodeIds, const std::shared_ptr<ASTNode>& node) {
    if (!node) return;
    int parentId = nodeIds[node.get()];
    for (const auto& child : node->getChildren()) {
        if (child && nodeIds.find(child.get()) != nodeIds.end()) {
            int childId = nodeIds[child.get()];
            file << "  node" << parentId << " -> node" << childId << ";\n";
            createEdgesForDOT(file, nodeIds, child);
        }
    }
}

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
    
    traverseASTForDOT(file, nodeIds, nodeId, root);
    file << "\n";
    createEdgesForDOT(file, nodeIds, root);
    
    file << "}\n";
    file.close();
}
                
int main() {
    std::cout << "=====================================" << std::endl;
    std::cout << "    LEXICAL ANALYSIS" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    Lexer lexer;
    if (!lexer.loadTokensFromFile("tokens.txt")) {
        std::cerr << "Failed to load token definitions!" << std::endl;
        return 1;
    }
    
    std::string filename = "test.txt";
    std::string sourceCode = readFile(filename);
    if (sourceCode.empty()) {
        std::cerr << "No source code found in " << filename << "!" << std::endl;
        return 1;
    }
    
    std::cout << "Loaded source code from " << filename << std::endl;
    std::cout << "\nSource code:\n" << sourceCode << std::endl;
    
    auto tokens = lexer.tokenize(sourceCode);
    writeTokensToFile(tokens, lexer, "output.txt");
    
    std::cout << "\n=====================================" << std::endl;
    std::cout << "    SYNTAX ANALYSIS (PARSING)" << std::endl;
    std::cout << "=====================================" << std::endl;
    
    Parser parser(lexer, tokens);
    
    std::cout << "\nStarting syntax analysis and AST construction..." << std::endl;
    
    bool parseSuccess = false;
    std::shared_ptr<ASTNode> astRoot = nullptr;
    
    try {
        astRoot = parser.program();
        parseSuccess = (astRoot != nullptr);
    } catch (const std::exception& e) {
        std::cerr << "ERROR during parsing: " << e.what() << std::endl;
        parseSuccess = false;
    }
    
    if (parseSuccess && astRoot) {
        std::cout << "✓ Syntax analysis: SUCCESS" << std::endl;
        std::cout << "The program is syntactically correct!" << std::endl;
        
        std::cout << "\n=====================================" << std::endl;
        std::cout << "    ABSTRACT SYNTAX TREE (AST)" << std::endl;
        std::cout << "=====================================" << std::endl;
        printAST(astRoot);
        
        std::string astTextFile = "ast.txt";
        writeASTToFile(astRoot, astTextFile);
        std::cout << "\nAST saved to: " << astTextFile << std::endl;
        
        std::string astDotFile = "ast.dot";
        exportASTToDOT(astRoot, astDotFile);
        std::cout << "DOT format saved to: " << astDotFile << std::endl;
        std::cout << "To visualize: dot -Tpng " << astDotFile << " -o ast.png" << std::endl;
        
        std::cout << "\n=====================================" << std::endl;
        std::cout << "    SEMANTIC ANALYSIS" << std::endl;
        std::cout << "=====================================" << std::endl;
        
        parser.printSemanticResults();
        
        std::string semanticFile = "semantic_results.txt";
        std::ofstream semFile(semanticFile);
        if (semFile.is_open()) {
            std::streambuf* coutbuf = std::cout.rdbuf();            std::cout.rdbuf(semFile.rdbuf());
            parser.printSemanticResults();
            std::cout.rdbuf(coutbuf);
            semFile.close();
            std::cout << "\nSemantic results saved to: " << semanticFile << std::endl;
        }
        std::cout << "\n=====================================" << std::endl;
        std::cout << "    POLIZ GENERATION" << std::endl;
        std::cout << "=====================================" << std::endl;
        
        std::string polizCode = astRoot->toPOLIZ();
        std::cout << "\nGenerated POLIZ:\n" << polizCode << std::endl;
        
        std::string polizFile = "poliz.txt";
        writePOLIZToFile(astRoot, polizFile);
        std::cout << "\nPOLIZ saved to: " << polizFile << std::endl;
    } else {
        std::cout << "✗ Syntax analysis: FAILED" << std::endl;
        std::cout << "The program contains syntax errors!" << std::endl;
        if (!astRoot) {
            std::cout << "AST root is null!" << std::endl;
        }
    }
    
    std::string outputFile = "syntax.txt";
    std::ofstream outFile(outputFile, std::ios::app);
    if (outFile.is_open()) {
        outFile << "\n\n=== PARSING RESULT ===" << std::endl;
        outFile << (parseSuccess ? "SUCCESS" : "FAILED") << std::endl;
        outFile.close();
    }
    
    return parseSuccess ? 0 : 1;
}