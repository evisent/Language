#include "semantic.h"
#include <iomanip>

bool SemanticAnalyzer::isNumericType(const Type& type) {
    return type.kind == DataType::INT || type.kind == DataType::FLOAT;
}

bool SemanticAnalyzer::isIntegerType(const Type& type) {
    return type.kind == DataType::INT || type.kind == DataType::CHAR || type.kind == DataType::BOOL;
}

bool SemanticAnalyzer::canImplicitlyConvert(const Type& from, const Type& to) {
    if (from.kind == to.kind) return true;
    
    if (from.kind == DataType::INT && to.kind == DataType::FLOAT) return true;
    
    if (from.kind == DataType::CHAR && to.kind == DataType::INT) return true;
    
    if (from.kind == DataType::BOOL && to.kind == DataType::INT) return true;
    
    return false;
}

Type SemanticAnalyzer::getCommonType(const Type& t1, const Type& t2) {
    if (t1.kind == DataType::FLOAT || t2.kind == DataType::FLOAT) {
        return Type(DataType::FLOAT);
    }
    if (t1.kind == DataType::INT || t2.kind == DataType::INT) {
        return Type(DataType::INT);
    }
    return t1;
}

void SemanticAnalyzer::addError(const std::string& msg) {
    // Проверяем, не было ли уже такой ошибки
    if (errorSet.find(msg) == errorSet.end()) {
        errorSet.insert(msg);
        errors.push_back("Semantic error: " + msg);
    }
}

void SemanticAnalyzer::addWarning(const std::string& msg) {
    if (warningSet.find(msg) == warningSet.end()) {
        warningSet.insert(msg);
        warnings.push_back("Warning: " + msg);
    }
}


void SemanticAnalyzer::enterScope() {
    scopeTable.enterScope();
}

void SemanticAnalyzer::exitScope() {
    scopeTable.exitScope();
}

bool SemanticAnalyzer::declareVariable(const std::string& name, const Type& type,
                                      bool isInitialized, int lineNumber) {
    if (scopeTable.lookupCurrentScope(name)) {
        addError("Variable '" + name + "' already declared in current scope (line " + 
                std::to_string(lineNumber) + ")");
        return false;
    }
    
    auto sym = std::make_shared<Symbol>(SymbolCategory::VARIABLE, name, type, lineNumber);
    sym->isInitialized = isInitialized;
    
    if (!scopeTable.addSymbol(sym)) {
        addError("Failed to declare variable '" + name + "'");
        return false;
    }
    
    return true;
}

bool SemanticAnalyzer::declareFunction(const std::string& name, const Type& returnType,
                                      const std::vector<Parameter>& params, int lineNumber) {
    if (functionTable.lookupFunction(name)) {
        addError("Function '" + name + "' already declared (line " + 
                std::to_string(lineNumber) + ")");
        return false;
    }
    
    return functionTable.addFunction(name, returnType, params, false, lineNumber);
}

bool SemanticAnalyzer::declareClass(const std::string& name, int lineNumber) {
    if (scopeTable.lookupSymbol(name)) {
        addError("Class '" + name + "' already declared (line " + 
                std::to_string(lineNumber) + ")");
        return false;
    }
    
    auto sym = std::make_shared<Symbol>(SymbolCategory::CLASS, name, Type(DataType::CLASS), lineNumber);
    
    if (!scopeTable.addSymbol(sym)) {
        addError("Failed to declare class '" + name + "'");
        return false;
    }
    
    return true;
}

std::shared_ptr<Symbol> SemanticAnalyzer::useVariable(const std::string& name, bool isAssigned) {
    auto sym = scopeTable.lookupSymbol(name);
    
    if (!sym) {
        addError("Variable '" + name + "' not declared");
        return nullptr;
    }
    
    if (sym->category != SymbolCategory::VARIABLE && 
        sym->category != SymbolCategory::PARAMETER &&
        sym->category != SymbolCategory::FIELD) {
        addError("'" + name + "' is not a variable");
        return nullptr;
    }
    
    if (isAssigned) {
        sym->isInitialized = true;
    } else {
        if (!sym->isInitialized && sym->category != SymbolCategory::PARAMETER) {
            addWarning("Variable '" + name + "' may be used uninitialized");
        }
    }
    
    sym->isUsed = true;
    return sym;
}

std::shared_ptr<Symbol> SemanticAnalyzer::useFunction(const std::string& name,
                                                      const std::vector<Type>& argTypes) {
    std::string errorMsg;
    if (!functionTable.checkFunctionCall(name, argTypes, errorMsg)) {
        addError(errorMsg);
        return nullptr;
    }
    
    auto sym = functionTable.lookupFunction(name);
    if (sym) {
        sym->isUsed = true;
    }
    
    return sym;
}

void SemanticAnalyzer::setCurrentFunction(const std::string& funcName) {
    if (funcName.empty()) {
        if (!currentFunction.empty()) currentFunction.pop();
        return;
    }
    
    auto sym = functionTable.lookupFunction(funcName);
    if (sym) {
        currentFunction.push(sym);
    } else {
        addError("Internal error: current function not found");
    }
}

void SemanticAnalyzer::checkReturnType(bool hasExpr, const Type& exprType) {
    if (currentFunction.empty()) {
        addError("'return' outside function");
        return;
    }
    
    auto funcType = currentFunction.top()->type;
    
    if (funcType.kind == DataType::VOID && hasExpr) {
        addError("Void function cannot return a value");
    } else if (funcType.kind != DataType::VOID && !hasExpr) {
        addError("Non-void function must return a value");
    } else if (hasExpr && !(exprType == funcType) && !canImplicitlyConvert(exprType, funcType)) {
        addError("Cannot convert return type from " + exprType.toString() +
                " to " + funcType.toString());
    }
}

void SemanticAnalyzer::enterClass(const std::string& className) {
    currentClass.push(className);
}

void SemanticAnalyzer::exitClass() {
    if (!currentClass.empty()) {
        currentClass.pop();
    }
}

bool SemanticAnalyzer::checkTypes(const Type& expected, const Type& actual, const std::string& context) {
    if (expected == actual || canImplicitlyConvert(actual, expected)) {
        return true;
    }
    
    addError("Type mismatch in " + context + ": expected " + expected.toString() +
            ", but got " + actual.toString());
    return false;
}

bool SemanticAnalyzer::checkBinaryOperation(const std::string& op, const Type& left, const Type& right) {
    if (op == "+" || op == "-" || op == "*" || op == "/") {
        if (!isNumericType(left) || !isNumericType(right)) {
            addError("Operator '" + op + "' requires numeric operands, but got " +
                    left.toString() + " and " + right.toString());
            return false;
        }
        return true;
    }
    
    if (op == "%") {
        if (!isIntegerType(left) || !isIntegerType(right)) {
            addError("Operator '%' requires integer operands, but got " +
                    left.toString() + " and " + right.toString());
            return false;
        }
        return true;
    }
    
    if (op == "<" || op == ">" || op == "<=" || op == ">=") {
        if (!isNumericType(left) || !isNumericType(right)) {
            addError("Comparison operator '" + op + "' requires numeric operands, but got " +
                    left.toString() + " and " + right.toString());
            return false;
        }
        return true;
    }
    
    if (op == "==" || op == "!=") {
        if (left.kind != right.kind && !canImplicitlyConvert(left, right) && !canImplicitlyConvert(right, left)) {
            addError("Cannot compare " + left.toString() + " and " + right.toString());
            return false;
        }
        return true;
    }
    
    if (op == "&&" || op == "||") {
        if (left.kind != DataType::BOOL || right.kind != DataType::BOOL) {
            addError("Logical operator '" + op + "' requires boolean operands, but got " +
                    left.toString() + " and " + right.toString());
            return false;
        }
        return true;
    }
    
    return true;
}

Type SemanticAnalyzer::getResultType(const std::string& op, const Type& left, const Type& right) {
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
        if (op == "%") return Type(DataType::INT);
        return getCommonType(left, right);
    }
    
    if (op == "<" || op == ">" || op == "<=" || op == ">=" || 
        op == "==" || op == "!=" || op == "&&" || op == "||") {
        return Type(DataType::BOOL);
    }
    
    return Type(DataType::UNKNOWN);
}

void SemanticAnalyzer::printResults(std::ostream& os) const {
    if (!errors.empty()) {
        os << "\n=== Semantic Errors ===\n";
        for (const auto& e : errors) {
            os << "  " << e << std::endl;
        }
    }
    
    if (!warnings.empty()) {
        os << "\n=== Semantic Warnings ===\n";
        for (const auto& w : warnings) {
            os << "  " << w << std::endl;
        }
    }
    
    if (errors.empty() && warnings.empty()) {
        os << "\n Semantic analysis completed with no errors or warnings" << std::endl;
    } else {
        os << "\n Semantic analysis found " << errors.size() << " errors and " 
           << warnings.size() << " warnings" << std::endl;
    }
}

void SemanticAnalyzer::clear() {
    errors.clear();
    warnings.clear();
    errorSet.clear();
    warningSet.clear();
    while (!currentFunction.empty()) currentFunction.pop();
    while (!currentClass.empty()) currentClass.pop();
    loopDepth = 0;
}