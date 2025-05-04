#include <cstdlib>
#include <iostream>

#include "converter.hpp"
#include "parser.hpp"

// void validate(ast::Definitions&& defs) {
//     std::queue<> ;
// }

int main(int argc, char *argv[]) {
    // auto res = parser::parse_string_expression("\\f.\\x.\\y.f y x").value();
    // std::cout << res->format() << '\n';

    // res = conv::to_ski(std::move(res));
    // std::cout << res->format_unlambda() << '\n';

    if (argc == 1) {
        std::cerr << "must provide a filename\n";
        return EXIT_FAILURE;
    }
    auto res = parser::parse_file(argv[1]);
    if (!res) {
        return EXIT_FAILURE;
    }
    ast::Definitions defs = std::move(res).value();
    // validate(std::move(defs));
}

/*

\x.print I
S (K print) (KI)
delay (K (print I))

\x.W
K W
or
\x.W
\x.SII(SII)
S(\x.SII)(\x.SII)
S(K(SII))(K(SII))
``s `k``sii `k``sii

*/
