#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ast {

struct Expression {
    virtual ~Expression() = default;
    virtual std::string format() const = 0;
};

using ExpressionPtr = std::unique_ptr<Expression>;

struct Variable : Expression {
    Variable(std::string name) : name(std::move(name)) {}
    std::string name;

    std::string format() const override { return name; }
};

struct Application : Expression {
    Application(ExpressionPtr lhs, ExpressionPtr rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    ExpressionPtr lhs, rhs;

    std::string format() const override { return '(' + lhs->format() + ") (" + rhs->format() + ')'; }
};

struct Abstraction : Expression {
    Abstraction(std::string name, ExpressionPtr body) : name(std::move(name)), body(std::move(body)) {}
    std::string name;
    ExpressionPtr body;

    std::string format() const override { return "\\" + name + '.' + body->format(); }
};

struct Definition {
    std::string name;
    ExpressionPtr value;
};

using Definitions = std::vector<Definition>;

}  // namespace ast

#endif
