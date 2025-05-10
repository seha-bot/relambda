#include "parser.hpp"

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/file.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/report_error.hpp>

namespace {

namespace grammar {

namespace dsl = lexy::dsl;

struct identifier {
    static constexpr auto rule = [] {
        // Alphabetic character or an underscore.
        auto head = dsl::ascii::alpha_underscore;
        // Alphabetic character or a digit or an underscore.
        auto tail = dsl::ascii::alpha_digit_underscore;
        auto id = dsl::identifier(head, tail);

        auto kw_let = LEXY_KEYWORD("let", id);

        return id.reserve(kw_let);
    }();

    static constexpr auto value = lexy::as_string<std::string>;
};

struct string {
    static constexpr auto escaped_symbols = lexy::symbol_table<char>  //
                                                .map<'"'>('"')
                                                .map<'\\'>('\\')
                                                .map<'n'>('\n');

    static constexpr auto rule = [] {
        // WARNING: Faulty. The grammar does not specify it like this.
        auto c = dsl::ascii::character - dsl::ascii::control;
        auto escape = dsl::backslash_escape.symbol<escaped_symbols>();
        return dsl::quoted(c, escape);
    }();

    static constexpr auto value = lexy::as_string<std::string> >> lexy::new_<ast::String, ast::ExpressionPtr>;
};

struct variable {
    static constexpr auto rule = dsl::p<identifier>;
    static constexpr auto value = lexy::new_<ast::Variable, ast::ExpressionPtr>;
};

struct abstraction {
    static constexpr auto rule =
        dsl::lit_c<'\\'> >> dsl::p<identifier> + dsl::lit_c<'.'> + dsl::recurse<struct expression>;
    static constexpr auto value = lexy::new_<ast::Abstraction, ast::ExpressionPtr>;
};

struct applications {
    static constexpr auto rule = dsl::list(dsl::p<variable> | dsl::p<string> | dsl::parenthesized(dsl::recurse<struct expression>));

    static constexpr auto value =
        lexy::fold<ast::ExpressionPtr>(nullptr, [](ast::ExpressionPtr acc, ast::ExpressionPtr x) -> ast::ExpressionPtr {
            if (!acc) {
                return x;
            }
            return std::make_unique<ast::Application>(std::move(acc), std::move(x));
        });
};

struct expression {
    static constexpr auto rule = dsl::p<abstraction> | dsl::p<applications>;
    static constexpr auto value = lexy::forward<ast::ExpressionPtr>;
};

struct definition {
    static constexpr auto rule = LEXY_LIT("let") >> dsl::p<identifier> + dsl::lit_c<'='> + dsl::p<expression>;
    static constexpr auto value = lexy::construct<ast::Definition>;
};

struct definitions {
    static constexpr auto rule = dsl::list(dsl::p<definition>);
    static constexpr auto value = lexy::as_list<ast::Definitions>;
};

template <typename Production>
struct enable_whitespace : lexy::transparent_production {
    static constexpr auto whitespace = dsl::ascii::space;
    static constexpr auto rule = dsl::p<Production>;
    static constexpr auto value = lexy::forward<typename decltype(Production::value)::return_type>;
};

}  // namespace grammar

}  // namespace

namespace parser {

std::optional<ast::Definitions> parse_file(char const *path) noexcept {
    auto file = lexy::read_file(path);
    if (!file) {
        lexy::file_error err = file.error();
        std::cerr << "reading file failed: ";
        if (err == lexy::file_error::os_error) {
            std::cerr << "os_error";
        } else if (err == lexy::file_error::file_not_found) {
            std::cerr << "file_not_found";
        } else if (err == lexy::file_error::permission_denied) {
            std::cerr << "permission_denied";
        }
        return std::nullopt;
    }

    // TODO: do error handling properly
    auto result = lexy::parse<grammar::enable_whitespace<grammar::definitions>>(file.buffer(), lexy_ext::report_error);
    return result ? std::optional{std::move(result).value()} : std::nullopt;
}

template <typename T, typename G>
std::optional<T> parse_string(std::string_view str) noexcept {
    auto result = lexy::parse<grammar::enable_whitespace<G>>(lexy::string_input(str), lexy_ext::report_error);
    return result ? std::optional{std::move(result).value()} : std::nullopt;
}

std::optional<ast::ExpressionPtr> parse_string_expression(std::string_view str) noexcept {
    return parse_string<ast::ExpressionPtr, grammar::expression>(str);
}

}  // namespace parser
