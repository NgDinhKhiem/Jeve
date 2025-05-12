#pragma once

#include "Forward.hpp"
#include "Value.hpp"
#include "Object.hpp"
#include "SymbolTable.hpp"
#include "ASTNode.hpp"
#include <stack>
#include <string>
#include <memory>

namespace jeve {

class JeveInterpreter {
private:
    GarbageCollector gc;
    std::unique_ptr<SymbolTable> globalScope;
    std::stack<std::unique_ptr<SymbolTable>> scopeStack;

public:
    JeveInterpreter() : globalScope(std::make_unique<SymbolTable>()) {
        scopeStack.push(std::make_unique<SymbolTable>(globalScope.get()));
    }

    void interpret(const std::string& code);

    template<typename T, typename... Args>
    Ref<T> createObject(Args&&... args) {
        T* obj = gc.createObject<T>(std::forward<Args>(args)...);
        return Ref<T>(obj, &gc);
    }

    GarbageCollector& getGC() { return gc; }
    SymbolTable& getCurrentScope() { return *scopeStack.top(); }
};

} // namespace jeve 