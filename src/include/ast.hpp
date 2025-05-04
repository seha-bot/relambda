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

bool is_variable(ast::ExpressionPtr const& expr);
bool is_abstraction(ast::ExpressionPtr const& expr);
bool is_application(ast::ExpressionPtr const& expr);
bool is_s(ast::ExpressionPtr const& expr);
bool is_k(ast::ExpressionPtr const& expr);
bool is_i(ast::ExpressionPtr const& expr);

struct Variable : Expression {
    Variable(std::string name) : name(std::move(name)) {}
    std::string name;

    std::string format() const override { return name; }
};

struct Application : Expression {
    Application(ExpressionPtr lhs, ExpressionPtr rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    ExpressionPtr lhs, rhs;

    std::string format() const override {
        std::string res;
        if (is_abstraction(lhs)) {
            res += '(' + lhs->format() + ')';
        } else {
            res += lhs->format();
        }
        res += ' ';

        if (is_abstraction(rhs) || is_application(rhs)) {
            res += '(' + rhs->format() + ')';
        } else {
            res += rhs->format();
        }
        return res;
    }
};

struct Abstraction : Expression {
    Abstraction(std::string name, ExpressionPtr body) : name(std::move(name)), body(std::move(body)) {}
    std::string name;
    ExpressionPtr body;

    std::string format() const override { return "\\" + name + '.' + body->format(); }
};

struct S : Expression {
    std::string format() const override { return "S"; }
};
struct K : Expression {
    std::string format() const override { return "K"; }
};
struct I : Expression {
    std::string format() const override { return "I"; }
};

struct Definition {
    std::string name;
    ExpressionPtr value;
};

using Definitions = std::vector<Definition>;

}  // namespace ast

#endif
