#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <algorithm>

enum class TokenType {
    // Keywords
    CLASS, CONSTRUCTOR, DESTRUCTOR, INT, CHAR, BOOL, FLOAT,
    IF, ELIF, ELSE, WHILE, FOR, RETURN, BREAK, CONTINUE,
    PRINT, READ, NEW, DELETE, SIZEOF, THIS,
    TRUE, FALSE, NULLPTR,
    
    // Operators
    ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, MUL_ASSIGN, DIV_ASSIGN, MOD_ASSIGN,
    SHL_ASSIGN, SHR_ASSIGN, AND_ASSIGN, XOR_ASSIGN, OR_ASSIGN,
    PLUS, MINUS, MUL, DIV, MOD,
    SHL, SHR, AND, OR, XOR,
    EQ, NE, LT, GT, LE, GE,
    LOGICAL_AND, LOGICAL_OR, LOGICAL_NOT,
    BITWISE_AND, BITWISE_OR, BITWISE_NOT, BITWISE_XOR,
    INCREMENT, DECREMENT,
    ARROW, DOT,
    
    // Punctuation
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    COMMA, SEMICOLON, COLON,
    
    // Literals
    IDENTIFIER, INTEGER_LITERAL, FLOAT_LITERAL, CHAR_LITERAL,
    STRING_LITERAL, BOOL_LITERAL,
    
    // Special
    END_OF_FILE, UNKNOWN
};

struct Token {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;
    
    Token(TokenType t, const std::string& v, size_t l, size_t c)
        : type(t), value(v), line(l), column(c) {}
};

class Lexer {
private:
    std::string source;
    size_t position;
    size_t line;
    size_t column;
    char current_char;
    
    static const std::unordered_map<std::string, TokenType> keywords;
    static const std::unordered_map<std::string, TokenType> operators;
    
public:
    Lexer(const std::string& input) 
        : source(input), position(0), line(1), column(1), current_char(0) {
        if (!source.empty()) {
            current_char = source[0];
        }
    }
    
    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        
        while (current_char != 0) {
            if (std::isspace(current_char)) {
                skipWhitespace();
                continue;
            }
            
            if (current_char == '/') {
                if (peek() == '/') {
                    skipLineComment();
                    continue;
                } else if (peek() == '*') {
                    skipBlockComment();
                    continue;
                }
            }
            
            if (current_char == '\'' || current_char == '"') {
                tokens.push_back(readStringOrCharLiteral());
                continue;
            }
            
            if (std::isdigit(current_char)) {
                tokens.push_back(readNumber());
                continue;
            }
            
            if (std::isalpha(current_char) || current_char == '_') {
                tokens.push_back(readIdentifierOrKeyword());
                continue;
            }
            
            tokens.push_back(readOperatorOrPunctuation());
        }
        
        tokens.emplace_back(TokenType::END_OF_FILE, "", line, column);
        return tokens;
    }
    
private:
    void advance() {
        if (current_char == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        
        position++;
        if (position >= source.length()) {
            current_char = 0;
        } else {
            current_char = source[position];
        }
    }
    
    char peek(size_t ahead = 1) const {
        if (position + ahead >= source.length()) {
            return 0;
        }
        return source[position + ahead];
    }
    
    void skipWhitespace() {
        while (current_char != 0 && std::isspace(current_char)) {
            advance();
        }
    }
    
    void skipLineComment() {
        while (current_char != 0 && current_char != '\n') {
            advance();
        }
        if (current_char == '\n') {
            advance();
        }
    }
    
    void skipBlockComment() {
        advance(); // skip '/'
        advance(); // skip '*'
        
        while (current_char != 0) {
            if (current_char == '*' && peek() == '/') {
                advance(); // skip '*'
                advance(); // skip '/'
                break;
            }
            advance();
        }
    }
    
    Token readStringOrCharLiteral() {
        char delimiter = current_char;
        size_t start_line = line;
        size_t start_column = column;
        std::string value;
        
        advance(); // skip opening quote
        
        while (current_char != 0 && current_char != delimiter) {
            if (current_char == '\\') {
                advance(); // skip backslash
                if (current_char != 0) {
                    value += escapeSequence(current_char);
                }
            } else {
                value += current_char;
            }
            advance();
        }
        
        if (current_char == delimiter) {
            advance(); // skip closing quote
        }
        
        if (delimiter == '\'') {
            return Token(TokenType::CHAR_LITERAL, value, start_line, start_column);
        } else {
            return Token(TokenType::STRING_LITERAL, value, start_line, start_column);
        }
    }
    
    char escapeSequence(char c) {
        switch (c) {
            case 'n': return '\n';
            case 't': return '\t';
            case 'r': return '\r';
            case '0': return '\0';
            case '\\': return '\\';
            case '\'': return '\'';
            case '"': return '"';
            default: return c;
        }
    }
    
    Token readNumber() {
        size_t start_line = line;
        size_t start_column = column;
        std::string value;
        bool is_float = false;
        
        // Read integer part
        while (current_char != 0 && (std::isdigit(current_char) || current_char == '.')) {
            if (current_char == '.') {
                if (is_float) break; // multiple dots not allowed
                is_float = true;
            }
            value += current_char;
            advance();
        }
        
        // Handle scientific notation
        if (current_char == 'e' || current_char == 'E') {
            is_float = true;
            value += current_char;
            advance();
            
            if (current_char == '+' || current_char == '-') {
                value += current_char;
                advance();
            }
            
            while (current_char != 0 && std::isdigit(current_char)) {
                value += current_char;
                advance();
            }
        }
        
        if (is_float) {
            return Token(TokenType::FLOAT_LITERAL, value, start_line, start_column);
        } else {
            return Token(TokenType::INTEGER_LITERAL, value, start_line, start_column);
        }
    }
    
    Token readIdentifierOrKeyword() {
        size_t start_line = line;
        size_t start_column = column;
        std::string value;
        
        while (current_char != 0 && 
               (std::isalnum(current_char) || current_char == '_')) {
            value += current_char;
            advance();
        }
        
        // Check if it's a keyword
        auto keyword_it = keywords.find(value);
        if (keyword_it != keywords.end()) {
            return Token(keyword_it->second, value, start_line, start_column);
        }
        
        // Check if it's a boolean literal
        if (value == "true" || value == "false") {
            return Token(TokenType::BOOL_LITERAL, value, start_line, start_column);
        }
        
        return Token(TokenType::IDENTIFIER, value, start_line, start_column);
    }
    
    Token readOperatorOrPunctuation() {
        size_t start_line = line;
        size_t start_column = column;
        std::string value;
        
        value += current_char;
        
        // Handle multi-character operators
        std::string two_char = value + (peek() != 0 ? std::string(1, peek()) : "");
        std::string three_char = two_char + (peek(2) != 0 ? std::string(1, peek(2)) : "");
        
        // Check for three-character operators first
        if (operators.find(three_char) != operators.end()) {
            value = three_char;
            advance(); advance(); advance();
            return Token(operators.at(three_char), value, start_line, start_column);
        }
        
        // Check for two-character operators
        if (operators.find(two_char) != operators.end()) {
            value = two_char;
            advance(); advance();
            return Token(operators.at(two_char), value, start_line, start_column);
        }
        
        // Single character operators and punctuation
        if (operators.find(value) != operators.end()) {
            advance();
            return Token(operators.at(value), value, start_line, start_column);
        }
        
        // Unknown character
        Token token(TokenType::UNKNOWN, value, start_line, start_column);
        advance();
        return token;
    }
};

// Initialize static keyword map
const std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"class", TokenType::CLASS},
    {"constructor", TokenType::CONSTRUCTOR},
    {"destructor", TokenType::DESTRUCTOR},
    {"int", TokenType::INT},
    {"char", TokenType::CHAR},
    {"bool", TokenType::BOOL},
    {"float", TokenType::FLOAT},
    {"if", TokenType::IF},
    {"elif", TokenType::ELIF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"return", TokenType::RETURN},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"print", TokenType::PRINT},
    {"read", TokenType::READ},
    {"new", TokenType::NEW},
    {"delete", TokenType::DELETE},
    {"sizeof", TokenType::SIZEOF},
    {"this", TokenType::THIS},
    {"nullptr", TokenType::NULLPTR}
};

// Initialize static operator map
const std::unordered_map<std::string, TokenType> Lexer::operators = {
    // Assignment operators
    {"=", TokenType::ASSIGN},
    {"+=", TokenType::PLUS_ASSIGN},
    {"-=", TokenType::MINUS_ASSIGN},
    {"*=", TokenType::MUL_ASSIGN},
    {"/=", TokenType::DIV_ASSIGN},
    {"%=", TokenType::MOD_ASSIGN},
    {"<<=", TokenType::SHL_ASSIGN},
    {">>=", TokenType::SHR_ASSIGN},
    {"&=", TokenType::AND_ASSIGN},
    {"^=", TokenType::XOR_ASSIGN},
    {"|=", TokenType::OR_ASSIGN},
    
    // Arithmetic operators
    {"+", TokenType::PLUS},
    {"-", TokenType::MINUS},
    {"*", TokenType::MUL},
    {"/", TokenType::DIV},
    {"%", TokenType::MOD},
    
    // Bitwise operators
    {"<<", TokenType::SHL},
    {">>", TokenType::SHR},
    {"&", TokenType::BITWISE_AND},
    {"|", TokenType::BITWISE_OR},
    {"^", TokenType::BITWISE_XOR},
    {"~", TokenType::BITWISE_NOT},
    
    // Comparison operators
    {"==", TokenType::EQ},
    {"!=", TokenType::NE},
    {"<", TokenType::LT},
    {">", TokenType::GT},
    {"<=", TokenType::LE},
    {">=", TokenType::GE},
    
    // Logical operators
    {"&&", TokenType::LOGICAL_AND},
    {"||", TokenType::LOGICAL_OR},
    {"!", TokenType::LOGICAL_NOT},
    
    // Increment/Decrement
    {"++", TokenType::INCREMENT},
    {"--", TokenType::DECREMENT},
    
    // Member access
    {"->", TokenType::ARROW},
    {".", TokenType::DOT},
    
    // Punctuation
    {"(", TokenType::LPAREN},
    {")", TokenType::RPAREN},
    {"{", TokenType::LBRACE},
    {"}", TokenType::RBRACE},
    {"[", TokenType::LBRACKET},
    {"]", TokenType::RBRACKET},
    {",", TokenType::COMMA},
    {";", TokenType::SEMICOLON},
    {":", TokenType::COLON}
};

// Utility function to print token names
std::string tokenTypeToString(TokenType type) {
    static const std::unordered_map<TokenType, std::string> typeNames = {
        {TokenType::CLASS, "CLASS"},
        {TokenType::CONSTRUCTOR, "CONSTRUCTOR"},
        {TokenType::DESTRUCTOR, "DESTRUCTOR"},
        {TokenType::INT, "INT"},
        {TokenType::CHAR, "CHAR"},
        {TokenType::BOOL, "BOOL"},
        {TokenType::FLOAT, "FLOAT"},
        {TokenType::IF, "IF"},
        {TokenType::ELIF, "ELIF"},
        {TokenType::ELSE, "ELSE"},
        {TokenType::WHILE, "WHILE"},
        {TokenType::FOR, "FOR"},
        {TokenType::RETURN, "RETURN"},
        {TokenType::BREAK, "BREAK"},
        {TokenType::CONTINUE, "CONTINUE"},
        {TokenType::PRINT, "PRINT"},
        {TokenType::READ, "READ"},
        {TokenType::NEW, "NEW"},
        {TokenType::DELETE, "DELETE"},
        {TokenType::SIZEOF, "SIZEOF"},
        {TokenType::THIS, "THIS"},
        {TokenType::TRUE, "TRUE"},
        {TokenType::FALSE, "FALSE"},
        {TokenType::NULLPTR, "NULLPTR"},
        {TokenType::ASSIGN, "ASSIGN"},
        {TokenType::PLUS_ASSIGN, "PLUS_ASSIGN"},
        {TokenType::MINUS_ASSIGN, "MINUS_ASSIGN"},
        {TokenType::MUL_ASSIGN, "MUL_ASSIGN"},
        {TokenType::DIV_ASSIGN, "DIV_ASSIGN"},
        {TokenType::MOD_ASSIGN, "MOD_ASSIGN"},
        {TokenType::SHL_ASSIGN, "SHL_ASSIGN"},
        {TokenType::SHR_ASSIGN, "SHR_ASSIGN"},
        {TokenType::AND_ASSIGN, "AND_ASSIGN"},
        {TokenType::XOR_ASSIGN, "XOR_ASSIGN"},
        {TokenType::OR_ASSIGN, "OR_ASSIGN"},
        {TokenType::PLUS, "PLUS"},
        {TokenType::MINUS, "MINUS"},
        {TokenType::MUL, "MUL"},
        {TokenType::DIV, "DIV"},
        {TokenType::MOD, "MOD"},
        {TokenType::SHL, "SHL"},
        {TokenType::SHR, "SHR"},
        {TokenType::AND, "AND"},
        {TokenType::OR, "OR"},
        {TokenType::XOR, "XOR"},
        {TokenType::EQ, "EQ"},
        {TokenType::NE, "NE"},
        {TokenType::LT, "LT"},
        {TokenType::GT, "GT"},
        {TokenType::LE, "LE"},
        {TokenType::GE, "GE"},
        {TokenType::LOGICAL_AND, "LOGICAL_AND"},
        {TokenType::LOGICAL_OR, "LOGICAL_OR"},
        {TokenType::LOGICAL_NOT, "LOGICAL_NOT"},
        {TokenType::BITWISE_AND, "BITWISE_AND"},
        {TokenType::BITWISE_OR, "BITWISE_OR"},
        {TokenType::BITWISE_NOT, "BITWISE_NOT"},
        {TokenType::BITWISE_XOR, "BITWISE_XOR"},
        {TokenType::INCREMENT, "INCREMENT"},
        {TokenType::DECREMENT, "DECREMENT"},
        {TokenType::ARROW, "ARROW"},
        {TokenType::DOT, "DOT"},
        {TokenType::LPAREN, "LPAREN"},
        {TokenType::RPAREN, "RPAREN"},
        {TokenType::LBRACE, "LBRACE"},
        {TokenType::RBRACE, "RBRACE"},
        {TokenType::LBRACKET, "LBRACKET"},
        {TokenType::RBRACKET, "RBRACKET"},
        {TokenType::COMMA, "COMMA"},
        {TokenType::SEMICOLON, "SEMICOLON"},
        {TokenType::COLON, "COLON"},
        {TokenType::IDENTIFIER, "IDENTIFIER"},
        {TokenType::INTEGER_LITERAL, "INTEGER_LITERAL"},
        {TokenType::FLOAT_LITERAL, "FLOAT_LITERAL"},
        {TokenType::CHAR_LITERAL, "CHAR_LITERAL"},
        {TokenType::STRING_LITERAL, "STRING_LITERAL"},
        {TokenType::BOOL_LITERAL, "BOOL_LITERAL"},
        {TokenType::END_OF_FILE, "END_OF_FILE"},
        {TokenType::UNKNOWN, "UNKNOWN"}
    };
    
    auto it = typeNames.find(type);
    return it != typeNames.end() ? it->second : "UNKNOWN";
}

// Example usage
int main() {
    std::string test_code = R"(
        class MyClass {
            constructor(int x) {
                this.value = x;
            }
            
            int getValue() {
                return this.value;
            }
        }
        
        int main() {
            MyClass obj = new MyClass(42);
            print(obj.getValue());
            return 0;
        }
    )";
    
    Lexer lexer(test_code);
    auto tokens = lexer.tokenize();
    
    for (const auto& token : tokens) {
        std::cout << "Line " << token.line << ", Col " << token.column 
                  << ": " << tokenTypeToString(token.type);
        if (!token.value.empty()) {
            std::cout << " (" << token.value << ")";
        }
        std::cout << std::endl;
    }
    
    return 0;
}