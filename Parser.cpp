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

// Вспомогательные методы для создания узлов
std::shared_ptr<ASTNode> Parser::createNode(NodeType type, const std::string& value) {
    return std::make_shared<ASTNode>(type, value);
}

std::shared_ptr<ASTNode> Parser::createBinaryExprNode(const std::string& op, 
                                                     std::shared_ptr<ASTNode> left, 
                                                     std::shared_ptr<ASTNode> right) {
    auto node = createNode(NodeType::BINARY_EXPR, op);
    if (left) node->addChild(left);
    if (right) node->addChild(right);
    return node;
}

std::shared_ptr<ASTNode> Parser::createUnaryExprNode(const std::string& op, 
                                                    std::shared_ptr<ASTNode> operand) {
    auto node = createNode(NodeType::UNARY_EXPR, op);
    if (operand) node->addChild(operand);
    return node;
}

std::shared_ptr<ASTNode> Parser::program() {
    auto programNode = createNode(NodeType::PROGRAM, "Program");
    
    // Парсим объявления до main
    while (!IsAtEnd()) {
        auto cur_token = PeekToken();
        auto next_token = PeekNextToken();
        
        // Проверяем, не начался ли main
        bool is_main = cur_token.first == 1 && cur_token.second == "int" && 
                      next_token.first == 4 && next_token.second == "main";
        if (is_main) {
            break;
        }
        
        // Парсим класс или функцию
        bool is_class = cur_token.first == 1 && cur_token.second == "class";
        
        if (is_class) {
            auto classNode = parseClass();
            if (!classNode) {
                std::cerr << "Failed to parse class" << std::endl;
                return nullptr;
            }
            programNode->addChild(classNode);
        } 
        else {
            // Проверяем, функция ли это
            auto funcNode = parseFunction();
            if (!funcNode) {
                std::cerr << "Failed to parse function" << std::endl;
                return nullptr;
            }
            programNode->addChild(funcNode);
        }
    }
    
    // Парсим main
    auto mainNode = parseMain();
    if (!mainNode) {
        std::cerr << "Failed to parse main function" << std::endl;
        return nullptr;
    }
    programNode->addChild(mainNode);
    
    // Проверяем, что после main ничего нет
    if (!IsAtEnd()) {
        std::cerr << "Syntax error: code after main function" << std::endl;
        return nullptr;
    }
    
    return programNode;
}

std::shared_ptr<ASTNode> Parser::parseClass() {
    if (PeekToken().first != 1 || PeekToken().second != "class") {
        std::cerr << "Syntax error: expected keyword 'class'" << std::endl;
        return nullptr;
    }
    GetNextToken(); // Потребляем "class"
    
    auto classNode = createNode(NodeType::CLASS_DECL);
    
    // Парсим имя класса
    auto classNameNode = parseClassName();
    if (!classNameNode) {
        std::cerr << "Failed to parse class name" << std::endl;
        return nullptr;
    }
    classNode->addChild(classNameNode);
    classNode->setValue(classNameNode->getValue());
    
    // Парсим тело класса
    auto classBodyNode = parseClassBody();
    if (!classBodyNode) {
        std::cerr << "Failed to parse class body" << std::endl;
        return nullptr;
    }
    classNode->addChild(classBodyNode);
    
    return classNode;
}

std::shared_ptr<ASTNode> Parser::parseFunction() {
    auto funcNode = createNode(NodeType::FUNCTION_DECL);
    
    // Парсим тип возвращаемого значения
    auto typeNode = parseType();
    if (!typeNode) {
        std::cerr << "Failed to parse type" << std::endl;
        return nullptr;
    }
    funcNode->addChild(typeNode);
    
    // Парсим имя функции
    auto nameNode = parseFunctionName();
    if (!nameNode) {
        std::cerr << "Failed to parse function name" << std::endl;
        return nullptr;
    }
    funcNode->addChild(nameNode);
    funcNode->setValue(nameNode->getValue());
    
    // Парсим параметры
    auto paramsNode = parseParameters();
    if (!paramsNode) {
        std::cerr << "Failed to parse parameters" << std::endl;
        return nullptr;
    }
    funcNode->addChild(paramsNode);
    
    // Парсим тело функции
    auto bodyNode = parseFunctionBody();
    if (!bodyNode) {
        std::cerr << "Failed to parse function body" << std::endl;
        return nullptr;
    }
    funcNode->addChild(bodyNode);
    
    return funcNode;
}

std::shared_ptr<ASTNode> Parser::parseMain() {
    // Проверяем "int"
    auto token = PeekToken();
    if (token.first != 1 || token.second != "int") {
        return nullptr; // Не main
    }
    GetNextToken(); // Потребляем "int"
    
    // Проверяем "main"
    token = PeekToken();
    if (token.first != 4 || token.second != "main") {
        return nullptr; // Не main
    }
    GetNextToken(); // Потребляем "main"
    
    auto mainNode = createNode(NodeType::MAIN_FUNCTION, "main");
    
    // Парсим параметры
    auto paramsNode = parseParameters();
    if (!paramsNode) {
        std::cerr << "Failed to parse main parameters" << std::endl;
        return nullptr;
    }
    mainNode->addChild(paramsNode);
    
    // Парсим тело
    auto bodyNode = parseMainBody();
    if (!bodyNode) {
        std::cerr << "Failed to parse main body" << std::endl;
        return nullptr;
    }
    mainNode->addChild(bodyNode);
    
    return mainNode;
}

std::shared_ptr<ASTNode> Parser::parseClassName() {
    auto identifierNode = parseIdentifier();
    if (!identifierNode) {
        std::cerr << "Failed to parse class name identifier" << std::endl;
        return nullptr;
    }
    // Возвращаем узел идентификатора как имя класса
    return identifierNode;
}

std::shared_ptr<ASTNode> Parser::parseClassBody() {
    auto token = GetNextToken();
    if (token.first != 3 || token.second != "{") {
        std::cerr << "Syntax error: expected {" << std::endl;
        return nullptr;
    }
    
    // Создаем узел для тела класса
    auto classBodyNode = createNode(NodeType::COMPOUND_STMT, "ClassBody");
    
    // Парсим члены класса (0 или более)
    token = PeekToken();
    while (token.first != 3 || token.second != "}") {
        auto memberNode = parseClassMember();
        if (!memberNode) {
            std::cerr << "Failed to parse class member" << std::endl;
            return nullptr;
        }
        classBodyNode->addChild(memberNode);
        token = PeekToken();
    }
    
    // Потребляем "}"
    GetNextToken();
    return classBodyNode;
}

std::shared_ptr<ASTNode> Parser::parseType() {
    auto t = PeekToken();
    if (t.first == 1 && (
        t.second == "int" || t.second == "char" ||
        t.second == "bool" || t.second == "float" || t.second == "void"))
    {
        auto node = createNode(NodeType::TYPE_SPECIFIER, t.second);
        GetNextToken();
        return node;
    }
    // class type: identifier
    if (t.first == 4) {
        auto node = createNode(NodeType::TYPE_SPECIFIER, t.second);
        GetNextToken();
        return node;
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseParameters() {
    auto token = GetNextToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '('" << std::endl;
        return nullptr;
    }
    
    // Создаем узел для списка параметров
    auto paramsNode = createNode(NodeType::COMPOUND_STMT, "Parameters");
    
    token = PeekToken();
    if (token.first == 3 && token.second == ")") {
        GetNextToken(); // Потребляем ")"
        return paramsNode; // Возвращаем пустой узел параметров
    }
    
    // Парсим первый параметр
    auto firstParam = parseParameter();
    if (!firstParam) {
        std::cerr << "Failed to parse parameter" << std::endl;
        return nullptr;
    }
    paramsNode->addChild(firstParam);
    
    // Парсим дополнительные параметры через запятую
    token = PeekToken();
    while (token.first == 3 && token.second == ",") {
        GetNextToken(); // Потребляем ","
        
        auto nextParam = parseParameter();
        if (!nextParam) {
            std::cerr << "Failed to parse parameter after comma" << std::endl;
            return nullptr;
        }
        paramsNode->addChild(nextParam);
        
        token = PeekToken();
    }
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ")") {
        std::cerr << "Syntax error: expected ')'" << std::endl;
        return nullptr;
    }
    
    return paramsNode;
}

std::shared_ptr<ASTNode> Parser::parseFunctionName() {
    auto identifierNode = parseIdentifier();
    if (!identifierNode) {
        std::cerr << "Failed to parse function name identifier" << std::endl;
        return nullptr;
    }
    // Возвращаем узел идентификатора как имя функции
    return identifierNode;
}

std::shared_ptr<ASTNode> Parser::parseFunctionBody() {
    auto bodyNode = parseCompoundStatement();
    if (!bodyNode) {
        std::cerr << "Failed to parse function body compound statement" << std::endl;
        return nullptr;
    }
    return bodyNode;
}

std::shared_ptr<ASTNode> Parser::parseMainBody() {
    auto bodyNode = parseCompoundStatement();
    if (!bodyNode) {
        std::cerr << "Failed to parse main body compound statement" << std::endl;
        return nullptr;
    }
    return bodyNode;
}

std::shared_ptr<ASTNode> Parser::parseIdentifier() {
    auto token = PeekToken();
    if (token.first == 4) {
        auto node = createNode(NodeType::IDENTIFIER, token.second);
        GetNextToken();
        return node;
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseClassMember() {
    std::shared_ptr<ASTNode> memberNode = nullptr;
    
    // Пробуем разные типы членов класса
    // Они возвращают nullptr, если не удалось распарсить
    if (!memberNode) memberNode = parseFieldDeclaration();
    if (!memberNode) memberNode = parseConstructor();
    if (!memberNode) memberNode = parseDestructor();
    if (!memberNode) memberNode = parseMethod();
    
    return memberNode; // Возвращаем nullptr или найденный узел
}

std::shared_ptr<ASTNode> Parser::parseParameter() {
    // Парсим тип
    auto typeNode = parseType();
    if (!typeNode) {
        return nullptr;
    }

    // Проверяем идентификатор
    auto t = PeekToken();
    if (t.first != 4) {
        return nullptr;  // Должен быть идентификатор
    }
    
    // Парсим идентификатор (имя параметра)
    auto identifierNode = parseIdentifier();
    if (!identifierNode) {
        return nullptr;
    }

    // Создаем узел параметра
    auto paramNode = createNode(NodeType::PARAMETER);
    paramNode->addChild(typeNode);      // Тип как первый ребенок
    paramNode->addChild(identifierNode); // Имя как второй ребенок
    
    return paramNode;
}

std::shared_ptr<ASTNode> Parser::parseCompoundStatement() {
    auto token = PeekToken();
    if (token.first != 3 || token.second != "{") {
        return nullptr;
    }
    GetNextToken();
    
    auto compoundNode = createNode(NodeType::COMPOUND_STMT);
    
    // Парсим statements
    token = PeekToken();
    while (token.first != 3 || token.second != "}") {
        auto stmt = parseStatement();
        if (!stmt) {
            std::cerr << "Failed to parse statement" << std::endl;
            return nullptr;
        }
        compoundNode->addChild(stmt);
        token = PeekToken();
    }
    
    // Потребляем "}"
    GetNextToken();
    return compoundNode;
}

std::shared_ptr<ASTNode> Parser::parseFieldDeclaration() {
    // Парсим тип
    auto typeNode = parseType();
    if (!typeNode) {
        return nullptr;
    }
    
    // Создаем узел объявления поля
    auto fieldNode = createNode(NodeType::FIELD_DECL);
    fieldNode->addChild(typeNode);
    
    // Первая переменная
    auto firstIdNode = parseIdentifier();
    if (!firstIdNode) {
        std::cerr << "Failed to parse identifier in field declaration" << std::endl;
        return nullptr;
    }
    fieldNode->addChild(firstIdNode);
    
    // Дополнительные переменные через запятую
    auto token = PeekToken();
    while (token.first == 3 && token.second == ",") {
        GetNextToken(); // Потребляем ","
        
        auto nextIdNode = parseIdentifier();
        if (!nextIdNode) {
            std::cerr << "Failed to parse identifier after comma in field declaration" << std::endl;
            return nullptr;
        }
        fieldNode->addChild(nextIdNode);
        
        token = PeekToken();
    }
    
    // Потребляем ";"
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after field declaration" << std::endl;
        return nullptr;
    }
    
    return fieldNode;
}

std::shared_ptr<ASTNode> Parser::parseConstructor() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "constructor") {
        // Не конструктор - возвращаем nullptr, а не false
        return nullptr;
    }
    GetNextToken(); // Потребляем "constructor"
    
    // Создаем узел конструктора
    auto constructorNode = createNode(NodeType::CONSTRUCTOR);
    
    // Парсим параметры конструктора
    auto paramsNode = parseConstructorParameters();
    if (!paramsNode) {
        std::cerr << "Failed to parse constructor parameters" << std::endl;
        return nullptr;
    }
    constructorNode->addChild(paramsNode);
    
    // Парсим тело конструктора
    auto bodyNode = parseConstructorBody();
    if (!bodyNode) {
        std::cerr << "Failed to parse constructor body" << std::endl;
        return nullptr;
    }
    constructorNode->addChild(bodyNode);
    
    return constructorNode;
}

std::shared_ptr<ASTNode> Parser::parseDestructor() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "destructor") {
        // Не деструктор - возвращаем nullptr
        return nullptr;
    }
    GetNextToken(); // Потребляем "destructor"
    
    // Создаем узел деструктора
    auto destructorNode = createNode(NodeType::DESTRUCTOR);
    
    // Проверяем параметры ()
    token = GetNextToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '(' after destructor" << std::endl;
        return nullptr;
    }
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ")") {
        std::cerr << "Syntax error: expected ')' in destructor parameters" << std::endl;
        return nullptr;
    }
    
    // Парсим тело деструктора
    auto bodyNode = parseDestructorBody();
    if (!bodyNode) {
        std::cerr << "Failed to parse destructor body" << std::endl;
        return nullptr;
    }
    destructorNode->addChild(bodyNode);
    
    return destructorNode;
}

std::shared_ptr<ASTNode> Parser::parseMethod() {
    // Парсим возвращаемый тип
    auto typeNode = parseType();
    if (!typeNode) {
        std::cerr << "Failed to parse method return type" << std::endl;
        return nullptr;
    }
    
    // Парсим имя метода
    auto nameNode = parseIdentifier();
    if (!nameNode) {
        std::cerr << "Failed to parse method name" << std::endl;
        return nullptr;
    }
    
    // Создаем узел метода
    auto methodNode = createNode(NodeType::METHOD);
    methodNode->addChild(typeNode); // Добавляем тип возвращаемого значения
    methodNode->addChild(nameNode); // Добавляем имя метода
    methodNode->setValue(nameNode->getValue()); // Устанавливаем имя как значение узла
    
    // Парсим параметры
    auto paramsNode = parseParameters();
    if (!paramsNode) {
        std::cerr << "Failed to parse method parameters" << std::endl;
        return nullptr;
    }
    methodNode->addChild(paramsNode);
    
    // Парсим тело метода
    auto bodyNode = parseMethodBody();
    if (!bodyNode) {
        std::cerr << "Failed to parse method body" << std::endl;
        return nullptr;
    }
    methodNode->addChild(bodyNode);
    
    return methodNode;
}

// Вспомогательные методы для конструктора и деструктора
std::shared_ptr<ASTNode> Parser::parseConstructorParameters() {
    return parseParameters(); // Используем тот же метод, что и для функций
}

std::shared_ptr<ASTNode> Parser::parseConstructorBody() {
    return parseCompoundStatement(); // Тело конструктора - составной оператор
}

std::shared_ptr<ASTNode> Parser::parseDestructorBody() {
    return parseCompoundStatement(); // Тело деструктора - составной оператор
}

std::shared_ptr<ASTNode> Parser::parseMethodBody() {
    return parseCompoundStatement(); // Тело метода - составной оператор
}

std::shared_ptr<ASTNode> Parser::parseStatement() {
    std::shared_ptr<ASTNode> stmtNode = nullptr;
    
    // Пробуем разные типы statement в порядке убывания приоритета
    if (!stmtNode) stmtNode = parseCompoundStatement();
    if (!stmtNode) stmtNode = parseIfStatement();
    if (!stmtNode) stmtNode = parseWhileStatement();
    if (!stmtNode) stmtNode = parseForStatement();
    if (!stmtNode) stmtNode = parseReturnStatement();
    if (!stmtNode) stmtNode = parseBreakStatement();
    if (!stmtNode) stmtNode = parseContinueStatement();
    if (!stmtNode) stmtNode = parsePrintStatement();
    if (!stmtNode) stmtNode = parseReadStatement();
    if (!stmtNode) stmtNode = parseExpressionStatement();
    if (!stmtNode) stmtNode = parseDeclarationStatement();
    
    return stmtNode;
}

std::shared_ptr<ASTNode> Parser::parseDeclarationStatement() {
    std::shared_ptr<ASTNode> declNode = nullptr;
    
    // Пробуем разные типы объявлений
    if (!declNode) declNode = parseVariableDeclaration();
    if (!declNode) declNode = parseArrayDeclaration();
    
    return declNode; // nullptr, если не удалось распарсить
}

std::shared_ptr<ASTNode> Parser::parseExpressionStatement() {
    auto exprNode = parseExpression();
    if (!exprNode) {
        std::cerr << "Failed to parse expression in expression statement" << std::endl;
        return nullptr;
    }
    
    auto token = PeekToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after expression" << std::endl;
        return nullptr;
    }
    GetNextToken(); // Потребляем ";"
    
    // Создаем узел для выражения как statement
    auto exprStmtNode = createNode(NodeType::EXPR_STMT);
    exprStmtNode->addChild(exprNode);
    
    return exprStmtNode;
}

std::shared_ptr<ASTNode> Parser::parseIfStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "if") {
        return nullptr;
    }
    GetNextToken(); // Потребляем "if"
    
    // Создаем узел if statement
    auto ifNode = createNode(NodeType::IF_STMT);
    
    // Парсим условие в скобках
    token = PeekToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '(' after if" << std::endl;
        return nullptr;
    }
    GetNextToken(); // Потребляем "("
    
    auto conditionNode = parseExpression();
    if (!conditionNode) {
        std::cerr << "Failed to parse if condition expression" << std::endl;
        return nullptr;
    }
    ifNode->addChild(conditionNode);
    
    token = PeekToken();
    if (token.first != 3 || token.second != ")") {
        std::cerr << "Syntax error: expected ')' after if condition" << std::endl;
        return nullptr;
    }
    GetNextToken(); // Потребляем ")"
    
    // Парсим тело if
    auto thenBodyNode = parseCompoundStatement();
    if (!thenBodyNode) {
        std::cerr << "Failed to parse if body compound statement" << std::endl;
        return nullptr;
    }
    ifNode->addChild(thenBodyNode);
    
    // Обрабатываем elif (0 или более)
    token = PeekToken();
    while (token.first == 1 && token.second == "elif") {
        GetNextToken(); // Потребляем "elif"
        
        // Для каждого elif создаем отдельный узел IF_STMT и добавляем его как child
        auto elifNode = createNode(NodeType::IF_STMT);
        
        // Парсим условие elif
        token = PeekToken();
        if (token.first != 3 || token.second != "(") {
            std::cerr << "Syntax error: expected '(' after elif" << std::endl;
            return nullptr;
        }
        GetNextToken(); // Потребляем "("
        
        auto elifConditionNode = parseExpression();
        if (!elifConditionNode) {
            std::cerr << "Failed to parse elif condition expression" << std::endl;
            return nullptr;
        }
        elifNode->addChild(elifConditionNode);
        
        token = PeekToken();
        if (token.first != 3 || token.second != ")") {
            std::cerr << "Syntax error: expected ')' after elif condition" << std::endl;
            return nullptr;
        }
        GetNextToken(); // Потребляем ")"
        
        // Парсим тело elif
        auto elifBodyNode = parseCompoundStatement();
        if (!elifBodyNode) {
            std::cerr << "Failed to parse elif body compound statement" << std::endl;
            return nullptr;
        }
        elifNode->addChild(elifBodyNode);
        
        // Добавляем elif как дополнительный child к основному if
        ifNode->addChild(elifNode);
        
        token = PeekToken();
    }
    
    // Обрабатываем else (опционально)
    token = PeekToken();
    if (token.first == 1 && token.second == "else") {
        GetNextToken(); // Потребляем "else"
        
        auto elseBodyNode = parseCompoundStatement();
        if (!elseBodyNode) {
            std::cerr << "Failed to parse else body compound statement" << std::endl;
            return nullptr;
        }
        
        // Создаем специальный узел для else (или используем тип ELSE_STMT, если определен)
        auto elseNode = createNode(NodeType::COMPOUND_STMT); // Или создайте ELSE_STMT в NodeType
        elseNode->setValue("else");
        elseNode->addChild(elseBodyNode);
        
        ifNode->addChild(elseNode);
    }
    
    return ifNode;
}

// ========== Реализация недостающих функций ==========

std::shared_ptr<ASTNode> Parser::parseWhileStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "while") {
        return nullptr;
    }
    GetNextToken(); // "while"
    
    // Создаем узел while statement
    auto whileNode = createNode(NodeType::WHILE_STMT);
    
    token = GetNextToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '(' after while" << std::endl;
        return nullptr;
    }
    
    // Парсим условие цикла
    auto conditionNode = parseExpression();
    if (!conditionNode) {
        std::cerr << "Failed to parse while condition" << std::endl;
        return nullptr;
    }
    whileNode->addChild(conditionNode);
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ")") {
        std::cerr << "Syntax error: expected ')' after while condition" << std::endl;
        return nullptr;
    }
    
    // Парсим тело цикла
    auto bodyNode = parseCompoundStatement();
    if (!bodyNode) {
        std::cerr << "Failed to parse while body" << std::endl;
        return nullptr;
    }
    whileNode->addChild(bodyNode);
    
    return whileNode;
}

std::shared_ptr<ASTNode> Parser::parseForStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "for") {
        return nullptr;
    }
    GetNextToken(); // "for"
    
    // Создаем узел for statement
    auto forNode = createNode(NodeType::FOR_STMT);
    
    token = GetNextToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '(' after for" << std::endl;
        return nullptr;
    }
    
    // Опциональная инициализация
    std::shared_ptr<ASTNode> initNode = nullptr;
    token = PeekToken();
    if (token.first != 3 || token.second != ";") {
        initNode = parseForInit();
        if (!initNode) {
            std::cerr << "Failed to parse for initialization" << std::endl;
            return nullptr;
        }
    }
    if (initNode) {
        forNode->addChild(initNode);
    } else {
        // Добавляем пустой узел для отсутствующей инициализации
        forNode->addChild(createNode(NodeType::EXPR_STMT, "empty"));
    }
    
    // Первая точка с запятой
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after for initialization" << std::endl;
        return nullptr;
    }
    
    // Опциональное условие
    std::shared_ptr<ASTNode> conditionNode = nullptr;
    token = PeekToken();
    if (token.first != 3 || token.second != ";") {
        conditionNode = parseExpression();
        if (!conditionNode) {
            std::cerr << "Failed to parse for condition" << std::endl;
            return nullptr;
        }
    }
    if (conditionNode) {
        forNode->addChild(conditionNode);
    } else {
        // Добавляем пустой узел для отсутствующего условия
        forNode->addChild(createNode(NodeType::EXPR_STMT, "empty"));
    }
    
    // Вторая точка с запятой
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after for condition" << std::endl;
        return nullptr;
    }
    
    // Опциональное выражение инкремента
    std::shared_ptr<ASTNode> incrementNode = nullptr;
    token = PeekToken();
    if (token.first != 3 || token.second != ")") {
        incrementNode = parseExpression();
        if (!incrementNode) {
            std::cerr << "Failed to parse for increment" << std::endl;
            return nullptr;
        }
    }
    if (incrementNode) {
        forNode->addChild(incrementNode);
    } else {
        // Добавляем пустой узел для отсутствующего инкремента
        forNode->addChild(createNode(NodeType::EXPR_STMT, "empty"));
    }
    
    // Закрывающая скобка
    token = GetNextToken();
    if (token.first != 3 || token.second != ")") {
        std::cerr << "Syntax error: expected ')' after for header" << std::endl;
        return nullptr;
    }
    
    // Тело цикла
    auto bodyNode = parseCompoundStatement();
    if (!bodyNode) {
        std::cerr << "Failed to parse for body" << std::endl;
        return nullptr;
    }
    forNode->addChild(bodyNode);
    
    return forNode;
}

std::shared_ptr<ASTNode> Parser::parseForInit() {
    // Пробуем объявление переменной
    auto varDeclNode = parseVariableDeclaration();
    if (varDeclNode) {
        return varDeclNode;
    }
    
    // Пробуем выражение statement
    auto exprStmtNode = parseExpressionStatement();
    if (exprStmtNode) {
        return exprStmtNode;
    }
    
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseReturnStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "return") {
        return nullptr;
    }
    GetNextToken(); // "return"
    
    // Создаем узел return statement
    auto returnNode = createNode(NodeType::RETURN_STMT);
    
    // Опциональное выражение
    token = PeekToken();
    if (token.first != 3 || token.second != ";") {
        auto exprNode = parseExpression();
        if (!exprNode) {
            std::cerr << "Failed to parse return expression" << std::endl;
            return nullptr;
        }
        returnNode->addChild(exprNode);
    }
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after return statement" << std::endl;
        return nullptr;
    }
    
    return returnNode;
}

std::shared_ptr<ASTNode> Parser::parseBreakStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "break") {
        return nullptr;
    }
    GetNextToken(); // "break"
    
    // Создаем узел break statement
    auto breakNode = createNode(NodeType::BREAK_STMT);
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after break statement" << std::endl;
        return nullptr;
    }
    
    return breakNode;
}

std::shared_ptr<ASTNode> Parser::parseContinueStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "continue") {
        return nullptr;
    }
    GetNextToken(); // "continue"
    
    // Создаем узел continue statement
    auto continueNode = createNode(NodeType::CONTINUE_STMT);
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after continue statement" << std::endl;
        return nullptr;
    }
    
    return continueNode;
}

std::shared_ptr<ASTNode> Parser::parsePrintStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "print") {
        return nullptr;
    }
    GetNextToken(); // "print"
    
    // Создаем узел print statement
    auto printNode = createNode(NodeType::PRINT_STMT);
    
    token = GetNextToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '(' after print" << std::endl;
        return nullptr;
    }
    
    // Парсим выражение для печати
    auto exprNode = parseExpression();
    if (!exprNode) {
        std::cerr << "Failed to parse print expression" << std::endl;
        return nullptr;
    }
    printNode->addChild(exprNode);
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ")") {
        std::cerr << "Syntax error: expected ')' after print expression" << std::endl;
        return nullptr;
    }
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after print statement" << std::endl;
        return nullptr;
    }
    
    return printNode;
}

std::shared_ptr<ASTNode> Parser::parseReadStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "read") {
        return nullptr;
    }
    GetNextToken(); // "read"
    
    // Создаем узел read statement
    auto readNode = createNode(NodeType::READ_STMT);
    
    token = GetNextToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '(' after read" << std::endl;
        return nullptr;
    }
    
    // Парсим идентификатор (имя переменной для чтения)
    auto identifierNode = parseIdentifier();
    if (!identifierNode) {
        std::cerr << "Failed to parse identifier for read" << std::endl;
        return nullptr;
    }
    readNode->addChild(identifierNode);
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ")") {
        std::cerr << "Syntax error: expected ')' after read identifier" << std::endl;
        return nullptr;
    }
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after read statement" << std::endl;
        return nullptr;
    }
    
    return readNode;
}

std::shared_ptr<ASTNode> Parser::parseVariableDeclaration() {
    // Парсим тип
    auto typeNode = parseType();
    if (!typeNode) {
        return nullptr;
    }
    
    // Парсим имя переменной
    auto identifierNode = parseIdentifier();
    if (!identifierNode) {
        std::cerr << "Failed to parse variable name" << std::endl;
        return nullptr;
    }
    
    // Создаем узел объявления переменной
    auto varDeclNode = createNode(NodeType::VARIABLE_DECL);
    varDeclNode->addChild(typeNode);
    varDeclNode->addChild(identifierNode);
    
    // Опциональная инициализация
    auto token = PeekToken();
    if (token.first == 2 && token.second == "=") {
        GetNextToken(); // "="
        
        auto initExprNode = parseExpression();
        if (!initExprNode) {
            std::cerr << "Failed to parse initialization expression" << std::endl;
            return nullptr;
        }
        varDeclNode->addChild(initExprNode);
    }
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after variable declaration" << std::endl;
        return nullptr;
    }
    
    return varDeclNode;
}

std::shared_ptr<ASTNode> Parser::parseArrayDeclaration() {
    // Парсим тип
    auto typeNode = parseType();
    if (!typeNode) {
        return nullptr;
    }
    
    // Парсим имя массива
    auto identifierNode = parseIdentifier();
    if (!identifierNode) {
        std::cerr << "Failed to parse array name" << std::endl;
        return nullptr;
    }
    
    // Создаем узел объявления массива
    auto arrayDeclNode = createNode(NodeType::ARRAY_DECL);
    arrayDeclNode->addChild(typeNode);
    arrayDeclNode->addChild(identifierNode);
    
    auto token = GetNextToken();
    if (token.first != 3 || token.second != "[") {
        std::cerr << "Syntax error: expected '[' for array declaration" << std::endl;
        return nullptr;
    }
    
    // Должен быть целочисленный литерал (NUMBER)
    token = GetNextToken();
    if (token.first != 5) {
        std::cerr << "Syntax error: expected integer literal for array size" << std::endl;
        return nullptr;
    }
    
    // Создаем узел для размера массива
    auto sizeNode = createNode(NodeType::LITERAL, token.second);
    arrayDeclNode->addChild(sizeNode);
    
    token = GetNextToken();
    if (token.first != 3 || token.second != "]") {
        std::cerr << "Syntax error: expected ']' after array size" << std::endl;
        return nullptr;
    }
    
    // Опциональная инициализация
    token = PeekToken();
    if (token.first == 2 && token.second == "=") {
        GetNextToken(); // "="
        
        token = GetNextToken();
        if (token.first != 3 || token.second != "{") {
            std::cerr << "Syntax error: expected '{' for array initialization" << std::endl;
            return nullptr;
        }
        
        // Создаем узел для списка инициализаторов
        auto initListNode = createNode(NodeType::COMPOUND_STMT, "InitializerList");
        
        // Опциональный список выражений
        token = PeekToken();
        if (token.first != 3 || token.second != "}") {
            auto exprListNode = parseExpressionList();
            if (!exprListNode) {
                return nullptr;
            }
            // Добавляем элементы из exprListNode в initListNode
            for (const auto& child : exprListNode->getChildren()) {
                initListNode->addChild(child);
            }
        }
        
        token = GetNextToken();
        if (token.first != 3 || token.second != "}") {
            std::cerr << "Syntax error: expected '}' after array initialization" << std::endl;
            return nullptr;
        }
        
        arrayDeclNode->addChild(initListNode);
    }
    
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after array declaration" << std::endl;
        return nullptr;
    }
    
    return arrayDeclNode;
}

std::shared_ptr<ASTNode> Parser::parseExpressionList() {
    // Создаем узел для списка выражений
    auto exprListNode = createNode(NodeType::COMPOUND_STMT, "ExpressionList");
    
    // Парсим первое выражение
    auto firstExpr = parseExpression();
    if (!firstExpr) {
        return nullptr;
    }
    exprListNode->addChild(firstExpr);
    
    // Парсим дополнительные выражения через запятую
    auto token = PeekToken();
    while (token.first == 3 && token.second == ",") {
        GetNextToken(); // ","
        
        auto nextExpr = parseExpression();
        if (!nextExpr) {
            return nullptr;
        }
        exprListNode->addChild(nextExpr);
        
        token = PeekToken();
    }
    
    return exprListNode;
}

// ========== Реализация parseExpression() и иерархии выражений ==========

std::shared_ptr<ASTNode> Parser::parseExpression() {
    return parseAssignmentExpression();
}

std::shared_ptr<ASTNode> Parser::parseAssignmentExpression() {
    // Парсим левую часть
    auto left = parseConditionalExpression();
    if (!left) {
        return nullptr;
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
        
        // Парсим правую часть
        auto right = parseAssignmentExpression();
        if (!right) {
            return nullptr;
        }
        
        // Создаем узел присваивания
        auto assignNode = createNode(NodeType::ASSIGN_EXPR, token.second);
        assignNode->addChild(left);
        assignNode->addChild(right);
        return assignNode;
    }
    
    // Если оператора присваивания нет, возвращаем левую часть
    return left;
}

std::shared_ptr<ASTNode> Parser::parseConditionalExpression() {
    return parseLogicalOrExpression();
}

std::shared_ptr<ASTNode> Parser::parseLogicalOrExpression() {
    // Парсим левую часть
    auto left = parseLogicalAndExpression();
    if (!left) {
        return nullptr;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && token.second == "||") {
        GetNextToken(); // "||"
        
        // Парсим правую часть
        auto right = parseLogicalAndExpression();
        if (!right) {
            return nullptr;
        }
        
        // Создаем бинарный операторный узел
        auto binaryNode = createBinaryExprNode("||", left, right);
        
        // Обновляем left для следующей итерации
        left = binaryNode;
        
        token = PeekToken();
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseLogicalAndExpression() {
    auto left = parseBitwiseOrExpression();
    if (!left) {
        return nullptr;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && token.second == "&&") {
        GetNextToken(); // "&&"
        
        auto right = parseBitwiseOrExpression();
        if (!right) {
            return nullptr;
        }
        
        auto binaryNode = createBinaryExprNode("&&", left, right);
        left = binaryNode;
        
        token = PeekToken();
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseBitwiseOrExpression() {
    auto left = parseBitwiseXorExpression();
    if (!left) {
        return nullptr;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && token.second == "|") {
        GetNextToken(); // "|"
        
        auto right = parseBitwiseXorExpression();
        if (!right) {
            return nullptr;
        }
        
        auto binaryNode = createBinaryExprNode("|", left, right);
        left = binaryNode;
        
        token = PeekToken();
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseBitwiseXorExpression() {
    auto left = parseBitwiseAndExpression();
    if (!left) {
        return nullptr;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && token.second == "^") {
        GetNextToken(); // "^"
        
        auto right = parseBitwiseAndExpression();
        if (!right) {
            return nullptr;
        }
        
        auto binaryNode = createBinaryExprNode("^", left, right);
        left = binaryNode;
        
        token = PeekToken();
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseBitwiseAndExpression() {
    auto left = parseEqualityExpression();
    if (!left) {
        return nullptr;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && token.second == "&") {
        GetNextToken(); // "&"
        
        auto right = parseEqualityExpression();
        if (!right) {
            return nullptr;
        }
        
        auto binaryNode = createBinaryExprNode("&", left, right);
        left = binaryNode;
        
        token = PeekToken();
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseEqualityExpression() {
    auto left = parseRelationalExpression();
    if (!left) {
        return nullptr;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && (token.second == "==" || token.second == "!=")) {
        std::string op = token.second;
        GetNextToken(); // "==" или "!="
        
        auto right = parseRelationalExpression();
        if (!right) {
            return nullptr;
        }
        
        auto binaryNode = createBinaryExprNode(op, left, right);
        left = binaryNode;
        
        token = PeekToken();
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseRelationalExpression() {
    auto left = parseShiftExpression();
    if (!left) {
        return nullptr;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && (token.second == "<" || token.second == ">" || 
                                token.second == "<=" || token.second == ">=")) {
        std::string op = token.second;
        GetNextToken(); // оператор сравнения
        
        auto right = parseShiftExpression();
        if (!right) {
            return nullptr;
        }
        
        auto binaryNode = createBinaryExprNode(op, left, right);
        left = binaryNode;
        
        token = PeekToken();
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseShiftExpression() {
    auto left = parseAdditiveExpression();
    if (!left) {
        return nullptr;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && (token.second == "<<" || token.second == ">>")) {
        std::string op = token.second;
        GetNextToken(); // "<<" или ">>"
        
        auto right = parseAdditiveExpression();
        if (!right) {
            return nullptr;
        }
        
        auto binaryNode = createBinaryExprNode(op, left, right);
        left = binaryNode;
        
        token = PeekToken();
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseAdditiveExpression() {
    auto left = parseMultiplicativeExpression();
    if (!left) {
        return nullptr;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && (token.second == "+" || token.second == "-")) {
        std::string op = token.second;
        GetNextToken(); // "+" или "-"
        
        auto right = parseMultiplicativeExpression();
        if (!right) {
            return nullptr;
        }
        
        auto binaryNode = createBinaryExprNode(op, left, right);
        left = binaryNode;
        
        token = PeekToken();
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseMultiplicativeExpression() {
    auto left = parseCastExpression();
    if (!left) {
        return nullptr;
    }
    
    auto token = PeekToken();
    while (token.first == 2 && (token.second == "*" || token.second == "/" || token.second == "%")) {
        std::string op = token.second;
        GetNextToken(); // "*", "/" или "%"
        
        auto right = parseCastExpression();
        if (!right) {
            return nullptr;
        }
        
        auto binaryNode = createBinaryExprNode(op, left, right);
        left = binaryNode;
        
        token = PeekToken();
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseCastExpression() {
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

std::shared_ptr<ASTNode> Parser::parseUnaryExpression() {
    auto token = PeekToken();
    
    // Унарные операторы: & * + - ! ~
    if (token.first == 2 && (token.second == "&" || token.second == "*" || 
                            token.second == "+" || token.second == "-" || 
                            token.second == "!" || token.second == "~")) {
        std::string op = token.second;
        GetNextToken(); // унарный оператор
        
        auto operand = parseCastExpression();
        if (!operand) {
            return nullptr;
        }
        
        // Создаем узел унарного выражения
        return createUnaryExprNode(op, operand);
    }
    
    // ++ или -- перед выражением (префиксные)
    if (token.first == 3 && (token.second == "++" || token.second == "--")) {
        std::string op = token.second;
        GetNextToken(); // "++" или "--"
        
        auto operand = parseCastExpression();
        if (!operand) {
            return nullptr;
        }
        
        // Создаем узел префиксного инкремента/декремента
        return createUnaryExprNode(op + "_prefix", operand);
    }
    
    // sizeof
    if (token.first == 1 && token.second == "sizeof") {
        GetNextToken(); // "sizeof"
        
        token = GetNextToken();
        if (token.first != 3 || token.second != "(") {
            return nullptr;
        }
        
        auto typeNode = parseType();
        if (!typeNode) {
            return nullptr;
        }
        
        token = GetNextToken();
        if (token.first != 3 || token.second != ")") {
            return nullptr;
        }
        
        // Создаем узел sizeof
        auto sizeofNode = createNode(NodeType::UNARY_EXPR, "sizeof");
        sizeofNode->addChild(typeNode);
        return sizeofNode;
    }
    
    // new
    if (token.first == 1 && token.second == "new") {
        GetNextToken(); // "new"
        
        auto typeNode = parseType();
        if (!typeNode) {
            return nullptr;
        }
        
        // Создаем узел new
        auto newNode = createNode(NodeType::UNARY_EXPR, "new");
        newNode->addChild(typeNode);
        
        token = PeekToken();
        if (token.first == 3 && token.second == "[") {
            GetNextToken(); // "["
            
            auto sizeExpr = parseExpression();
            if (!sizeExpr) {
                return nullptr;
            }
            newNode->addChild(sizeExpr);
            
            token = GetNextToken();
            if (token.first != 3 || token.second != "]") {
                return nullptr;
            }
        }
        
        return newNode;
    }
    
    // delete
    if (token.first == 1 && token.second == "delete") {
        GetNextToken(); // "delete"
        
        // Проверяем наличие []
        bool isArray = false;
        token = PeekToken();
        if (token.first == 3 && token.second == "[") {
            GetNextToken(); // "["
            token = GetNextToken();
            if (token.first != 3 || token.second != "]") {
                return nullptr;
            }
            isArray = true;
        }
        
        auto operand = parseCastExpression();
        if (!operand) {
            return nullptr;
        }
        
        // Создаем узел delete
        std::string op = isArray ? "delete[]" : "delete";
        auto deleteNode = createNode(NodeType::UNARY_EXPR, op);
        deleteNode->addChild(operand);
        return deleteNode;
    }
    
    // Если не унарный оператор, пробуем постфиксное выражение
    return parsePostfixExpression();
}

std::shared_ptr<ASTNode> Parser::parsePostfixExpression() {
    // Парсим первичное выражение
    auto expr = parsePrimaryExpression();
    if (!expr) {
        return nullptr;
    }
    
    // Обрабатываем постфиксные операторы (0 или более)
    while (true) {
        auto token = PeekToken();
        
        // Индексация: "[" expression "]"
        if (token.first == 3 && token.second == "[") {
            GetNextToken(); // "["
            
            auto indexExpr = parseExpression();
            if (!indexExpr) {
                return nullptr;
            }
            
            token = GetNextToken();
            if (token.first != 3 || token.second != "]") {
                return nullptr;
            }
            
            // Создаем узел доступа к массиву
            auto arrayAccessNode = createNode(NodeType::ARRAY_ACCESS_EXPR);
            arrayAccessNode->addChild(expr);      // База (массив)
            arrayAccessNode->addChild(indexExpr); // Индекс
            expr = arrayAccessNode;
            continue;
        }
        
        // Вызов функции: "(" [expression_list] ")"
        if (token.first == 3 && token.second == "(") {
            GetNextToken(); // "("
            
            // Создаем узел вызова функции
            auto callNode = createNode(NodeType::CALL_EXPR);
            callNode->addChild(expr); // Функция
            
            token = PeekToken();
            if (token.first != 3 || token.second != ")") {
                auto argsNode = parseExpressionList();
                if (!argsNode) {
                    return nullptr;
                }
                // Добавляем аргументы
                for (const auto& arg : argsNode->getChildren()) {
                    callNode->addChild(arg);
                }
            }
            
            token = GetNextToken();
            if (token.first != 3 || token.second != ")") {
                return nullptr;
            }
            
            expr = callNode;
            continue;
        }
        
        // Доступ к полю: "." identifier
        if (token.first == 3 && token.second == ".") {
            GetNextToken(); // "."
            
            auto memberNode = parseIdentifier();
            if (!memberNode) {
                return nullptr;
            }
            
            // Создаем узел доступа к полю
            auto memberAccessNode = createNode(NodeType::MEMBER_ACCESS_EXPR, ".");
            memberAccessNode->addChild(expr);     // Объект
            memberAccessNode->addChild(memberNode); // Поле
            expr = memberAccessNode;
            continue;
        }
        
        // Доступ через указатель: "->" identifier
        if (token.first == 3 && token.second == "->") {
            GetNextToken(); // "->"
            
            auto memberNode = parseIdentifier();
            if (!memberNode) {
                return nullptr;
            }
            
            // Создаем узел доступа через указатель
            auto ptrAccessNode = createNode(NodeType::MEMBER_ACCESS_EXPR, "->");
            ptrAccessNode->addChild(expr);      // Указатель
            ptrAccessNode->addChild(memberNode); // Поле
            expr = ptrAccessNode;
            continue;
        }
        
        // Постфиксный инкремент/декремент: "++" или "--"
        if (token.first == 3 && (token.second == "++" || token.second == "--")) {
            std::string op = token.second + "_postfix";
            GetNextToken(); // "++" или "--"
            
            // Создаем узел постфиксного инкремента/декремента
            auto postfixNode = createNode(NodeType::UNARY_EXPR, op);
            postfixNode->addChild(expr);
            expr = postfixNode;
            continue;
        }
        
        // Больше нет постфиксных операторов
        break;
    }
    
    return expr;
}

std::shared_ptr<ASTNode> Parser::parsePrimaryExpression() {
    auto token = PeekToken();
    
    // Идентификатор
    if (token.first == 4) {
        auto node = createNode(NodeType::IDENTIFIER, token.second);
        GetNextToken();
        return node;
    }
    
    // Числовой литерал
    if (token.first == 5) {
        auto node = createNode(NodeType::LITERAL, token.second);
        GetNextToken();
        return node;
    }
    
    // Строковый литерал
    if (token.first == 6) {
        auto node = createNode(NodeType::LITERAL, token.second);
        GetNextToken();
        return node;
    }
    
    // Символьный литерал
    if (token.first == 7) {
        auto node = createNode(NodeType::LITERAL, token.second);
        GetNextToken();
        return node;
    }
    
    // true/false
    if (token.first == 1 && (token.second == "true" || token.second == "false")) {
        auto node = createNode(NodeType::LITERAL, token.second);
        GetNextToken();
        return node;
    }
    
    // Выражение в скобках
    if (token.first == 3 && token.second == "(") {
        GetNextToken(); // "("
        auto expr = parseExpression();
        if (!expr) return nullptr;
        
        token = GetNextToken();
        if (token.first != 3 || token.second != ")") return nullptr;
        
        return expr;
    }
    
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseLiteral() {
    auto token = PeekToken();
    
    // Целочисленный или вещественный литерал (NUMBER)
    if (token.first == 5) {
        auto node = createNode(NodeType::LITERAL, token.second);
        GetNextToken();
        return node;
    }
    
    // Символьный литерал
    if (token.first == 7) { // CHAR_LITERAL
        auto node = createNode(NodeType::LITERAL, token.second);
        GetNextToken();
        return node;
    }
    
    // Строковый литерал
    if (token.first == 6) { // STRING_LITERAL
        auto node = createNode(NodeType::LITERAL, token.second);
        GetNextToken();
        return node;
    }
    
    // Логические литералы
    if (token.first == 1 && (token.second == "true" || token.second == "false")) {
        auto node = createNode(NodeType::LITERAL, token.second);
        GetNextToken();
        return node;
    }
    
    // nullptr
    if (token.first == 1 && token.second == "nullptr") {
        auto node = createNode(NodeType::LITERAL, "nullptr");
        GetNextToken();
        return node;
    }
    
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseLetter() {
    // Эта функция может быть не нужна, так как лексер уже разобрал идентификаторы
    // Но если требуется по грамматике, можно реализовать проверку
    auto token = PeekToken();
    if (token.second.length() == 1) {
        char c = token.second[0];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
            // Создаем узел для буквы (возможно, как литерал или специальный тип)
            auto node = createNode(NodeType::LITERAL, token.second);
            GetNextToken();
            return node;
        }
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseDigit() {
    // Аналогично, может быть не нужна
    auto token = PeekToken();
    if (token.second.length() == 1) {
        char c = token.second[0];
        if (c >= '0' && c <= '9') {
            // Создаем узел для цифры
            auto node = createNode(NodeType::LITERAL, token.second);
            GetNextToken();
            return node;
        }
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseAssignmentOperator() {
    auto token = PeekToken();
    if (token.first == 2) { // OPERATOR
        std::string op = token.second;
        if (op == "=" || op == "+=" || op == "-=" || op == "*=" || 
            op == "/=" || op == "%=" || op == "<<=" || op == ">>=" || 
            op == "&=" || op == "^=" || op == "|=") {
            // Создаем узел оператора
            auto node = createNode(NodeType::OPERATOR, op);
            GetNextToken();
            return node;
        }
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseUnaryOperator() {
    auto token = PeekToken();
    if (token.first == 2) { // OPERATOR
        std::string op = token.second;
        if (op == "&" || op == "*" || op == "+" || op == "-" || 
            op == "!" || op == "~") {
            // Создаем узел унарного оператора
            auto node = createNode(NodeType::OPERATOR, op);
            GetNextToken();
            return node;
        }
    }
    return nullptr;
}