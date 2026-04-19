#pragma once

#include "Lexer.h"
#include "ASTnode.h"
#include "semantic.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <stack>
#include <memory>

class Parser {
private:
    Lexer lexer;
    std::vector<std::pair<int, std::string>> tokens;
    size_t cur = 0;  // Используйте size_t вместо long long для совместимости
    
    // Поля для семантического анализа
    SemanticAnalyzer semanticAnalyzer;
    bool enableSemantic = true;
    std::stack<std::string> currentFunction;  // для отслеживания текущей функции
    
    // Работа с токенами
    std::pair<int, std::string> GetNextToken();
    std::pair<int, std::string> PeekToken();
    std::pair<int, std::string> PeekNextToken();
    bool IsAtEnd();
    
    // Вспомогательные функции для семантики
    Type getExpressionType(const std::shared_ptr<ASTNode>& expr);
    DataType stringToDataType(const std::string& typeStr);
    std::vector<Type> collectArgumentTypes(const std::shared_ptr<ASTNode>& argsNode);
    
    // Парсинг классов
    std::shared_ptr<ASTNode> parseClass();
    std::shared_ptr<ASTNode> parseClassName();
    std::shared_ptr<ASTNode> parseClassBody();
    std::shared_ptr<ASTNode> parseClassMember();
    
    // Парсинг функций
    std::shared_ptr<ASTNode> parseFunction();
    std::shared_ptr<ASTNode> parseMain();
    std::shared_ptr<ASTNode> parseFunctionName();
    std::shared_ptr<ASTNode> parseFunctionBody();
    std::shared_ptr<ASTNode> parseMainBody();
    std::shared_ptr<ASTNode> parseParameters();
    std::shared_ptr<ASTNode> parseParameter();
    
    // Парсинг методов и конструкторов
    std::shared_ptr<ASTNode> parseMethod();
    std::shared_ptr<ASTNode> parseMethodBody();
    std::shared_ptr<ASTNode> parseConstructor();
    std::shared_ptr<ASTNode> parseConstructorParameters();
    std::shared_ptr<ASTNode> parseConstructorBody();
    std::shared_ptr<ASTNode> parseDestructor();
    std::shared_ptr<ASTNode> parseDestructorBody();
    
    // Парсинг объявлений
    std::shared_ptr<ASTNode> parseFieldDeclaration();
    std::shared_ptr<ASTNode> parseVariableDeclaration();
    std::shared_ptr<ASTNode> parseArrayDeclaration();
    std::shared_ptr<ASTNode> parseDeclarationStatement();
    
    // Парсинг операторов
    std::shared_ptr<ASTNode> parseStatement();
    std::shared_ptr<ASTNode> parseCompoundStatement();
    std::shared_ptr<ASTNode> parseExpressionStatement();
    std::shared_ptr<ASTNode> parseIfStatement();
    std::shared_ptr<ASTNode> parseWhileStatement();
    std::shared_ptr<ASTNode> parseForStatement();
    std::shared_ptr<ASTNode> parseForInit();
    std::shared_ptr<ASTNode> parseReturnStatement();
    std::shared_ptr<ASTNode> parseBreakStatement();
    std::shared_ptr<ASTNode> parseContinueStatement();
    std::shared_ptr<ASTNode> parsePrintStatement();
    std::shared_ptr<ASTNode> parseReadStatement();
    
    // Парсинг выражений
    std::shared_ptr<ASTNode> parseExpression();
    std::shared_ptr<ASTNode> parseExpressionList();
    std::shared_ptr<ASTNode> parseAssignmentExpression();
    std::shared_ptr<ASTNode> parseConditionalExpression();
    std::shared_ptr<ASTNode> parseLogicalOrExpression();
    std::shared_ptr<ASTNode> parseLogicalAndExpression();
    std::shared_ptr<ASTNode> parseBitwiseOrExpression();
    std::shared_ptr<ASTNode> parseBitwiseXorExpression();
    std::shared_ptr<ASTNode> parseBitwiseAndExpression();
    std::shared_ptr<ASTNode> parseEqualityExpression();
    std::shared_ptr<ASTNode> parseRelationalExpression();
    std::shared_ptr<ASTNode> parseShiftExpression();
    std::shared_ptr<ASTNode> parseAdditiveExpression();
    std::shared_ptr<ASTNode> parseMultiplicativeExpression();
    std::shared_ptr<ASTNode> parseCastExpression();
    std::shared_ptr<ASTNode> parseUnaryExpression();
    std::shared_ptr<ASTNode> parsePostfixExpression();
    std::shared_ptr<ASTNode> parsePrimaryExpression();
    
    // Базовые элементы
    std::shared_ptr<ASTNode> parseType();
    std::shared_ptr<ASTNode> parseIdentifier();
    std::shared_ptr<ASTNode> parseLiteral();
    std::shared_ptr<ASTNode> parseLetter();
    std::shared_ptr<ASTNode> parseDigit();
    std::shared_ptr<ASTNode> parseAssignmentOperator();
    std::shared_ptr<ASTNode> parseUnaryOperator();
    
    // Создание узлов AST
    std::shared_ptr<ASTNode> createNode(NodeType type, const std::string& value = "");
    std::shared_ptr<ASTNode> createBinaryExprNode(const std::string& op, 
                                                  std::shared_ptr<ASTNode> left, 
                                                  std::shared_ptr<ASTNode> right);
    std::shared_ptr<ASTNode> createUnaryExprNode(const std::string& op, 
                                                 std::shared_ptr<ASTNode> operand);
    
public:
    Parser(Lexer& lex, const std::vector<std::pair<int, std::string>>& toks) 
        : lexer(lex), tokens(toks) {
        semanticAnalyzer.enterScope(); // глобальная область видимости
    }
    
    std::shared_ptr<ASTNode> program();
    
    // Методы для доступа к результатам семантического анализа
    bool hasSemanticErrors() const { return semanticAnalyzer.hasErrors(); }
    bool hasSemanticWarnings() const { return semanticAnalyzer.hasWarnings(); }
    void printSemanticResults() const { semanticAnalyzer.printResults(); }
    
    // Очистка семантического анализатора (для повторного использования)
    void clearSemantic() { 
        semanticAnalyzer.clear(); 
        semanticAnalyzer.enterScope();
    }
    
    // Включение/выключение семантического анализа
    void setSemanticEnabled(bool enabled) { enableSemantic = enabled; }
};

