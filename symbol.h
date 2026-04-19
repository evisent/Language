#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

enum class DataType {
    VOID,
    INT,
    FLOAT,
    CHAR,
    BOOL,
    CLASS,
    UNKNOWN
};

enum class SymbolCategory {
    VARIABLE,
    FUNCTION,
    METHOD,
    CONSTRUCTOR,
    DESTRUCTOR,
    CLASS,
    PARAMETER,
    FIELD
};

struct Type {
    DataType kind;
    std::shared_ptr<Type> elementType;
    std::string className;              
    
    Type(DataType k = DataType::UNKNOWN) : kind(k), elementType(nullptr) {}
    Type(DataType k, std::shared_ptr<Type> elem) : kind(k), elementType(elem) {}
    
    bool operator==(const Type& other) const {
        if (kind != other.kind) return false;
        if (kind == DataType::CLASS) return className == other.className;
        return true;
    }
    
    bool operator!=(const Type& other) const {
        return !(*this == other);
    }
    
    std::string toString() const {
        switch (kind) {
            case DataType::VOID:   return "void";
            case DataType::INT:    return "int";
            case DataType::FLOAT:  return "float";
            case DataType::CHAR:   return "char";
            case DataType::BOOL:   return "bool";
            case DataType::CLASS:  return "class " + className;
            case DataType::UNKNOWN: return "unknown";
            default: return "unknown";
        }
    }
};

struct Parameter {
    std::string name;
    Type type;
    
    Parameter() {}
    Parameter(const std::string& n, const Type& t) : name(n), type(t) {}
};

struct Symbol {
    SymbolCategory category;
    std::string name;
    Type type;
    
    std::vector<Parameter> parameters;
    bool defined = false;
    
    bool isInitialized = false;
    bool isUsed = false;
    int lineNumber = 0;
    
    std::unordered_map<std::string, std::shared_ptr<Symbol>> fields;
    std::unordered_map<std::string, std::shared_ptr<Symbol>> methods;
    
    Symbol() : category(SymbolCategory::VARIABLE), type(DataType::UNKNOWN) {}
    
    Symbol(SymbolCategory cat, const std::string& n, const Type& t, int line = 0)
        : category(cat), name(n), type(t), lineNumber(line) {}
};