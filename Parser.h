#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "ASTnode.h"
#include <string>
#include <vector>
#include <unordered_map>

class Parser{
private:
    Lexer lexer;
    std::vector<std::pair<int, std::string>> tokens;
    long long cur = 0;
    
    // Вспомогательные методы доступа к токенам
    std::pair<int, std::string> GetNextToken();
    std::pair<int, std::string> PeekToken();
    std::pair<int, std::string> PeekNextToken();
    bool IsAtEnd();
    
    // Основные парсеры (уже были)
    std::shared_ptr<ASTNode> parseClass();
    std::shared_ptr<ASTNode> parseFunction();
    std::shared_ptr<ASTNode> parseMain();
    std::shared_ptr<ASTNode> parseClassName();
    std::shared_ptr<ASTNode> parseClassBody();
    std::shared_ptr<ASTNode> parseType();
    std::shared_ptr<ASTNode> parseParameters();
    std::shared_ptr<ASTNode> parseFunctionName();
    std::shared_ptr<ASTNode> parseFunctionBody();
    std::shared_ptr<ASTNode> parseMainBody();
    std::shared_ptr<ASTNode> parseIdentifier();
    std::shared_ptr<ASTNode> parseClassMember();
    std::shared_ptr<ASTNode> parseParameter();
    std::shared_ptr<ASTNode> parseCompoundStatement();
    std::shared_ptr<ASTNode> parseLetter();
    std::shared_ptr<ASTNode> parseDigit();
    std::shared_ptr<ASTNode> parseFieldDeclaration();
    std::shared_ptr<ASTNode> parseConstructor();
    std::shared_ptr<ASTNode> parseDestructor();
    std::shared_ptr<ASTNode> parseMethod();
    std::shared_ptr<ASTNode> parseStatement();
    std::shared_ptr<ASTNode> parseConstructorParameters();
    std::shared_ptr<ASTNode> parseConstructorBody();
    std::shared_ptr<ASTNode> parseDestructorBody();
    std::shared_ptr<ASTNode> parseMethodBody();
    std::shared_ptr<ASTNode> parseDeclarationStatement();
    std::shared_ptr<ASTNode> parseExpressionStatement();
    std::shared_ptr<ASTNode> parseIfStatement();
    std::shared_ptr<ASTNode> parseWhileStatement();
    std::shared_ptr<ASTNode> parseForStatement();
    std::shared_ptr<ASTNode> parseReturnStatement();
    std::shared_ptr<ASTNode> parseBreakStatement();
    std::shared_ptr<ASTNode> parseContinueStatement();
    std::shared_ptr<ASTNode> parsePrintStatement();
    std::shared_ptr<ASTNode> parseReadStatement();
    std::shared_ptr<ASTNode> parseVariableDeclaration();
    std::shared_ptr<ASTNode> parseArrayDeclaration();
    std::shared_ptr<ASTNode> parseExpression();
    
    // Новые парсеры для выражений (добавлены)
    std::shared_ptr<ASTNode> parseForInit();
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
    std::shared_ptr<ASTNode> parseLiteral();
    
    // Вспомогательные функции для операторов
    std::shared_ptr<ASTNode> parseAssignmentOperator();
    std::shared_ptr<ASTNode> parseUnaryOperator();
    std::shared_ptr<ASTNode> createNode(NodeType type, const std::string& value = "");
    std::shared_ptr<ASTNode> createBinaryExprNode(const std::string& op, 
                                                  std::shared_ptr<ASTNode> left, 
                                                  std::shared_ptr<ASTNode> right);
    std::shared_ptr<ASTNode> createUnaryExprNode(const std::string& op, 
                                                 std::shared_ptr<ASTNode> operand);
    
public:
    // Конструктор
    Parser(Lexer& lex, std::vector<std::pair<int, std::string>> toks) : lexer(lex), tokens(toks){}
    
    // Основной метод
    std::shared_ptr<ASTNode> program();
};

#endif