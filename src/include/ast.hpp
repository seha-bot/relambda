#ifndef AST_HPP
#define AST_HPP

#include <algorithm>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ast {

using ExpressionPtr = std::unique_ptr<struct Expression>;

struct Definition {
    std::string name;
    ExpressionPtr value;
};

using Definitions = std::vector<Definition>;

struct Expression {
    virtual ~Expression() = default;
    virtual std::string format() const noexcept = 0;
    virtual std::string format_unlambda(Definitions const& env) const noexcept = 0;
};

bool is_variable(ExpressionPtr const& expr);
bool is_abstraction(ExpressionPtr const& expr);
bool is_application(ExpressionPtr const& expr);
bool is_s(ExpressionPtr const& expr);
bool is_k(ExpressionPtr const& expr);
bool is_i(ExpressionPtr const& expr);
bool is_d(ExpressionPtr const& expr);
bool is_d_app(ExpressionPtr const& expr);

struct Variable : Expression {
    Variable(std::string name) : name(std::move(name)) {}
    std::string name;

    std::string format() const noexcept override { return name; }
    std::string format_unlambda(Definitions const& env) const noexcept override {
        auto it = std::ranges::find_if(env, [this](Definition const& def) { return def.name == name; });
        if (it == env.end()) {
            std::cerr << "logic_error: can't format undefined names\n";
            std::terminate();
        }
        return it->value->format_unlambda(env);
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

    std::string format_unlambda(Definitions const& env) const noexcept override {
        return '`' + lhs->format_unlambda(env) + rhs->format_unlambda(env);
    }
};

struct Abstraction : Expression {
    Abstraction(std::string name, ExpressionPtr body) : name(std::move(name)), body(std::move(body)) {}
    std::string name;
    ExpressionPtr body;

    std::string format() const noexcept override { return "\\" + name + '.' + body->format(); }
    std::string format_unlambda(Definitions const&) const noexcept override {
        std::cerr << "abstractions don't exist in unlambda\n";
        std::terminate();
    }
};

struct S : Expression {
    std::string format() const noexcept override { return "S"; }
    std::string format_unlambda(Definitions const&) const noexcept override { return "s"; }
};
struct K : Expression {
    std::string format() const noexcept override { return "K"; }
    std::string format_unlambda(Definitions const&) const noexcept override { return "k"; }
};
struct I : Expression {
    std::string format() const noexcept override { return "I"; }
    std::string format_unlambda(Definitions const&) const noexcept override { return "i"; }
};

// special unlambda delay combinator
struct D : Expression {
    std::string format() const noexcept override { return "D"; }
    std::string format_unlambda(Definitions const&) const noexcept override { return "d"; }
};

}  // namespace ast

#endif
