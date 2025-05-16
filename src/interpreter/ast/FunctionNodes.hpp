#pragma once

#include "../ASTNode.hpp"
#include "../Forward.hpp"
#include <vector>

namespace jeve {

class JeveInterpreter;

class FunctionCallNode : public ASTNode {
private:
    std::string name;
    std::vector<Ref<ASTNode>> arguments;
    JeveInterpreter* interpreter;

public:
    FunctionCallNode(const std::string& n, const std::vector<Ref<ASTNode>>& args, JeveInterpreter* interp = nullptr)
        : name(n), arguments(args), interpreter(interp) {}

    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "FunctionCallNode"; }
};

class UserFunctionNode : public ASTNode {
private:
    std::string name;
    std::vector<std::string> params;
    Ref<ASTNode> body;
    JeveInterpreter* interpreter;

public:
    UserFunctionNode(const std::string& n, const std::vector<std::string>& p, Ref<ASTNode> b, JeveInterpreter* interp = nullptr)
        : name(n), params(p), body(b), interpreter(interp) {}
    const std::string& getName() const { return name; }
    const std::vector<std::string>& getParams() const { return params; }
    Ref<ASTNode> getBody() const { return body; }
    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "UserFunctionNode(" + name + ")"; }
};

} // namespace jeve 