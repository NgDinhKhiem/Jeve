#pragma once

#include "../ASTNode.hpp"
#include "../Forward.hpp"

namespace jeve {

class PrintNode : public ASTNode {
private:
    Ref<ASTNode> expression;

public:
    PrintNode(Ref<ASTNode> expr) : expression(expr) {}

    ASTNode* getExpression() const { return expression.get(); }

    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "PrintNode"; }
};

class InputNode : public ASTNode {
private:
    std::string type;  // Optional type annotation

public:
    InputNode(const std::string& t = "") : type(t) {}

    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "InputNode"; }
};

} // namespace jeve 