#pragma once

#include "../ASTNode.hpp"
#include "../Forward.hpp"
#include <vector>

namespace jeve {

class FunctionCallNode : public ASTNode {
private:
    std::string name;
    std::vector<Ref<ASTNode>> arguments;

public:
    FunctionCallNode(const std::string& n, const std::vector<Ref<ASTNode>>& args)
        : name(n), arguments(args) {}

    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "FunctionCallNode"; }
};

class UserFunctionNode : public ASTNode {
private:
    std::string name;
    std::vector<std::string> params;
    Ref<ASTNode> body;
public:
    UserFunctionNode(const std::string& n, const std::vector<std::string>& p, Ref<ASTNode> b)
        : name(n), params(p), body(b) {}
    const std::string& getName() const { return name; }
    const std::vector<std::string>& getParams() const { return params; }
    Ref<ASTNode> getBody() const { return body; }
    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "UserFunctionNode(" + name + ")"; }
};

} // namespace jeve 