#pragma once

#include "ASTNode.hpp"
#include <string>

namespace jeve {

class NumberNode : public ASTNode {
private:
    int64_t value;

public:
    NumberNode(int64_t val) : value(val) {}
    Value evaluate(SymbolTable&) override {
        return Value(value);
    }
    std::string toString() const override { return "NumberNode"; }
};

class StringNode : public ASTNode {
private:
    std::string value;

public:
    StringNode(const std::string& val) : value(val) {}
    Value evaluate(SymbolTable&) override {
        return Value(value);
    }
    std::string toString() const override { return "StringNode"; }
};

class IdentifierNode : public ASTNode {
private:
    std::string name;

public:
    IdentifierNode(const std::string& n) : name(n) {}
    Value evaluate(SymbolTable& scope) override {
        return scope.get(name);
    }

    const std::string& getName() const { return name; }
    std::string toString() const override { return "IdentifierNode"; }
};

class BooleanNode : public ASTNode {
private:
    bool value;

public:
    BooleanNode(bool val) : value(val) {}

    Value evaluate(SymbolTable&) override {
        return Value(value);
    }

    std::string toString() const override { return "BooleanNode"; }
};

} // namespace jeve 