#include "ast.hpp"

namespace ast {

bool is_variable(ExpressionPtr const& expr) { return dynamic_cast<Variable *>(expr.get()) != nullptr; }
bool is_abstraction(ExpressionPtr const& expr) { return dynamic_cast<Abstraction *>(expr.get()) != nullptr; }
bool is_application(ExpressionPtr const& expr) { return dynamic_cast<Application *>(expr.get()) != nullptr; }
bool is_s(ExpressionPtr const& expr) { return dynamic_cast<S *>(expr.get()) != nullptr; }
bool is_k(ExpressionPtr const& expr) { return dynamic_cast<K *>(expr.get()) != nullptr; }
bool is_i(ExpressionPtr const& expr) { return dynamic_cast<I *>(expr.get()) != nullptr; }
bool is_d(ExpressionPtr const& expr) { return dynamic_cast<D *>(expr.get()) != nullptr; }
bool is_d_app(ExpressionPtr const& expr) {
    if (is_application(expr)) {
        return is_d(dynamic_cast<Application*>(expr.get())->lhs);
    }
    return false;
 }

}  // namespace ast
