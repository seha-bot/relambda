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

ast::ExpressionPtr to_ski(ast::ExpressionPtr expr) {
    if (is_abstraction(expr)) {
        auto& abs = static_cast<ast::Abstraction&>(*expr);
        // abs.body = to_ski(std::move(abs.body));

        // \x.x = I
        if (is_variable(abs.body)) {
            auto& var = static_cast<ast::Variable&>(*abs.body);
            if (abs.name == var.name) {
                return std::make_unique<ast::I>();
            }
        }

        // \x.F = D (K F) (F does not contain x)
        if (!contains(abs.body, abs.name)) {
            // simplification: \x.F = K F where F is a variable or a combinator
            if (is_variable(abs.body) || is_s(abs.body) || is_k(abs.body) || is_i(abs.body)) {
                return std::make_unique<ast::Application>(std::make_unique<ast::K>(), to_ski(std::move(abs.body)));
            }

            // TODO: this can be further simplified if you can prove termination or prove that there won't be side
            // effects without D.
            return std::make_unique<ast::Application>(
                std::make_unique<ast::D>(),
                std::make_unique<ast::Application>(std::make_unique<ast::K>(), to_ski(std::move(abs.body))));
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

            auto fst = std::make_unique<ast::Abstraction>(abs.name, std::move(app.lhs));
            auto snd = std::make_unique<ast::Abstraction>(abs.name, std::move(app.rhs));
            return std::make_unique<ast::Application>(
                std::make_unique<ast::Application>(std::make_unique<ast::S>(), to_ski(std::move(fst))),
                to_ski(std::move(snd)));
        }

        abs.body = to_ski(std::move(abs.body));
        return to_ski(std::move(expr));
    } else if (is_application(expr)) {
        auto& app = static_cast<ast::Application&>(*expr);
        app.lhs = to_ski(std::move(app.lhs));
        app.rhs = to_ski(std::move(app.rhs));
        return expr;
    } else /* is_variable || is_s || is_k || is_i */ {
        return expr;
    }
}

}  // namespace conv

#endif
