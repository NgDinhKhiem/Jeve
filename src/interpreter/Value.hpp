#pragma once
#include <variant>
#include "Object.hpp"

#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <variant>
#include <mutex>
#include <atomic>

namespace jeve {

// Forward declaration
class Value;
class Object;

// Define a type for arrays of values using shared_ptr with manual reference counting
class ValueArray : public Object {
private:
    std::vector<Value> elements;
    mutable std::mutex mutex;
    std::atomic<int> refCount;
    
    friend class Value; // Allow Value to access private members

public:
    ValueArray(ObjectPool* pool = nullptr) : Object(pool), refCount(1) {}
    
    explicit ValueArray(const std::vector<Value>& vals, ObjectPool* pool = nullptr) : Object(pool), elements(vals), refCount(1) {}
    
    ValueArray(const ValueArray& other) {
        std::lock_guard<std::mutex> lock(other.mutex);
        elements = other.elements;
        refCount.store(1);
    }
    
    // Thread-safe reference counting
    void addRef() {
        refCount.fetch_add(1, std::memory_order_relaxed);
    }
    
    // Returns true if this was the last reference
    bool release() {
        return refCount.fetch_sub(1, std::memory_order_release) == 1;
    }
    
    // Thread-safe access to elements
    std::vector<Value>& getElements() {
        return elements;
    }
    
    const std::vector<Value>& getElements() const {
        return elements;
    }
    
    size_t size() const {
        return elements.size();
    }
    
    void push_back(const Value& value) {
        elements.push_back(value);
    }
    
    // Get reference count (for Value class)
    int getRefCount() const {
        return refCount.load(std::memory_order_acquire);
    }
    
    Value& at(size_t index);
    const Value& at(size_t index) const;

    std::string toString() const override {
        return "<array>";
    }
};

class Value {
public:
    enum class Type {
        Integer,
        Float,
        Boolean,
        String,
        Array,
        Object,
        Null
    };

private:
    // Use std::variant to store different value types
    using ValueVariant = std::variant<
        std::monostate,          // for Null
        int64_t,                 // for Integer
        double,                  // for Float
        bool,                    // for Boolean
        std::string,             // for String
        Ref<ValueArray>,         // for Array - Ref counted
        Object*                 // for Object
    >;
    
    ValueVariant data;
    Type type;
    
    // Clean up array if needed
    void cleanupArray() {
        if (type == Type::Array) {
            auto arr = std::get<Ref<ValueArray>>(data);
            // Ref will handle cleanup
        }
    }
    
    // Helper for deep copying
    void copyFrom(const Value& other) {
        type = other.type;
        
        if (type == Type::Array && std::holds_alternative<Ref<ValueArray>>(other.data)) {
            auto otherArr = std::get<Ref<ValueArray>>(other.data);
                data = otherArr;
        } else {
            data = other.data;
        }
    }

public:
    // Default constructor - null value
    Value() : data(std::monostate()), type(Type::Null) {}
    
    // Integer constructor
    Value(int64_t val) : data(val), type(Type::Integer) {}
    
    // Float constructor
    Value(double val) : data(val), type(Type::Float) {}
    
    // Boolean constructor
    Value(bool val) : data(val), type(Type::Boolean) {}
    
    // String constructors
    Value(const std::string& val) : data(val), type(Type::String) {}
    Value(const char* val) : data(std::string(val)), type(Type::String) {}
    
    // Array constructor
    Value(const std::vector<Value>& vals, ObjectPool* pool = nullptr)
        : data(Ref<ValueArray>(new ValueArray(vals, pool))), type(Type::Array) {}
    
    // Object constructor
    Value(const Ref<Object>& obj) : data(obj.get()), type(Type::Object) {}
    
    // Copy constructor
    Value(const Value& other) {
        copyFrom(other);
    }
    
    // Move constructor
    Value(Value&& other) noexcept {
        type = other.type;
        data = std::move(other.data);
        other.type = Type::Null;
        other.data = std::monostate();
    }
    
    // Copy assignment
    Value& operator=(const Value& other) {
        if (this != &other) {
            cleanupArray();
            copyFrom(other);
        }
        return *this;
    }
    
    // Move assignment
    Value& operator=(Value&& other) noexcept {
        if (this != &other) {
            cleanupArray();
            type = other.type;
            data = std::move(other.data);
            other.type = Type::Null;
            other.data = std::monostate();
        }
        return *this;
    }
    
    // Destructor
    ~Value() {
        cleanupArray();
    }
    
    // Type inspection
    Type getType() const { return type; }
    bool isNull() const { return type == Type::Null; }
    
    // Value getters
    int64_t getInteger() const {
        if (type != Type::Integer) throw std::runtime_error("Value is not an integer");
        return std::get<int64_t>(data);
    }
    
    double getFloat() const {
        if (type != Type::Float) throw std::runtime_error("Value is not a float");
        return std::get<double>(data);
    }
    
    bool getBoolean() const {
        if (type != Type::Boolean) {
            throw std::runtime_error("Value is not a boolean");
        }
        return std::get<bool>(data);
    }
    
    const std::string& getString() const {
        if (type != Type::String) throw std::runtime_error("Value is not a string");
        return std::get<std::string>(data);
    }
    
    // Ensure the array is unique before modification (copy-on-write)
    Ref<ValueArray> prepareArrayForModification() {
        if (type != Type::Array) throw std::runtime_error("Value is not an array");
        
        auto arr = std::get<Ref<ValueArray>>(data);
        if (!arr) throw std::runtime_error("Null array value");
        
        // If refCount > 1, create a new copy
        if (arr->getRefCount() > 1) {
            Ref<ValueArray> newArr = Ref<ValueArray>(new ValueArray(*arr));
            arr->release();
            data = newArr;
            return newArr;
        }
        
        return arr;
    }
    
    // Array access - mutable
    std::vector<Value>& getArray() {
        return prepareArrayForModification()->getElements();
    }
    
    // Array access - const
    const std::vector<Value>& getArray() const {
        if (type != Type::Array) throw std::runtime_error("Value is not an array");
        
        auto arr = std::get<Ref<ValueArray>>(data);
        if (!arr) throw std::runtime_error("Null array value");
        
        return arr->getElements();
    }
    
    // Array element access with bounds checking
    Value& at(size_t index) {
        auto arr = prepareArrayForModification();
        if (index >= arr->size()) {
            throw std::out_of_range("Array index out of bounds");
        }
        return arr->getElements()[index];
    }
    
    const Value& at(size_t index) const {
        if (type != Type::Array) throw std::runtime_error("Value is not an array");
        
        auto arr = std::get<Ref<ValueArray>>(data);
        if (!arr) throw std::runtime_error("Null array value");
        
        if (index >= arr->size()) {
            throw std::out_of_range("Array index out of bounds");
        }
        
        return arr->getElements()[index];
    }
    
    // String representation
    std::string toString() const {
        std::ostringstream oss;
        
        switch (type) {
            case Type::Integer:
                oss << getInteger();
                break;
            case Type::Float:
                oss << getFloat();
                break;
            case Type::Boolean:
                oss << (getBoolean() ? "true" : "false");
                break;
            case Type::String:
                oss << getString();
                break;
            case Type::Array: {
                if (type != Type::Array) return "null";
                
                auto arr = std::get<Ref<ValueArray>>(data);
                if (!arr) return "null";
                
                const auto& elements = arr->getElements();
                
                oss << "[";
                for (size_t i = 0; i < elements.size(); ++i) {
                    if (i > 0) oss << ", ";
                    // Avoid recursion for self-referential arrays
                    const Value& element = elements[i];
                    if (element.getType() == Type::Array) {
                        oss << "[...]"; // Simplified representation for nested arrays
                    } else {
                        oss << element.toString();
                    }
                }
                oss << "]";
                break;
            }
            case Type::Object:
                oss << "<object>";
                break;
            case Type::Null:
                oss << "null";
                break;
        }
        
        return oss.str();
    }
    
    // Helper method to create an empty array
    static Value createEmptyArray(ObjectPool* pool = nullptr) {
        return Value(std::vector<Value>(), pool);
    }
    
    // Helper method to append a value to an array
    void appendToArray(const Value& value) {
        auto arr = prepareArrayForModification();
        arr->push_back(value);
    }

    bool toBoolean() const {
        switch (type) {
            case Type::Boolean:
                return std::get<bool>(data);
            case Type::Integer:
                return std::get<int64_t>(data) != 0;
            case Type::Float:
                return std::get<double>(data) != 0.0;
            case Type::String:
                return !std::get<std::string>(data).empty();
            case Type::Array: {
                auto arr = std::get<Ref<ValueArray>>(data);
                return arr && !arr->getElements().empty();
            }
            default:
                return false;
        }
    }

    Object* getObject() const {
        if (type != Type::Object) throw std::runtime_error("Value is not an object");
        return std::get<Object*>(data);
    }
};

// Now we can define these methods that needed the full Value definition
inline Value& ValueArray::at(size_t index) {
    if (index >= elements.size()) {
        throw std::out_of_range("Array index out of bounds");
    }
    return elements[index];
}

inline const Value& ValueArray::at(size_t index) const {
    if (index >= elements.size()) {
        throw std::out_of_range("Array index out of bounds");
    }
    return elements[index];
}

} // namespace jeve 