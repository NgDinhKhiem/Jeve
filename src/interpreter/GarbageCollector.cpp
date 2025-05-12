#include "Object.hpp"
#include "ASTNode.hpp"
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>

namespace jeve {

// Deadlock detection timeout in milliseconds
const long long DEADLOCK_TIMEOUT_MS = 1000;

void GarbageCollector::mark(Object* obj) {
    if (!obj || obj->isMarked()) {
        return;
    }
    
    obj->mark();
    
    // We'll use a non-recursive approach to avoid stack overflow
    if (auto* node = dynamic_cast<ASTNode*>(obj)) {
        // Add to mark stack for later processing
        markStack.push(obj);
    }
}

void GarbageCollector::processMarkStack() {
    // Track time to detect potential deadlocks
    auto startTime = std::chrono::steady_clock::now();
    size_t processedCount = 0;
    
    while (!markStack.empty() && !deadlockDetected.load()) {
        // Check for potential deadlock
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - startTime).count();
            
        // If we've been processing too long without making progress, we might be in a deadlock
        if (elapsedMs > DEADLOCK_TIMEOUT_MS && processedCount < 10) {
            std::cout << "[WARNING] Potential deadlock detected in garbage collection. Recovering..." << std::endl;
            
            // Set deadlock detected flag
            setDeadlockDetected(true);
            
            // Clear the mark stack to break the deadlock
            while (!markStack.empty()) {
                markStack.pop();
            }
            
            // Force collection to finish
            return;
        }
        
        Object* obj = markStack.top();
        markStack.pop();
        processedCount++;
        
        // Process based on object type
        if (auto* binaryOp = dynamic_cast<BinaryOpNode*>(obj)) {
            if (auto* left = binaryOp->getLeft()) {
                mark(left);
            }
            if (auto* right = binaryOp->getRight()) {
                mark(right);
            }
        }
        else if (auto* printNode = dynamic_cast<PrintNode*>(obj)) {
            if (auto* expr = printNode->getExpression()) {
                mark(expr);
            }
        }
        else if (auto* statementNode = dynamic_cast<StatementNode*>(obj)) {
            if (auto* stmt = statementNode->getStatement()) {
                mark(stmt);
            }
            if (auto* next = statementNode->getNext()) {
                mark(next);
            }
        }
    }
}

void GarbageCollector::collect() {
    // Reset deadlock detection state
    resetDeadlockDetection();
    
    // Use a timeout for the entire collection process
    auto startTime = std::chrono::steady_clock::now();
    
    // Simple guard against recursive collection
    if (isCollecting) {
        return;
    }
    
    isCollecting = true;
    
    // Clear the mark stack first
    while (!markStack.empty()) {
        markStack.pop();
    }
    
    try {
        // Unmark all objects
        for (Object* obj : objects) {
            if (obj) {
                obj->unmark();
            }
            
            // Check for timeout
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                currentTime - startTime).count();
                
            if (elapsedMs > DEADLOCK_TIMEOUT_MS || deadlockDetected.load()) {
                std::cout << "[WARNING] Garbage collection taking too long. Aborting." << std::endl;
                isCollecting = false;
                return;
            }
        }
        
        // Mark all objects with references
        for (Object* obj : objects) {
            if (obj && obj->getRefCount() > 0) {
                mark(obj);
            }
            
            // Check for timeout
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                currentTime - startTime).count();
                
            if (elapsedMs > DEADLOCK_TIMEOUT_MS || deadlockDetected.load()) {
                std::cout << "[WARNING] Garbage collection taking too long. Aborting." << std::endl;
                isCollecting = false;
                return;
            }
        }
        
        // Process the mark stack
        processMarkStack();
        
        // If a deadlock was detected, skip the sweep phase
        if (deadlockDetected.load()) {
            std::cout << "[WARNING] Skipping sweep phase due to deadlock." << std::endl;
            isCollecting = false;
            return;
        }
        
        // Count before sweep
        size_t beforeCount = objects.size();
        
        // Create a list of objects to delete
        std::vector<Object*> objectsToDelete;
        
        // First pass: identify objects to delete
        for (auto it = objects.begin(); it != objects.end();) {
            Object* obj = *it;
            
            // Check for timeout
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                currentTime - startTime).count();
                
            if (elapsedMs > DEADLOCK_TIMEOUT_MS * 2 || deadlockDetected.load()) {
                std::cout << "[WARNING] Garbage collection timeout during sweep. Aborting." << std::endl;
                isCollecting = false;
                return;
            }
            
            if (!obj || !obj->isMarked()) {
                // Add to deletion list
                if (obj) objectsToDelete.push_back(obj);
                it = objects.erase(it);
            } else {
                ++it;
            }
        }
        
        // Second pass: now delete the objects
        for (Object* obj : objectsToDelete) {
            try {
                delete obj;
            } catch (const std::exception& e) {
                std::cout << "[ERROR] Exception during object deletion: " << e.what() << std::endl;
            }
        }
        
        // Count after sweep
        size_t afterCount = objects.size();
        size_t freedCount = beforeCount - afterCount;
        
        if (freedCount > 0) {
            std::cout << "[GC] Freed " << freedCount << " objects" << std::endl;
        }
        
        // Log memory usage
        if (logger) {
            logger->logMemoryUsage(getObjectCount(), getHeapUsage(), initialHeap, maxHeap);
        }
    }
    catch (const std::exception& e) {
        std::cout << "[ERROR] Exception during garbage collection: " << e.what() << std::endl;
        std::cout << "[RECOVERY] Continuing execution..." << std::endl;
        setDeadlockDetected(true);
    }
    
    isCollecting = false;
}

bool GarbageCollector::shouldCollect() const {
    // More conservative threshold (80% instead of 70%)
    size_t threshold = maxHeap * 0.8;
    size_t usage = getHeapUsage();
    
    // Also collect if we have too many objects
    return usage > threshold || objects.size() > 10000;
}

void GarbageCollector::checkAndCollect() {
    // Don't collect if we've detected a deadlock recently
    if (deadlockDetected.load()) {
        std::cout << "[WARNING] Skipping garbage collection due to recent deadlock." << std::endl;
        // Reset the flag after a timeout
        resetDeadlockDetection();
        return;
    }
    
    // Add deadlock protection
    std::thread([this]() {
        if (!isCollecting && shouldCollect()) {
            auto startTime = std::chrono::steady_clock::now();
            
            // Start collection
            collect();
            
            // Check if collection took too long
            auto endTime = std::chrono::steady_clock::now();
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                endTime - startTime).count();
                
            if (elapsedMs > DEADLOCK_TIMEOUT_MS * 3) {
                std::cout << "[WARNING] Garbage collection took " << elapsedMs 
                          << "ms, which is longer than expected. Potential performance issue." << std::endl;
            }
        }
    }).detach();
}

} // namespace jeve 