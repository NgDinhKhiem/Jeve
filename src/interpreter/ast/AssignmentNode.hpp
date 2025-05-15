#pragma once

#include "../ASTNode.hpp"
#include "../Forward.hpp"

namespace jeve {

class AssignmentNode : public ASTNode {
private:
    std::string name;
    Ref<ASTNode> value;
    std::string type;

public:
    AssignmentNode(const std::string& name, Ref<ASTNode> value, const std::string& type = "")
        : name(name), value(value), type(type) {}

    Value evaluate(SymbolTable& scope) override {
        Value result = value->evaluate(scope);
        scope.set(name, result);
        return result;
    }

    std::string toString() const override { return "AssignmentNode(" + name + ")"; }
};

} // namespace jeve 