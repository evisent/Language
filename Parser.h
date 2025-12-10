#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
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
    
    // Новые парсеры для выражений (добавлены)
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
    
    // Вспомогательные функции для операторов
    bool parseAssignmentOperator();
    bool parseUnaryOperator();
    
public:
    // Конструктор
    Parser(Lexer& lex, std::vector<std::pair<int, std::string>> toks) : lexer(lex), tokens(toks){}
    
    // Основной метод
    bool program();
};

#endif