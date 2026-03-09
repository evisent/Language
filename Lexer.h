#ifndef LEXER_H
#define LEXER_H

#include "Trie.h"
#include <string>
#include <vector>
#include <unordered_map>

class Lexer {
private:
    TokenTrie trie;
    
public:
    bool loadTokensFromFile(const std::string& filename);
    std::vector<std::pair<int, std::string>> tokenize(const std::string& input);
    std::string getTokenTypeName(int type);
    void printTokens(const std::vector<std::pair<int, std::string>>& tokens);
};

#endif