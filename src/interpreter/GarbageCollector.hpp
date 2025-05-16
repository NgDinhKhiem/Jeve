#pragma once

#include "Object.hpp"
#include "ObjectPool.hpp"
#include <vector>
#include <stack>
#include <memory>
#include <string>
#include <chrono>
#include <fstream>
#include <iomanip>

extern bool g_jeve_debug;

namespace jeve {

// Global GC pointer for use across the codebase
extern GarbageCollector* g_jeve_gc;

class MemoryLogger {
private:
    std::ofstream logFile;
    bool isEnabled;
    size_t processCount;

public:
    MemoryLogger(const std::string& filename = "memory_usage.csv", bool enabled = true) 
        : isEnabled(enabled), processCount(0) {
        if (enabled) {
            logFile.open(filename);
            if (!logFile.is_open()) {
                throw std::runtime_error("Could not open memory log file: " + filename);
            }
            // Write CSV header
            logFile << "ProcessCount,ObjectCount,HeapUsage,InitialHeap,MaxHeap\n";
        }
    }

    ~MemoryLogger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    void logMemoryUsage(size_t objectCount, size_t heapUsage, size_t initialHeap, size_t maxHeap) {
        if (!isEnabled || !logFile.is_open()) return;

        processCount++;
        
        logFile << processCount << ","
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

class GarbageCollector {
private:
    std::vector<Object*> objects;
    std::stack<Object*> markStack;
    bool isCollecting;
    size_t initialHeap;
    size_t maxHeap;
    std::unique_ptr<MemoryLogger> logger;
    ObjectPool objectPool;

public:
    GarbageCollector(size_t initialHeapSize = 1024 * 1024, 
                    size_t maxHeapSize = 64 * 1024 * 1024,
                    const std::string& logFile = "memory_usage.csv")
        : isCollecting(false), 
          initialHeap(initialHeapSize), 
          maxHeap(maxHeapSize),
          logger(std::make_unique<MemoryLogger>(logFile)) {}

    ~GarbageCollector() { 
        try {
            collect();
        } catch (...) {
            std::cerr << "Error during final garbage collection" << std::endl;
        }
    }

    template<typename T, typename... Args>
    T* createObject(Args&&... args) {
        // Calculate current memory usage
        size_t currentUsage = getHeapUsage();
        
        // Check if adding a new object would exceed the maximum heap size
        if (currentUsage >= maxHeap) {
            // Try an emergency collection
            collect();
            // Check again after collection
            currentUsage = getHeapUsage();
            if (currentUsage >= maxHeap) {
                std::cout << "Memory limit reached! Current usage: " << currentUsage 
                          << " bytes, Max heap: " << maxHeap << " bytes" << std::endl;
                printStats();
                throw std::runtime_error("Out of memory: max heap size reached");
            }
        }

        // Create the object using the pool
        T* obj = objectPool.acquire<T>(std::forward<Args>(args)...);
        obj->setPool(&objectPool);
        objects.push_back(obj);
        
        // Log memory usage
        logger->logMemoryUsage(getObjectCount(), getHeapUsage(), initialHeap, maxHeap);
        
        return obj;
    }

    void mark(Object* obj);
    void processMarkStack();
    void collect();
    bool shouldCollect() const;
    void checkAndCollect();

    // Memory usage reporting
    size_t getObjectCount() const { return objects.size(); }

    // Improved heap usage calculation
    size_t getHeapUsage() const { 
        // Base usage from object pointers
        size_t usage = objects.size() * sizeof(Object*);
        
        // Add estimated size of each object
        usage += objects.size() * 32;
        
        // Add stack memory usage
        usage += markStack.size() * sizeof(Object*);
        
        return usage;
    }

    size_t getInitialHeap() const { return initialHeap; }
    size_t getMaxHeap() const { return maxHeap; }

    void printStats() const {
        if (g_jeve_debug) {
            std::cout << "[GC] Objects: " << getObjectCount()
                      << ", Heap usage: " << getHeapUsage() << " bytes"
                      << ", Initial heap: " << getInitialHeap() << " bytes"
                      << ", Max heap: " << getMaxHeap() << " bytes" << std::endl;
        }
        objectPool.printStats();
    }

    // Memory logging control
    void enableLogging() { logger->enable(); }
    void disableLogging() { logger->disable(); }
    bool isLoggingEnabled() const { return logger->isLoggingEnabled(); }

    ObjectPool* getObjectPool() { return &objectPool; }
};

} // namespace jeve 