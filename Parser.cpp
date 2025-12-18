#include "Parser.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <string>

std::pair<int, std::string> Parser::GetNextToken() {
    if (cur >= tokens.size()) return {0, ""};
    return tokens[cur++];
}

std::pair<int, std::string> Parser::PeekToken() {
    if (cur >= tokens.size()) return {0, ""};
    return tokens[cur];
}

std::pair<int, std::string> Parser::PeekNextToken() {
    if (cur + 1 >= tokens.size()) return {0, ""};
    return tokens[cur + 1];
}

bool Parser::IsAtEnd() {
    return cur >= tokens.size() || tokens[cur].first == 0;
}

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
    while (!IsAtEnd()) {
        auto cur_token = PeekToken();
        auto next_token = PeekNextToken();
        bool is_main = cur_token.first == 1 && cur_token.second == "int" && 
                      next_token.first == 4 && next_token.second == "main";
        if (is_main) {
            break;
        }
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
            auto funcNode = parseFunction();
            if (!funcNode) {
                std::cerr << "Failed to parse function" << std::endl;
                return nullptr;
            }
            programNode->addChild(funcNode);
        }
    }
    auto mainNode = parseMain();
    if (!mainNode) {
        std::cerr << "Failed to parse main function" << std::endl;
        return nullptr;
    }
    programNode->addChild(mainNode);
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
    GetNextToken();
    auto classNode = createNode(NodeType::CLASS_DECL);
    auto classNameNode = parseClassName();
    if (!classNameNode) {
        std::cerr << "Failed to parse class name" << std::endl;
        return nullptr;
    }
    classNode->addChild(classNameNode);
    classNode->setValue(classNameNode->getValue());
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
    auto typeNode = parseType();
    if (!typeNode) {
        std::cerr << "Failed to parse type" << std::endl;
        return nullptr;
    }
    funcNode->addChild(typeNode);
    auto nameNode = parseFunctionName();
    if (!nameNode) {
        std::cerr << "Failed to parse function name" << std::endl;
        return nullptr;
    }
    funcNode->addChild(nameNode);
    funcNode->setValue(nameNode->getValue());
    auto paramsNode = parseParameters();
    if (!paramsNode) {
        std::cerr << "Failed to parse parameters" << std::endl;
        return nullptr;
    }
    funcNode->addChild(paramsNode);
    auto bodyNode = parseFunctionBody();
    if (!bodyNode) {
        std::cerr << "Failed to parse function body" << std::endl;
        return nullptr;
    }
    funcNode->addChild(bodyNode);
    return funcNode;
}

std::shared_ptr<ASTNode> Parser::parseMain() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "int") {
        return nullptr;
    }
    GetNextToken();
    token = PeekToken();
    if (token.first != 4 || token.second != "main") {
        return nullptr;
    }
    GetNextToken();
    auto mainNode = createNode(NodeType::MAIN_FUNCTION, "main");
    auto paramsNode = parseParameters();
    if (!paramsNode) {
        std::cerr << "Failed to parse main parameters" << std::endl;
        return nullptr;
    }
    mainNode->addChild(paramsNode);
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
    return identifierNode;
}

std::shared_ptr<ASTNode> Parser::parseClassBody() {
    auto token = GetNextToken();
    if (token.first != 3 || token.second != "{") {
        std::cerr << "Syntax error: expected {" << std::endl;
        return nullptr;
    }
    auto classBodyNode = createNode(NodeType::COMPOUND_STMT, "ClassBody");
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
    auto paramsNode = createNode(NodeType::COMPOUND_STMT, "Parameters");
    token = PeekToken();
    if (token.first == 3 && token.second == ")") {
        GetNextToken();
        return paramsNode;
    }
    auto firstParam = parseParameter();
    if (!firstParam) {
        std::cerr << "Failed to parse parameter" << std::endl;
        return nullptr;
    }
    paramsNode->addChild(firstParam);
    token = PeekToken();
    while (token.first == 3 && token.second == ",") {
        GetNextToken();
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
    if (!memberNode) memberNode = parseMethod();
    if (!memberNode) memberNode = parseFieldDeclaration();
    if (!memberNode) memberNode = parseConstructor();
    if (!memberNode) memberNode = parseDestructor();
    return memberNode;
}

std::shared_ptr<ASTNode> Parser::parseParameter() {
    auto typeNode = parseType();
    if (!typeNode) {
        return nullptr;
    }
    auto t = PeekToken();
    if (t.first != 4) {
        return nullptr;
    }
    auto identifierNode = parseIdentifier();
    if (!identifierNode) {
        return nullptr;
    }
    auto paramNode = createNode(NodeType::PARAMETER);
    paramNode->addChild(typeNode);
    paramNode->addChild(identifierNode);
    return paramNode;
}

std::shared_ptr<ASTNode> Parser::parseCompoundStatement() {
    auto token = PeekToken();
    if (token.first != 3 || token.second != "{") {
        return nullptr;
    }
    GetNextToken();
    auto compoundNode = createNode(NodeType::COMPOUND_STMT);
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
    GetNextToken();
    return compoundNode;
}

std::shared_ptr<ASTNode> Parser::parseFieldDeclaration() {
    auto typeNode = parseType();
    if (!typeNode) {
        return nullptr;
    }
    auto fieldNode = createNode(NodeType::FIELD_DECL);
    fieldNode->addChild(typeNode);
    auto firstIdNode = parseIdentifier();
    if (!firstIdNode) {
        std::cerr << "Failed to parse identifier in field declaration" << std::endl;
        return nullptr;
    }
    fieldNode->addChild(firstIdNode);
    auto token = PeekToken();
    while (token.first == 3 && token.second == ",") {
        GetNextToken();
        auto nextIdNode = parseIdentifier();
        if (!nextIdNode) {
            std::cerr << "Failed to parse identifier after comma in field declaration" << std::endl;
            return nullptr;
        }
        fieldNode->addChild(nextIdNode);
        token = PeekToken();
    }
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
        return nullptr;
    }
    GetNextToken();
    auto constructorNode = createNode(NodeType::CONSTRUCTOR);
    auto paramsNode = parseConstructorParameters();
    if (!paramsNode) {
        std::cerr << "Failed to parse constructor parameters" << std::endl;
        return nullptr;
    }
    constructorNode->addChild(paramsNode);
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
        return nullptr;
    }
    GetNextToken();
    auto destructorNode = createNode(NodeType::DESTRUCTOR);
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
    auto bodyNode = parseDestructorBody();
    if (!bodyNode) {
        std::cerr << "Failed to parse destructor body" << std::endl;
        return nullptr;
    }
    destructorNode->addChild(bodyNode);
    return destructorNode;
}

std::shared_ptr<ASTNode> Parser::parseMethod() {
    auto typeNode = parseType();
    if (!typeNode) {
        std::cerr << "Failed to parse method return type" << std::endl;
        return nullptr;
    }
    auto nameNode = parseIdentifier();
    if (!nameNode) {
        std::cerr << "Failed to parse method name" << std::endl;
        return nullptr;
    }
    auto methodNode = createNode(NodeType::METHOD);
    methodNode->addChild(typeNode);
    methodNode->addChild(nameNode);
    methodNode->setValue(nameNode->getValue());
    auto paramsNode = parseParameters();
    if (!paramsNode) {
        std::cerr << "Failed to parse method parameters" << std::endl;
        return nullptr;
    }
    methodNode->addChild(paramsNode);
    auto bodyNode = parseMethodBody();
    if (!bodyNode) {
        std::cerr << "Failed to parse method body" << std::endl;
        return nullptr;
    }
    methodNode->addChild(bodyNode);
    return methodNode;
}

std::shared_ptr<ASTNode> Parser::parseConstructorParameters() {
    return parseParameters();
}

std::shared_ptr<ASTNode> Parser::parseConstructorBody() {
    return parseCompoundStatement();
}

std::shared_ptr<ASTNode> Parser::parseDestructorBody() {
    return parseCompoundStatement();
}

std::shared_ptr<ASTNode> Parser::parseMethodBody() {
    return parseCompoundStatement();
}

std::shared_ptr<ASTNode> Parser::parseStatement() {
    std::shared_ptr<ASTNode> stmtNode = nullptr;
    if (!stmtNode) stmtNode = parseExpressionStatement();
    if (!stmtNode) stmtNode = parseCompoundStatement();
    if (!stmtNode) stmtNode = parseIfStatement();
    if (!stmtNode) stmtNode = parseWhileStatement();
    if (!stmtNode) stmtNode = parseForStatement();
    if (!stmtNode) stmtNode = parseReturnStatement();
    if (!stmtNode) stmtNode = parseBreakStatement();
    if (!stmtNode) stmtNode = parseContinueStatement();
    if (!stmtNode) stmtNode = parsePrintStatement();
    if (!stmtNode) stmtNode = parseReadStatement();
    if (!stmtNode) stmtNode = parseDeclarationStatement();
    return stmtNode;
}

std::shared_ptr<ASTNode> Parser::parseDeclarationStatement() {
    std::shared_ptr<ASTNode> declNode = nullptr;
    if (!declNode) declNode = parseVariableDeclaration();
    if (!declNode) declNode = parseArrayDeclaration();
    return declNode;
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
    GetNextToken();
    auto exprStmtNode = createNode(NodeType::EXPR_STMT);
    exprStmtNode->addChild(exprNode);
    return exprStmtNode;
}

std::shared_ptr<ASTNode> Parser::parseIfStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "if") {
        return nullptr;
    }
    GetNextToken();
    auto ifNode = createNode(NodeType::IF_STMT);
    token = PeekToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '(' after if" << std::endl;
        return nullptr;
    }
    GetNextToken();
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
    GetNextToken();
    auto thenBodyNode = parseCompoundStatement();
    if (!thenBodyNode) {
        std::cerr << "Failed to parse if body compound statement" << std::endl;
        return nullptr;
    }
    ifNode->addChild(thenBodyNode);
    token = PeekToken();
    while (token.first == 1 && token.second == "elif") {
        GetNextToken();
        auto elifNode = createNode(NodeType::IF_STMT);
        token = PeekToken();
        if (token.first != 3 || token.second != "(") {
            std::cerr << "Syntax error: expected '(' after elif" << std::endl;
            return nullptr;
        }
        GetNextToken();
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
        GetNextToken();
        auto elifBodyNode = parseCompoundStatement();
        if (!elifBodyNode) {
            std::cerr << "Failed to parse elif body compound statement" << std::endl;
            return nullptr;
        }
        elifNode->addChild(elifBodyNode);
        ifNode->addChild(elifNode);
        token = PeekToken();
    }
    token = PeekToken();
    if (token.first == 1 && token.second == "else") {
        GetNextToken();
        auto elseBodyNode = parseCompoundStatement();
        if (!elseBodyNode) {
            std::cerr << "Failed to parse else body compound statement" << std::endl;
            return nullptr;
        }
        auto elseNode = createNode(NodeType::COMPOUND_STMT);
        elseNode->setValue("else");
        elseNode->addChild(elseBodyNode);
        ifNode->addChild(elseNode);
    }
    
    return ifNode;
}

std::shared_ptr<ASTNode> Parser::parseWhileStatement() {
    auto token = PeekToken();
    if (token.first != 1 || token.second != "while") {
        return nullptr;
    }
    GetNextToken();
    auto whileNode = createNode(NodeType::WHILE_STMT);
    token = GetNextToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '(' after while" << std::endl;
        return nullptr;
    }
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
    GetNextToken();
    auto forNode = createNode(NodeType::FOR_STMT);
    token = GetNextToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '(' after for" << std::endl;
        return nullptr;
    }
    std::shared_ptr<ASTNode> initNode = nullptr;
    token = PeekToken();
    if (token.first != 3 || token.second != ";") {
        initNode = parseForInit();
        if (!initNode) {
            std::cerr << "Failed to parse for initialization" << std::endl;
            return nullptr;
        }
    }
    else{
        token = GetNextToken();
    }
    if (initNode) {
        forNode->addChild(initNode);
    } else {
        forNode->addChild(createNode(NodeType::EXPR_STMT, "empty"));
    }
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
        forNode->addChild(createNode(NodeType::EXPR_STMT, "empty"));
    }
    token = GetNextToken();
    if (token.first != 3 || token.second != ";") {
        std::cerr << "Syntax error: expected ';' after for condition" << std::endl;
        return nullptr;
    }
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
        forNode->addChild(createNode(NodeType::EXPR_STMT, "empty"));
    }
    token = GetNextToken();
    if (token.first != 3 || token.second != ")") {
        std::cerr << "Syntax error: expected ')' after for header" << std::endl;
        return nullptr;
    }
    auto bodyNode = parseCompoundStatement();
    if (!bodyNode) {
        std::cerr << "Failed to parse for body" << std::endl;
        return nullptr;
    }
    forNode->addChild(bodyNode);
    return forNode;
}

std::shared_ptr<ASTNode> Parser::parseForInit() {
    auto varDeclNode = parseVariableDeclaration();
    if (varDeclNode) {
        return varDeclNode;
    }
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
    GetNextToken();
    auto returnNode = createNode(NodeType::RETURN_STMT);
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
    GetNextToken();
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
    GetNextToken();
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
    GetNextToken();
    auto printNode = createNode(NodeType::PRINT_STMT);
    token = GetNextToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '(' after print" << std::endl;
        return nullptr;
    }
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
    GetNextToken();
    auto readNode = createNode(NodeType::READ_STMT);
    token = GetNextToken();
    if (token.first != 3 || token.second != "(") {
        std::cerr << "Syntax error: expected '(' after read" << std::endl;
        return nullptr;
    }
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
    auto typeNode = parseType();
    if (!typeNode) {
        return nullptr;
    }
    auto identifierNode = parseIdentifier();
    if (!identifierNode) {
        std::cerr << "Failed to parse variable name" << std::endl;
        return nullptr;
    }
    auto varDeclNode = createNode(NodeType::VARIABLE_DECL);
    varDeclNode->addChild(typeNode);
    varDeclNode->addChild(identifierNode);
    auto token = PeekToken();
    if (token.first == 2 && token.second == "=") {
        GetNextToken();
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
    auto typeNode = parseType();
    if (!typeNode) {
        return nullptr;
    }
    auto identifierNode = parseIdentifier();
    if (!identifierNode) {
        std::cerr << "Failed to parse array name" << std::endl;
        return nullptr;
    }
    auto arrayDeclNode = createNode(NodeType::ARRAY_DECL);
    arrayDeclNode->addChild(typeNode);
    arrayDeclNode->addChild(identifierNode);
    auto token = GetNextToken();
    if (token.first != 3 || token.second != "[") {
        std::cerr << "Syntax error: expected '[' for array declaration" << std::endl;
        return nullptr;
    }
    token = GetNextToken();
    if (token.first != 5) {
        std::cerr << "Syntax error: expected integer literal for array size" << std::endl;
        return nullptr;
    }
    auto sizeNode = createNode(NodeType::LITERAL, token.second);
    arrayDeclNode->addChild(sizeNode);
    token = GetNextToken();
    if (token.first != 3 || token.second != "]") {
        std::cerr << "Syntax error: expected ']' after array size" << std::endl;
        return nullptr;
    }
    token = PeekToken();
    if (token.first == 2 && token.second == "=") {
        GetNextToken();
        token = GetNextToken();
        if (token.first != 3 || token.second != "{") {
            std::cerr << "Syntax error: expected '{' for array initialization" << std::endl;
            return nullptr;
        }
        auto initListNode = createNode(NodeType::COMPOUND_STMT, "InitializerList");
        token = PeekToken();
        if (token.first != 3 || token.second != "}") {
            auto exprListNode = parseExpressionList();
            if (!exprListNode) {
                return nullptr;
            }
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
    auto exprListNode = createNode(NodeType::COMPOUND_STMT, "ExpressionList");
    auto firstExpr = parseExpression();
    if (!firstExpr) {
        return nullptr;
    }
    exprListNode->addChild(firstExpr);
    auto token = PeekToken();
    while (token.first == 3 && token.second == ",") {
        GetNextToken();
        auto nextExpr = parseExpression();
        if (!nextExpr) {
            return nullptr;
        }
        exprListNode->addChild(nextExpr);
        token = PeekToken();
    }
    return exprListNode;
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    return parseAssignmentExpression();
}

std::shared_ptr<ASTNode> Parser::parseAssignmentExpression() {
    auto left = parseConditionalExpression();
    if (!left) {
        return nullptr;
    }
    auto token = PeekToken();
    if (token.first == 2 && (token.second == "=" || token.second == "+=" || 
                             token.second == "-=" || token.second == "*=" || 
                             token.second == "/=" || token.second == "%=" || 
                             token.second == "<<=" || token.second == ">>=" || 
                             token.second == "&=" || token.second == "^=" || 
                             token.second == "|=")) {
        GetNextToken();
        auto right = parseAssignmentExpression();
        if (!right) {
            return nullptr;
        }
        auto assignNode = createNode(NodeType::ASSIGN_EXPR, token.second);
        assignNode->addChild(left);
        assignNode->addChild(right);
        return assignNode;
    }
    return left;
}

std::shared_ptr<ASTNode> Parser::parseConditionalExpression() {
    return parseLogicalOrExpression();
}

std::shared_ptr<ASTNode> Parser::parseLogicalOrExpression() {
    auto left = parseLogicalAndExpression();
    if (!left) {
        return nullptr;
    }
    auto token = PeekToken();
    while (token.first == 2 && token.second == "||") {
        GetNextToken();
        auto right = parseLogicalAndExpression();
        if (!right) {
            return nullptr;
        }
        auto binaryNode = createBinaryExprNode("||", left, right);
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
        GetNextToken();
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
        GetNextToken();
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
        GetNextToken();
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
        GetNextToken();
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
        GetNextToken();   
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
        GetNextToken();   
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
        GetNextToken();   
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
        GetNextToken();   
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
        GetNextToken();   
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
    size_t savedPos = cur;
    auto token = PeekToken();
    if (token.first == 3 && token.second == "(") {
        GetNextToken();
        auto typeNode = parseType();
        if (typeNode) {
            token = PeekToken();
            if (token.first == 3 && token.second == ")") {
                GetNextToken();
                auto operand = parseCastExpression();
                if (!operand) {
                    return nullptr;
                }
                auto castNode = createNode(NodeType::UNARY_EXPR, "cast");
                castNode->addChild(typeNode);
                castNode->addChild(operand);
                return castNode;
            }
        }
        cur = savedPos;
    }
    return parseUnaryExpression();
}

std::shared_ptr<ASTNode> Parser::parseUnaryExpression() {
    auto token = PeekToken();
    if (token.first == 2 && (token.second == "&" || token.second == "*" || 
                            token.second == "+" || token.second == "-" || 
                            token.second == "!" || token.second == "~")) {
        std::string op = token.second;
        GetNextToken();
        auto operand = parseCastExpression();
        if (!operand) {
            return nullptr;
        }
        return createUnaryExprNode(op, operand);
    }
    if (token.first == 3 && (token.second == "++" || token.second == "--")) {
        std::string op = token.second;
        GetNextToken();
        auto operand = parseCastExpression();
        if (!operand) {
            return nullptr;
        }
        return createUnaryExprNode(op + "_prefix", operand);
    }
    if (token.first == 1 && token.second == "sizeof") {
        GetNextToken();
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
        auto sizeofNode = createNode(NodeType::UNARY_EXPR, "sizeof");
        sizeofNode->addChild(typeNode);
        return sizeofNode;
    }

    if (token.first == 1 && token.second == "new") {
        GetNextToken();
        auto typeNode = parseType();
        if (!typeNode) {
            return nullptr;
        }
        auto newNode = createNode(NodeType::UNARY_EXPR, "new");
        newNode->addChild(typeNode);
        token = PeekToken();
        if (token.first == 3 && token.second == "[") {
            GetNextToken();
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
    
    if (token.first == 1 && token.second == "delete") {
        GetNextToken();
        bool isArray = false;
        token = PeekToken();
        if (token.first == 3 && token.second == "[") {
            GetNextToken();
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
        std::string op = isArray ? "delete[]" : "delete";
        auto deleteNode = createNode(NodeType::UNARY_EXPR, op);
        deleteNode->addChild(operand);
        return deleteNode;
    }

    return parsePostfixExpression();
}

std::shared_ptr<ASTNode> Parser::parsePostfixExpression() {
    auto expr = parsePrimaryExpression();
    if (!expr) {
        return nullptr;
    }
    while (true) {
        auto token = PeekToken();
        if (token.first == 3 && token.second == "[") {
            GetNextToken();
            auto indexExpr = parseExpression();
            if (!indexExpr) {
                return nullptr;
            }
            token = GetNextToken();
            if (token.first != 3 || token.second != "]") {
                return nullptr;
            }
            auto arrayAccessNode = createNode(NodeType::ARRAY_ACCESS_EXPR);
            arrayAccessNode->addChild(expr);
            arrayAccessNode->addChild(indexExpr);
            expr = arrayAccessNode;
            continue;
        }
        if (token.first == 3 && token.second == "(") {
            GetNextToken();
            auto callNode = createNode(NodeType::CALL_EXPR);
            callNode->addChild(expr);
            token = PeekToken();
            if (token.first != 3 || token.second != ")") {
                auto argsNode = parseExpressionList();
                if (!argsNode) {
                    return nullptr;
                }
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
        if (token.first == 3 && token.second == ".") {
            GetNextToken();
            auto memberNode = parseIdentifier();
            if (!memberNode) {
                return nullptr;
            }
            auto memberAccessNode = createNode(NodeType::MEMBER_ACCESS_EXPR, ".");
            memberAccessNode->addChild(expr);
            memberAccessNode->addChild(memberNode);
            expr = memberAccessNode;
            continue;
        }
        if (token.first == 3 && token.second == "->") {
            GetNextToken();
            auto memberNode = parseIdentifier();
            if (!memberNode) {
                return nullptr;
            }
            auto ptrAccessNode = createNode(NodeType::MEMBER_ACCESS_EXPR, "->");
            ptrAccessNode->addChild(expr);
            ptrAccessNode->addChild(memberNode);
            expr = ptrAccessNode;
            continue;
        }
        
        if (token.first == 3 && (token.second == "++" || token.second == "--")) {
            std::string op = token.second + "_postfix";
            GetNextToken();
            auto postfixNode = createNode(NodeType::UNARY_EXPR, op);
            postfixNode->addChild(expr);
            expr = postfixNode;
            continue;
        }
        break;
    }
    return expr;
}

std::shared_ptr<ASTNode> Parser::parsePrimaryExpression() {
    auto token = PeekToken();
    if (token.first == 4) {
        auto node = createNode(NodeType::IDENTIFIER, token.second);
        GetNextToken();
        return node;
    }
    if (token.first == 5) {
        auto node = createNode(NodeType::LITERAL, token.second);
        GetNextToken();
        return node;
    }
    if (token.first == 6) {
        auto node = createNode(NodeType::LITERAL, token.second);
        GetNextToken();
        return node;
    }
    if (token.first == 7) {
        auto node = createNode(NodeType::LITERAL, token.second);
        GetNextToken();
        return node;
    }
    if (token.first == 1 && (token.second == "true" || token.second == "false")) {
        auto node = createNode(NodeType::LITERAL, token.second);
        GetNextToken();
        return node;
    }
    if (token.first == 1 && token.second == "nullptr") {
        auto node = createNode(NodeType::LITERAL, "nullptr");
        GetNextToken();
        return node;
    }
    if (token.first == 1 && token.second == "this") {
        auto node = createNode(NodeType::IDENTIFIER, "this");
        GetNextToken();
        return node;
    }
    if (token.first == 3 && token.second == "(") {
        GetNextToken();
        auto expr = parseExpression();
        if (!expr) {
            return nullptr;
        }
        
        token = GetNextToken();
        if (token.first != 3 || token.second != ")") {
            std::cerr << "Syntax error: expected ')' after expression" << std::endl;
            return nullptr;
        }
        
        return expr;
    }
    
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseLiteral() {
    auto token = PeekToken();
    if (token.first == 5) {
        auto node = createNode(NodeType::LITERAL, token.second);
        GetNextToken();
        return node;
    }

    if (token.first == 7) {
        auto node = createNode(NodeType::LITERAL, token.second);
        GetNextToken();
        return node;
    }

    if (token.first == 6) {
        auto node = createNode(NodeType::LITERAL, token.second);
        GetNextToken();
        return node;
    }

    if (token.first == 1 && (token.second == "true" || token.second == "false")) {
        auto node = createNode(NodeType::LITERAL, token.second);
        GetNextToken();
        return node;
    }

    if (token.first == 1 && token.second == "nullptr") {
        auto node = createNode(NodeType::LITERAL, "nullptr");
        GetNextToken();
        return node;
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseLetter() {
    auto token = PeekToken();
    if (token.second.length() == 1) {
        char c = token.second[0];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
            auto node = createNode(NodeType::LITERAL, token.second);
            GetNextToken();
            return node;
        }
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseDigit() {
    auto token = PeekToken();
    if (token.second.length() == 1) {
        char c = token.second[0];
        if (c >= '0' && c <= '9') {
            auto node = createNode(NodeType::LITERAL, token.second);
            GetNextToken();
            return node;
        }
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseAssignmentOperator() {
    auto token = PeekToken();
    if (token.first == 2) {
        std::string op = token.second;
        if (op == "=" || op == "+=" || op == "-=" || op == "*=" || 
            op == "/=" || op == "%=" || op == "<<=" || op == ">>=" || 
            op == "&=" || op == "^=" || op == "|=") {
            auto node = createNode(NodeType::OPERATOR, op);
            GetNextToken();
            return node;
        }
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseUnaryOperator() {
    auto token = PeekToken();
    if (token.first == 2) {
        std::string op = token.second;
        if (op == "&" || op == "*" || op == "+" || op == "-" || 
            op == "!" || op == "~") {
            auto node = createNode(NodeType::OPERATOR, op);
            GetNextToken();
            return node;
        }
    }
    return nullptr;
}