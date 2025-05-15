#pragma once

#include "../ASTNode.hpp"
#include "../Forward.hpp"

namespace jeve {

class ConcatNode : public ASTNode {
private:
    Ref<ASTNode> left;
    Ref<ASTNode> right;

public:
    ConcatNode(Ref<ASTNode> left, Ref<ASTNode> right)
        : left(left), right(right) {}

    Value evaluate(SymbolTable& scope) override {
        Value leftVal = left->evaluate(scope);
        Value rightVal = right->evaluate(scope);
        
        // Convert both values to strings and concatenate
        std::string result = leftVal.toString() + rightVal.toString();
        return Value(result);
    }

    std::string toString() const override { return "ConcatNode"; }
};

} // namespace jeve 