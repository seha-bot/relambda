#ifndef PARSER_HPP
#define PARSER_HPP

#include <optional>
#include <string_view>

#include "ast.hpp"

namespace parser {

/// @brief Parses file for definitions and reports errors to cerr.
/// @return Parsed result on success, nullopt otherwise.
std::optional<ast::Definitions> parse_file(char const *path) noexcept;

std::optional<ast::ExpressionPtr> parse_string_expression(std::string_view str) noexcept;

}  // namespace parser

#endif
