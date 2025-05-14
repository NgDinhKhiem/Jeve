#pragma once

#include "../Object.hpp"
#include "../SymbolTable.hpp"
#include "../Value.hpp"
#include <string>

namespace jeve {

class ASTNode : public Object {
public:
    virtual ~ASTNode() = default;
    virtual Value evaluate(SymbolTable& scope) = 0;
    virtual std::string toString() const = 0;
};

} // namespace jeve 