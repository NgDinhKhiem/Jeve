#include "GCNodes.hpp"
#include <iostream>

namespace jeve {

Value DebugGCNode::evaluate(SymbolTable& scope) {
    if (gc) {
        std::cout << "GC Stats:" << std::endl;
        std::cout << "  Objects: " << gc->getObjectCount() << std::endl;
        std::cout << "  Heap usage: " << gc->getHeapUsage() << " bytes" << std::endl;
        gc->printStats();
    }
    return Value();
}

Value CleanGCNode::evaluate(SymbolTable& scope) {
    if (gc) {
        gc->collect();
    }
    return Value();
}

} // namespace jeve 