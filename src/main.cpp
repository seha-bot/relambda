#include <iostream>

#include "converter.hpp"
#include "parser.hpp"

int main(int, char *[]) {
    auto res = parser::parse_string_expression("\\x.x").value();
    std::cout << res->format() << '\n';

    res = conv::to_ski(std::move(res));
    std::cout << res->format() << '\n';

    // std::vector<ast::Definition> const& defs = result.value();
    // for (ast::Definition const& def : defs) {
    //     std::cout << def.name << ": ";
    //     def.value->print();
    //     std::cout << '\n';
    // }
}
