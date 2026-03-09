#include "ASTNode.h"
#include <sstream>

ASTNode::ASTNode(NodeType type, const std::string& value) 
    : type_(type), value_(value) {}

NodeType ASTNode::getType() const { return type_; }
std::string ASTNode::getValue() const { return value_; }
const std::vector<std::shared_ptr<ASTNode>>& ASTNode::getChildren() const { return children_; }

void ASTNode::addChild(const std::shared_ptr<ASTNode>& child) {
    if (child) {
        children_.push_back(child);
    }
}

void ASTNode::addChildFront(const std::shared_ptr<ASTNode>& child) {
    if (child) {
        children_.insert(children_.begin(), child);
    }
}

void ASTNode::setValue(const std::string& value) {
    value_ = value;
}

std::string ASTNode::toString() const {
    std::stringstream ss;
    ss << nodeTypeToString(type_);
    if (!value_.empty()) {
        ss << " [" << value_ << "]";
    }
    return ss.str();
}

std::string ASTNode::nodeTypeToString(NodeType type) {
    switch (type) {
        case NodeType::PROGRAM: return "Program";
        case NodeType::CLASS_DECL: return "ClassDecl";
        case NodeType::FUNCTION_DECL: return "FunctionDecl";
        case NodeType::MAIN_FUNCTION: return "MainFunction";
        case NodeType::VARIABLE_DECL: return "VariableDecl";
        case NodeType::ARRAY_DECL: return "ArrayDecl";
        case NodeType::FIELD_DECL: return "FieldDecl";
        case NodeType::CONSTRUCTOR: return "Constructor";
        case NodeType::DESTRUCTOR: return "Destructor";
        case NodeType::METHOD: return "Method";
        case NodeType::PARAMETER: return "Parameter";
        case NodeType::COMPOUND_STMT: return "CompoundStmt";
        case NodeType::IF_STMT: return "IfStmt";
        case NodeType::WHILE_STMT: return "WhileStmt";
        case NodeType::FOR_STMT: return "ForStmt";
        case NodeType::RETURN_STMT: return "ReturnStmt";
        case NodeType::BREAK_STMT: return "BreakStmt";
        case NodeType::CONTINUE_STMT: return "ContinueStmt";
        case NodeType::PRINT_STMT: return "PrintStmt";
        case NodeType::READ_STMT: return "ReadStmt";
        case NodeType::EXPR_STMT: return "ExprStmt";
        case NodeType::ASSIGN_EXPR: return "AssignExpr";
        case NodeType::BINARY_EXPR: return "BinaryExpr";
        case NodeType::UNARY_EXPR: return "UnaryExpr";
        case NodeType::CALL_EXPR: return "CallExpr";
        case NodeType::ARRAY_ACCESS_EXPR: return "ArrayAccessExpr";
        case NodeType::MEMBER_ACCESS_EXPR: return "MemberAccessExpr";
        case NodeType::IDENTIFIER: return "Identifier";
        case NodeType::LITERAL: return "Literal";
        case NodeType::TYPE_SPECIFIER: return "TypeSpecifier";
        case NodeType::OPERATOR: return "Operator";
        default: return "Unknown";
    }
}