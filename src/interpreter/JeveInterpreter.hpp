#pragma once

#include "ASTNode.hpp"
#include "Forward.hpp"
#include "Value.hpp"
#include "Object.hpp"
#include "SymbolTable.hpp"
#include "GarbageCollector.hpp"
#include <stack>
#include <string>
#include <memory>
#include <iostream>

namespace jeve {

class JeveInterpreter {
private:
    GarbageCollector gc;
    std::unique_ptr<SymbolTable> globalScope;
    std::stack<std::unique_ptr<SymbolTable>> scopeStack;

public:
    JeveInterpreter(size_t initialHeap = 1 * 1024 * 1024, size_t maxHeap = 64 * 1024 * 1024) 
        : gc(initialHeap, maxHeap), globalScope(std::make_unique<SymbolTable>()) {
        scopeStack.push(std::make_unique<SymbolTable>(globalScope.get()));
        gc.setInterpreter(this);
    }

    void interpret(const std::string& code);

    template<typename T, typename... Args>
    Ref<T> createObject(Args&&... args) {
        // Create object only when needed during interpretation
        T* obj = gc.createObject<T>(std::forward<Args>(args)...);
        if (g_jeve_debug) {
            std::cout << "[DEBUG] Created object of type " << typeid(T).name() << std::endl;
        }
        return Ref<T>(obj);
    }

    GarbageCollector& getGC() { return gc; }
    SymbolTable& getCurrentScope() { return *scopeStack.top(); }
    SymbolTable* getGlobalScope() { return globalScope.get(); }
};

} // namespace jeve 