#pragma once
#include "symbol.h"
#include <unordered_map>
#include <memory>
#include <iostream>

class FunctionTable {
private:
    std::unordered_map<std::string, std::shared_ptr<Symbol>> functions;
    
public:
    bool addFunction(const std::string& name, const Type& returnType,
                    const std::vector<Parameter>& params, bool defined = false, int lineNumber = 0) {
        if (functions.find(name) != functions.end()) {
            return false;
        }
        
        auto sym = std::make_shared<Symbol>(SymbolCategory::FUNCTION, name, returnType, lineNumber);
        sym->parameters = params;
        sym->defined = defined;
        functions[name] = sym;
        return true;
    }
    
    std::shared_ptr<Symbol> lookupFunction(const std::string& name) {
        auto it = functions.find(name);
        return (it != functions.end()) ? it->second : nullptr;
    }
    
    bool setDefined(const std::string& name) {
        auto it = functions.find(name);
        if (it != functions.end()) {
            it->second->defined = true;
            return true;
        }
        return false;
    }
    
    void markAsUsed(const std::string& name) {
        auto it = functions.find(name);
        if (it != functions.end()) {
            it->second->isUsed = true;
        }
    }
    
    bool checkFunctionCall(const std::string& name, const std::vector<Type>& argTypes, std::string& errorMsg) {
        auto sym = lookupFunction(name);
        if (!sym) {
            errorMsg = "Function '" + name + "' not declared";
            return false;
        }
        
        if (sym->parameters.size() != argTypes.size()) {
            errorMsg = "Function '" + name + "' expects " + std::to_string(sym->parameters.size()) +
                      " arguments, but " + std::to_string(argTypes.size()) + " provided";
            return false;
        }
        
        for (size_t i = 0; i < argTypes.size(); ++i) {
            if (!(argTypes[i] == sym->parameters[i].type)) {
                errorMsg = "Argument " + std::to_string(i + 1) + " of function '" + name +
                          "' should be " + sym->parameters[i].type.toString() +
                          ", but " + argTypes[i].toString() + " provided";
                return false;
            }
        }
        
        return true;
    }
    
    void printFunctions() const {
        std::cout << "Declared functions:" << std::endl;
        for (const auto& [name, sym] : functions) {
            std::cout << "  " << name << " : " << sym->type.toString() << "(";
            for (size_t i = 0; i < sym->parameters.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << sym->parameters[i].type.toString();
            }
            std::cout << ")" << (sym->defined ? " [defined]" : " [declared]") << std::endl;
        }
    }
};