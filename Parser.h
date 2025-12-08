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
    bool parseCompound_Statement();
    bool parseLetter();
    bool parseDigit();
    std::pair<int, std::string> Parser::GetNextToken();
    std::pair<int, std::string> Parser::PeekToken();
    std::pair<int, std::string> Parser::PeekNextToken();
};

#endif