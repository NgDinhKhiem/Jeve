#pragma once

#include "Forward.hpp"
#include "Value.hpp"
#include <unordered_map>
#include <string>
#include <stdexcept>

namespace jeve {

class SymbolTable {
private:
    std::unordered_map<std::string, Value> symbols;
    SymbolTable* parent;

public:
    SymbolTable(SymbolTable* p = nullptr) : parent(p) {}

    void set(const std::string& name, const Value& value) {
        symbols[name] = value;
    }

    void set(const std::string& name, Value&& value) {
        symbols[name] = std::move(value);
    }

    const Value& get(const std::string& name) const {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return it->second;
        }
        if (parent) {
            return parent->get(name);
        }
        static const Value empty;
        return empty;
    }

    bool has(const std::string& name) const {
        return symbols.find(name) != symbols.end() || 
               (parent && parent->has(name));
    }

    Value& getMutable(const std::string& name) {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return it->second;
        }
        if (parent) {
            return parent->getMutable(name);
        }
        throw std::runtime_error("Variable not found: " + name);
    }
};

} // namespace jeve 