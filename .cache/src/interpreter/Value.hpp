#pragma once

#include <string>
#include <stdexcept>
#include <sstream>

namespace jeve {

class Value {
public:
    enum class Type {
        Integer,
        Float,
        Boolean,
        String,
        Null
    };

private:
    Type type;
    union {
        int64_t integer;
        double floating;
        bool boolean;
        std::string* string;
    } data;

public:
    Value() : type(Type::Null) {}
    Value(int64_t val) : type(Type::Integer) { data.integer = val; }
    Value(double val) : type(Type::Float) { data.floating = val; }
    Value(bool val) : type(Type::Boolean) { data.boolean = val; }
    Value(const std::string& val) : type(Type::String) { data.string = new std::string(val); }
    Value(const char* val) : type(Type::String) { data.string = new std::string(val); }
    
    ~Value() {
        clear();
    }
    
    Value(const Value& other) : type(other.type) {
        switch (type) {
            case Type::String:
                data.string = new std::string(*other.data.string);
                break;
            case Type::Integer:
                data.integer = other.data.integer;
                break;
            case Type::Float:
                data.floating = other.data.floating;
                break;
            case Type::Boolean:
                data.boolean = other.data.boolean;
                break;
            case Type::Null:
                break;
        }
    }
    
    Value& operator=(const Value& other) {
        if (this != &other) {
            clear();
            type = other.type;
            switch (type) {
                case Type::String:
                    data.string = new std::string(*other.data.string);
                    break;
                case Type::Integer:
                    data.integer = other.data.integer;
                    break;
                case Type::Float:
                    data.floating = other.data.floating;
                    break;
                case Type::Boolean:
                    data.boolean = other.data.boolean;
                    break;
                case Type::Null:
                    break;
            }
        }
        return *this;
    }
    
    Value(Value&& other) noexcept : type(other.type) {
        data = other.data;
        other.type = Type::Null;
    }
    
    Value& operator=(Value&& other) noexcept {
        if (this != &other) {
            clear();
            type = other.type;
            data = other.data;
            other.type = Type::Null;
        }
        return *this;
    }
    
    Type getType() const { return type; }
    
    int64_t getInteger() const {
        if (type != Type::Integer) throw std::runtime_error("Value is not an integer");
        return data.integer;
    }
    
    double getFloat() const {
        if (type != Type::Float) throw std::runtime_error("Value is not a float");
        return data.floating;
    }
    
    bool getBoolean() const {
        if (type != Type::Boolean) throw std::runtime_error("Value is not a boolean");
        return data.boolean;
    }
    
    const std::string& getString() const {
        if (type != Type::String) throw std::runtime_error("Value is not a string");
        return *data.string;
    }
    
    bool isNull() const { return type == Type::Null; }

    std::string toString() const {
        std::ostringstream oss;
        switch (type) {
            case Type::String:
                return *data.string;
            case Type::Integer:
                oss << data.integer;
                return oss.str();
            case Type::Float:
                oss << data.floating;
                return oss.str();
            case Type::Boolean:
                return data.boolean ? "true" : "false";
            case Type::Null:
                return "null";
        }
        return "";  // Should never reach here
    }

private:
    void clear() {
        if (type == Type::String && data.string != nullptr) {
            delete data.string;
            data.string = nullptr;
        }
        type = Type::Null;
    }
};

} // namespace jeve 