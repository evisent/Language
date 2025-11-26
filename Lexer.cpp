#include "Lexer.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <iostream>
#include <iomanip>

bool Lexer::loadTokensFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open tokens file: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string token;
        int type;

        if (iss >> token >> type) {
            trie.insert(token, type, token);
        }
    }

    file.close();
    std::cout << "Loaded tokens from " << filename << std::endl;
    return true;
}

std::vector<std::pair<int, std::string>> Lexer::tokenize(const std::string& input) {
    std::vector<std::pair<int, std::string>> tokens;
    size_t pos = 0;

    while (pos < input.length()) {
        // Skip whitespace
        if (std::isspace(input[pos])) {
            ++pos;
            continue;
        }

        // Skip single-line comments
        if (pos + 1 < input.length() && input[pos] == '/' && input[pos + 1] == '/') {
            while (pos < input.length() && input[pos] != '\n') {
                ++pos;
            }
            continue;
        }

        // Skip multi-line comments
        if (pos + 1 < input.length() && input[pos] == '/' && input[pos + 1] == '*') {
            pos += 2;
            while (pos + 1 < input.length() && !(input[pos] == '*' && input[pos + 1] == '/')) {
                ++pos;
            }
            if (pos + 1 < input.length()) {
                pos += 2;
            }
            continue;
        }

        // Try to find reserved words and operators
        size_t start = pos;
        std::string tokenType;
        int foundType = trie.findLongestToken(input, pos, tokenType);

        if (foundType != -1) {
            std::string token = input.substr(start, pos - start);
            tokens.emplace_back(foundType, token);
            continue;
        }

        // Identifiers (тип 4)
        if (std::isalpha(input[pos]) || input[pos] == '_') {
            size_t start = pos;
            while (pos < input.length() &&
                (std::isalnum(input[pos]) || input[pos] == '_')) {
                ++pos;
            }
            std::string identifier = input.substr(start, pos - start);
            tokens.emplace_back(4, identifier);
            continue;
        }

        // Numbers (тип 5)
        if (std::isdigit(input[pos])) {
            size_t start = pos;
            bool hasDot = false;
            bool hasExponent = false;
            
            while (pos < input.length()) {
                if (std::isdigit(input[pos])) {
                    ++pos;
                } else if (input[pos] == '.' && !hasDot && !hasExponent) {
                    hasDot = true;
                    ++pos;
                } else if ((input[pos] == 'e' || input[pos] == 'E') && !hasExponent) {
                    hasExponent = true;
                    ++pos;
                    if (pos < input.length() && (input[pos] == '+' || input[pos] == '-')) {
                        ++pos;
                    }
                } else {
                    break;
                }
            }
            
            std::string number = input.substr(start, pos - start);
            tokens.emplace_back(5, number);
            continue;
        }

        // String literals (тип 6)
        if (input[pos] == '"') {
            size_t start = pos;
            ++pos;
            while (pos < input.length() && input[pos] != '"') {
                if (input[pos] == '\\' && pos + 1 < input.length()) {
                    ++pos; // Skip escape character
                }
                ++pos;
            }
            if (pos < input.length()) {
                ++pos; // Skip closing quote
            }
            std::string strLiteral = input.substr(start, pos - start);
            tokens.emplace_back(6, strLiteral);
            continue;
        }

        // Character literals (тип 7)
        if (input[pos] == '\'') {
            size_t start = pos;
            ++pos;
            if (pos < input.length() && input[pos] == '\\') {
                ++pos; // Skip escape character
            }
            if (pos < input.length()) {
                ++pos; // Skip character
            }
            if (pos < input.length() && input[pos] == '\'') {
                ++pos; // Skip closing quote
            }
            std::string charLiteral = input.substr(start, pos - start);
            tokens.emplace_back(7, charLiteral);
            continue;
        }

        // Unknown character (тип -1)
        tokens.emplace_back(-1, std::string(1, input[pos++]));
    }

    // Add EOF token (тип 0)
    tokens.emplace_back(0, "");
    return tokens;
}

std::string Lexer::getTokenTypeName(int type) {
    switch (type) {
        case 0: return "END_OF_FILE";
        case 1: return "KEYWORD";
        case 2: return "OPERATOR";
        case 3: return "PUNCTUATION";
        case 4: return "IDENTIFIER";
        case 5: return "NUMBER";
        case 6: return "STRING_LITERAL";
        case 7: return "CHAR_LITERAL";
        case -1: return "UNKNOWN";
        default: return "TYPE_" + std::to_string(type);
    }
}

void Lexer::printTokens(const std::vector<std::pair<int, std::string>>& tokens) {
    std::cout << std::left << std::setw(20) << "TYPE" << "VALUE" << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    
    for (const auto& token : tokens) {
        std::string typeName = getTokenTypeName(token.first);
        std::cout << std::left << std::setw(20) << typeName << "'" << token.second << "'" << std::endl;
    }
}