#pragma once

#include "Object.hpp"
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>

namespace jeve {

class ObjectPool {
private:
    std::vector<Object*> objects;
    size_t maxSize;
    size_t currentSize;

public:
    ObjectPool(size_t max = 1024 * 1024)
        : maxSize(max), currentSize(0) {}

    ~ObjectPool() {
        // Don't delete objects here - they are managed by reference counting
        objects.clear();
    }

    template<typename T, typename... Args>
    T* acquire(Args&&... args) {
        if (currentSize >= maxSize) {
            throw std::runtime_error("Object pool size limit reached");
        }

        // Create object only when needed
        T* obj = new T(std::forward<Args>(args)...);
        objects.push_back(obj);
        currentSize++;
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
        std::cout << "[ObjectPool] Current size: " << currentSize 
                  << ", Max size: " << maxSize << std::endl;
    }
};

} // namespace jeve 