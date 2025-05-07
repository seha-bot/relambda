#include <cstdlib>
#include <iostream>

#include "converter.hpp"
#include "parser.hpp"

// for validation
#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include "ast.hpp"

std::vector<std::string> missing_names(ast::ExpressionPtr& def, std::unordered_set<std::string>& loc_env) {
    if (auto *var = dynamic_cast<ast::Variable *>(def.get())) {
        if (loc_env.contains(var->name)) {
            return {};
        }
        return {var->name};
    } else if (auto *app = dynamic_cast<ast::Application *>(def.get())) {
        auto lhs = missing_names(app->lhs, loc_env);
        auto rhs = missing_names(app->rhs, loc_env);
        lhs.insert(lhs.end(), rhs.begin(), rhs.end());
        return lhs;
    } else if (auto *abs = dynamic_cast<ast::Abstraction *>(def.get())) {
        bool has_inserted = loc_env.insert(abs->name).second;
        auto res = missing_names(abs->body, loc_env);
        if (has_inserted) {
            loc_env.erase(abs->name);
        }
        return res;
    } else {
        throw std::logic_error{"unexpected ast node"};
    }
}

// unfortunately reports to cerr itself
std::optional<std::string> translate(ast::Definitions&& defs, bool do_ski) {
    if (defs.empty()) {
        return "";
    }

    // TODO: refactor this to be checked inside the parser for easy error reporting.
    std::unordered_map<std::string, std::vector<std::string>> report;
    std::unordered_set<std::string> loc_env;
    for (ast::Definition& def : defs) {
        auto has_inserted = report.emplace(def.name, missing_names(def.value, loc_env)).second;
        if (!has_inserted) {
            std::cerr << "multiple definitions for \"" << def.name << "\" detected.\n";
            return std::nullopt;
        }
        def.value = conv::to_ski(std::move(def.value));
    }

    bool really_bad = false;
    for (auto const& [def_name, missing_names] : report) {
        for (std::string const& name : missing_names) {
            if (!report.contains(name)) {
                std::cerr << "undefined name: " << name << '\n';
                really_bad = true;
            }
            if (name == def_name) {
                std::cerr << "circular dependencies not yet allowed for: " << name << '\n';
                really_bad = true;
            }
        }
    }

    if (really_bad) {
        return std::nullopt;
    }

    auto it = std::ranges::find_if(defs, [](ast::Definition const& x) { return x.name == "main"; });
    if (it == defs.end()) {
        std::cerr << "no main detected\n";
        return std::nullopt;
    }
    if (do_ski) {
        return it->value->format();
    }
    return it->value->format_unlambda(defs);

    // auto it = std::ranges::find_if(defs, [](ast::Definition const& x) { return x.name == "Y"; });
    // return it->value->format();
}

int main(int argc, char *argv[]) {
    // auto res = parser::parse_string_expression("\\x.\\y.\\z.x z(y z)").value();
    // res = conv::to_ski(std::move(res));
    // std::cout << res->format() << '\n';
    // std::cout << res->format_unlambda({}) << '\n';

    if (argc == 1) {
        std::cerr << "must provide a filename\n";
        return EXIT_FAILURE;
    }
    auto res = parser::parse_file(argv[1]);
    if (!res) {
        return EXIT_FAILURE;
    }
    ast::Definitions defs = std::move(res).value();
    if (auto res = translate(std::move(defs), argc == 3)) {
        std::cout << *res << '\n';
    }
}
