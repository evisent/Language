#include "Parser.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <string>

std::pair<int, std::string> Parser::GetNextToken() {
    // Должен возвращать текущий токен и сдвигать позицию
    if (cur >= tokens.size()) return {0, ""}; // EOF
    return tokens[cur++];
}

std::pair<int, std::string> Parser::PeekToken() {
    // Просмотр текущего токена без сдвига
    if (cur >= tokens.size()) return {0, ""};
    return tokens[cur];
}

std::pair<int, std::string> Parser::PeekNextToken() {
    // Просмотр следующего токена без сдвига
    if (cur + 1 >= tokens.size()) return {0, ""};
    return tokens[cur + 1];
}

bool Parser::IsAtEnd() {
    return cur >= tokens.size() || tokens[cur].first == 0;
}

bool Parser::program() {
    // Парсим (<class> | <function>)*
    while (!IsAtEnd()) {
        auto cur_token = PeekToken();
        auto next_token = PeekNextToken();
        
        // Проверяем, не начался ли main
        bool is_main = cur_token.first == 1 && cur_token.second == "int" && 
                       next_token.first == 4 && next_token.second == "main";
        
        if (is_main) {
            break; // Переходим к парсингу main
        }
        
        // Проверяем, class это или function
        bool is_class = cur_token.first == 1 && cur_token.second == "class";
        bool is_function = (cur_token.first == 1 || cur_token.first == 4) && 
                           (cur_token.second == "int" || cur_token.second == "char" || 
                            cur_token.second == "bool" || cur_token.second == "float" || 
                            cur_token.second == "void" || cur_token.first == 4);
        
        if (is_class) {
            if (!parseClass()) {
                std::cerr << "Failed to parse class" << std::endl;
                return false;
            }
        } 
        else if (is_function) {
            if (!parseFunction()) {
                std::cerr << "Failed to parse function" << std::endl;
                return false;
            }
        }
        else {
            std::cerr << "Syntax error: expected class or function, got '" 
                      << cur_token.second << "'" << std::endl;
            return false;
        }
    }
    
    // Парсим <main>
    if (!parseMain()) {
        std::cerr << "Failed to parse main function" << std::endl;
        return false;
    }
    
    // Проверяем, что после main ничего нет
    if (!IsAtEnd()) {
        std::cerr << "Syntax error: code after main function" << std::endl;
        return false;
    }
    
    return true;
}

bool Parser::parseClass() {
    if (PeekToken().first != 1 || PeekToken().second != "class") {
        std::cerr << "Syntax error: expected keyword 'class'" << std::endl;
        return false;
    }
    GetNextToken();
    if (!parseClassName()) {
        std::cerr << "Failed to parse class name" << std::endl;
        return false;
    }
    if (!parseClassBody()) {
        std::cerr << "Failed to parse class body" << std::endl;
        return false;
    }
    return true;
}

bool Parser::parseFunction(){
    if (!parseType()) {
        std::cerr << "Failed to parse type" << std::endl;
        return false;
    }
    if (!parseFunctionName()) {
        std::cerr << "Failed to parse function name" << std::endl;
        return false;
    }
    if (!parseParameters()) {
        std::cerr << "Failed to parse parameters" << std::endl;
        return false;
    }
    if (!parseFunctionBody()) {
        std::cerr << "Failed to parse function body" << std::endl;
        return false;
    }
    return true;
}

bool Parser::parseMain(){
    GetNextToken();
    GetNextToken();
    auto cur_token = PeekToken();
    if (!parseParameters()) {
        std::cerr << "Failed to parse parameters" << std::endl;
        return false;
    }
    if (!parseMainBody()) {
        std::cerr << "Failed to parse main body" << std::endl;
        return false;
    }
    return true;
}

bool Parser::parseClassName(){
    if (!parseIdentifier()) {
        std::cerr << "Failed to parse identifier" << std::endl;
        return false;
    }
    return true;
}

bool Parser::parseClassBody(){
    auto cur_token = GetNextToken();
    if(cur_token.first != 3 || cur_token.second != "{"){
        std::cerr << "Syntax error: expected {";
        return false;
    }
    if (!parseClassMember()) {
        std::cerr << "Failed to parse class member" << std::endl;
        return false;
    }
    cur_token = PeekToken();
    if(cur_token.first != 3 || cur_token.second != "}"){
        std::cerr << "Syntax error: expected }";
        return false;
    }
    return true;
}

bool Parser::parseType(){
    auto cur_token = PeekToken();
    if(cur_token.first != 1 || (cur_token.second != "int" && cur_token.second != "char" && 
        cur_token.second != "bool" && cur_token.second != "float")){
            if (!parseClassName()) {
            std::cerr << "Failed to parse class name" << std::endl;
            return false;
        }
    }
    else{
        GetNextToken();
    }
    return true;
}

bool Parser::parseParameters() {
    auto token = GetNextToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '('" << std::endl;
        return false;
    }
    token = PeekToken();
    if (token.first == 3 && token.second == ")") {
        GetNextToken();
        return true;
    }
    if (!parseParameter()) {
        std::cerr << "Failed to parse parameter" << std::endl;
        return false;
    }
    token = PeekToken();
    while (token.first == 3 && token.second == ",") {
        GetNextToken();
        if (!parseParameter()) {
            std::cerr << "Failed to parse parameter after comma" << std::endl;
            return false;
        }
        token = PeekToken();
    }
    token = GetNextToken();
    if (token.first != 3 || token.second != ")") {
        std::cerr << "Syntax error: expected ')'" << std::endl;
        return false;
    }
    return true;
}

bool Parser::parseFunctionName(){
    if (!parseIdentifier()) {
        std::cerr << "Failed to parse identifier" << std::endl;
        return false;
    }
    return true;
}

bool Parser::parseFunctionBody(){
    if (!parseCompound_Statement()) {
        std::cerr << "Failed to parse compound statement" << std::endl;
        return false;
    }
    return true;
}

bool Parser::parseMainBody(){
    if (!parseCompound_Statement()) {
        std::cerr << "Failed to parse compound statement" << std::endl;
        return false;
    }
    return true;
}

bool Parser::parseIdentifier(){
    auto cur_token = GetNextToken();
    if(cur_token.first != 4) return false;
    return true;
}

bool Parser::parseClassMember(){}

bool Parser::parseParameter(){}

bool Parser::parseCompound_Statement(){}

bool Parser::parseLetter(){}

bool Parser::parseDigit(){}