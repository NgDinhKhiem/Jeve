#pragma once

#include "../ASTNode.hpp"
#include <string>

namespace jeve {

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
    
    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "BinaryOpNode"; }
};

class UnaryOpNode : public ASTNode {
private:
    Ref<ASTNode> operand;
    std::string op;

public:
    UnaryOpNode(Ref<ASTNode> op, const std::string& o)
        : operand(op), op(o) {}

    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "UnaryOpNode"; }
};

} // namespace jeve 