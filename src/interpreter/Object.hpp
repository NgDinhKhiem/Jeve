#pragma once

#include "Forward.hpp"
#include <vector>
#include <stack>
#include <memory>
#include <algorithm>
#include <iostream>
#include <string>
#include <cctype>
#include <fstream>
#include <chrono>
#include <iomanip>
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

class MemoryLogger {
private:
    std::ofstream logFile;
    bool isEnabled;
    std::chrono::system_clock::time_point startTime;

public:
    MemoryLogger(const std::string& filename = "memory_usage.csv", bool enabled = true) 
        : isEnabled(enabled), startTime(std::chrono::system_clock::now()) {
        if (enabled) {
            logFile.open(filename);
            if (!logFile.is_open()) {
                throw std::runtime_error("Could not open memory log file: " + filename);
            }
            // Write CSV header
            logFile << "Timestamp,ElapsedMs,ObjectCount,HeapUsage,InitialHeap,MaxHeap\n";
        }
    }

    ~MemoryLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void logMemoryUsage(size_t objectCount, size_t heapUsage, size_t initialHeap, size_t maxHeap) {
        if (!isEnabled || !logFile.is_open()) return;

        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
        
        // Get current timestamp
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&time);
        
        logFile << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << ","
                << elapsed.count() << ","
                << objectCount << ","
                << heapUsage << ","
                << initialHeap << ","
                << maxHeap << "\n";
        
        logFile.flush();
    }

    void enable() { isEnabled = true; }
    void disable() { isEnabled = false; }
    bool isLoggingEnabled() const { return isEnabled; }
};

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
    size_t initialHeap;
    size_t maxHeap;
    std::unique_ptr<MemoryLogger> logger;
    
    // Add deadlock detection state
    std::atomic<bool> deadlockDetected;

public:
    GarbageCollector(size_t initialHeapSize = 1024 * 1024, 
                    size_t maxHeapSize = 64 * 1024 * 1024,
                    const std::string& logFile = "memory_usage.csv")
        : isCollecting(false), 
          initialHeap(initialHeapSize), 
          maxHeap(maxHeapSize),
          logger(std::make_unique<MemoryLogger>(logFile)),
          deadlockDetected(false) {}

    ~GarbageCollector() { 
        // Make sure we don't get stuck in the destructor
        try {
            collect();
        } catch (...) {
            std::cerr << "Error during final garbage collection" << std::endl;
        }
    }

    template<typename T, typename... Args>
    T* createObject(Args&&... args) {
        // Check if we should collect garbage before creating a new object
        checkAndCollect();
        
        // Calculate current memory usage
        size_t currentUsage = getHeapUsage();
        
        // Check if adding a new object would exceed the maximum heap size
        if (currentUsage >= maxHeap) {
            // Try an emergency collection
            collect();
            
            // Check again after collection
            currentUsage = getHeapUsage();
            if (currentUsage >= maxHeap) {
                // Print memory stats before throwing
                std::cout << "Memory limit reached! Current usage: " << currentUsage 
                          << " bytes, Max heap: " << maxHeap << " bytes" << std::endl;
                printStats();
                throw std::runtime_error("Out of memory: max heap size reached");
            }
        }
        
        // Create the object
        T* obj = new T(std::forward<Args>(args)...);
        objects.push_back(obj);
        
        // Log memory usage
        logger->logMemoryUsage(getObjectCount(), getHeapUsage(), initialHeap, maxHeap);
        
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
    void processMarkStack();
    void collect();
    bool shouldCollect() const;
    void checkAndCollect();
    
    // Deadlock detection methods
    bool isDeadlockDetected() const { return deadlockDetected.load(); }
    void setDeadlockDetected(bool value) { deadlockDetected.store(value); }
    void resetDeadlockDetection() { deadlockDetected.store(false); }

    // Memory usage reporting
    size_t getObjectCount() const { return objects.size(); }

    // Improved heap usage calculation
    size_t getHeapUsage() const { 
        // Base usage from object pointers
        size_t usage = objects.size() * sizeof(Object*);
        
        // Add estimated size of each object (use a more conservative estimate)
        // This helps prevent overestimating memory usage which could trigger
        // unnecessary collections or out-of-memory errors
        usage += objects.size() * 32; // Reduced from 64 bytes per object
        
        // Add stack memory usage
        usage += markStack.size() * sizeof(Object*);
        
        return usage;
    }

    size_t getInitialHeap() const { return initialHeap; }
    size_t getMaxHeap() const { return maxHeap; }

    void printStats() const {
        std::cout << "[GC] Objects: " << getObjectCount()
                  << ", Heap usage: " << getHeapUsage() << " bytes"
                  << ", Initial heap: " << getInitialHeap() << " bytes"
                  << ", Max heap: " << getMaxHeap() << " bytes" << std::endl;
    }

    // Memory logging control
    void enableLogging() { logger->enable(); }
    void disableLogging() { logger->disable(); }
    bool isLoggingEnabled() const { return logger->isLoggingEnabled(); }
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