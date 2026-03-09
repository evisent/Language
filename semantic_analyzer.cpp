#include "semantic_analyzer.h"
#include <iomanip>

// Вспомогательные функции
bool SemanticAnalyzer::isNumericType(const Type& type) {
    return type.kind == DataType::INT || type.kind == DataType::FLOAT;
}

bool SemanticAnalyzer::isIntegerType(const Type& type) {
    return type.kind == DataType::INT || type.kind == DataType::CHAR || type.kind == DataType::BOOL;
}

bool SemanticAnalyzer::canImplicitlyConvert(const Type& from, const Type& to) {
    if (from.kind == to.kind) return true;
    
    // int -> float
    if (from.kind == DataType::INT && to.kind == DataType::FLOAT) return true;
    
    // char -> int
    if (from.kind == DataType::CHAR && to.kind == DataType::INT) return true;
    
    // bool -> int
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

// Публичные методы для добавления ошибок и предупреждений
void SemanticAnalyzer::addError(const std::string& msg) {
    errors.push_back("Semantic error: " + msg);
}

void SemanticAnalyzer::addWarning(const std::string& msg) {
    warnings.push_back("Warning: " + msg);
}

// Управление областями видимости
void SemanticAnalyzer::enterScope() {
    scopeTable.enterScope();
}

void SemanticAnalyzer::exitScope() {
    scopeTable.exitScope();
}

// Объявление переменной
bool SemanticAnalyzer::declareVariable(const std::string& name, const Type& type,
                                      bool isInitialized, int lineNumber) {
    // Проверяем, не объявлена ли уже переменная в текущей области
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

// Объявление функции
bool SemanticAnalyzer::declareFunction(const std::string& name, const Type& returnType,
                                      const std::vector<Parameter>& params, int lineNumber) {
    if (functionTable.lookupFunction(name)) {
        addError("Function '" + name + "' already declared (line " + 
                std::to_string(lineNumber) + ")");
        return false;
    }
    
    return functionTable.addFunction(name, returnType, params, false, lineNumber);
}

// Объявление класса
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

// Использование переменной
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

// Использование функции
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

// Управление текущей функцией
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

// Проверка return
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

// Управление классами
void SemanticAnalyzer::enterClass(const std::string& className) {
    currentClass.push(className);
}

void SemanticAnalyzer::exitClass() {
    if (!currentClass.empty()) {
        currentClass.pop();
    }
}

// Проверка типов
bool SemanticAnalyzer::checkTypes(const Type& expected, const Type& actual, const std::string& context) {
    if (expected == actual || canImplicitlyConvert(actual, expected)) {
        return true;
    }
    
    addError("Type mismatch in " + context + ": expected " + expected.toString() +
            ", but got " + actual.toString());
    return false;
}

// Проверка бинарных операций
bool SemanticAnalyzer::checkBinaryOperation(const std::string& op, const Type& left, const Type& right) {
    // Арифметические операции
    if (op == "+" || op == "-" || op == "*" || op == "/") {
        if (!isNumericType(left) || !isNumericType(right)) {
            addError("Operator '" + op + "' requires numeric operands, but got " +
                    left.toString() + " and " + right.toString());
            return false;
        }
        return true;
    }
    
    // Операция остатка
    if (op == "%") {
        if (!isIntegerType(left) || !isIntegerType(right)) {
            addError("Operator '%' requires integer operands, but got " +
                    left.toString() + " and " + right.toString());
            return false;
        }
        return true;
    }
    
    // Операции сравнения
    if (op == "<" || op == ">" || op == "<=" || op == ">=") {
        if (!isNumericType(left) || !isNumericType(right)) {
            addError("Comparison operator '" + op + "' requires numeric operands, but got " +
                    left.toString() + " and " + right.toString());
            return false;
        }
        return true;
    }
    
    // Операции равенства
    if (op == "==" || op == "!=") {
        if (left.kind != right.kind && !canImplicitlyConvert(left, right) && !canImplicitlyConvert(right, left)) {
            addError("Cannot compare " + left.toString() + " and " + right.toString());
            return false;
        }
        return true;
    }
    
    // Логические операции
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

// Получение результирующего типа операции
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

// Печать результатов
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
    while (!currentFunction.empty()) currentFunction.pop();
    while (!currentClass.empty()) currentClass.pop();
    loopDepth = 0;
    
    // Очищаем таблицы (простой способ - создать новые)
    // В реальном коде нужно добавить методы очистки в ScopeTable и FunctionTable
}