#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <string>
#include <string_view>

#include "converter.hpp"
#include "parser.hpp"

std::string parse(std::string_view src) {
    std::optional<ast::ExpressionPtr> res = parser::parse_string_expression(src);
    if (!res) {
        throw std::runtime_error{"parsing failed"};
    }
    return (*res)->format();
}

std::string convert(std::string_view src) {
    std::optional<ast::ExpressionPtr> res = parser::parse_string_expression(src);
    if (!res) {
        throw std::runtime_error{"parsing failed"};
    }
    res = conv::to_ski(*std::move(res));
    return (*res)->format();
}

TEST_CASE("Expression parsing", "[expression]") {
    REQUIRE(parse("x x") == "x x");
    REQUIRE(parse("x ( x  x ) ") == "x (x x)");
    REQUIRE(parse("\\x.x") == "\\x.x");
    REQUIRE(parse("\\x.x x") == "\\x.x x");
    REQUIRE(parse("  \\   x . x  x  ") == "\\x.x x");
    REQUIRE(parse("(\\x.x) (\\x.x)") == "(\\x.x) (\\x.x)");
    REQUIRE(parse("\\x.x \\x.x") == "\\x.x");
    REQUIRE(parse("\\x.x (\\x.x)") == "\\x.x (\\x.x)");
}

TEST_CASE("SKI conversion", "[ski]") {
    REQUIRE(convert("\\x.\\y.\\z.x z(y z)") == "S");
    REQUIRE(convert("\\x.\\y.x") == "K");
    REQUIRE(convert("\\x.x") == "I");
    REQUIRE(convert("\\x.x x") == "S I I");
    REQUIRE(convert("(\\x.x x)  (\\x.x x)") == "S I I (S I I)");
    REQUIRE(convert("\\f.(\\x.f(x x))(\\x.f(x x))") == "S (S (S (K S) K) (K (S I I))) (S (S (K S) K) (K (S I I)))");
    REQUIRE(convert("\\f.(\\x.x x)(\\x.f(x x))") == "S (K (S I I)) (S (S (K S) K) (K (S I I)))");
    REQUIRE(convert("\\f.\\x.\\y.f y x") == "S (S (K S) (S (K K) S)) (K K)");
}
