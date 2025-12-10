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
    if(parseMain()) return true;
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

// Исправляем parseMain()
bool Parser::parseMain() {
    // Проверяем "int"
    auto token = PeekToken();
    if (token.first != 1 || token.second != "int") {
        return false; // Не main
    }
    GetNextToken(); // Потребляем "int"
    
    // Проверяем "main"
    token = PeekToken();
    if (token.first != 4 || token.second != "main") {
        return false; // Не main
    }
    GetNextToken(); // Потребляем "main"
    
    // Парсим параметры
    if (!parseParameters()) {
        std::cerr << "Failed to parse main parameters" << std::endl;
        return false;
    }
    
    // Парсим тело
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

bool Parser::parseClassBody() {
    auto token = GetNextToken();
    if (token.first != 3 || token.second != "{") {
        std::cerr << "Syntax error: expected {" << std::endl;
        return false;
    }
    
    // Парсим члены класса (0 или более)
    token = PeekToken();
    while (token.first != 3 || token.second != "}") {
        if (!parseClassMember()) {
            std::cerr << "Failed to parse class member" << std::endl;
            return false;
        }
        token = PeekToken();
    }
    
    // Потребляем "}"
    GetNextToken();
    return true;
}

bool Parser::parseType() {
    auto t = PeekToken();

    // builtin types
    if (t.first == 1 && (
        t.second == "int" || t.second == "char" ||
        t.second == "bool" || t.second == "float" || t.second == "void"))
    {
        GetNextToken();
        return true;
    }

    // class type: identifier
    if (t.first == 4) { // IDENTIFIER
        GetNextToken();
        return true;
    }

    return false;
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
    if (!parseCompoundStatement()) {
        std::cerr << "Failed to parse compound statement" << std::endl;
        return false;
    }
    return true;
}

bool Parser::parseMainBody(){
    auto token = PeekToken();
    if (!parseCompoundStatement()) {
        std::cerr << "Failed to parse compound statement" << std::endl;
        return false;
    }
    return true;
}

bool Parser::parseIdentifier() {
    auto token = PeekToken();  // Только смотрим, не потребляем!
    if (token.first == 4) {
        GetNextToken();  // Потребляем здесь
        return true;
    }
    return false;
}

bool Parser::parseClassMember(){
    if(parseFieldDeclaration()){
        return true;
    }
    if(parseConstructor()){
        return true;
    }
    if(parseDestructor()){
        return true;
    }
    if(parseMethod()){
        return true;
    }
    return false;
}

bool Parser::parseParameter() {
    if (!parseType()) return false;

    auto t = PeekToken();
    if (t.first != 4) return false;  // must be identifier
    GetNextToken();
    return true;
}

bool Parser::parseCompoundStatement() {
    auto token = PeekToken();
    if (token.first != 3 || token.second != "{") {
        std::cerr << "Syntax error: expected '{'" << std::endl;
        return false;
    }
    GetNextToken();
    // Парсим statements (0 или более)
    token = PeekToken();
    while (token.first != 3 || token.second != "}") {
        std::cout << token.second;
        if (!parseStatement()) {
            std::cerr << "Failed to parse statement" << std::endl;
            return false;
        }
        token = PeekToken();
    }
    // Потребляем "}"
    GetNextToken();
    return true;
}

bool Parser::parseFieldDeclaration() {
    if (!parseType()) {
        return false;
    }
    // Первая переменная
    if (!parseIdentifier()) {
        std::cerr << "Failed to parse identifier" << std::endl;
        return false;
    }
    // Дополнительные переменные через запятую
    auto token = PeekToken();
    while (token.first == 3 && token.second == ",") {
        GetNextToken(); // Потребляем ","
        if (!parseIdentifier()) {
            std::cerr << "Failed to parse identifier after comma" << std::endl;
            return false;
        }
        token = PeekToken();
    }
    // Потребляем ";"
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';'" << std::endl;
        return false;
    }
    return true;
}

bool Parser::parseConstructor(){
    auto token = PeekToken();
    if(token.first != 1 || token.second != "constructor"){
        std::cerr << "Syntax error: expected keyword 'constructor'" << std::endl;
        return false;
    }
    GetNextToken();
    if (!parseConstructorParameters()) {
        std::cerr << "Failed to parse constructor parameters" << std::endl;
        return false;
    }
    if (!parseConstructorBody()) {
        std::cerr << "Failed to parse constructor body" << std::endl;
        return false;
    }
    return true;
}

bool Parser::parseDestructor(){
    auto token = PeekToken();
    if(token.first != 1 || token.second != "destructor"){
        std::cerr << "Syntax error: expected keyword 'constructor'" << std::endl;
        return false;
    }
    GetNextToken();
    token = PeekToken();
    if(token.first != 3 || token.second != "("){
        std::cerr << "Syntax error: expected (" << std::endl;
        return false;
    }
    GetNextToken();
    token = PeekToken();
    if(token.first != 3 || token.second != ")"){
        std::cerr << "Syntax error: expected )" << std::endl;
        return false;
    }
    GetNextToken();
    if (!parseDestructorBody()) {
        std::cerr << "Failed to parse destructor body" << std::endl;
        return false;
    }
    return true;
}

bool Parser::parseMethod(){
    if (!parseType()) {
        std::cerr << "Failed to parse type" << std::endl;
        return false;
    }
    if (!parseIdentifier()) {
        std::cerr << "Failed to parse method name" << std::endl;
        return false;
    }
    if (!parseParameters()) {
        std::cerr << "Failed to parse parameters" << std::endl;
        return false;
    }
    if (!parseMethodBody()) {
        std::cerr << "Failed to parse method body" << std::endl;
        return false;
    }
    return true;
}

bool Parser::parseStatement(){
    if(parseCompoundStatement()){
        return true;
    }
    if(parseIfStatement()){
        return true;
    }
    if(parseWhileStatement()){
        return true;
    }
    if(parseForStatement()){
        return true;
    }
    if(parseReturnStatement()){
        return true;
    }
    if(parseBreakStatement()){
        return true;
    }
    if(parseContinueStatement()){
        return true;
    }
    if(parsePrintStatement()){
        return true;
    }
    if(parseReadStatement()){
        return true;
    }
    if(parseExpressionStatement()){
        return true;
    }
    if(parseDeclarationStatement()){
        return true;
    }
    return false;
}

bool Parser::parseConstructorParameters(){
    if(!parseParameters()){
        return false;
    }
    return true;
}

bool Parser::parseConstructorBody(){
    if(!parseCompoundStatement()){
        return false;
    }
    return true;
}

bool Parser::parseDestructorBody(){
    if(!parseCompoundStatement()){
        return false;
    }
    return true;
}

bool Parser::parseDeclarationStatement(){
    std::cout << PeekToken().second;
    if(parseVariableDeclaration()){
        return true;
    }
    if(parseArrayDeclaration()){
        return true;
    }
    return false;
}

bool Parser::parseExpressionStatement(){
    if(!parseExpression()){
        std::cerr << "Failed to parse expression" << std::endl;
        return false;
    }
    auto token = PeekToken();
    if(token.first != 3 || token.second != ";"){
        std::cerr << "Syntax error: expected ;" << std::endl;
        return false;
    }
    GetNextToken();
    return true;
}

bool Parser::parseIfStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "if") {
        return false;
    }
    GetNextToken(); // Потребляем "if"
    token = PeekToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected (" << std::endl;
        return false;
    }
    GetNextToken(); // Потребляем "("
    if (!parseExpression()) {
        std::cerr << "Failed to parse expression" << std::endl;
        return false;
    }
    token = PeekToken();
    if (token.first != 3 || token.second != ")") {
        std::cerr << "Syntax error: expected )" << std::endl;
        return false;
    }
    GetNextToken(); // Потребляем ")"
    if (!parseCompoundStatement()) {
        std::cerr << "Failed to parse compound statement" << std::endl;
        return false;
    }
    // Обрабатываем elif (0 или более)
    token = PeekToken();
    while (token.first == 1 && token.second == "elif") {
        GetNextToken(); // Потребляем "elif"
        token = PeekToken();
        if (token.first != 3 || token.second != "(") {
            std::cerr << "Syntax error: expected (" << std::endl;
            return false;
        }
        GetNextToken(); // Потребляем "("
        
        if (!parseExpression()) {
            std::cerr << "Failed to parse expression" << std::endl;
            return false;
        }
        token = PeekToken();
        if (token.first != 3 || token.second != ")") {
            std::cerr << "Syntax error: expected )" << std::endl;
            return false;
        }
        GetNextToken(); // Потребляем ")"
        if (!parseCompoundStatement()) {
            std::cerr << "Failed to parse compound statement" << std::endl;
            return false;
        }
        token = PeekToken();
    }
    // Обрабатываем else (опционально)
    token = PeekToken();
    if (token.first == 1 && token.second == "else") {
        GetNextToken(); // Потребляем "else"
        if (!parseCompoundStatement()) {
            std::cerr << "Failed to parse compound statement" << std::endl;
            return false;
        }
    }
    return true;
}

// ========== Реализация недостающих функций ==========

bool Parser::parseWhileStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "while") {
        return false;
    }
    GetNextToken(); // "while"
    
    token = GetNextToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '(' after while" << std::endl;
        return false;
    }
    
    if (!parseExpression()) {
        std::cerr << "Failed to parse while condition" << std::endl;
        return false;
    }
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ")") {
        std::cerr << "Syntax error: expected ')' after while condition" << std::endl;
        return false;
    }
    
    if (!parseCompoundStatement()) {
        std::cerr << "Failed to parse while body" << std::endl;
        return false;
    }
    
    return true;
}

bool Parser::parseForStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "for") {
        return false;
    }
    GetNextToken(); // "for"
    
    token = GetNextToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '(' after for" << std::endl;
        return false;
    }
    
    // Опциональная инициализация
    token = PeekToken();
    if (token.first != 3 || token.second != ";") {
        if (!parseForInit()) {
            std::cerr << "Failed to parse for initialization" << std::endl;
            return false;
        }
    }
    
    // Первая точка с запятой
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after for initialization" << std::endl;
        return false;
    }
    
    // Опциональное условие
    token = PeekToken();
    if (token.first != 3 || token.second != ";") {
        if (!parseExpression()) {
            std::cerr << "Failed to parse for condition" << std::endl;
            return false;
        }
    }
    
    // Вторая точка с запятой
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after for condition" << std::endl;
        return false;
    }
    
    // Опциональное выражение инкремента
    token = PeekToken();
    if (token.first != 3 || token.second != ")") {
        if (!parseExpression()) {
            std::cerr << "Failed to parse for increment" << std::endl;
            return false;
        }
    }
    
    // Закрывающая скобка
    token = GetNextToken();
    if (token.first != 3 || token.second != ")") {
        std::cerr << "Syntax error: expected ')' after for header" << std::endl;
        return false;
    }
    
    // Тело цикла
    if (!parseCompoundStatement()) {
        std::cerr << "Failed to parse for body" << std::endl;
        return false;
    }
    
    return true;
}

bool Parser::parseForInit() {
    // Пробуем объявление переменной
    if (parseVariableDeclaration()) {
        return true;
    }
    
    // Пробуем выражение
    if (parseExpressionStatement()) {
        return true;
    }
    
    return false;
}

bool Parser::parseReturnStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "return") {
        return false;
    }
    GetNextToken(); // "return"
    
    // Опциональное выражение
    token = PeekToken();
    if (token.first != 3 || token.second != ";") {
        if (!parseExpression()) {
            std::cerr << "Failed to parse return expression" << std::endl;
            return false;
        }
    }
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after return statement" << std::endl;
        return false;
    }
    
    return true;
}

bool Parser::parseBreakStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "break") {
        return false;
    }
    GetNextToken(); // "break"
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after break statement" << std::endl;
        return false;
    }
    
    return true;
}

bool Parser::parseContinueStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "continue") {
        return false;
    }
    GetNextToken(); // "continue"
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after continue statement" << std::endl;
        return false;
    }
    
    return true;
}

bool Parser::parsePrintStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "print") {
        return false;
    }
    GetNextToken(); // "print"
    
    token = GetNextToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '(' after print" << std::endl;
        return false;
    }
    
    if (!parseExpression()) {
        std::cerr << "Failed to parse print expression" << std::endl;
        return false;
    }
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ")") {
        std::cerr << "Syntax error: expected ')' after print expression" << std::endl;
        return false;
    }
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after print statement" << std::endl;
        return false;
    }
    
    return true;
}

bool Parser::parseReadStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "read") {
        return false;
    }
    GetNextToken(); // "read"
    
    token = GetNextToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '(' after read" << std::endl;
        return false;
    }
    
    if (!parseIdentifier()) {
        std::cerr << "Failed to parse identifier for read" << std::endl;
        return false;
    }
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ")") {
        std::cerr << "Syntax error: expected ')' after read identifier" << std::endl;
        return false;
    }
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after read statement" << std::endl;
        return false;
    }
    
    return true;
}

bool Parser::parseVariableDeclaration() {
    if (!parseType()) {
        return false;
    }
    std::cout << PeekToken().second;
    if (!parseIdentifier()) {
        std::cerr << "Failed to parse variable name" << std::endl;
        return false;
    }
    
    // Опциональная инициализация
    auto token = PeekToken();
    if (token.first == 2 && token.second == "=") {
        GetNextToken(); // "="
        if (!parseExpression()) {
            std::cerr << "Failed to parse initialization expression" << std::endl;
            return false;
        }
    }
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after variable declaration" << std::endl;
        return false;
    }
    
    return true;
}

bool Parser::parseArrayDeclaration() {
    if (!parseType()) {
        return false;
    }
    
    if (!parseIdentifier()) {
        std::cerr << "Failed to parse array name" << std::endl;
        return false;
    }
    
    auto token = GetNextToken();
    if (token.first != 3 || token.second != "[") {
        std::cerr << "Syntax error: expected '[' for array declaration" << std::endl;
        return false;
    }
    
    // Должен быть целочисленный литерал (NUMBER)
    token = GetNextToken();
    if (token.first != 5) {
        std::cerr << "Syntax error: expected integer literal for array size" << std::endl;
        return false;
    }
    
    token = GetNextToken();
    if (token.first != 3 || token.second != "]") {
        std::cerr << "Syntax error: expected ']' after array size" << std::endl;
        return false;
    }
    
    // Опциональная инициализация
    token = PeekToken();
    if (token.first == 2 && token.second == "=") {
        GetNextToken(); // "="
        token = GetNextToken();
        if (token.first != 3 || token.second != "{") {
            std::cerr << "Syntax error: expected '{' for array initialization" << std::endl;
            return false;
        }
        
        // Опциональный список выражений
        token = PeekToken();
        if (token.first != 3 || token.second != "}") {
            if (!parseExpressionList()) {
                return false;
            }
        }
        
        token = GetNextToken();
        if (token.first != 3 || token.second != "}") {
            std::cerr << "Syntax error: expected '}' after array initialization" << std::endl;
            return false;
        }
    }
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after array declaration" << std::endl;
        return false;
    }
    
    return true;
}

bool Parser::parseExpressionList() {
    if (!parseExpression()) {
        return false;
    }
    
    auto token = PeekToken();
    while (token.first == 3 && token.second == ",") {
        GetNextToken(); // ","
        if (!parseExpression()) {
            return false;
        }
        token = PeekToken();
    }
    
    return true;
}

// ========== Реализация parseExpression() и иерархии выражений ==========

bool Parser::parseExpression() {
    return parseAssignmentExpression();
}

bool Parser::parseAssignmentExpression() {
    // Парсим условное выражение (левую часть)
    if (!parseConditionalExpression()) {
        return false;
    }
    
    // Проверяем оператор присваивания
    auto token = PeekToken();
    if (token.first == 2 && (token.second == "=" || token.second == "+=" || 
                             token.second == "-=" || token.second == "*=" || 
                             token.second == "/=" || token.second == "%=" || 
                             token.second == "<<=" || token.second == ">>=" || 
                             token.second == "&=" || token.second == "^=" || 
                             token.second == "|=")) {
        GetNextToken(); // оператор присваивания
        // Парсим правую часть (рекурсивно)
        return parseAssignmentExpression();
    }
    
    // Если оператора присваивания нет, это просто условное выражение
    return true;
}

bool Parser::parseConditionalExpression() {
    return parseLogicalOrExpression();
}

bool Parser::parseLogicalOrExpression() {
    if (!parseLogicalAndExpression()) {
        return false;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && token.second == "||") {
        GetNextToken(); // "||"
        if (!parseLogicalAndExpression()) {
            return false;
        }
        token = PeekToken();
    }
    
    return true;
}

bool Parser::parseLogicalAndExpression() {
    if (!parseBitwiseOrExpression()) {
        return false;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && token.second == "&&") {
        GetNextToken(); // "&&"
        if (!parseBitwiseOrExpression()) {
            return false;
        }
        token = PeekToken();
    }
    
    return true;
}

bool Parser::parseBitwiseOrExpression() {
    if (!parseBitwiseXorExpression()) {
        return false;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && token.second == "|") {
        GetNextToken(); // "|"
        if (!parseBitwiseXorExpression()) {
            return false;
        }
        token = PeekToken();
    }
    
    return true;
}

bool Parser::parseBitwiseXorExpression() {
    if (!parseBitwiseAndExpression()) {
        return false;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && token.second == "^") {
        GetNextToken(); // "^"
        if (!parseBitwiseAndExpression()) {
            return false;
        }
        token = PeekToken();
    }
    
    return true;
}

bool Parser::parseBitwiseAndExpression() {
    if (!parseEqualityExpression()) {
        return false;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && token.second == "&") {
        GetNextToken(); // "&"
        if (!parseEqualityExpression()) {
            return false;
        }
        token = PeekToken();
    }
    
    return true;
}

bool Parser::parseEqualityExpression() {
    if (!parseRelationalExpression()) {
        return false;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && (token.second == "==" || token.second == "!=")) {
        GetNextToken(); // "==" или "!="
        if (!parseRelationalExpression()) {
            return false;
        }
        token = PeekToken();
    }
    
    return true;
}

bool Parser::parseRelationalExpression() {
    if (!parseShiftExpression()) {
        return false;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && (token.second == "<" || token.second == ">" || 
                                token.second == "<=" || token.second == ">=")) {
        GetNextToken(); // оператор сравнения
        if (!parseShiftExpression()) {
            return false;
        }
        token = PeekToken();
    }
    
    return true;
}

bool Parser::parseShiftExpression() {
    if (!parseAdditiveExpression()) {
        return false;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && (token.second == "<<" || token.second == ">>")) {
        GetNextToken(); // "<<" или ">>"
        if (!parseAdditiveExpression()) {
            return false;
        }
        token = PeekToken();
    }
    
    return true;
}

bool Parser::parseAdditiveExpression() {
    if (!parseMultiplicativeExpression()) {
        return false;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && (token.second == "+" || token.second == "-")) {
        GetNextToken(); // "+" или "-"
        if (!parseMultiplicativeExpression()) {
            return false;
        }
        token = PeekToken();
    }
    return true;
}

bool Parser::parseMultiplicativeExpression() {
    if (!parseCastExpression()) {
        return false;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && (token.second == "*" || token.second == "/" || token.second == "%")) {
        GetNextToken(); // "*", "/" или "%"
        if (!parseCastExpression()) {
            return false;
        }
        token = PeekToken();
    }
    
    return true;
}

bool Parser::parseCastExpression() {
    auto token = PeekToken();
    
    // Проверяем преобразование типа: "(" <type> ")" 
    if (token.first == 3 && token.second == "(") {
        GetNextToken(); // "("
        
        // Пробуем распарсить тип
        if (parseType()) {
            token = PeekToken();
            if (token.first == 3 && token.second == ")") {
                GetNextToken(); // ")"
                return parseCastExpression();
            }
        }
    }
    
    // Если не преобразование типа, пробуем унарное выражение
    return parseUnaryExpression();
}

bool Parser::parseUnaryExpression() {
    auto token = PeekToken();
    
    // Унарные операторы: & * + - ! ~
    if (token.first == 2 && (token.second == "&" || token.second == "*" || 
                            token.second == "+" || token.second == "-" || 
                            token.second == "!" || token.second == "~")) {
        GetNextToken(); // унарный оператор
        return parseCastExpression();
    }
    
    // ++ или -- перед выражением
    if (token.first == 3 && (token.second == "++" || token.second == "--")) {
        GetNextToken(); // "++" или "--"
        return parseCastExpression();
    }
    
    // sizeof
    if (token.first == 1 && token.second == "sizeof") {
        GetNextToken(); // "sizeof"
        token = GetNextToken();
        if (token.first != 3 || token.second != "(") return false;
        if (!parseType()) return false;
        token = GetNextToken();
        if (token.first != 3 || token.second != ")") return false;
        return true;
    }
    
    // new
    if (token.first == 1 && token.second == "new") {
        GetNextToken(); // "new"
        if (!parseType()) return false;
        token = PeekToken();
        if (token.first == 3 && token.second == "[") {
            GetNextToken(); // "["
            if (!parseExpression()) return false;
            token = GetNextToken();
            if (token.first != 3 || token.second != "]") return false;
        }
        return true;
    }
    
    // delete
    if (token.first == 1 && token.second == "delete") {
        GetNextToken(); // "delete"
        token = PeekToken();
        if (token.first == 3 && token.second == "[") {
            GetNextToken(); // "["
            token = GetNextToken();
            if (token.first != 3 || token.second != "]") return false;
        }
        return parseCastExpression();
    }
    
    // Если не унарный оператор, пробуем постфиксное выражение
    return parsePostfixExpression();
}

bool Parser::parsePostfixExpression() {
    // Парсим первичное выражение (идентификатор, число и т.д.)
    if (!parsePrimaryExpression()) {
        return false;
    }
    
    // Обрабатываем постфиксные операторы (0 или более)
    while (true) {
        auto token = PeekToken();
        
        // Индексация: "[" expression "]"
        if (token.first == 3 && token.second == "[") {
            GetNextToken(); // "["
            if (!parseExpression()) return false;
            token = GetNextToken();
            if (token.first != 3 || token.second != "]") return false;
            continue;
        }
        
        // Вызов функции: "(" [expression_list] ")"
        if (token.first == 3 && token.second == "(") {
            GetNextToken(); // "("
            token = PeekToken();
            if (token.first != 3 || token.second != ")") {
                if (!parseExpressionList()) return false;
            }
            token = GetNextToken();
            if (token.first != 3 || token.second != ")") return false;
            continue;
        }
        
        // Доступ к полю: "." identifier
        if (token.first == 3 && token.second == ".") {
            GetNextToken(); // "."
            if (!parseIdentifier()) return false;
            continue;
        }
        
        // Доступ через указатель: "->" identifier
        if (token.first == 3 && token.second == "->") {
            GetNextToken(); // "->"
            if (!parseIdentifier()) return false;
            continue;
        }
        
        // Постфиксный инкремент/декремент: "++" или "--"
        if (token.first == 3 && (token.second == "++" || token.second == "--")) {
            GetNextToken(); // "++" или "--"
            continue;
        }
        
        // Больше нет постфиксных операторов
        break;
    }
    
    return true;
}

bool Parser::parsePrimaryExpression() {
    auto token = PeekToken();
    
    // Идентификатор
    if (token.first == 4) {
        GetNextToken();
        return true;
    }
    
    // Литерал (число)
    if (token.first == 5) {
        GetNextToken();
        return true;
    }
    
    // Символьный литерал
    if (token.first == 7) {
        GetNextToken();
        return true;
    }
    
    // Строковый литерал
    if (token.first == 6) {
        GetNextToken();
        return true;
    }
    
    // true/false
    if (token.first == 1 && (token.second == "true" || token.second == "false")) {
        GetNextToken();
        return true;
    }
    
    // nullptr
    if (token.first == 1 && token.second == "nullptr") {
        GetNextToken();
        return true;
    }
    
    // Выражение в скобках
    if (token.first == 3 && token.second == "(") {
        GetNextToken(); // "("
        if (!parseExpression()) return false;
        token = GetNextToken();
        if (token.first != 3 || token.second != ")") return false;
        return true;
    }
    
    // this
    if (token.first == 1 && token.second == "this") {
        GetNextToken();
        return true;
    }
    
    return false;
}

bool Parser::parseLiteral() {
    auto token = PeekToken();
    
    // Целочисленный или вещественный литерал (NUMBER)
    if (token.first == 5) {
        GetNextToken();
        return true;
    }
    
    // Символьный литерал
    if (token.first == 7) { // CHAR_LITERAL
        GetNextToken();
        return true;
    }
    
    // Строковый литерал
    if (token.first == 6) { // STRING_LITERAL
        GetNextToken();
        return true;
    }
    
    // Логические литералы
    if (token.first == 1 && (token.second == "true" || token.second == "false")) {
        GetNextToken();
        return true;
    }
    
    // nullptr
    if (token.first == 1 && token.second == "nullptr") {
        GetNextToken();
        return true;
    }
    return false;
}

bool Parser::parseMethodBody() {
    return parseCompoundStatement(); // Согласно грамматике, тело метода = составной оператор
}

bool Parser::parseLetter() {
    // Эта функция может быть не нужна, так как лексер уже разобрал идентификаторы
    // Но если требуется по грамматике, можно реализовать проверку
    auto token = PeekToken();
    if (token.second.length() == 1) {
        char c = token.second[0];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
            GetNextToken();
            return true;
        }
    }
    return false;
}

bool Parser::parseDigit() {
    // Аналогично, может быть не нужна
    auto token = PeekToken();
    if (token.second.length() == 1) {
        char c = token.second[0];
        if (c >= '0' && c <= '9') {
            GetNextToken();
            return true;
        }
    }
    return false;
}

bool Parser::parseAssignmentOperator() {
    auto token = PeekToken();
    if (token.first == 2) { // OPERATOR
        std::string op = token.second;
        if (op == "=" || op == "+=" || op == "-=" || op == "*=" || 
            op == "/=" || op == "%=" || op == "<<=" || op == ">>=" || 
            op == "&=" || op == "^=" || op == "|=") {
            GetNextToken();
            return true;
        }
    }
    return false;
}

bool Parser::parseUnaryOperator() {
    auto token = PeekToken();
    if (token.first == 2) { // OPERATOR
        std::string op = token.second;
        if (op == "&" || op == "*" || op == "+" || op == "-" || 
            op == "!" || op == "~") {
            GetNextToken();
            return true;
        }
    }
    return false;
}