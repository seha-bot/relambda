#include "converter.hpp"

#include <functional>
#include <utility>

namespace {

bool mentions(ast::ExpressionPtr const& expr, std::string const& name) {
    if (is_abstraction(expr)) {
        auto& abs = static_cast<ast::Abstraction&>(*expr);
        if (abs.name == name) {
            return false;
        }
        return mentions(abs.body, name);
    } else if (is_application(expr)) {
        auto& app = static_cast<ast::Application&>(*expr);
        return mentions(app.lhs, name) || mentions(app.rhs, name);
    } else if (is_variable(expr)) {
        auto& var = static_cast<ast::Variable&>(*expr);
        return var.name == name;
    } else {
        return false;
    }
}

bool is_combinator(ast::ExpressionPtr const& expr) { return is_s(expr) || is_k(expr) || is_i(expr) || is_d(expr); }

bool match_app(ast::ExpressionPtr const& expr, auto&& lhs_f, auto&& rhs_f) {
    if (auto const *app = dynamic_cast<ast::Application *>(expr.get())) {
        return std::invoke(std::forward<decltype(lhs_f)>(lhs_f), std::as_const(app->lhs)) &&
               std::invoke(std::forward<decltype(rhs_f)>(rhs_f), std::as_const(app->rhs));
    }
    return false;
}

bool is_pure(ast::ExpressionPtr const& expr) {
    // S pure pure (experimental and undocumented)
    auto spp = match_app(expr, [](auto& expr) { return match_app(expr, ast::is_s, is_pure); }, is_pure);
    return spp || match_app(expr, is_combinator, is_pure) ||       // WARNING: experimental and undocumented
           match_app(expr, ast::is_variable, ast::is_variable) ||  // WARNING: experimental and undocumented
           is_variable(expr) || is_combinator(expr) || match_app(expr, ast::is_d, [](auto&) { return true; });
}

ast::ExpressionPtr apply(ast::ExpressionPtr x, ast::ExpressionPtr y) {
    return std::make_unique<ast::Application>(std::move(x), std::move(y));
}

ast::ExpressionPtr apply_d(ast::ExpressionPtr x) { return apply(std::make_unique<ast::D>(), std::move(x)); }

ast::ExpressionPtr preprocess(ast::ExpressionPtr expr) {
    if (is_abstraction(expr)) {
        auto& abs = static_cast<ast::Abstraction&>(*expr);
        abs.body = preprocess(std::move(abs.body));
        return expr;
    } else if (is_application(expr)) {
        auto& app = static_cast<ast::Application&>(*expr);
        app.lhs = preprocess(std::move(app.lhs));

        if (!is_variable(app.rhs)) {
            app.rhs = apply_d(preprocess(std::move(app.rhs)));
        }

        return expr;
    } else /* is_variable */ {
        return expr;
    }
}

namespace transformations {

ast::ExpressionPtr transform(ast::ExpressionPtr expr);

std::pair<ast::ExpressionPtr, bool> identity(ast::ExpressionPtr expr) {
    if (auto *abs = dynamic_cast<ast::Abstraction *>(expr.get())) {
        if (auto *var = dynamic_cast<ast::Variable *>(abs->body.get())) {
            if (var->name == abs->name) {
                return {std::make_unique<ast::I>(), true};
            }
        }
    }
    return {std::move(expr), false};
}

std::pair<ast::ExpressionPtr, bool> constant_expression(ast::ExpressionPtr expr) {
    if (auto *abs = dynamic_cast<ast::Abstraction *>(expr.get())) {
        if (!mentions(abs->body, abs->name)) {
            // Q: will it be better to check if the body is pure AFTER transforming?
            // Check if there are other places which would benefit from this.
            // This could allow to only check for SKI purity. Instead of writing code to check wheter
            // \x.x x is pure, we can transform to SII and check the S pure pure condition.
            // Idk which consequences this change would make.
            // NEW QUESTION WHICH COULD CLEAR THINGS UP:
            // Does the T function preserve purity? Can it make an unpure function become pure and vice versa?
            if (is_pure(abs->body)) {
                return {apply(std::make_unique<ast::K>(), transform(std::move(abs->body))), true};
            }
            return {apply_d(apply(std::make_unique<ast::K>(), transform(std::move(abs->body)))), true};
        }
    }
    return {std::move(expr), false};
}

std::pair<ast::ExpressionPtr, bool> application_in_abstraction(ast::ExpressionPtr expr) {
    auto *abs = dynamic_cast<ast::Abstraction *>(expr.get());
    if (!abs) {
        return {std::move(expr), false};
    }

    auto *app = dynamic_cast<ast::Application *>(abs->body.get());
    if (!app) {
        return {std::move(expr), false};
    }

    if (!mentions(app->lhs, abs->name)) {
        if (auto *var = dynamic_cast<ast::Variable *>(app->rhs.get())) {
            if (var->name == abs->name) {
                if (is_pure(app->lhs)) {
                    return {transform(std::move(app->lhs)), true};
                }
                return {apply_d(transform(std::move(app->lhs))), true};
            }
        }
    }

    app->lhs = transform(std::make_unique<ast::Abstraction>(abs->name, std::move(app->lhs)));
    app->rhs = transform(std::make_unique<ast::Abstraction>(abs->name, std::move(app->rhs)));
    return {apply(apply(std::make_unique<ast::S>(), std::move(app->lhs)), std::move(app->rhs)), true};
}

std::pair<ast::ExpressionPtr, bool> nested_abstraction(ast::ExpressionPtr expr) {
    auto *abs = dynamic_cast<ast::Abstraction *>(expr.get());
    if (!abs) {
        return {std::move(expr), false};
    }

    if (dynamic_cast<ast::Abstraction *>(abs->body.get())) {
        abs->body = transform(std::move(abs->body));
        return {transform(std::move(expr)), true};
    }
    return {std::move(expr), false};
}

std::pair<ast::ExpressionPtr, bool> application(ast::ExpressionPtr expr) {
    if (auto *app = dynamic_cast<ast::Application *>(expr.get())) {
        app->lhs = transform(std::move(app->lhs));
        app->rhs = transform(std::move(app->rhs));
        return {std::move(expr), true};
    }
    return {std::move(expr), false};
}

std::pair<ast::ExpressionPtr, bool> combinator_or_variable(ast::ExpressionPtr expr) {
    if (is_combinator(expr) || is_variable(expr)) {
        return {std::move(expr), true};
    }
    return {std::move(expr), false};
}

ast::ExpressionPtr transform(ast::ExpressionPtr expr) {
    if (!expr) {
        throw std::logic_error{"Fatal error. Transformation called with a null pointer. Please report this."};
    }

    bool ok;
    std::tie(expr, ok) = identity(std::move(expr));
    if (ok) {
        return expr;
    }
    std::tie(expr, ok) = constant_expression(std::move(expr));
    if (ok) {
        return expr;
    }
    std::tie(expr, ok) = application_in_abstraction(std::move(expr));
    if (ok) {
        return expr;
    }
    std::tie(expr, ok) = nested_abstraction(std::move(expr));
    if (ok) {
        return expr;
    }
    std::tie(expr, ok) = application(std::move(expr));
    if (ok) {
        return expr;
    }
    std::tie(expr, ok) = combinator_or_variable(std::move(expr));
    if (ok) {
        return expr;
    }

    if (!expr) {
        throw std::logic_error{"Fatal error. A transformation lost its parameter. Please report this."};
    }

    throw std::logic_error{
        "This point should never be reachable. Please report this error. Transformations exhausted: " + expr->format()};
}

}  // namespace transformations

}  // namespace

ast::ExpressionPtr conv::to_ski(ast::ExpressionPtr expr) {
    return transformations::transform(preprocess(std::move(expr)));
}