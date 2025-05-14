#include "ArrayFunctionNodes.hpp"
#include "BasicNodes.hpp"
#include <iostream>

namespace jeve {

Value ArrayNode::evaluate(SymbolTable& scope) {
    try {
        std::vector<Value> values;
        values.reserve(elements.size());
        
        for (const auto& elem : elements) {
            values.push_back(elem->evaluate(scope));
        }
        
        return Value(values);
    } catch (const std::exception& e) {
        std::cerr << "Error in ArrayNode::evaluate: " << e.what() << std::endl;
        return Value::createEmptyArray();
    }
}

Value ArrayAccessNode::evaluate(SymbolTable& scope) {
    Value arrayValue = array->evaluate(scope);
    Value indexValue = index->evaluate(scope);
    
    if (arrayValue.getType() != Value::Type::Array) {
        throw std::runtime_error("Cannot access non-array value with index");
    }
    
    if (indexValue.getType() != Value::Type::Integer) {
        throw std::runtime_error("Array index must be an integer");
    }
    
    int64_t idx = indexValue.getInteger();
    if (idx < 0) {
        throw std::runtime_error("Array index out of bounds: " + std::to_string(idx));
    }
    
    try {
        return arrayValue.at(static_cast<size_t>(idx));
    } catch (const std::out_of_range& e) {
        throw std::runtime_error("Array index out of bounds: " + std::to_string(idx));
    }
}

Value ArrayAssignmentNode::evaluate(SymbolTable& scope) {
    std::string varName;
    if (auto* idNode = dynamic_cast<IdentifierNode*>(array.get())) {
        varName = idNode->getName();
    } else {
        throw std::runtime_error("Cannot assign to non-identifier array");
    }
    
    Value arrayValue = scope.get(varName);
    Value indexValue = index->evaluate(scope);
    Value newValue = value->evaluate(scope);
    
    if (arrayValue.getType() != Value::Type::Array) {
        throw std::runtime_error("Cannot assign to non-array value");
    }
    
    if (indexValue.getType() != Value::Type::Integer) {
        throw std::runtime_error("Array index must be an integer");
    }
    
    int64_t idx = indexValue.getInteger();
    if (idx < 0) {
        throw std::runtime_error("Array index out of bounds: " + std::to_string(idx));
    }
    
    try {
        std::vector<Value>& arrayData = arrayValue.getArray();
        if (idx >= static_cast<int64_t>(arrayData.size())) {
            throw std::runtime_error("Array index out of bounds: " + std::to_string(idx));
        }
        
        arrayData[idx] = newValue;
        scope.set(varName, arrayValue);
        
        return newValue;
    } catch (const std::out_of_range& e) {
        throw std::runtime_error("Array index out of bounds: " + std::to_string(idx));
    }
}

Value FunctionCallNode::evaluate(SymbolTable& scope) {
    if (name == "print") {
        if (arguments.size() != 1) {
            throw std::runtime_error("print() expects exactly one argument");
        }
        return PrintNode(arguments[0]).evaluate(scope);
    }
    if (name == "input") {
        if (arguments.size() > 1) {
            throw std::runtime_error("input() expects at most one type argument");
        }
        std::string type = arguments.empty() ? "" : arguments[0]->evaluate(scope).getString();
        return InputNode(type).evaluate(scope);
    }
    throw std::runtime_error("Unknown function: " + name);
}

Value PrintNode::evaluate(SymbolTable& scope) {
    Value result = expression->evaluate(scope);
    if (result.getType() == Value::Type::String) {
        std::cout << result.getString() << std::endl;
    } else if (result.getType() == Value::Type::Integer) {
        std::cout << result.getInteger() << std::endl;
    } else if (result.getType() == Value::Type::Float) {
        std::cout << result.getFloat() << std::endl;
    } else if (result.getType() == Value::Type::Boolean) {
        std::cout << (result.getBoolean() ? "true" : "false") << std::endl;
    }
    return result;
}

Value InputNode::evaluate(SymbolTable&) {
    std::string input;
    std::getline(std::cin, input);
    
    if (type == "int") {
        try {
            return Value(std::stoll(input));
        } catch (...) {
            throw std::runtime_error("Invalid integer input");
        }
    } else if (type == "float") {
        try {
            return Value(std::stod(input));
        } catch (...) {
            throw std::runtime_error("Invalid float input");
        }
    } else if (type == "bool") {
        if (input == "true") return Value(true);
        if (input == "false") return Value(false);
        throw std::runtime_error("Invalid boolean input");
    }
    
    return Value(input);
}

Value DebugGCNode::evaluate(SymbolTable&) {
    if (gc) {
        gc->printStats();
    } else {
        std::cout << "[DebugGC] Garbage collector not available" << std::endl;
    }
    return Value();
}

Value CleanGCNode::evaluate(SymbolTable&) {
    if (gc) {
        gc->collect();
    }
    return Value();
}

} // namespace jeve 