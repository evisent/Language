#pragma once
#include "symbol.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>

class ScopeTable {
private:
    using Scope = std::unordered_map<std::string, std::shared_ptr<Symbol>>;
    std::vector<Scope> scopes; 
    
public:
    void enterScope() {
        scopes.emplace_back();
    }
    
    void exitScope() {
        if (!scopes.empty()) {
            for (const auto& [name, sym] : scopes.back()) {
                if (sym->category == SymbolCategory::VARIABLE && !sym->isUsed) {
                    std::cout << "Warning: Variable '" << name << "' declared but never used (line " 
                              << sym->lineNumber << ")" << std::endl;
                }
            }
            scopes.pop_back();
        }
    }
    
    bool addSymbol(const std::shared_ptr<Symbol>& symbol) {
        if (scopes.empty()) {
            scopes.emplace_back();
        }
        
        auto& currentScope = scopes.back();
        if (currentScope.find(symbol->name) != currentScope.end()) {
            return false;  
        }
        
        currentScope[symbol->name] = symbol;
        return true;
    }
    
    std::shared_ptr<Symbol> lookupSymbol(const std::string& name) {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return found->second;
            }
        }
        return nullptr;
    }
    
    std::shared_ptr<Symbol> lookupCurrentScope(const std::string& name) {
        if (scopes.empty()) return nullptr;
        auto& current = scopes.back();
        auto it = current.find(name);
        return (it != current.end()) ? it->second : nullptr;
    }
    
    void markAsUsed(const std::string& name) {
        auto sym = lookupSymbol(name);
        if (sym) {
            sym->isUsed = true;
        }
    }
    
    bool setInitialized(const std::string& name) {
        auto sym = lookupSymbol(name);
        if (sym) {
            sym->isInitialized = true;
            return true;
        }
        return false;
    }
    
    int currentDepth() const {
        return scopes.size();
    }
    
    void printScope() const {
        if (scopes.empty()) return;
        std::cout << "Current scope variables:" << std::endl;
        for (const auto& [name, sym] : scopes.back()) {
            std::cout << "  " << name << " : " << sym->type.toString() 
                      << (sym->isInitialized ? " (initialized)" : " (uninitialized)")
                      << (sym->isUsed ? " (used)" : " (unused)") << std::endl;
        }
    }
};