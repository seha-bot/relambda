#include <iostream>

#include "parser.hpp"

namespace ast {

struct S : Expression {
    std::string format() const override { return "S"; }
};
struct K : Expression {
    std::string format() const override { return "K"; }
};
struct I : Expression {
    std::string format() const override { return "I"; }
};

}  // namespace ast

bool is_variable(ast::ExpressionPtr const& expr) { return dynamic_cast<ast::Variable *>(expr.get()) != nullptr; }
bool is_abstraction(ast::ExpressionPtr const& expr) { return dynamic_cast<ast::Abstraction *>(expr.get()) != nullptr; }
bool is_application(ast::ExpressionPtr const& expr) { return dynamic_cast<ast::Application *>(expr.get()) != nullptr; }

bool is_s(ast::ExpressionPtr const& expr) { return dynamic_cast<ast::S *>(expr.get()) != nullptr; }
bool is_k(ast::ExpressionPtr const& expr) { return dynamic_cast<ast::K *>(expr.get()) != nullptr; }
bool is_i(ast::ExpressionPtr const& expr) { return dynamic_cast<ast::I *>(expr.get()) != nullptr; }

// bool contains(ast::ExpressionPtr const& expr) {
//     if (is_abstraction(expr)) {
//     }
// }

ast::ExpressionPtr to_ski(ast::ExpressionPtr expr) {
    if (is_abstraction(expr)) {
        auto& abs = static_cast<ast::Abstraction&>(*expr);
        // \x.x = I
        if (is_variable(abs.body)) {
            auto& var = static_cast<ast::Variable&>(*abs.body);
            if (abs.name == var.name) {
                return std::make_unique<ast::I>();
            }
        }

        // \x.F G = S(\x.F)(\x.G)
        if (is_application(abs.body)) {
            auto& app = static_cast<ast::Application&>(*abs.body);

            // \x.F x = F
            if (is_variable(app.rhs)) {
                auto& var = static_cast<ast::Variable&>(*app.rhs);
                if (abs.name == var.name) {
                    // TODO
                    // if not app.rhs.contains(abs.name)
                }
            }

            auto fst = std::make_unique<ast::Abstraction>(abs.name, std::move(app.lhs));
            auto snd = std::make_unique<ast::Abstraction>(abs.name, std::move(app.rhs));
            return std::make_unique<ast::Application>(
                std::make_unique<ast::Application>(std::make_unique<ast::S>(), to_ski(std::move(fst))),
                to_ski(std::move(snd)));
        }

        // is_variable || is_s || is_k || is_i
        // \x.y = K y
        if (!is_abstraction(abs.body)) {
            return std::make_unique<ast::Application>(std::make_unique<ast::K>(), std::move(abs.body));
        }

        // \x.F
        abs.body = to_ski(std::move(abs.body));
        return to_ski(std::move(expr));
    } else if (is_application(expr)) {
        auto& app = static_cast<ast::Application&>(*expr);
        app.lhs = to_ski(std::move(app.lhs));
        app.rhs = to_ski(std::move(app.rhs));
        return expr;
    } else /* is_variable || is_s || is_k || is_i */ {
        // throw std::logic_error{"variables should've been replaced at this point"};
        return expr;
    }
}

int main(int, char *[]) {
    // auto res = parser::parse_string_expression("\\f.\\x.\\y.f y x").value();
    // auto res = parser::parse_string_expression("\\f.(\\x.f(x x))(\\x.f(x x))").value();
    // auto res = parser::parse_string_expression("\\x.\\y.\\z.x z(y z)").value();
    // auto res = parser::parse_string_expression("\\x.\\y.x").value();
    // auto res = parser::parse_string_expression("\\x.x").value();
    auto res = parser::parse_string_expression("\\x.\\y.\\z.x z(y z)").value();
    std::cout << res->format() << '\n';

    res = to_ski(std::move(res));
    std::cout << res->format() << '\n';

    // std::vector<ast::Definition> const& defs = result.value();
    // for (ast::Definition const& def : defs) {
    //     std::cout << def.name << ": ";
    //     def.value->print();
    //     std::cout << '\n';
    // }
}

