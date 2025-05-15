#include "FunctionNodes.hpp"
#include "BasicNodes.hpp"
#include <stdexcept>
#include "ControlFlowNodes.hpp"

namespace jeve {

Value FunctionCallNode::evaluate(SymbolTable& scope) {
    // Built-in: print
    if (name == "print") {
        if (arguments.size() != 1) {
            throw std::runtime_error("print() takes exactly 1 argument");
        }
        Value arg = arguments[0]->evaluate(scope);
        return Value();
    }
    // Built-in: insert(arr, index, value)
    if (name == "insert") {
        if (arguments.size() != 3) {
            throw std::runtime_error("insert() takes exactly 3 arguments: insert(arr, index, value). Got: " + std::to_string(arguments.size()));
        }
        // Only allow identifier as first argument
        auto* idNode = dynamic_cast<IdentifierNode*>(arguments[0].get());
        if (!idNode) {
            throw std::runtime_error("insert: first argument must be an array variable");
        }
        Value& arrRef = scope.getMutable(idNode->getName());
        Value idx = arguments[1]->evaluate(scope);
        Value val = arguments[2]->evaluate(scope);
        if (arrRef.getType() != Value::Type::Array) {
            throw std::runtime_error("insert: first argument must be an array");
        }
        if (idx.getType() != Value::Type::Integer) {
            throw std::runtime_error("insert: index must be an integer");
        }
        int64_t index = idx.getInteger();
        auto& elements = arrRef.getArray();
        if (index < 0 || static_cast<size_t>(index) > elements.size()) {
            throw std::runtime_error("insert: index out of bounds");
        }
        elements.insert(elements.begin() + index, val);
        return Value();
    }
    // Built-in: delete(arr, index)
    if (name == "delete") {
        if (arguments.size() != 2) {
            throw std::runtime_error("delete() takes exactly 2 arguments: delete(arr, index). Got: " + std::to_string(arguments.size()));
        }
        auto* idNode = dynamic_cast<IdentifierNode*>(arguments[0].get());
        if (!idNode) {
            throw std::runtime_error("delete: first argument must be an array variable");
        }
        Value& arrRef = scope.getMutable(idNode->getName());
        Value idx = arguments[1]->evaluate(scope);
        if (arrRef.getType() != Value::Type::Array) {
            throw std::runtime_error("delete: first argument must be an array");
        }
        if (idx.getType() != Value::Type::Integer) {
            throw std::runtime_error("delete: index must be an integer");
        }
        int64_t index = idx.getInteger();
        auto& elements = arrRef.getArray();
        if (index < 0 || static_cast<size_t>(index) >= elements.size()) {
            throw std::runtime_error("delete: index out of bounds");
        }
        elements.erase(elements.begin() + index);
        return Value();
    }
    // Check for user-defined function
    if (scope.has(name)) {
        Value funcVal = scope.get(name);
        if (funcVal.getType() == Value::Type::Object) {
            auto* userFunc = dynamic_cast<UserFunctionNode*>(funcVal.getObject());
            if (userFunc) {
                const auto& params = userFunc->getParams();
                if (params.size() != arguments.size()) {
                    throw std::runtime_error("Function '" + name + "' expects " + std::to_string(params.size()) + " arguments, got " + std::to_string(arguments.size()));
                }
                // Create a new scope for the function call
                SymbolTable localScope(&scope);
                for (size_t i = 0; i < params.size(); ++i) {
                    localScope.set(params[i], arguments[i]->evaluate(scope));
                }
                // Evaluate the function body
                try {
                    return userFunc->getBody()->evaluate(localScope);
                } catch (const ReturnException& e) {
                    return e.getValue();
                }
            }
        }
    }
    throw std::runtime_error("Unknown function: '" + name + "' with args=" + std::to_string(arguments.size()));
}

Value UserFunctionNode::evaluate(SymbolTable& scope) {
    // When evaluating a function definition, just return itself (or store in symbol table in parser)
    return Value();
}

} // namespace jeve 