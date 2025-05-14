#pragma once

#include "ASTNode.hpp"
#include <string>
#include <vector>

namespace jeve {

class ArrayNode : public ASTNode {
private:
    std::vector<Ref<ASTNode>> elements;

public:
    ArrayNode(const std::vector<Ref<ASTNode>>& elems) : elements(elems) {}

    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "ArrayNode"; }
};

class ArrayAccessNode : public ASTNode {
private:
    Ref<ASTNode> array;
    Ref<ASTNode> index;

public:
    ArrayAccessNode(Ref<ASTNode> arr, Ref<ASTNode> idx) : array(arr), index(idx) {}

    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "ArrayAccessNode"; }
};

class ArrayAssignmentNode : public ASTNode {
private:
    Ref<ASTNode> array;
    Ref<ASTNode> index;
    Ref<ASTNode> value;

public:
    ArrayAssignmentNode(Ref<ASTNode> arr, Ref<ASTNode> idx, Ref<ASTNode> val) 
        : array(arr), index(idx), value(val) {}

    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "ArrayAssignmentNode"; }
};

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

class DebugGCNode : public ASTNode {
private:
    GarbageCollector* gc;

public:
    DebugGCNode(GarbageCollector* gc = nullptr) : gc(gc) {}

    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "DebugGCNode"; }
};

class CleanGCNode : public ASTNode {
private:
    GarbageCollector* gc;

public:
    CleanGCNode(GarbageCollector* gc = nullptr) : gc(gc) {}

    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "CleanGCNode"; }
};

} // namespace jeve 