#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <string>
#include <string_view>

#include "parser.hpp"

std::string parse(std::string_view src) {
    std::optional<ast::ExpressionPtr> res = parser::parse_string_expression(src);
    if (!res) {
        throw std::runtime_error{"parsing failed"};
    }
    return (*res)->format();
}

TEST_CASE("Expression", "[expression]") {
    REQUIRE(parse("x x") == "(x) (x)");
    REQUIRE(parse("x ( x  x ) ") == "(x) ((x) (x))");
    REQUIRE(parse("\\x.x") == "\\x.x");
    REQUIRE(parse("\\x.x x") == "\\x.(x) (x)");
    REQUIRE(parse("  \\   x . x  x  ") == "\\x.(x) (x)");
    REQUIRE(parse("(\\x.x) (\\x.x)") == "(\\x.x) (\\x.x)");
    REQUIRE(parse("\\x.x \\x.x") == "\\x.x");
    REQUIRE(parse("\\x.x (\\x.x)") == "\\x.(x) (\\x.x)");
}
