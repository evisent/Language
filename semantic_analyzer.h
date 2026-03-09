#pragma once
#include "scope_table.h"
#include "function_table.h"
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stack>
#include <memory>

class SemanticAnalyzer {
private:
    ScopeTable scopeTable;
    FunctionTable functionTable;
    
    std::stack<std::shared_ptr<Symbol>> currentFunction;
    std::stack<std::string> currentClass;
    int loopDepth = 0;
    
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    
    // Вспомогательные функции (приватные)
    bool isNumericType(const Type& type);
    bool isIntegerType(const Type& type);
    bool canImplicitlyConvert(const Type& from, const Type& to);
    Type getCommonType(const Type& t1, const Type& t2);
    
public:
    SemanticAnalyzer() {
        enterScope(); // глобальная область видимости
    }
    
    // Управление областями видимости
    void enterScope();
    void exitScope();
    
    // Объявление символов
    bool declareVariable(const std::string& name, const Type& type, 
                         bool isInitialized = false, int lineNumber = 0);
    bool declareFunction(const std::string& name, const Type& returnType,
                        const std::vector<Parameter>& params, int lineNumber = 0);
    bool declareClass(const std::string& name, int lineNumber = 0);
    
    // Использование символов
    std::shared_ptr<Symbol> useVariable(const std::string& name, bool isAssigned = false);
    std::shared_ptr<Symbol> useFunction(const std::string& name, 
                                        const std::vector<Type>& argTypes);
    
    // Управление функциями
    void setCurrentFunction(const std::string& funcName);
    void checkReturnType(bool hasExpr, const Type& exprType = Type(DataType::VOID));
    
    // Управление циклами
    void enterLoop() { loopDepth++; }
    void exitLoop() { if (loopDepth > 0) loopDepth--; }
    bool isInLoop() const { return loopDepth > 0; }
    
    // Управление классами
    void enterClass(const std::string& className);
    void exitClass();
    
    // Проверка типов
    bool checkTypes(const Type& expected, const Type& actual, const std::string& context);
    bool checkBinaryOperation(const std::string& op, const Type& left, const Type& right);
    Type getResultType(const std::string& op, const Type& left, const Type& right);
    
    // ПУБЛИЧНЫЕ МЕТОДЫ ДЛЯ ДОБАВЛЕНИЯ ОШИБОК И ПРЕДУПРЕЖДЕНИЙ
    void addError(const std::string& msg);
    void addWarning(const std::string& msg);
    
    // Результаты анализа
    bool hasErrors() const { return !errors.empty(); }
    bool hasWarnings() const { return !warnings.empty(); }
    const std::vector<std::string>& getErrors() const { return errors; }
    const std::vector<std::string>& getWarnings() const { return warnings; }
    void printResults(std::ostream& os = std::cout) const;
    void clear();
    
    // Отладка
    void printCurrentScope() const { scopeTable.printScope(); }
    void printFunctions() const { functionTable.printFunctions(); }
};