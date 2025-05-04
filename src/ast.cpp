#include "ast.hpp"

namespace ast {

bool is_variable(ast::ExpressionPtr const& expr) { return dynamic_cast<ast::Variable *>(expr.get()) != nullptr; }
bool is_abstraction(ast::ExpressionPtr const& expr) { return dynamic_cast<ast::Abstraction *>(expr.get()) != nullptr; }
bool is_application(ast::ExpressionPtr const& expr) { return dynamic_cast<ast::Application *>(expr.get()) != nullptr; }
bool is_s(ast::ExpressionPtr const& expr) { return dynamic_cast<ast::S *>(expr.get()) != nullptr; }
bool is_k(ast::ExpressionPtr const& expr) { return dynamic_cast<ast::K *>(expr.get()) != nullptr; }
bool is_i(ast::ExpressionPtr const& expr) { return dynamic_cast<ast::I *>(expr.get()) != nullptr; }

}  // namespace ast
