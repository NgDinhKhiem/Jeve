#pragma once

#include "ASTNode.hpp"
#include <string>

namespace jeve {

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

    Value evaluate(SymbolTable& scope) override;
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

    void addStatement(Ref<ASTNode> stmt);
    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "BlockNode"; }
};

class IfNode : public ASTNode {
private:
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
private:
    Ref<ASTNode> condition;
    Ref<BlockNode> body;

public:
    WhileNode(Ref<ASTNode> cond, Ref<BlockNode> b)
        : condition(cond), body(b) {}

    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "WhileNode"; }
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

    Value evaluate(SymbolTable& scope) override;
    std::string toString() const override { return "ForNode"; }
};

} // namespace jeve 