#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <memory>
#include <string>
#include <utility>

// input & parse action
#include <iostream>
#include <lexy/action/parse.hpp>
#include <lexy/input/argv_input.hpp>
#include <lexy_ext/report_error.hpp>

namespace ast {

struct Expression {
    virtual ~Expression() = 0;
    virtual void print() const = 0;
};
Expression::~Expression() = default;

struct Variable : Expression {
    Variable(std::string name) : name(std::move(name)) {}
    std::string name;

    void print() const override { std::cout << name; }
};

struct Application : Expression {
    Application(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs)
        : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    std::unique_ptr<Expression> lhs, rhs;

    void print() const override {
        std::cout << '(';
        lhs->print();
        std::cout << ") (";
        rhs->print();
        std::cout << ')';
    }
};

struct Abstraction : Expression {
    Abstraction(std::string name, std::unique_ptr<Expression> body) : name(std::move(name)), body(std::move(body)) {}
    std::string name;
    std::unique_ptr<Expression> body;

    void print() const override {
        std::cout << "\\" << name << '.';
        body->print();
    }
};

}  // namespace ast

namespace {

namespace grammar {

namespace dsl = lexy::dsl;

struct whitespace {
    static constexpr auto rule = dsl::whitespace(dsl::ascii::space);
};
static constexpr auto ws = dsl::inline_<whitespace>;

struct identifier {
    static constexpr auto rule = [] {
        auto head = dsl::ascii::alpha_underscore;
        auto tail = dsl::ascii::alpha_digit_underscore;
        auto id = dsl::identifier(head, tail);

        auto kw_let = LEXY_KEYWORD("let", id);

        return id.reserve(kw_let);
    }();

    static constexpr auto value = lexy::as_string<std::string>;
};

struct variable {
    static constexpr auto rule = dsl::p<identifier>;
    static constexpr auto value = lexy::new_<ast::Variable, std::unique_ptr<ast::Expression>>;
};

struct abstraction {
    static constexpr auto rule =
        dsl::lit_c<'\\'> >> ws + dsl::p<identifier> + ws + dsl::lit_c<'.'> + ws + dsl::recurse<struct expression>;
    static constexpr auto value = lexy::new_<ast::Abstraction, std::unique_ptr<ast::Expression>>;
};

struct nothing {
    static constexpr auto rule = dsl::nullopt;
    static inline constexpr auto value = lexy::callback<std::unique_ptr<ast::Expression>>(
        [](lexy::nullopt) { return std::unique_ptr<ast::Expression>{}; });
};

struct subexpression : lexy::expression_production {
    static constexpr auto atom =
        dsl::p<variable> | dsl::parenthesized(dsl::recurse<struct expression>) | dsl::else_ >> dsl::p<nothing>;

    struct application : dsl::infix_op_left {
        static constexpr auto op = dsl::op(dsl::lit_c<' '> >> dsl::while_(dsl::lit_c<' '>));
        using operand = dsl::atom;
    };

    using operation = application;

    static constexpr auto value = lexy::callback<std::unique_ptr<ast::Expression>>(
        [](std::unique_ptr<ast::Expression> x) { return x; },
        [](std::unique_ptr<ast::Expression> lhs, lexy::op<application::op>,
           std::unique_ptr<ast::Expression> rhs) -> std::unique_ptr<ast::Expression> {
            if (!lhs) return rhs;
            if (!rhs) return lhs;
            return std::make_unique<ast::Application>(std::move(lhs), std::move(rhs));
        });
};

struct expression {
    static constexpr auto rule = dsl::p<abstraction> | dsl::else_ >> dsl::p<subexpression>;
    static constexpr auto value = lexy::forward<std::unique_ptr<ast::Expression>>;
};

// struct definition {
//     static constexpr auto rule = [] {
//         return LEXY_LIT("let") + dsl::p<identifier> + dsl::lit_c<'='> + dsl::p<expression>;
//     }();
// };

}  // namespace grammar

}  // namespace

int main(int argc, char *argv[]) {
    auto input = lexy::argv_input(argc, argv);
    auto result = lexy::parse<grammar::expression>(input, lexy_ext::report_error);
    if (!result) {
        return 1;
    }

    std::unique_ptr<ast::Expression> const& val = result.value();
    val->print();
    std::cout << '\n';
}
