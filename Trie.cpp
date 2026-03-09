#include "trie.h"

TokenTrie::TokenTrie() : root(std::make_shared<TrieNode>()) {}

void TokenTrie::insert(const std::string& token, int index, const std::string& type) {
    auto node = root;
    for (char c : token) {
        if (node->children.find(c) == node->children.end()) {
            node->children[c] = std::make_shared<TrieNode>();
        }
        node = node->children[c];
    }
    node->is_end = true;
    node->token_index = index;
    node->token_type = type;
}

int TokenTrie::findLongestToken(const std::string& input, size_t& pos, std::string& tokenType) const {
    auto node = root;
    size_t start = pos;
    size_t last_match_end = start;
    int last_match_index = -1;
    std::string last_match_type;

    while (pos < input.length() && node->children.find(input[pos]) != node->children.end()) {
        node = node->children[input[pos]];
        pos++;
        
        if (node->is_end) {
            last_match_end = pos;
            last_match_index = node->token_index;
            last_match_type = node->token_type;
        }
    }

    if (last_match_index != -1) {
        pos = last_match_end;
        tokenType = last_match_type;
        return last_match_index;
    }

    pos = start;
    return -1;
}