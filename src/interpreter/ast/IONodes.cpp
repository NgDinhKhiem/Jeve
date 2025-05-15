#include "IONodes.hpp"
#include <iostream>
#include <stdexcept>

namespace jeve {

Value PrintNode::evaluate(SymbolTable& scope) {
    Value result = expression->evaluate(scope);
    std::cout << result.toString() << std::endl;
    return result;
}

Value InputNode::evaluate(SymbolTable& scope) {
    (void)scope;
    std::string input;
    std::getline(std::cin, input);
    
    if (type.empty()) {
        // Try to infer type
        if (input == "true" || input == "false") {
            return Value(input == "true");
        }
        
        try {
            if (input.find('.') != std::string::npos) {
                return Value(std::stod(input));
            } else {
                return Value(static_cast<int64_t>(std::stoll(input)));
            }
        } catch (...) {
            // If conversion fails, treat as string
            return Value(input);
        }
    }
    
    // Handle explicit type conversion
    if (type == "int") {
        return Value(static_cast<int64_t>(std::stod(input)));
    } else if (type == "float") {
        return Value(static_cast<float>(std::stod(input)));
    } else if (type == "bool") {
        return Value(input == "true");
    } else if (type == "string") {
        return Value(input);
    }
    
    throw std::runtime_error("Unsupported type: " + type);
}

} // namespace jeve 