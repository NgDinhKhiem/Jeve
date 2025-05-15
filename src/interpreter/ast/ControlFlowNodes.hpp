#pragma once

#include "../ASTNode.hpp"
#include <string>

namespace jeve {

class StatementNode : public ASTNode {
    Ref<ASTNode> statement;
    Ref<StatementNode> next;
    GarbageCollector* gc;
public:
    StatementNode(Ref<ASTNode> stmt, GarbageCollector* g) : statement(stmt), next(), gc(g) {}
    void setNext(Ref<StatementNode> n) { next = n; }
    ASTNode* getStatement() const { return statement.get(); }
    StatementNode* getNext() const { return next.get(); }
    Value evaluate(SymbolTable& scope) override;
    GarbageCollector* getGC() const { return gc; }
    std::string toString() const override { return "StatementNode"; }
};

class BlockNode : public ASTNode {
    Ref<StatementNode> first;
    Ref<StatementNode> last;
    GarbageCollector* gc;
public:
    BlockNode(GarbageCollector* g) : first(), last(), gc(g) {}
    void addStatement(Ref<ASTNode> stmt);
    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "BlockNode"; }
};

class IfNode : public ASTNode {
    Ref<ASTNode> condition;
    Ref<BlockNode> thenBlock;
    Ref<BlockNode> elseBlock;
public:
    IfNode(Ref<ASTNode> cond, Ref<BlockNode> then, Ref<BlockNode> else_ = Ref<BlockNode>())
        : condition(cond), thenBlock(then), elseBlock(else_) {}
    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "IfNode"; }
};

class WhileNode : public ASTNode {
    Ref<ASTNode> condition;
    Ref<BlockNode> body;
public:
    WhileNode(Ref<ASTNode> cond, Ref<BlockNode> b) : condition(cond), body(b) {}
    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "WhileNode"; }
};

class ForNode : public ASTNode {
    std::string varName;
    Ref<ASTNode> start, end, step;
    Ref<BlockNode> body;
public:
    ForNode(const std::string& var, Ref<ASTNode> s, Ref<ASTNode> e, Ref<ASTNode> st, Ref<BlockNode> b)
        : varName(var), start(s), end(e), step(st), body(b) {}
    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "ForNode"; }
};

class ReturnException : public std::exception {
    Value value;
public:
    ReturnException(const Value& v) : value(v) {}
    const Value& getValue() const { return value; }
};

class ReturnNode : public ASTNode {
    Ref<ASTNode> expr;
public:
    ReturnNode(Ref<ASTNode> e) : expr(e) {}
    Value evaluate(SymbolTable& scope) override { throw ReturnException(expr->evaluate(scope)); }
    std::string toString() const override { return "ReturnNode"; }
};

} // namespace jeve 