#pragma once

#include "Forward.hpp"
#include "SymbolTable.hpp"
#include "Value.hpp"
#include "Object.hpp"
#include <string>
#include <iostream>
#include <regex>
#include <vector>
#include <memory>

namespace jeve {

// Forward declarations
class GarbageCollector;

class ASTNode : public Object {
public:
    virtual ~ASTNode() = default;
    virtual Value evaluate(SymbolTable& scope) = 0;
};

} // namespace jeve 