#include <iostream>

#include "parser.hpp"

int main(int argc, char *argv[]) {
    auto res = parser::parse_string_expression("x (x)").value();
    std::cout << res->format() << '\n';

    // std::vector<ast::Definition> const& defs = result.value();
    // for (ast::Definition const& def : defs) {
    //     std::cout << def.name << ": ";
    //     def.value->print();
    //     std::cout << '\n';
    // }
}
