#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include <string>
#include <vector>
#include <unordered_map>

class Parser{
private:
    bool IsAtEnd();
    Lexer lexer;
    std::vector<std::pair<int, std::string>> tokens;
    long long cur = 0;
public:
    bool program();
    bool parseClass();
    bool parseFunction();
    bool parseMain();
    bool parseClassName();
    bool parseClassBody();
    bool parseType();
    bool parseParameters();
    bool parseFunctionName();
    bool parseFunctionBody();
    bool parseMainBody();
    bool parseIdentifier();
    bool parseClassMember();
    bool parseParameter();
    bool parseCompoundStatement();
    bool parseLetter();
    bool parseDigit();
    bool parseFieldDeclaration();
    bool parseConstructor();
    bool parseDestructor();
    bool parseMethod();
    bool parseStatement();
    bool parseConstructorParameters();
    bool parseConstructorBody();
    bool parseDestructorBody();
    bool parseMethodBody();
    bool parseDeclarationStatement();
    bool parseExpressionStatement();
    bool parseIfStatement();
    bool parseWhileStatement();
    bool parseForStatement();
    bool parseReturnStatement();
    bool parseBreakStatement();
    bool parseContinueStatement();
    bool parsePrintStatement();
    bool parseReadStatement();
    bool parseVariableDeclaration();
    bool parseArrayDeclaration();
    bool parseExpression();
    bool parseForInit();
    bool parseExpressionList();
    bool parseAssignmentExpression();
    bool parseConditionalExpression();
    bool parseLogicalOrExpression();
    bool parseLogicalAndExpression();
    bool parseBitwiseOrExpression();
    bool parseBitwiseXorExpression();
    bool parseBitwiseAndExpression();
    bool parseEqualityExpression();
    bool parseRelationalExpression();
    bool parseShiftExpression();
    bool parseAdditiveExpression();
    bool parseMultiplicativeExpression();
    bool parseCastExpression();
    bool parseUnaryExpression();
    bool parsePostfixExpression();
    bool parsePrimaryExpression();
    bool parseLiteral();
    std::pair<int, std::string> Parser::GetNextToken();
    std::pair<int, std::string> Parser::PeekToken();
    std::pair<int, std::string> Parser::PeekNextToken();
};

#endif