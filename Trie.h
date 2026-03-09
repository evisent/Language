#ifndef TRIE_H
#define TRIE_H

#include <string>
#include <unordered_map>
#include <memory>

struct TrieNode {
    std::unordered_map<char, std::shared_ptr<TrieNode>> children;
    int token_index;
    std::string token_type;
    bool is_end;
    
    TrieNode() : is_end(false), token_index(-1) {}
};

class TokenTrie {
private:
    std::shared_ptr<TrieNode> root;
    
public:
    TokenTrie();
    void insert(const std::string& token, int index, const std::string& type);
    int findLongestToken(const std::string& input, size_t& pos, std::string& tokenType) const;
};

#endif