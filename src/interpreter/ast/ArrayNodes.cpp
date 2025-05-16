#include "ArrayNodes.hpp"
#include "BasicNodes.hpp"
#include "../Forward.hpp"
#include "../JeveInterpreter.hpp"
#include "../Object.hpp"
#include "../ObjectPool.hpp"
#include "../Value.hpp"
#include "../SymbolTable.hpp"
#include "../ASTNode.hpp"
#include "../GarbageCollector.hpp"
#include <stdexcept>
#include <iostream>

namespace jeve {

Value ArrayNode::evaluate(SymbolTable& scope) {
    std::vector<Value> result;
    for (const auto& elem : elements) {
        result.push_back(elem->evaluate(scope));
    }
    // Get the GC from the scope (assume scope has a getGC or pass it in)
    // For now, use global GC if available, or refactor as needed
    // This is a placeholder; you may need to pass the GC or ObjectPool explicitly
    extern jeve::GarbageCollector* g_jeve_gc;
    ObjectPool* pool = g_jeve_gc ? g_jeve_gc->getObjectPool() : nullptr;
    return Value(result, pool);
}

Value ArrayAccessNode::evaluate(SymbolTable& scope) {
    Value arr = array->evaluate(scope);
    Value idx = index->evaluate(scope);
    
    if (arr.getType() != Value::Type::Array) {
        throw std::runtime_error("Cannot index into non-array value");
    }
    
    if (idx.getType() != Value::Type::Integer) {
        throw std::runtime_error("Array index must be an integer");
    }
    
    int64_t index = idx.getInteger();
    const auto& elements = arr.getArray();
    
    if (index < 0 || static_cast<size_t>(index) >= elements.size()) {
        throw std::runtime_error("Array index out of bounds");
    }
    
    return elements[index];
}

Value ArrayAssignmentNode::evaluate(SymbolTable& scope) {
    // Try to update the array in the symbol table if possible
    if (auto* idNode = dynamic_cast<IdentifierNode*>(array.get())) {
        Value& arrRef = scope.getMutable(idNode->getName());
        Value idx = index->evaluate(scope);
        Value val = value->evaluate(scope);
        if (arrRef.getType() != Value::Type::Array) {
            throw std::runtime_error("Cannot index into non-array value");
        }
        if (idx.getType() != Value::Type::Integer) {
            throw std::runtime_error("Array index must be an integer");
        }
        int64_t indexVal = idx.getInteger();
        auto& elements = arrRef.getArray();
        if (indexVal < 0 || static_cast<size_t>(indexVal) >= elements.size()) {
            throw std::runtime_error("Array index out of bounds");
        }
        if (interpreter) {
            switch (val.getType()) {
                case Value::Type::Integer:
                    elements[indexVal] = Value(Ref<Object>(interpreter->createObject<NumberNode>(val.getInteger())));
                    break;
                case Value::Type::Float:
                    elements[indexVal] = Value(Ref<Object>(interpreter->createObject<NumberNode>(val.getFloat())));
                    break;
                case Value::Type::String:
                    elements[indexVal] = Value(Ref<Object>(interpreter->createObject<StringNode>(val.getString())));
                    break;
                case Value::Type::Boolean:
                    elements[indexVal] = Value(Ref<Object>(interpreter->createObject<BooleanNode>(val.getBoolean())));
                    break;
                case Value::Type::Array:
                    elements[indexVal] = val; // Arrays are already GC-managed
                    break;
                default:
                    elements[indexVal] = val;
                    break;
            }
        } else {
            elements[indexVal] = val;
        }
        return val;
    }
    // Fallback: evaluate as before (for arr[0][1] = x, etc.)
    Value arr = array->evaluate(scope);
    Value idx = index->evaluate(scope);
    Value val = value->evaluate(scope);
    if (arr.getType() != Value::Type::Array) {
        throw std::runtime_error("Cannot index into non-array value");
    }
    if (idx.getType() != Value::Type::Integer) {
        throw std::runtime_error("Array index must be an integer");
    }
    int64_t indexVal = idx.getInteger();
    auto& elements = arr.getArray();
    if (indexVal < 0 || static_cast<size_t>(indexVal) >= elements.size()) {
        throw std::runtime_error("Array index out of bounds");
    }
    if (interpreter) {
        switch (val.getType()) {
            case Value::Type::Integer:
                elements[indexVal] = Value(Ref<Object>(interpreter->createObject<NumberNode>(val.getInteger())));
                break;
            case Value::Type::Float:
                elements[indexVal] = Value(Ref<Object>(interpreter->createObject<NumberNode>(val.getFloat())));
                break;
            case Value::Type::String:
                elements[indexVal] = Value(Ref<Object>(interpreter->createObject<StringNode>(val.getString())));
                break;
            case Value::Type::Boolean:
                elements[indexVal] = Value(Ref<Object>(interpreter->createObject<BooleanNode>(val.getBoolean())));
                break;
            case Value::Type::Array:
                elements[indexVal] = val; // Arrays are already GC-managed
                break;
            default:
                elements[indexVal] = val;
                break;
        }
    } else {
        elements[indexVal] = val;
    }
    return val;
}

} // namespace jeve 