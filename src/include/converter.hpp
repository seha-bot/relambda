#ifndef CONVERTER_HPP
#define CONVERTER_HPP

#include "ast.hpp"

namespace conv {

bool contains(ast::ExpressionPtr const& expr, std::string const& name) {
    if (is_abstraction(expr)) {
        auto& abs = static_cast<ast::Abstraction&>(*expr);
        if (abs.name == name) {
            return false;
        }
        return contains(abs.body, name);
    } else if (is_application(expr)) {
        auto& app = static_cast<ast::Application&>(*expr);
        return contains(app.lhs, name) || contains(app.rhs, name);
    } else if (is_variable(expr)) {
        auto& var = static_cast<ast::Variable&>(*expr);
        return var.name == name;
    } else {
        return false;
    }
}

#define DD(...) std::make_unique<ast::Application>(std::make_unique<ast::D>(), __VA_ARGS__)

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

        // \x.F = D (K F) (F does not contain x)
        if (!contains(abs.body, abs.name)) {
            return std::make_unique<ast::Application>(std::make_unique<ast::K>(), to_ski(std::move(abs.body)));
        }

        // \x.F G = S(\x.F)(\x.G)
        if (is_application(abs.body)) {
            auto& app = static_cast<ast::Application&>(*abs.body);

            // \x.F x = F
            if (is_variable(app.rhs)) {
                auto& var = static_cast<ast::Variable&>(*app.rhs);
                if (abs.name == var.name) {
                    if (!contains(app.lhs, abs.name)) {
                        return std::move(app.lhs);
                    }
                }
            }

            bool was_combinator = is_s(app.lhs) || is_k(app.lhs) || is_i(app.lhs) || is_d(app.lhs);
            auto fst = to_ski(std::make_unique<ast::Abstraction>(abs.name, std::move(app.lhs)));
            auto snd = to_ski(std::make_unique<ast::Abstraction>(
                abs.name, was_combinator ? std::move(app.rhs) : DD(std::move(app.rhs))));
            return std::make_unique<ast::Application>(
                std::make_unique<ast::Application>(std::make_unique<ast::S>(), std::move(fst)), std::move(snd));
        }

        abs.body = to_ski(std::move(abs.body));
        return to_ski(std::move(expr));
    } else if (is_application(expr)) {
        auto& app = static_cast<ast::Application&>(*expr);
        app.lhs = to_ski(std::move(app.lhs));
        app.rhs = to_ski(is_s(app.lhs) || is_k(app.lhs) || is_i(app.lhs) || is_d(app.lhs) ? std::move(app.rhs)
                                                                                          : DD(std::move(app.rhs)));
        return expr;
    } else /* is_variable || is_s || is_k || is_i */ {
        return expr;
    }
}

}  // namespace conv

#endif

/*

Y = S(K(SII))(S(S(KS)K)(K(SII)))
    S(K(SII))(S(S(KS)K)(K(S(Kd)(SII))))

*/