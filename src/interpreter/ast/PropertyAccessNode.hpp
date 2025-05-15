#pragma once

#include "../ASTNode.hpp"
#include <string>

namespace jeve {

class PropertyAccessNode : public ASTNode {
private:
    Ref<ASTNode> object;
    std::string property;

public:
    PropertyAccessNode(Ref<ASTNode> obj, const std::string& prop)
        : object(obj), property(prop) {}

    Value evaluate(SymbolTable& scope) override {
        Value objValue = object->evaluate(scope);
        
        if (property == "length") {
            if (objValue.getType() == Value::Type::Array) {
                return Value(static_cast<int64_t>(objValue.getArray().size()));
            } else if (objValue.getType() == Value::Type::String) {
                return Value(static_cast<int64_t>(objValue.getString().length()));
            }
        }
        
        throw std::runtime_error("Unknown property: " + property);
    }

    std::string toString() const override { return "PropertyAccessNode"; }
};

} // namespace jeve 