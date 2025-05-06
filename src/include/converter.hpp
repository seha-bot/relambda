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

ast::ExpressionPtr make_lazy(ast::ExpressionPtr expr) {
    if (is_abstraction(expr)) {
        auto& abs = static_cast<ast::Abstraction&>(*expr);
        abs.body = make_lazy(std::move(abs.body));
        return expr;
    } else if (is_application(expr)) {
        auto& app = static_cast<ast::Application&>(*expr);
        app.lhs = make_lazy(std::move(app.lhs));

        if (!is_variable(app.rhs)) {
            app.rhs = DD(make_lazy(std::move(app.rhs)));
        }

        return expr;
    } else /* is_variable */ {
        return expr;
    }
}

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

        // \x.F = K F (F does not contain x)
        if (!contains(abs.body, abs.name)) {
            if (is_variable(abs.body) || is_s(abs.body) || is_k(abs.body) || is_i(abs.body) || is_d(abs.body) ||
                is_d_app(abs.body)) {
                return std::make_unique<ast::Application>(std::make_unique<ast::K>(), to_ski(std::move(abs.body)));
            }
            return DD(std::make_unique<ast::Application>(std::make_unique<ast::K>(), to_ski(std::move(abs.body))));
        }

        // \x.F G = S(\x.F)(\x.G)
        if (is_application(abs.body)) {
            auto& app = static_cast<ast::Application&>(*abs.body);

            // \x.D (F x) = F | D F
            if (is_d(app.lhs) && is_application(app.rhs)) {
                auto& rhs_app = static_cast<ast::Application&>(*app.rhs);
                if (is_variable(rhs_app.rhs)) {
                    auto& var = static_cast<ast::Variable&>(*rhs_app.rhs);
                    if (abs.name == var.name && !contains(rhs_app.lhs, abs.name)) {
                        rhs_app.lhs = to_ski(std::move(rhs_app.lhs));
                        if (is_variable(rhs_app.lhs) || is_s(rhs_app.lhs) || is_k(rhs_app.lhs) || is_i(rhs_app.lhs) ||
                            is_d(rhs_app.lhs) || is_d_app(rhs_app.lhs)) {
                            return std::move(rhs_app.lhs);
                        }
                        return DD(std::move(rhs_app.lhs));
                    }
                }
            }

            // \x.F x = F | D F
            if (is_variable(app.rhs)) {
                auto& var = static_cast<ast::Variable&>(*app.rhs);
                if (abs.name == var.name && !contains(app.lhs, abs.name)) {
                    app.lhs = to_ski(std::move(app.lhs));
                    if (is_variable(app.lhs) || is_s(app.lhs) || is_k(app.lhs) || is_i(app.lhs) || is_d(app.lhs) ||
                        is_d_app(app.lhs)) {
                        return std::move(app.lhs);
                    }
                    return DD(std::move(app.lhs));
                }
            }

            auto fst = to_ski(std::make_unique<ast::Abstraction>(abs.name, std::move(app.lhs)));
            auto snd = to_ski(std::make_unique<ast::Abstraction>(abs.name, std::move(app.rhs)));
            return std::make_unique<ast::Application>(
                std::make_unique<ast::Application>(std::make_unique<ast::S>(), std::move(fst)), std::move(snd));
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

/*

Y = S(K(SII))(S(S(KS)K)(K(SII)))
    S(K(SII))(S(S(KS)K)(K(S(Kd)(SII))))

*/