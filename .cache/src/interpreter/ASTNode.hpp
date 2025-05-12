#pragma once

#include "Forward.hpp"
#include "SymbolTable.hpp"
#include "Value.hpp"
#include "Object.hpp"
#include <string>
#include <iostream>
#include <regex>
#include <vector>

namespace jeve {

class ASTNode : public Object {
public:
    virtual ~ASTNode() = default;
    virtual Value evaluate(SymbolTable& scope) = 0;
};

class NumberNode : public ASTNode {
private:
    int64_t value;

public:
    NumberNode(int64_t val) : value(val) {}
    Value evaluate(SymbolTable& scope) override {
        return Value(value);
    }
};

class StringNode : public ASTNode {
private:
    std::string value;

public:
    StringNode(const std::string& val) : value(val) {}
    Value evaluate(SymbolTable& scope) override {
        return Value(value);
    }
};

class IdentifierNode : public ASTNode {
private:
    std::string name;

public:
    IdentifierNode(const std::string& n) : name(n) {}
    Value evaluate(SymbolTable& scope) override {
        return scope.get(name);
    }
};

class BinaryOpNode : public ASTNode {
private:
    Ref<ASTNode> left;
    Ref<ASTNode> right;
    std::string op;

public:
    BinaryOpNode(Ref<ASTNode> l, Ref<ASTNode> r, const std::string& o)
        : left(l), right(r), op(o) {}
    
    ASTNode* getLeft() const { return left.get(); }
    ASTNode* getRight() const { return right.get(); }
    
    Value evaluate(SymbolTable& scope) override {
        Value lval = left->evaluate(scope);
        Value rval = right->evaluate(scope);
        
        if (lval.getType() == Value::Type::Integer && rval.getType() == Value::Type::Integer) {
            int64_t l = lval.getInteger();
            int64_t r = rval.getInteger();
            
            if (op == "+") return Value(l + r);
            if (op == "-") return Value(l - r);
            if (op == "*") return Value(l * r);
            if (op == "/") {
                if (r == 0) throw std::runtime_error("Division by zero");
                return Value(l / r);
            }
            if (op == "==") return Value(l == r);
            if (op == "!=") return Value(l != r);
            if (op == "<") return Value(l < r);
            if (op == ">") return Value(l > r);
            if (op == "<=") return Value(l <= r);
            if (op == ">=") return Value(l >= r);
        } else if (lval.getType() == Value::Type::Float || rval.getType() == Value::Type::Float) {
            double l = (lval.getType() == Value::Type::Float) ? lval.getFloat() : static_cast<double>(lval.getInteger());
            double r = (rval.getType() == Value::Type::Float) ? rval.getFloat() : static_cast<double>(rval.getInteger());
            
            if (op == "+") return Value(l + r);
            if (op == "-") return Value(l - r);
            if (op == "*") return Value(l * r);
            if (op == "/") {
                if (r == 0.0) throw std::runtime_error("Division by zero");
                return Value(l / r);
            }
            if (op == "==") return Value(l == r);
            if (op == "!=") return Value(l != r);
            if (op == "<") return Value(l < r);
            if (op == ">") return Value(l > r);
            if (op == "<=") return Value(l <= r);
            if (op == ">=") return Value(l >= r);
        } else if (lval.getType() == Value::Type::String || rval.getType() == Value::Type::String) {
            std::string lstr = lval.toString();
            std::string rstr = rval.toString();
            
            if (op == "+") return Value(lstr + rstr);
            if (op == "==") return Value(lstr == rstr);
            if (op == "!=") return Value(lstr != rstr);
        }
        
        throw std::runtime_error("Invalid operation between types");
    }
};

class PrintNode : public ASTNode {
private:
    Ref<ASTNode> expression;

    std::string processEscapeSequences(const std::string& str) {
        std::string result;
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '\\' && i + 1 < str.length()) {
                switch (str[i + 1]) {
                    case 'n': result += '\n'; break;
                    case 't': result += '\t'; break;
                    case 'r': result += '\r'; break;
                    case '\\': result += '\\'; break;
                    case '"': result += '"'; break;
                    default: result += str[i]; continue;
                }
                ++i;
            } else {
                result += str[i];
            }
        }
        return result;
    }

public:
    PrintNode(Ref<ASTNode> expr) : expression(expr) {}

    ASTNode* getExpression() const { return expression.get(); }

    Value evaluate(SymbolTable& scope) override {
        Value result = expression->evaluate(scope);
        if (result.getType() == Value::Type::String) {
            std::cout << processEscapeSequences(result.getString()) << std::endl;
        } else if (result.getType() == Value::Type::Integer) {
            std::cout << result.getInteger() << std::endl;
        } else if (result.getType() == Value::Type::Float) {
            std::cout << result.getFloat() << std::endl;
        } else if (result.getType() == Value::Type::Boolean) {
            std::cout << (result.getBoolean() ? "true" : "false") << std::endl;
        }
        return result;
    }
};

class StatementNode : public ASTNode {
private:
    Ref<ASTNode> statement;
    Ref<StatementNode> next;
    GarbageCollector* gc;

public:
    StatementNode(Ref<ASTNode> stmt, GarbageCollector* g) 
        : statement(stmt), next(), gc(g) {}

    void setNext(Ref<StatementNode> n) {
        next = n;
    }

    ASTNode* getStatement() const { return statement.get(); }
    StatementNode* getNext() const { return next.get(); }

    Value evaluate(SymbolTable& scope) override {
        Value result = statement->evaluate(scope);
        if (next) {
            result = next->evaluate(scope);
        }
        return result;
    }

    GarbageCollector* getGC() const { return gc; }
};

class BlockNode : public ASTNode {
private:
    Ref<StatementNode> first;
    Ref<StatementNode> last;
    GarbageCollector* gc;

public:
    BlockNode(GarbageCollector* g) : first(), last(), gc(g) {}

    void addStatement(Ref<ASTNode> stmt) {
        Ref<StatementNode> node(new StatementNode(stmt, gc), gc);
        if (!first) {
            first = node;
            last = node;
        } else {
            last->setNext(node);
            last = node;
        }
    }

    Value evaluate(SymbolTable& scope) override {
        if (!first) {
            return Value();
        }
        return first->evaluate(scope);
    }
};

class AssignmentNode : public ASTNode {
private:
    std::string name;
    Ref<ASTNode> expression;
    std::string type;  // Optional type annotation

public:
    AssignmentNode(const std::string& n, Ref<ASTNode> expr, const std::string& t = "")
        : name(n), expression(expr), type(t) {}

    Value evaluate(SymbolTable& scope) override {
        Value value = expression->evaluate(scope);
        
        // Type checking if type is specified
        if (!type.empty()) {
            if (type == "int" && value.getType() != Value::Type::Integer) {
                throw std::runtime_error("Type mismatch: expected int");
            }
            if (type == "string" && value.getType() != Value::Type::String) {
                throw std::runtime_error("Type mismatch: expected string");
            }
            if (type == "float" && value.getType() != Value::Type::Float) {
                throw std::runtime_error("Type mismatch: expected float");
            }
            if (type == "bool" && value.getType() != Value::Type::Boolean) {
                throw std::runtime_error("Type mismatch: expected bool");
            }
        }
        
        scope.set(name, value);
        return value;
    }
};

class InputNode : public ASTNode {
private:
    std::string type;  // Optional type annotation

public:
    InputNode(const std::string& t = "") : type(t) {}

    Value evaluate(SymbolTable& scope) override {
        std::string input;
        std::getline(std::cin, input);
        
        if (type == "int") {
            try {
                size_t pos;
                int64_t num = std::stoll(input, &pos);
                if (pos == input.length()) {
                    return Value(num);
                }
                throw std::runtime_error("Invalid integer input");
            } catch (...) {
                throw std::runtime_error("Invalid integer input");
            }
        } else if (type == "float") {
            try {
                size_t pos;
                double num = std::stod(input, &pos);
                if (pos == input.length()) {
                    return Value(num);
                }
                throw std::runtime_error("Invalid float input");
            } catch (...) {
                throw std::runtime_error("Invalid float input");
            }
        } else if (type == "bool") {
            if (input == "true") return Value(true);
            if (input == "false") return Value(false);
            throw std::runtime_error("Invalid boolean input");
        }
        
        return Value(input);  // Default to string
    }
};

class ConcatNode : public ASTNode {
private:
    Ref<ASTNode> left;
    Ref<ASTNode> right;

public:
    ConcatNode(Ref<ASTNode> l, Ref<ASTNode> r) : left(l), right(r) {}

    Value evaluate(SymbolTable& scope) override {
        Value lval = left->evaluate(scope);
        Value rval = right->evaluate(scope);
        
        std::string lstr = lval.toString();
        std::string rstr = rval.toString();
        
        return Value(lstr + rstr);
    }
};

class IfNode : public ASTNode {
private:
    Ref<ASTNode> condition;
    Ref<BlockNode> thenBlock;
    Ref<BlockNode> elseBlock;

public:
    IfNode(Ref<ASTNode> cond, Ref<BlockNode> then, Ref<BlockNode> else_ = Ref<BlockNode>())
        : condition(cond), thenBlock(then), elseBlock(else_) {}

    Value evaluate(SymbolTable& scope) override {
        Value cond = condition->evaluate(scope);
        if (cond.getType() != Value::Type::Boolean) {
            throw std::runtime_error("Condition must be a boolean");
        }
        
        if (cond.getBoolean()) {
            return thenBlock->evaluate(scope);
        } else if (elseBlock) {
            return elseBlock->evaluate(scope);
        }
        return Value();
    }
};

class WhileNode : public ASTNode {
private:
    Ref<ASTNode> condition;
    Ref<BlockNode> body;

public:
    WhileNode(Ref<ASTNode> cond, Ref<BlockNode> b)
        : condition(cond), body(b) {}

    Value evaluate(SymbolTable& scope) override {
        Value result;
        while (true) {
            Value cond = condition->evaluate(scope);
            if (cond.getType() != Value::Type::Boolean) {
                throw std::runtime_error("Condition must be a boolean");
            }
            
            if (!cond.getBoolean()) {
                break;
            }
            
            result = body->evaluate(scope);
        }
        return result;
    }
};

class FunctionCallNode : public ASTNode {
private:
    std::string name;
    std::vector<Ref<ASTNode>> arguments;

public:
    FunctionCallNode(const std::string& n, const std::vector<Ref<ASTNode>>& args)
        : name(n), arguments(args) {}

    Value evaluate(SymbolTable& scope) override {
        if (name == "print") {
            if (arguments.size() != 1) {
                throw std::runtime_error("print() expects exactly one argument");
            }
            return PrintNode(arguments[0]).evaluate(scope);
        }
        if (name == "input") {
            if (arguments.size() > 1) {
                throw std::runtime_error("input() expects at most one type argument");
            }
            std::string type = arguments.empty() ? "" : arguments[0]->evaluate(scope).getString();
            return InputNode(type).evaluate(scope);
        }
        throw std::runtime_error("Unknown function: " + name);
    }
};

} // namespace jeve 