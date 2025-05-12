#pragma once

#include "Forward.hpp"
#include <vector>
#include <stack>
#include <memory>
#include <algorithm>

namespace jeve {

// Forward declarations
class Object;
class ASTNode;
class BinaryOpNode;
class PrintNode;
class StatementNode;

class Object {
private:
    size_t refCount;
    bool marked;

public:
    Object() : refCount(0), marked(false) {}
    virtual ~Object() = default;

    void incrementRefCount() { refCount++; }
    void decrementRefCount() { if (refCount > 0) refCount--; }
    size_t getRefCount() const { return refCount; }
    bool isMarked() const { return marked; }
    void mark() { marked = true; }
    void unmark() { marked = false; }
};

class GarbageCollector {
private:
    std::vector<Object*> objects;
    std::stack<Object*> markStack;
    bool isCollecting;

public:
    GarbageCollector() : isCollecting(false) {}
    ~GarbageCollector() { collect(); }

    template<typename T, typename... Args>
    T* createObject(Args&&... args) {
        T* obj = new T(std::forward<Args>(args)...);
        objects.push_back(obj);
        return obj;
    }

    void decrementRefCount(Object* obj) {
        if (obj && !isCollecting) {
            obj->decrementRefCount();
            if (obj->getRefCount() == 0) {
                collect();
            }
        }
    }

    void mark(Object* obj);
    void collect();
};

template<typename T>
class Ref {
private:
    T* ptr;
    GarbageCollector* gc;

public:
    Ref() : ptr(nullptr), gc(nullptr) {}
    Ref(T* p, GarbageCollector* g) : ptr(p), gc(g) {
        if (ptr) ptr->incrementRefCount();
    }
    
    ~Ref() { 
        if (ptr) gc->decrementRefCount(ptr);
    }
    
    Ref(const Ref& other) : ptr(other.ptr), gc(other.gc) {
        if (ptr) ptr->incrementRefCount();
    }
    
    template<typename U>
    Ref(const Ref<U>& other) : ptr(other.get()), gc(other.getGC()) {
        if (ptr) ptr->incrementRefCount();
    }
    
    Ref& operator=(const Ref& other) {
        if (this != &other) {
            if (ptr) gc->decrementRefCount(ptr);
            ptr = other.ptr;
            gc = other.gc;
            if (ptr) ptr->incrementRefCount();
        }
        return *this;
    }
    
    template<typename U>
    Ref& operator=(const Ref<U>& other) {
        if (ptr) gc->decrementRefCount(ptr);
        ptr = other.get();
        gc = other.getGC();
        if (ptr) ptr->incrementRefCount();
        return *this;
    }
    
    T* get() const { return ptr; }
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    GarbageCollector* getGC() const { return gc; }
    
    explicit operator bool() const { return ptr != nullptr; }
    
    bool operator==(std::nullptr_t) const { return ptr == nullptr; }
    bool operator!=(std::nullptr_t) const { return ptr != nullptr; }
};

} // namespace jeve 