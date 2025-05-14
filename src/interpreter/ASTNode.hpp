#pragma once

#include "Forward.hpp"
#include "SymbolTable.hpp"
#include "Value.hpp"
#include "Object.hpp"
#include <string>
#include <iostream>
#include <regex>
#include <vector>
#include <memory>

namespace jeve {

// Forward declarations
class GarbageCollector;

class ASTNode : public Object {
public:
    virtual ~ASTNode() = default;
    virtual Value evaluate(SymbolTable& scope) = 0;
};

class NumberNode : public ASTNode {
private:
    int64_t value;

public:
    NumberNode(int64_t val) : value(val) {}
    Value evaluate(SymbolTable&) override {
        return Value(value);
    }
    std::string toString() const override { return "NumberNode"; }
};

class StringNode : public ASTNode {
private:
    std::string value;

public:
    StringNode(const std::string& val) : value(val) {}
    Value evaluate(SymbolTable&) override {
        return Value(value);
    }
    std::string toString() const override { return "StringNode"; }
};

class IdentifierNode : public ASTNode {
private:
    std::string name;

public:
    IdentifierNode(const std::string& n) : name(n) {}
    Value evaluate(SymbolTable& scope) override {
        return scope.get(name);
    }

    const std::string& getName() const { return name; }
    std::string toString() const override { return "IdentifierNode"; }
};

class BinaryOpNode : public ASTNode {
private:
    Ref<ASTNode> left;
    Ref<ASTNode> right;
    std::string op;

public:
    BinaryOpNode(Ref<ASTNode> l, Ref<ASTNode> r, const std::string& o)
        : left(l), right(r), op(o) {}
    
    ASTNode* getLeft() const { return left.get(); }
    ASTNode* getRight() const { return right.get(); }
    
    Value evaluate(SymbolTable& scope) override {
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
            // Efficient array concatenation using the new Value implementation
            const auto& leftArray = lval.getArray();
            const auto& rightArray = rval.getArray();
            
            // Create a new array with the combined capacity
            std::vector<Value> result;
            result.reserve(leftArray.size() + rightArray.size());
            
            // Copy elements from both arrays
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
    std::string toString() const override { return "BinaryOpNode"; }
};

class PrintNode : public ASTNode {
private:
    Ref<ASTNode> expression;

    std::string processEscapeSequences(const std::string& str) {
        std::string result;
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '\\' && i + 1 < str.length()) {
                switch (str[i + 1]) {
                    case 'n': result += '\n'; break;
                    case 't': result += '\t'; break;
                    case 'r': result += '\r'; break;
                    case '\\': result += '\\'; break;
                    case '"': result += '"'; break;
                    default: result += str[i]; continue;
                }
                ++i;
            } else {
                result += str[i];
            }
        }
        return result;
    }

public:
    PrintNode(Ref<ASTNode> expr) : expression(expr) {}

    ASTNode* getExpression() const { return expression.get(); }

    Value evaluate(SymbolTable& scope) override {
        Value result = expression->evaluate(scope);
        if (result.getType() == Value::Type::String) {
            std::cout << processEscapeSequences(result.getString()) << std::endl;
        } else if (result.getType() == Value::Type::Integer) {
            std::cout << result.getInteger() << std::endl;
        } else if (result.getType() == Value::Type::Float) {
            std::cout << result.getFloat() << std::endl;
        } else if (result.getType() == Value::Type::Boolean) {
            std::cout << (result.getBoolean() ? "true" : "false") << std::endl;
        }
        return result;
    }

    std::string toString() const override { return "PrintNode"; }
};

class StatementNode : public ASTNode {
private:
    Ref<ASTNode> statement;
    Ref<StatementNode> next;
    GarbageCollector* gc;

public:
    StatementNode(Ref<ASTNode> stmt, GarbageCollector* g) 
        : statement(stmt), next(), gc(g) {}

    void setNext(Ref<StatementNode> n) {
        next = n;
    }

    ASTNode* getStatement() const { return statement.get(); }
    StatementNode* getNext() const { return next.get(); }

    Value evaluate(SymbolTable& scope) override {
        Value result = statement->evaluate(scope);
        if (next.get() != nullptr) {
            result = next->evaluate(scope);
        }
        return result;
    }

    GarbageCollector* getGC() const { return gc; }

    std::string toString() const override { return "StatementNode"; }
};

class BlockNode : public ASTNode {
private:
    Ref<StatementNode> first;
    Ref<StatementNode> last;
    GarbageCollector* gc;

public:
    BlockNode(GarbageCollector* g) : first(), last(), gc(g) {}

    void addStatement(Ref<ASTNode> stmt) {
        Ref<StatementNode> node(new StatementNode(stmt, gc));
        if (first.get() == nullptr) {
            first = node;
            last = node;
        } else {
            last->setNext(node);
            last = node;
        }
    }

    Value evaluate(SymbolTable& scope) override {
        if (first.get() == nullptr) {
            return Value();
        }
        return first->evaluate(scope);
    }

    std::string toString() const override { return "BlockNode"; }
};

class AssignmentNode : public ASTNode {
private:
    std::string name;
    Ref<ASTNode> expression;
    std::string type;  // Optional type annotation

public:
    AssignmentNode(const std::string& n, Ref<ASTNode> expr, const std::string& t = "")
        : name(n), expression(expr), type(t) {}

    Value evaluate(SymbolTable& scope) override {
        Value value = expression->evaluate(scope);
        
        // Type checking if type is specified
        if (!type.empty()) {
            if (type == "int" && value.getType() != Value::Type::Integer) {
                throw std::runtime_error("Type mismatch: expected int");
            }
            if (type == "string" && value.getType() != Value::Type::String) {
                throw std::runtime_error("Type mismatch: expected string");
            }
            if (type == "float" && value.getType() != Value::Type::Float) {
                throw std::runtime_error("Type mismatch: expected float");
            }
            if (type == "bool" && value.getType() != Value::Type::Boolean) {
                throw std::runtime_error("Type mismatch: expected bool");
            }
        }
        
        scope.set(name, value);
        return value;
    }

    std::string toString() const override { return "AssignmentNode"; }
};

class InputNode : public ASTNode {
private:
    std::string type;  // Optional type annotation

public:
    InputNode(const std::string& t = "") : type(t) {}

    Value evaluate(SymbolTable&) override {
        std::string input;
        std::getline(std::cin, input);
        
        if (type == "int") {
            try {
                return Value(std::stoll(input));
            } catch (...) {
                throw std::runtime_error("Invalid integer input");
            }
        } else if (type == "float") {
            try {
                return Value(std::stod(input));
            } catch (...) {
                throw std::runtime_error("Invalid float input");
            }
        } else if (type == "bool") {
            if (input == "true") return Value(true);
            if (input == "false") return Value(false);
            throw std::runtime_error("Invalid boolean input");
        }
        
        return Value(input);
    }

    std::string toString() const override { return "InputNode"; }
};

class ConcatNode : public ASTNode {
private:
    Ref<ASTNode> left;
    Ref<ASTNode> right;

public:
    ConcatNode(Ref<ASTNode> l, Ref<ASTNode> r) : left(l), right(r) {}

    Value evaluate(SymbolTable& scope) override {
        Value lval = left->evaluate(scope);
        Value rval = right->evaluate(scope);
        return Value(lval.toString() + rval.toString());
    }

    std::string toString() const override { return "ConcatNode"; }
};

class IfNode : public ASTNode {
private:
    Ref<ASTNode> condition;
    Ref<BlockNode> thenBlock;
    Ref<BlockNode> elseBlock;

public:
    IfNode(Ref<ASTNode> cond, Ref<BlockNode> then, Ref<BlockNode> else_ = Ref<BlockNode>())
        : condition(cond), thenBlock(then), elseBlock(else_) {}

    Value evaluate(SymbolTable& scope) override {
        Value cond = condition->evaluate(scope);
        if (cond.toBoolean()) {
            return thenBlock->evaluate(scope);
        } else if (elseBlock.get() != nullptr) {
            return elseBlock->evaluate(scope);
        }
        return Value();
    }

    std::string toString() const override { return "IfNode"; }
};

class WhileNode : public ASTNode {
private:
    Ref<ASTNode> condition;
    Ref<BlockNode> body;

public:
    WhileNode(Ref<ASTNode> cond, Ref<BlockNode> b)
        : condition(cond), body(b) {}

    Value evaluate(SymbolTable& scope) override {
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

    std::string toString() const override { return "WhileNode"; }
};

class FunctionCallNode : public ASTNode {
private:
    std::string name;
    std::vector<Ref<ASTNode>> arguments;

public:
    FunctionCallNode(const std::string& n, const std::vector<Ref<ASTNode>>& args)
        : name(n), arguments(args) {}

    Value evaluate(SymbolTable& scope) override {
        if (name == "print") {
            if (arguments.size() != 1) {
                throw std::runtime_error("print() expects exactly one argument");
            }
            return PrintNode(arguments[0]).evaluate(scope);
        }
        if (name == "input") {
            if (arguments.size() > 1) {
                throw std::runtime_error("input() expects at most one type argument");
            }
            std::string type = arguments.empty() ? "" : arguments[0]->evaluate(scope).getString();
            return InputNode(type).evaluate(scope);
        }
        throw std::runtime_error("Unknown function: " + name);
    }

    std::string toString() const override { return "FunctionCallNode"; }
};

class ArrayNode : public ASTNode {
private:
    std::vector<Ref<ASTNode>> elements;

public:
    ArrayNode(const std::vector<Ref<ASTNode>>& elems) : elements(elems) {}

    Value evaluate(SymbolTable& scope) override {
        try {
            // Create a vector to hold the array elements
            std::vector<Value> values;
            values.reserve(elements.size());
            
            // Evaluate each element and add it to the array
            for (const auto& elem : elements) {
                values.push_back(elem->evaluate(scope));
            }
            
            // Create and return a Value containing the array
            return Value(values);
        } catch (const std::exception& e) {
            std::cerr << "Error in ArrayNode::evaluate: " << e.what() << std::endl;
            // Return an empty array instead of crashing
            return Value::createEmptyArray();
        }
    }

    std::string toString() const override { return "ArrayNode"; }
};

class ArrayAccessNode : public ASTNode {
private:
    Ref<ASTNode> array;
    Ref<ASTNode> index;

public:
    ArrayAccessNode(Ref<ASTNode> arr, Ref<ASTNode> idx) : array(arr), index(idx) {}

    Value evaluate(SymbolTable& scope) override {
        Value arrayValue = array->evaluate(scope);
        Value indexValue = index->evaluate(scope);
        
        if (arrayValue.getType() != Value::Type::Array) {
            throw std::runtime_error("Cannot access non-array value with index");
        }
        
        if (indexValue.getType() != Value::Type::Integer) {
            throw std::runtime_error("Array index must be an integer");
        }
        
        int64_t idx = indexValue.getInteger();
        if (idx < 0) {
            throw std::runtime_error("Array index out of bounds: " + std::to_string(idx));
        }
        
        try {
            // Use the new at() method with bounds checking
            return arrayValue.at(static_cast<size_t>(idx));
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Array index out of bounds: " + std::to_string(idx));
        }
    }

    std::string toString() const override { return "ArrayAccessNode"; }
};

class ForNode : public ASTNode {
private:
    std::string varName;
    Ref<ASTNode> start;
    Ref<ASTNode> end;
    Ref<ASTNode> step;
    Ref<BlockNode> body;

public:
    ForNode(const std::string& var, Ref<ASTNode> s, Ref<ASTNode> e, 
            Ref<ASTNode> st, Ref<BlockNode> b)
        : varName(var), start(s), end(e), step(st), body(b) {}

    Value evaluate(SymbolTable& scope) override {
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

    std::string toString() const override { return "ForNode"; }
};

class SmartLoopNode : public ASTNode {
private:
    std::string varName;
    std::string indexName;
    Ref<ASTNode> array;
    Ref<BlockNode> body;

public:
    SmartLoopNode(const std::string& var, const std::string& idx, 
                 Ref<ASTNode> arr, Ref<BlockNode> b)
        : varName(var), indexName(idx), array(arr), body(b) {}

    Value evaluate(SymbolTable& scope) override {
        Value arr = array->evaluate(scope);
        if (arr.getType() != Value::Type::Array) {
            throw std::runtime_error("Smart loop requires an array");
        }
        
        Value result;
        const auto& elements = arr.getArray();
        for (size_t i = 0; i < elements.size(); ++i) {
            scope.set(indexName, Value(static_cast<int64_t>(i)));
            scope.set(varName, elements[i]);
            result = body->evaluate(scope);
        }
        return result;
    }

    std::string toString() const override { return "SmartLoopNode"; }
};

class ArrayAssignmentNode : public ASTNode {
private:
    Ref<ASTNode> array;
    Ref<ASTNode> index;
    Ref<ASTNode> value;

public:
    ArrayAssignmentNode(Ref<ASTNode> arr, Ref<ASTNode> idx, Ref<ASTNode> val) 
        : array(arr), index(idx), value(val) {}

    Value evaluate(SymbolTable& scope) override {
        // Get the variable name from the identifier node
        std::string varName;
        if (auto* idNode = dynamic_cast<IdentifierNode*>(array.get())) {
            varName = idNode->getName();
        } else {
            throw std::runtime_error("Cannot assign to non-identifier array");
        }
        
        // Get the array value from the scope
        Value arrayValue = scope.get(varName);
        Value indexValue = index->evaluate(scope);
        Value newValue = value->evaluate(scope);
        
        if (arrayValue.getType() != Value::Type::Array) {
            throw std::runtime_error("Cannot assign to non-array value");
        }
        
        if (indexValue.getType() != Value::Type::Integer) {
            throw std::runtime_error("Array index must be an integer");
        }
        
        int64_t idx = indexValue.getInteger();
        if (idx < 0) {
            throw std::runtime_error("Array index out of bounds: " + std::to_string(idx));
        }
        
        try {
            // Get the array reference for modification and update the value
            std::vector<Value>& arrayData = arrayValue.getArray();
            if (idx >= static_cast<int64_t>(arrayData.size())) {
                throw std::runtime_error("Array index out of bounds: " + std::to_string(idx));
            }
            
            arrayData[idx] = newValue;
            
            // Update the variable with the modified array
            scope.set(varName, arrayValue);
            
            return newValue;
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Array index out of bounds: " + std::to_string(idx));
        }
    }

    std::string toString() const override { return "ArrayAssignmentNode"; }
};

class PropertyAccessNode : public ASTNode {
private:
    Ref<ASTNode> object;
    std::string property;

public:
    PropertyAccessNode(Ref<ASTNode> obj, const std::string& prop) 
        : object(obj), property(prop) {}

    Value evaluate(SymbolTable& scope) override {
        Value objectValue = object->evaluate(scope);
        
        if (property == "length") {
            if (objectValue.getType() == Value::Type::Array) {
                return Value(static_cast<int64_t>(objectValue.getArray().size()));
            } else if (objectValue.getType() == Value::Type::String) {
                return Value(static_cast<int64_t>(objectValue.getString().length()));
            }
            throw std::runtime_error("Object does not have length property");
        }
        
        throw std::runtime_error("Unknown property: " + property);
    }

    std::string toString() const override { return "PropertyAccessNode"; }
};

// Add DebugGCNode class to print memory usage statistics
class DebugGCNode : public ASTNode {
private:
    GarbageCollector* gc;

public:
    DebugGCNode(GarbageCollector* gc = nullptr) : gc(gc) {}

    Value evaluate(SymbolTable&) override {
        if (gc) {
            gc->printStats();
        } else {
            std::cout << "[DebugGC] Garbage collector not available" << std::endl;
        }
        return Value();
    }

    std::string toString() const override { return "DebugGCNode"; }
};

// Add CleanGCNode class to force garbage collection
class CleanGCNode : public ASTNode {
private:
    GarbageCollector* gc;

public:
    CleanGCNode(GarbageCollector* gc = nullptr) : gc(gc) {}

    Value evaluate(SymbolTable&) override {
        if (gc) {
            gc->collect();
        }
        return Value();
    }

    std::string toString() const override { return "CleanGCNode"; }
};

class UnaryOpNode : public ASTNode {
private:
    Ref<ASTNode> operand;
    std::string op;

public:
    UnaryOpNode(Ref<ASTNode> op, const std::string& o)
        : operand(op), op(o) {}

    Value evaluate(SymbolTable& scope) override {
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

    std::string toString() const override { return "UnaryOpNode"; }
};

class BooleanNode : public ASTNode {
private:
    bool value;

public:
    BooleanNode(bool val) : value(val) {}

    Value evaluate(SymbolTable&) override {
        return Value(value);
    }

    std::string toString() const override { return "BooleanNode"; }
};

} // namespace jeve 