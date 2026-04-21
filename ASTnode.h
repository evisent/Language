#ifndef ASTNODE_H
#define ASTNODE_H

#include <string>
#include <vector>
#include <memory>

enum class NodeType {
    PROGRAM,
    CLASS_DECL,
    FUNCTION_DECL,
    MAIN_FUNCTION,
    VARIABLE_DECL,
    ARRAY_DECL,
    FIELD_DECL,
    CONSTRUCTOR,
    DESTRUCTOR,
    METHOD,
    PARAMETER,
    COMPOUND_STMT,
    IF_STMT,
    WHILE_STMT,
    FOR_STMT,
    RETURN_STMT,
    BREAK_STMT,
    CONTINUE_STMT,
    PRINT_STMT,
    READ_STMT,
    EXPR_STMT,
    ASSIGN_EXPR,
    BINARY_EXPR,
    UNARY_EXPR,
    CALL_EXPR,
    ARRAY_ACCESS_EXPR,
    MEMBER_ACCESS_EXPR,
    IDENTIFIER,
    LITERAL,
    TYPE_SPECIFIER,
    OPERATOR
};

class ASTNode {
public:
    ASTNode(NodeType type, const std::string& value = "");
    
    NodeType getType() const;
    std::string getValue() const;
    const std::vector<std::shared_ptr<ASTNode>>& getChildren() const;
    
    void addChild(const std::shared_ptr<ASTNode>& child);
    void addChildFront(const std::shared_ptr<ASTNode>& child);
    void setValue(const std::string& value);
    
    std::string toString() const;
    static std::string nodeTypeToString(NodeType type);

    std::string toPOLIZ() const;
    
private:
    NodeType type_;
    std::string value_;
    std::vector<std::shared_ptr<ASTNode>> children_;
    void generatePOLIZForNode(std::stringstream& ss, bool inExpression = false) const;
    static int labelCounter;
    static std::string generateLabel();
};

#endif