#include "ControlFlowNodes.hpp"

namespace jeve {

Value StatementNode::evaluate(SymbolTable& scope) {
    try {
        Value result = statement->evaluate(scope);
        if (next.get() != nullptr) {
            result = next->evaluate(scope);
        }
        return result;
    } catch (const ReturnException& e) {
        throw; // Propagate return up
    }
}

void BlockNode::addStatement(Ref<ASTNode> stmt) {
    Ref<StatementNode> node(new StatementNode(stmt, gc));
    if (first.get() == nullptr) {
        first = node;
        last = node;
    } else {
        last->setNext(node);
        last = node;
    }
}

Value BlockNode::evaluate(SymbolTable& scope) {
    if (first.get() == nullptr) {
        return Value();
    }
    try {
        return first->evaluate(scope);
    } catch (const ReturnException& e) {
        throw; // Propagate return up
    }
}

Value IfNode::evaluate(SymbolTable& scope) {
    Value cond = condition->evaluate(scope);
    if (cond.toBoolean()) {
        return thenBlock->evaluate(scope);
    } else if (elseBlock.get() != nullptr) {
        return elseBlock->evaluate(scope);
    }
    return Value();
}

Value WhileNode::evaluate(SymbolTable& scope) {
    Value result;
    while (true) {
        Value cond = condition->evaluate(scope);
        if (cond.getType() != Value::Type::Boolean) {
            throw std::runtime_error("Condition must be a boolean");
        }
        
        if (!cond.getBoolean()) {
            break;
        }
        
        result = body->evaluate(scope);
    }
    return result;
}

Value ForNode::evaluate(SymbolTable& scope) {
    Value startVal = start->evaluate(scope);
    Value endVal = end->evaluate(scope);
    Value stepVal = step.get() != nullptr ? step->evaluate(scope) : Value(static_cast<int64_t>(1));
    
    if (startVal.getType() != Value::Type::Integer || 
        endVal.getType() != Value::Type::Integer || 
        stepVal.getType() != Value::Type::Integer) {
        throw std::runtime_error("For loop requires integer values");
    }
    
    int64_t start = startVal.getInteger();
    int64_t end = endVal.getInteger();
    int64_t step = stepVal.getInteger();
    
    if (step == 0) {
        throw std::runtime_error("For loop step cannot be zero");
    }
    
    Value result;
    if (step > 0) {
        for (int64_t i = start; i <= end; i += step) {
            scope.set(varName, Value(i));
            result = body->evaluate(scope);
        }
    } else {
        for (int64_t i = start; i >= end; i += step) {
            scope.set(varName, Value(i));
            result = body->evaluate(scope);
        }
    }
    
    return result;
}

} // namespace jeve 