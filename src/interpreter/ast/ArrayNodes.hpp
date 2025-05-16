#pragma once

#include "../ASTNode.hpp"
#include "../Forward.hpp"
#include <vector>

namespace jeve {

class JeveInterpreter;

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
    JeveInterpreter* interpreter;

public:
    ArrayAssignmentNode(Ref<ASTNode> arr, Ref<ASTNode> idx, Ref<ASTNode> val, JeveInterpreter* interp = nullptr) 
        : array(arr), index(idx), value(val), interpreter(interp) {}

    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "ArrayAssignmentNode"; }
};

// Note: Arrays are now GC-managed and allocated via the ObjectPool.

} // namespace jeve 