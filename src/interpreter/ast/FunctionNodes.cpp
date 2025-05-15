#include "FunctionNodes.hpp"
#include "BasicNodes.hpp"
#include <stdexcept>
#include "ControlFlowNodes.hpp"

namespace jeve {

Value FunctionCallNode::evaluate(SymbolTable& scope) {
    // Built-in functions
    if (name == "print") {
        if (arguments.size() != 1) throw std::runtime_error("print() takes 1 argument");
        arguments[0]->evaluate(scope);
        return Value();
    }
    if (name == "insert") {
        if (arguments.size() != 3) throw std::runtime_error("insert() needs 3 args");
        auto* idNode = dynamic_cast<IdentifierNode*>(arguments[0].get());
        if (!idNode) throw std::runtime_error("insert: first arg must be array variable");
        Value& arr = scope.getMutable(idNode->getName());
        int64_t idx = arguments[1]->evaluate(scope).getInteger();
        Value val = arguments[2]->evaluate(scope);
        auto& elems = arr.getArray();
        if (idx < 0 || static_cast<size_t>(idx) > elems.size()) throw std::runtime_error("insert: index out of bounds");
        elems.insert(elems.begin() + idx, val);
        return Value();
    }
    if (name == "delete") {
        if (arguments.size() != 2) throw std::runtime_error("delete() needs 2 args");
        auto* idNode = dynamic_cast<IdentifierNode*>(arguments[0].get());
        if (!idNode) throw std::runtime_error("delete: first arg must be array variable");
        Value& arr = scope.getMutable(idNode->getName());
        int64_t idx = arguments[1]->evaluate(scope).getInteger();
        auto& elems = arr.getArray();
        if (idx < 0 || static_cast<size_t>(idx) >= elems.size()) throw std::runtime_error("delete: index out of bounds");
        elems.erase(elems.begin() + idx);
        return Value();
    }

    // User-defined functions
    if (scope.has(name)) {
        Value funcVal = scope.get(name);
        if (funcVal.getType() == Value::Type::Object) {
            auto* userFunc = dynamic_cast<UserFunctionNode*>(funcVal.getObject());
            if (userFunc) {
                const auto& params = userFunc->getParams();
                if (params.size() != arguments.size())
                    throw std::runtime_error("Function '" + name + "' expects " + std::to_string(params.size()) + " arguments");
                SymbolTable localScope(&scope);
                for (size_t i = 0; i < params.size(); ++i)
                    localScope.set(params[i], arguments[i]->evaluate(scope));
                try {
                    return userFunc->getBody()->evaluate(localScope);
                } catch (const ReturnException& e) {
                    return e.getValue();
                }
            }
        }
    }
    throw std::runtime_error("Unknown function: '" + name + "'");
}

Value UserFunctionNode::evaluate(SymbolTable&) {
    // NOTE: Function definitions are handled by the parser; nothing to do at runtime.
    return Value();
}

} // namespace jeve 