#pragma once

#include "Object.hpp"
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include <typeinfo>

extern bool g_jeve_debug;

namespace jeve {

class ObjectPool {
private:
    std::vector<Object*> objects;
    size_t maxSize;
    size_t currentSize;

public:
    ObjectPool(size_t max = 16 * 1024 * 1024)
        : maxSize(max), currentSize(0) {}

    ~ObjectPool() {
        // Don't delete objects here - they are managed by reference counting
        objects.clear();
    }

    template<typename T, typename... Args>
    T* acquire(Args&&... args) {
        if (currentSize >= maxSize) {
            if (g_jeve_debug) {
                std::cout << "[ObjectPool] Size limit reached! Current: " << currentSize 
                          << ", Max: " << maxSize << std::endl;
            }
            throw std::runtime_error("Object pool size limit reached");
        }
        T* obj = new T(std::forward<Args>(args)...);
        objects.push_back(obj);
        currentSize++;
        if (g_jeve_debug) {
            std::cout << "[ObjectPool] Created " << typeid(T).name() 
                      << " (Total objects: " << currentSize << ")" << std::endl;
        }
        return obj;
    }

    void release(Object* obj) {
        if (!obj) return;
        auto it = std::find(objects.begin(), objects.end(), obj);
        if (it != objects.end()) {
            objects.erase(it);
            currentSize--;
            // Don't delete here - let the garbage collector handle it
        }
    }

    void printStats() const {
        if (g_jeve_debug) {
            std::cout << "[ObjectPool] Current size: " << currentSize 
                      << ", Max size: " << maxSize << std::endl;
        }
    }
};

} // namespace jeve 