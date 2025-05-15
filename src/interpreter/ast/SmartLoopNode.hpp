#pragma once

#include "../ASTNode.hpp"
#include <string>

namespace jeve {

class SmartLoopNode : public ASTNode {
private:
    std::string valueName;
    std::string indexName;
    Ref<ASTNode> array;
    Ref<BlockNode> body;

public:
    SmartLoopNode(const std::string& valName, const std::string& idxName,
                 Ref<ASTNode> arr, Ref<BlockNode> b)
        : valueName(valName), indexName(idxName), array(arr), body(b) {}

    Value evaluate(SymbolTable& scope) override {
        Value arrayValue = array->evaluate(scope);
        
        if (arrayValue.getType() != Value::Type::Array) {
            throw std::runtime_error("Cannot iterate over non-array value");
        }
        
        const auto& elements = arrayValue.getArray();
        Value result;
        
        for (size_t i = 0; i < elements.size(); ++i) {
            scope.set(indexName, Value(static_cast<int64_t>(i)));
            scope.set(valueName, elements[i]);
            result = body->evaluate(scope);
        }
        
        return result;
    }

    std::string toString() const override { return "SmartLoopNode"; }
};

} // namespace jeve 