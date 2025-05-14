#include "OperatorNodes.hpp"
#include <cmath>

namespace jeve {

Value BinaryOpNode::evaluate(SymbolTable& scope) {
    Value lval = left->evaluate(scope);
    Value rval = right->evaluate(scope);
    
    if (lval.getType() == Value::Type::Integer && rval.getType() == Value::Type::Integer) {
        int64_t l = lval.getInteger();
        int64_t r = rval.getInteger();
        
        if (op == "+") return Value(l + r);
        if (op == "-") return Value(l - r);
        if (op == "*") return Value(l * r);
        if (op == "/") {
            if (r == 0) throw std::runtime_error("Division by zero");
            return Value(l / r);
        }
        if (op == "%") {
            if (r == 0) throw std::runtime_error("Modulo by zero");
            return Value(l % r);
        }
        if (op == "==") return Value(l == r);
        if (op == "!=") return Value(l != r);
        if (op == "<") return Value(l < r);
        if (op == ">") return Value(l > r);
        if (op == "<=") return Value(l <= r);
        if (op == ">=") return Value(l >= r);
        if (op == "&") return Value(l != 0 && r != 0);  // Logical AND
        if (op == "|") return Value(l != 0 || r != 0);  // Logical OR
    } else if (lval.getType() == Value::Type::Float || rval.getType() == Value::Type::Float) {
        double l = (lval.getType() == Value::Type::Float) ? lval.getFloat() : static_cast<double>(lval.getInteger());
        double r = (rval.getType() == Value::Type::Float) ? rval.getFloat() : static_cast<double>(rval.getInteger());
        
        if (op == "+") return Value(l + r);
        if (op == "-") return Value(l - r);
        if (op == "*") return Value(l * r);
        if (op == "/") {
            if (r == 0.0) throw std::runtime_error("Division by zero");
            return Value(l / r);
        }
        if (op == "%") {
            if (r == 0.0) throw std::runtime_error("Modulo by zero");
            return Value(std::fmod(l, r));
        }
        if (op == "==") return Value(l == r);
        if (op == "!=") return Value(l != r);
        if (op == "<") return Value(l < r);
        if (op == ">") return Value(l > r);
        if (op == "<=") return Value(l <= r);
        if (op == ">=") return Value(l >= r);
        if (op == "&") return Value(l != 0.0 && r != 0.0);  // Logical AND
        if (op == "|") return Value(l != 0.0 || r != 0.0);  // Logical OR
    } else if (lval.getType() == Value::Type::String || rval.getType() == Value::Type::String) {
        std::string lstr = lval.toString();
        std::string rstr = rval.toString();
        
        if (op == "+") return Value(lstr + rstr);
        if (op == "==") return Value(lstr == rstr);
        if (op == "!=") return Value(lstr != rstr);
        if (op == "&") return Value(!lstr.empty() && !rstr.empty());  // Logical AND
        if (op == "|") return Value(!lstr.empty() || !rstr.empty());  // Logical OR
    } else if (lval.getType() == Value::Type::Boolean && rval.getType() == Value::Type::Boolean) {
        bool l = lval.getBoolean();
        bool r = rval.getBoolean();
        
        if (op == "==") return Value(l == r);
        if (op == "!=") return Value(l != r);
        if (op == "&") return Value(l && r);
        if (op == "|") return Value(l || r);
    } else if (lval.getType() == Value::Type::Array && rval.getType() == Value::Type::Array && op == "+") {
        const auto& leftArray = lval.getArray();
        const auto& rightArray = rval.getArray();
        
        std::vector<Value> result;
        result.reserve(leftArray.size() + rightArray.size());
        
        result.insert(result.end(), leftArray.begin(), leftArray.end());
        result.insert(result.end(), rightArray.begin(), rightArray.end());
        
        return Value(result);
    }
    
    // Handle mixed type logical operations
    if (op == "&" || op == "|") {
        bool l = lval.toBoolean();
        bool r = rval.toBoolean();
        if (op == "&") return Value(l && r);
        if (op == "|") return Value(l || r);
    }
    
    throw std::runtime_error("Invalid operation between types");
}

Value UnaryOpNode::evaluate(SymbolTable& scope) {
    Value val = operand->evaluate(scope);
    if (op == "-") {
        if (val.getType() == Value::Type::Integer) {
            return Value(-val.getInteger());
        } else if (val.getType() == Value::Type::Float) {
            return Value(-val.getFloat());
        }
    } else if (op == "!") {
        return Value(!val.toBoolean());
    }
    throw std::runtime_error("Invalid unary operation");
}

} // namespace jeve 