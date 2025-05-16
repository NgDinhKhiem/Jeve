#include "Object.hpp"
#include "ObjectPool.hpp"
#include "ASTNode.hpp"
#include "ast/OperatorNodes.hpp"
#include "ast/IONodes.hpp"
#include "ast/ControlFlowNodes.hpp"
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>

namespace jeve {

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
            while (!markStack.empty()) {
        Object* obj = markStack.top();
        markStack.pop();
        
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
    if (isCollecting) return;
    isCollecting = true;
    
    // Mark phase - only mark objects that are currently in use
    for (auto* obj : objects) {
        if (obj->getRefCount() > 0) {
                mark(obj);
        }
            }
            
    // Process mark stack to mark all reachable objects
        processMarkStack();
        
    // Sweep phase: remove unmarked objects
    auto it = objects.begin();
    while (it != objects.end()) {
        if (!(*it)->isMarked()) {
            // Object is not reachable, remove it
            Object* obj = *it;
            it = objects.erase(it);
            if (obj->getPool()) {
                obj->getPool()->release(obj);
            }
            delete obj;
            } else {
            (*it)->unmark();
                ++it;
            }
    }
    
    isCollecting = false;
    
    // Log memory usage after collection
    logger->logMemoryUsage(getObjectCount(), getHeapUsage(), initialHeap, maxHeap);
}

bool GarbageCollector::shouldCollect() const {
    // More conservative threshold (80% instead of 70%)
    size_t threshold = maxHeap * 0.8;
    size_t usage = getHeapUsage();
    
    // Also collect if we have too many objects
    return usage > threshold || objects.size() > 10000;
}

void GarbageCollector::checkAndCollect() {
    std::thread([this]() {
        if (!isCollecting && shouldCollect()) {
            collect();
        }
    }).detach();
}

} // namespace jeve 