#include "JeveInterpreter.hpp"
#include <iostream>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace jeve {

class ParseError : public std::runtime_error {
private:
    size_t line;
    size_t column;

public:
    ParseError(const std::string& message, size_t l, size_t c)
        : std::runtime_error(message), line(l), column(c) {}

    std::string getFormattedMessage() const {
        std::ostringstream oss;
        oss << "Error at line " << line << ", column " << column << ": " << what();
        return oss.str();
    }
};

// Lexer implementation
enum class TokenType {
    NUMBER,
    STRING,
    IDENTIFIER,
    KEYWORD,
    OPERATOR,
    PUNCTUATION,
    TYPE,
    EOF_TOKEN
};

struct Token {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;
};

class Lexer {
private:
    std::string input;
    size_t position;
    size_t line;
    size_t column;
    static const std::unordered_map<std::string, TokenType> keywords;
    static const std::unordered_map<std::string, TokenType> types;

public:
    Lexer(const std::string& code) 
        : input(code), position(0), line(1), column(1) {}

    Token nextToken() {
        while (position < input.length()) {
            char current = input[position];
            
            // Handle comments
            if (current == '/' && position + 1 < input.length() && input[position + 1] == '/') {
                // Skip to end of line
                while (position < input.length() && input[position] != '\n') {
                    position++;
                    column++;
                }
                continue;
            }
            
            if (std::isspace(current)) {
                if (current == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                position++;
                continue;
            }
            
            if (std::isdigit(current) || (current == '.' && position + 1 < input.length() && std::isdigit(input[position + 1]))) {
                return readFloatOrInt();
            }
            
            if (current == '"') {
                return readString();
            }
            
            if (std::isalpha(current) || current == '_') {
                return readIdentifier();
            }
            
            if (current == '+' || current == '-' || current == '*' || current == '/' || 
                current == '=' || current == '<' || current == '>' || current == '!') {
                return readOperator();
            }
            
            if (current == ';' || current == '(' || current == ')' || 
                current == '{' || current == '}' || current == ':' || current == ',') {
                position++;
                column++;
                return {TokenType::PUNCTUATION, std::string(1, current), line, column - 1};
            }
            
            throw ParseError("Unexpected character: " + std::string(1, current), line, column);
        }
        
        return {TokenType::EOF_TOKEN, "", line, column};
    }

private:
    Token readNumber() {
        size_t start = position;
        while (position < input.length() && std::isdigit(input[position])) {
            position++;
            column++;
        }
        return {TokenType::NUMBER, input.substr(start, position - start), line, column - (position - start)};
    }
    
    Token readString() {
        position++; // Skip opening quote
        column++;
        size_t start = position;
        while (position < input.length() && input[position] != '"') {
            if (input[position] == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            position++;
        }
        if (position >= input.length()) {
            throw ParseError("Unterminated string literal", line, column);
        }
        std::string value = input.substr(start, position - start);
        position++; // Skip closing quote
        column++;
        return {TokenType::STRING, value, line, column - value.length() - 2};
    }
    
    Token readIdentifier() {
        size_t start = position;
        while (position < input.length() && (std::isalnum(input[position]) || input[position] == '_')) {
            position++;
            column++;
        }
        std::string value = input.substr(start, position - start);
        
        // Check if it's a keyword
        auto it = keywords.find(value);
        if (it != keywords.end()) {
            return {it->second, value, line, column - value.length()};
        }
        
        // Check if it's a type
        auto typeIt = types.find(value);
        if (typeIt != types.end()) {
            return {TokenType::TYPE, value, line, column - value.length()};
        }
        
        return {TokenType::IDENTIFIER, value, line, column - value.length()};
    }

    Token readOperator() {
        char current = input[position];
        position++;
        column++;
        
        // Handle two-character operators
        if (position < input.length()) {
            char next = input[position];
            if ((current == '=' && next == '=') ||
                (current == '!' && next == '=') ||
                (current == '<' && next == '=') ||
                (current == '>' && next == '=')) {
                position++;
                column++;
                return {TokenType::OPERATOR, std::string(1, current) + next, line, column - 2};
            }
        }
        
        return {TokenType::OPERATOR, std::string(1, current), line, column - 1};
    }

    Token readFloatOrInt() {
        size_t start = position;
        bool seenDot = false;
        while (position < input.length() && (std::isdigit(input[position]) || input[position] == '.')) {
            if (input[position] == '.') {
                if (seenDot) break; // Only one dot allowed
                seenDot = true;
            }
            position++;
            column++;
        }
        std::string numStr = input.substr(start, position - start);
        if (seenDot) {
            return {TokenType::NUMBER, numStr, line, column - (position - start)}; // We'll distinguish in parser
        } else {
            return {TokenType::NUMBER, numStr, line, column - (position - start)};
        }
    }
};

// Initialize static members
const std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"print", TokenType::KEYWORD},
    {"if", TokenType::KEYWORD},
    {"else", TokenType::KEYWORD},
    {"while", TokenType::KEYWORD}
};

const std::unordered_map<std::string, TokenType> Lexer::types = {
    {"int", TokenType::TYPE},
    {"string", TokenType::TYPE},
    {"float", TokenType::TYPE},
    {"bool", TokenType::TYPE}
};

// Parser implementation
class Parser {
private:
    Lexer lexer;
    Token currentToken;
    JeveInterpreter& interpreter;

public:
    Parser(const std::string& code, JeveInterpreter& interp)
        : lexer(code), interpreter(interp) {
        currentToken = lexer.nextToken();
    }

    Ref<ASTNode> parse() {
        Ref<BlockNode> block = interpreter.createObject<BlockNode>(&interpreter.getGC());
        
        while (currentToken.type != TokenType::EOF_TOKEN) {
            block->addStatement(parseStatement());
        }
        
        return block;
    }

private:
    Ref<ASTNode> parseStatement() {
        if (currentToken.type == TokenType::KEYWORD) {
            if (currentToken.value == "print") {
                currentToken = lexer.nextToken(); // Skip 'print'
                Ref<ASTNode> expr;
                
                // Handle both print expr; and print(expr);
                if (currentToken.type == TokenType::PUNCTUATION && currentToken.value == "(") {
                    currentToken = lexer.nextToken();
                    expr = parseExpression();
                    if (currentToken.type != TokenType::PUNCTUATION || currentToken.value != ")") {
                        throw ParseError("Expected ')' after print(", 
                                       currentToken.line, currentToken.column);
                    }
                    currentToken = lexer.nextToken();
                } else {
                    expr = parseExpression();
                }
                
                if (currentToken.type != TokenType::PUNCTUATION || currentToken.value != ";") {
                    throw ParseError("Expected semicolon after print statement", 
                                   currentToken.line, currentToken.column);
                }
                currentToken = lexer.nextToken();
                return interpreter.createObject<PrintNode>(expr);
            }
            else if (currentToken.value == "if") {
                currentToken = lexer.nextToken(); // Skip 'if'
                
                if (currentToken.type != TokenType::PUNCTUATION || currentToken.value != "(") {
                    throw ParseError("Expected '(' after if", 
                                   currentToken.line, currentToken.column);
                }
                currentToken = lexer.nextToken();
                
                Ref<ASTNode> condition = parseExpression();
                
                if (currentToken.type != TokenType::PUNCTUATION || currentToken.value != ")") {
                    throw ParseError("Expected ')' after if condition", 
                                   currentToken.line, currentToken.column);
                }
                currentToken = lexer.nextToken();
                
                if (currentToken.type != TokenType::PUNCTUATION || currentToken.value != "{") {
                    throw ParseError("Expected '{' after if condition", 
                                   currentToken.line, currentToken.column);
                }
                currentToken = lexer.nextToken();
                
                Ref<BlockNode> thenBlock = interpreter.createObject<BlockNode>(&interpreter.getGC());
                while (currentToken.type != TokenType::PUNCTUATION || currentToken.value != "}") {
                    thenBlock->addStatement(parseStatement());
                }
                currentToken = lexer.nextToken();
                
                Ref<BlockNode> elseBlock;
                if (currentToken.type == TokenType::KEYWORD && currentToken.value == "else") {
                    currentToken = lexer.nextToken();
                    
                    if (currentToken.type != TokenType::PUNCTUATION || currentToken.value != "{") {
                        throw ParseError("Expected '{' after else", 
                                       currentToken.line, currentToken.column);
                    }
                    currentToken = lexer.nextToken();
                    
                    elseBlock = interpreter.createObject<BlockNode>(&interpreter.getGC());
                    while (currentToken.type != TokenType::PUNCTUATION || currentToken.value != "}") {
                        elseBlock->addStatement(parseStatement());
                    }
                    currentToken = lexer.nextToken();
                }
                
                return interpreter.createObject<IfNode>(condition, thenBlock, elseBlock);
            }
            else if (currentToken.value == "while") {
                currentToken = lexer.nextToken(); // Skip 'while'
                
                if (currentToken.type != TokenType::PUNCTUATION || currentToken.value != "(") {
                    throw ParseError("Expected '(' after while", 
                                   currentToken.line, currentToken.column);
                }
                currentToken = lexer.nextToken();
                
                Ref<ASTNode> condition = parseExpression();
                
                if (currentToken.type != TokenType::PUNCTUATION || currentToken.value != ")") {
                    throw ParseError("Expected ')' after while condition", 
                                   currentToken.line, currentToken.column);
                }
                currentToken = lexer.nextToken();
                
                if (currentToken.type != TokenType::PUNCTUATION || currentToken.value != "{") {
                    throw ParseError("Expected '{' after while condition", 
                                   currentToken.line, currentToken.column);
                }
                currentToken = lexer.nextToken();
                
                Ref<BlockNode> body = interpreter.createObject<BlockNode>(&interpreter.getGC());
                while (currentToken.type != TokenType::PUNCTUATION || currentToken.value != "}") {
                    body->addStatement(parseStatement());
                }
                currentToken = lexer.nextToken();
                
                return interpreter.createObject<WhileNode>(condition, body);
            }
        }
        else if (currentToken.type == TokenType::IDENTIFIER) {
            std::string name = currentToken.value;
            currentToken = lexer.nextToken();
            
            std::string type;
            if (currentToken.type == TokenType::PUNCTUATION && currentToken.value == ":") {
                currentToken = lexer.nextToken();
                if (currentToken.type != TokenType::TYPE) {
                    throw ParseError("Expected type after ':' in variable declaration", currentToken.line, currentToken.column);
                }
                type = currentToken.value;
                currentToken = lexer.nextToken();
            }
            
            if (currentToken.type == TokenType::OPERATOR && currentToken.value == "=") {
                currentToken = lexer.nextToken();
                Ref<ASTNode> expr = parseExpression();
                if (currentToken.type != TokenType::PUNCTUATION || currentToken.value != ";") {
                    throw ParseError("Expected semicolon after assignment", currentToken.line, currentToken.column);
                }
                currentToken = lexer.nextToken();
                return interpreter.createObject<AssignmentNode>(name, expr, type);
            }
            else if (currentToken.type == TokenType::PUNCTUATION && currentToken.value == "(") {
                // Function call
                currentToken = lexer.nextToken();
                std::vector<Ref<ASTNode>> arguments;
                
                if (currentToken.type != TokenType::PUNCTUATION || currentToken.value != ")") {
                    do {
                        arguments.push_back(parseExpression());
                        if (currentToken.type == TokenType::PUNCTUATION && currentToken.value == ")") {
                            break;
                        }
                        if (currentToken.type != TokenType::PUNCTUATION || currentToken.value != ",") {
                            throw ParseError("Expected ',' or ')' in function call", 
                                           currentToken.line, currentToken.column);
                        }
                        currentToken = lexer.nextToken();
                    } while (true);
                }
                currentToken = lexer.nextToken();
                
                if (currentToken.type != TokenType::PUNCTUATION || currentToken.value != ";") {
                    throw ParseError("Expected semicolon after function call", 
                                   currentToken.line, currentToken.column);
                }
                currentToken = lexer.nextToken();
                
                return interpreter.createObject<FunctionCallNode>(name, arguments);
            }
        }
        
        Ref<ASTNode> expr = parseExpression();
        if (currentToken.type == TokenType::PUNCTUATION && currentToken.value == ";") {
            currentToken = lexer.nextToken();
        }
        return expr;
    }

    Ref<ASTNode> parseExpression() {
        Ref<ASTNode> left = parseTerm();
        
        while (currentToken.type == TokenType::OPERATOR &&
               (currentToken.value == "+" || currentToken.value == "-" ||
                currentToken.value == "==" || currentToken.value == "!=" ||
                currentToken.value == "<" || currentToken.value == ">" ||
                currentToken.value == "<=" || currentToken.value == ">=")) {
            std::string op = currentToken.value;
            currentToken = lexer.nextToken();
            Ref<ASTNode> right = parseTerm();
            
            if (op == "+") {
                // Check if either operand is a string for concatenation
                if (left->evaluate(interpreter.getCurrentScope()).getType() == Value::Type::String ||
                    right->evaluate(interpreter.getCurrentScope()).getType() == Value::Type::String) {
                    left = interpreter.createObject<ConcatNode>(left, right);
                } else {
                    left = interpreter.createObject<BinaryOpNode>(left, right, op);
                }
            } else {
                left = interpreter.createObject<BinaryOpNode>(left, right, op);
            }
        }
        
        return left;
    }

    Ref<ASTNode> parseTerm() {
        Ref<ASTNode> left = parseFactor();
        
        while (currentToken.type == TokenType::OPERATOR &&
               (currentToken.value == "*" || currentToken.value == "/")) {
            std::string op = currentToken.value;
            currentToken = lexer.nextToken();
            Ref<ASTNode> right = parseFactor();
            left = interpreter.createObject<BinaryOpNode>(left, right, op);
        }
        
        return left;
    }

    Ref<ASTNode> parseFactor() {
        if (currentToken.type == TokenType::PUNCTUATION && currentToken.value == "(") {
            currentToken = lexer.nextToken();
            Ref<ASTNode> expr = parseExpression();
            if (currentToken.type != TokenType::PUNCTUATION || currentToken.value != ")") {
                throw ParseError("Expected closing parenthesis", 
                               currentToken.line, currentToken.column);
            }
            currentToken = lexer.nextToken();
            return expr;
        }
        
        Token token = currentToken;
        currentToken = lexer.nextToken();
        
        if (token.type == TokenType::NUMBER) {
            if (token.value.find('.') != std::string::npos) {
                return interpreter.createObject<NumberNode>(std::stod(token.value)); // Use double for float
            } else {
                return interpreter.createObject<NumberNode>(std::stoll(token.value));
            }
        }
        
        if (token.type == TokenType::STRING) {
            return interpreter.createObject<StringNode>(token.value);
        }
        
        if (token.type == TokenType::IDENTIFIER) {
            // Check if it's a function call
            if (currentToken.type == TokenType::PUNCTUATION && currentToken.value == "(") {
                currentToken = lexer.nextToken();
                std::vector<Ref<ASTNode>> arguments;
                
                if (currentToken.type != TokenType::PUNCTUATION || currentToken.value != ")") {
                    do {
                        arguments.push_back(parseExpression());
                        if (currentToken.type == TokenType::PUNCTUATION && currentToken.value == ")") {
                            break;
                        }
                        if (currentToken.type != TokenType::PUNCTUATION || currentToken.value != ",") {
                            throw ParseError("Expected ',' or ')' in function call", 
                                           currentToken.line, currentToken.column);
                        }
                        currentToken = lexer.nextToken();
                    } while (true);
                }
                currentToken = lexer.nextToken();
                return interpreter.createObject<FunctionCallNode>(token.value, arguments);
            }
            return interpreter.createObject<IdentifierNode>(token.value);
        }
        
        throw ParseError("Unexpected token: " + token.value, 
                        token.line, token.column);
    }
};

void JeveInterpreter::interpret(const std::string& code) {
    try {
        Parser parser(code, *this);
        Ref<ASTNode> ast = parser.parse();
        ast->evaluate(*globalScope);
    } catch (const ParseError& e) {
        throw std::runtime_error(e.getFormattedMessage());
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Interpreter error: ") + e.what());
    }
}

} // namespace jeve 