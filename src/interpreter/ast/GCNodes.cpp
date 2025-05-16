#include "GCNodes.hpp"
#include <iostream>

extern bool g_jeve_debug;

namespace jeve {

Value DebugGCNode::evaluate(SymbolTable& scope) {
    if (gc && g_jeve_debug) {
        std::cout << "GC Stats (GC: " << gc << ", Pool: " << gc->getObjectPool() << "):" << std::endl;
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