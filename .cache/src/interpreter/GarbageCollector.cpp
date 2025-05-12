#include "Object.hpp"
#include "ASTNode.hpp"
#include <algorithm>

namespace jeve {

void GarbageCollector::mark(Object* obj) {
    if (obj && !obj->isMarked()) {
        obj->mark();
        markStack.push(obj);
        
        // Mark all referenced objects
        if (auto* node = dynamic_cast<ASTNode*>(obj)) {
            // Mark child nodes
            if (auto* binaryOp = dynamic_cast<BinaryOpNode*>(node)) {
                if (binaryOp->getLeft()) mark(binaryOp->getLeft());
                if (binaryOp->getRight()) mark(binaryOp->getRight());
            }
            else if (auto* printNode = dynamic_cast<PrintNode*>(node)) {
                if (printNode->getExpression()) mark(printNode->getExpression());
            }
            else if (auto* statementNode = dynamic_cast<StatementNode*>(node)) {
                if (statementNode->getStatement()) mark(statementNode->getStatement());
                if (statementNode->getNext()) mark(statementNode->getNext());
            }
        }
    }
}

void GarbageCollector::collect() {
    if (isCollecting) return;
    isCollecting = true;

    // Mark phase
    for (Object* obj : objects) {
        if (obj->getRefCount() > 0) {
            mark(obj);
        }
    }

    // Process mark stack
    while (!markStack.empty()) {
        Object* obj = markStack.top();
        markStack.pop();
        // Additional marking logic if needed
    }

    // Sweep phase
    objects.erase(
        std::remove_if(objects.begin(), objects.end(),
            [](Object* obj) {
                if (!obj->isMarked()) {
                    delete obj;
                    return true;
                }
                obj->unmark();
                return false;
            }
        ),
        objects.end()
    );

    isCollecting = false;
}

} // namespace jeve 