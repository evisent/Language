#include "ASTNode.h"
#include <sstream>

#include "ASTNode.h"
#include <sstream>
#include <iostream>

int ASTNode::labelCounter = 0;

std::string ASTNode::generateLabel() {
    return "L" + std::to_string(labelCounter++);
}

std::string ASTNode::toPOLIZ() const {
    std::stringstream ss;
    labelCounter = 0;
    generatePOLIZForNode(ss);
    return ss.str();
}

void ASTNode::generatePOLIZForNode(std::stringstream& ss, bool inExpression) const {
    switch (type_) {
        case NodeType::PROGRAM:
            for (const auto& child : children_) {
                child->generatePOLIZForNode(ss);
            }
            break;
            
        case NodeType::MAIN_FUNCTION:
            ss << "MAIN:\n";
            for (const auto& child : children_) {
                child->generatePOLIZForNode(ss);
            }
            ss << "HLT\n";
            break;
            
        case NodeType::FUNCTION_DECL:
            ss << "FUNC " << value_ << ":\n";
            for (const auto& child : children_) {
                child->generatePOLIZForNode(ss);
            }
            ss << "RET\n";
            break;
            
        case NodeType::COMPOUND_STMT:
            for (const auto& child : children_) {
                child->generatePOLIZForNode(ss);
            }
            break;
            
        case NodeType::VARIABLE_DECL:
            if (children_.size() >= 2) {
                ss << "DECL " << children_[0]->getValue() << " " << value_ << "\n";
                if (children_.size() > 2) {
                    children_[2]->generatePOLIZForNode(ss, true);
                    ss << "ST " << children_[0]->getValue() << "\n";
                }
            }
            break;
            
        case NodeType::ARRAY_DECL:
            if (children_.size() >= 3) {
                ss << "ADECL " << children_[0]->getValue() << " " << value_;
                if (!children_[1]->getValue().empty()) {
                    ss << "[" << children_[1]->getValue() << "]";
                }
                ss << "\n";
            }
            break;
            
        case NodeType::ASSIGN_EXPR:
            if (children_.size() >= 2) {
                children_[1]->generatePOLIZForNode(ss, true);
                ss << "ST " << children_[0]->getValue() << "\n";
            }
            break;
            
        case NodeType::BINARY_EXPR:
            if (children_.size() >= 3) {
                children_[0]->generatePOLIZForNode(ss, true);
                children_[1]->generatePOLIZForNode(ss, true);
                ss << children_[2]->getValue() << "\n";
            }
            break;
            
        case NodeType::UNARY_EXPR:
            if (children_.size() >= 2) {
                children_[0]->generatePOLIZForNode(ss, true);
                ss << children_[1]->getValue() << "\n";
            }
            break;
            
        case NodeType::IDENTIFIER:
            ss << "LD " << value_ << "\n";
            break;
            
        case NodeType::LITERAL:
            ss << "PUSH " << value_ << "\n";
            break;
            
        case NodeType::IF_STMT:
            if (children_.size() >= 2) {
                std::string elseLabel = generateLabel();
                std::string endLabel = generateLabel();
                children_[0]->generatePOLIZForNode(ss, true);
                ss << "JZ " << elseLabel << "\n";
                children_[1]->generatePOLIZForNode(ss);
                ss << "JMP " << endLabel << "\n";
                ss << elseLabel << ":\n";
                if (children_.size() >= 3) {
                    children_[2]->generatePOLIZForNode(ss);
                }
                ss << endLabel << ":\n";
            }
            break;
            
        case NodeType::WHILE_STMT:
            if (children_.size() >= 2) {
                std::string startLabel = generateLabel();
                std::string endLabel = generateLabel();
                
                ss << startLabel << ":\n";
                children_[0]->generatePOLIZForNode(ss, true);
                ss << "JZ " << endLabel << "\n";
                children_[1]->generatePOLIZForNode(ss);
                ss << "JMP " << startLabel << "\n";
                ss << endLabel << ":\n";
            }
            break;
            
        case NodeType::FOR_STMT:
            if (children_.size() >= 4) {
                std::string startLabel = generateLabel();
                std::string endLabel = generateLabel();
                std::string incLabel = generateLabel();
                
                // Инициализация
                if (children_[0]) {
                    children_[0]->generatePOLIZForNode(ss);
                }
                
                ss << startLabel << ":\n";
                // Условие
                if (children_[1]) {
                    children_[1]->generatePOLIZForNode(ss, true);
                    ss << "JZ " << endLabel << "\n";
                }
                children_[3]->generatePOLIZForNode(ss);
                
                ss << incLabel << ":\n";
                if (children_[2]) {
                    children_[2]->generatePOLIZForNode(ss);
                }
                ss << "JMP " << startLabel << "\n";
                
                ss << endLabel << ":\n";
            }
            break;
            
        case NodeType::RETURN_STMT:
            if (!children_.empty()) {
                children_[0]->generatePOLIZForNode(ss, true);
            }
            ss << "RET\n";
            break;
            
        case NodeType::PRINT_STMT:
            if (!children_.empty()) {
                children_[0]->generatePOLIZForNode(ss, true);
                ss << "PRINT\n";
            }
            break;
            
        case NodeType::READ_STMT:
            if (!children_.empty()) {
                ss << "READ " << children_[0]->getValue() << "\n";
            }
            break;
            
        case NodeType::CALL_EXPR:
            for (const auto& child : children_) {
                child->generatePOLIZForNode(ss, true);
            }
            ss << "CALL " << value_ << "\n";
            break;
            
        case NodeType::ARRAY_ACCESS_EXPR:
            if (children_.size() >= 2) {
                children_[1]->generatePOLIZForNode(ss, true);
                ss << "LDA " << children_[0]->getValue() << "\n";
            }
            break;
            
        case NodeType::BREAK_STMT:
            ss << "BREAK\n";
            break;
            
        case NodeType::CONTINUE_STMT:
            ss << "CONTINUE\n";
            break;
            
        default:
            for (const auto& child : children_) {
                child->generatePOLIZForNode(ss);
            }
            break;
    }
}

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