#ifndef AST_HPP
#define AST_HPP

#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ast {

struct Expression {
    virtual ~Expression() = default;
    virtual std::string format() const noexcept = 0;
    virtual std::string format_unlambda() const noexcept = 0;
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

    std::string format() const noexcept override { return name; }
    std::string format_unlambda() const noexcept override {
        std::cerr << "variables don't exist in unlambda";
        std::terminate();
    }
};

struct Application : Expression {
    Application(ExpressionPtr lhs, ExpressionPtr rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    ExpressionPtr lhs, rhs;

    std::string format() const noexcept override {
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

    std::string format_unlambda() const noexcept override {
        return '`' + lhs->format_unlambda() + rhs->format_unlambda();
    }
};

struct Abstraction : Expression {
    Abstraction(std::string name, ExpressionPtr body) : name(std::move(name)), body(std::move(body)) {}
    std::string name;
    ExpressionPtr body;

    std::string format() const noexcept override { return "\\" + name + '.' + body->format(); }
    std::string format_unlambda() const noexcept override {
        std::cerr << "abstractions don't exist in unlambda";
        std::terminate();
    }
};

struct S : Expression {
    std::string format() const noexcept override { return "S"; }
    std::string format_unlambda() const noexcept override { return "s"; }
};
struct K : Expression {
    std::string format() const noexcept override { return "K"; }
    std::string format_unlambda() const noexcept override { return "k"; }
};
struct I : Expression {
    std::string format() const noexcept override { return "I"; }
    std::string format_unlambda() const noexcept override { return "i"; }
};

// special unlambda delay combinator
struct D : Expression {
    std::string format() const noexcept override { return "D"; }
    std::string format_unlambda() const noexcept override { return "d"; }
};

struct Definition {
    std::string name;
    ExpressionPtr value;
};

using Definitions = std::vector<Definition>;

}  // namespace ast

#endif
