#pragma once

#include "../ASTNode.hpp"
#include "../Forward.hpp"

namespace jeve {

class DebugGCNode : public ASTNode {
private:
    GarbageCollector* gc;

public:
    DebugGCNode(GarbageCollector* gc = nullptr) : gc(gc) {}

    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "DebugGCNode"; }
};

class CleanGCNode : public ASTNode {
private:
    GarbageCollector* gc;

public:
    CleanGCNode(GarbageCollector* gc = nullptr) : gc(gc) {}

    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "CleanGCNode"; }
};

} // namespace jeve 