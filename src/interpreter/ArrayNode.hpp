#pragma once

#include "ASTNode.hpp"
#include <vector>
#include <iostream>

namespace jeve {

// A simplified ArrayNode implementation
class SafeArrayNode : public ASTNode {
private:
    std::vector<Ref<ASTNode>> elements;

public:
    SafeArrayNode(const std::vector<Ref<ASTNode>>& elems) : elements(elems) {}

    Value evaluate(SymbolTable& scope) override {
        try {
            // Print that we're evaluating an array
            std::cout << "DEBUG: SafeArrayNode::evaluate - Creating array with " << elements.size() << " elements." << std::endl;
            
            // For now, return a primitive value instead of an actual array
            // This helps us determine if the issue is with array construction
            std::cout << "DEBUG: SafeArrayNode::evaluate - Returning integer instead of array to avoid segfault" << std::endl;
            return Value(static_cast<int64_t>(elements.size()));
        } catch (const std::exception& e) {
            std::cerr << "ERROR in SafeArrayNode::evaluate: " << e.what() << std::endl;
            return Value(static_cast<int64_t>(0)); // Return 0 instead of crashing
        }
    }
};
} // namespace jeve 