#pragma once

#include "Forward.hpp"
#include "ObjectPool.hpp"
#include "GarbageCollector.hpp"
#include <vector>
#include <stack>
#include <memory>
#include <algorithm>
#include <iostream>
#include <string>
#include <cctype>
#include <atomic>

namespace jeve {

// Forward declarations
class Object;
class ASTNode;
class BinaryOpNode;
class PrintNode;
class StatementNode;

// Utility function to parse memory size strings (e.g., "64m", "1g", "512k")
inline size_t parseMemorySize(const std::string& sizeStr) {
    size_t multiplier = 1;
    size_t value = 0;
    size_t i = 0;
    
    // Parse the numeric part
    while (i < sizeStr.length() && std::isdigit(sizeStr[i])) {
        value = value * 10 + (sizeStr[i] - '0');
        i++;
    }
    
    // Parse the suffix
    if (i < sizeStr.length()) {
        char suffix = std::tolower(sizeStr[i]);
        switch (suffix) {
            case 'k': multiplier = 1024; break;
            case 'm': multiplier = 1024 * 1024; break;
            case 'g': multiplier = 1024 * 1024 * 1024; break;
            default: throw std::runtime_error("Invalid memory size suffix: " + sizeStr);
        }
    }
    
    return value * multiplier;
}

class Object {
protected:
    bool marked = false;
    std::atomic<int> refCount{0};  // Use atomic for thread safety
    ObjectPool* pool;

public:
    Object(ObjectPool* p = nullptr) : pool(p) {}
    virtual ~Object() = default;

    void incrementRefCount() {
        refCount++;
    }
    
    void decrementRefCount() {
        if (refCount.fetch_sub(1) == 1) {
            // Only delete if we were the last reference
            if (pool) {
                pool->release(this);
            }
            // Don't delete here - let the garbage collector handle it
            // This prevents double deletion
        }
    }
    
    int getRefCount() const { return refCount; }
    
    void mark() { marked = true; }
    void unmark() { marked = false; }
    bool isMarked() const { return marked; }
    
    void setPool(ObjectPool* p) { pool = p; }
    ObjectPool* getPool() const { return pool; }

    virtual std::string toString() const = 0;
};

template<typename T>
class Ref {
private:
    T* ptr;

public:
    Ref() : ptr(nullptr) {}
    
    explicit Ref(T* p) : ptr(p) {
        if (ptr) ptr->incrementRefCount();
    }
    
    Ref(const Ref& other) : ptr(other.ptr) {
        if (ptr) ptr->incrementRefCount();
    }
    
    Ref(Ref&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }
    
    template<typename U, typename = std::enable_if_t<std::is_convertible<U*, T*>::value>>
    Ref(const Ref<U>& other) : ptr(other.get()) {
        if (ptr) ptr->incrementRefCount();
    }
    
    ~Ref() {
        if (ptr) ptr->decrementRefCount();
    }
    
    Ref& operator=(const Ref& other) {
        if (this != &other) {
            if (ptr) ptr->decrementRefCount();
            ptr = other.ptr;
            if (ptr) ptr->incrementRefCount();
        }
        return *this;
    }
    
    Ref& operator=(Ref&& other) noexcept {
        if (this != &other) {
            if (ptr) ptr->decrementRefCount();
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }
    
    template<typename U, typename = std::enable_if_t<std::is_convertible<U*, T*>::value>>
    Ref& operator=(const Ref<U>& other) {
        if (ptr) ptr->decrementRefCount();
        ptr = other.get();
        if (ptr) ptr->incrementRefCount();
        return *this;
    }
    
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    T* get() const { return ptr; }
    
    bool operator==(const Ref& other) const { return ptr == other.ptr; }
    bool operator!=(const Ref& other) const { return ptr != other.ptr; }
    bool operator==(std::nullptr_t) const { return ptr == nullptr; }
    bool operator!=(std::nullptr_t) const { return ptr != nullptr; }
    
    explicit operator bool() const { return ptr != nullptr; }
};

} // namespace jeve 