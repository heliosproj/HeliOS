/*
 * Nomic C Semantic Source Code Analyzer
 * (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 * This file is part of Nomic.
 *
 * Nomic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nomic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nomic. If not, see <https://www.gnu.org/licenses/>.
 */

#include "nomic/dsl/dsl.h"
#include "nomic/dsl/tokenizer.h"
#include "nomic/dsl/parser.h"
#include <spdlog/spdlog.h>

namespace nomic::dsl {

// DSLParser::Impl implementation
class DSLParser::Impl {
public:
    Impl() {
        spdlog::debug("DSLParser initialized");
    }

    std::unique_ptr<ASTNode> parse(const std::string& expression) {
        spdlog::debug("Parsing DSL expression: {}", expression);

        try {
            // Tokenize
            Tokenizer tokenizer(expression);
            auto tokens = tokenizer.tokenize();

            // Parse
            Parser parser(tokens);
            return parser.parse();
        } catch (const std::exception& e) {
            spdlog::error("Failed to parse expression '{}': {}", expression, e.what());
            return nullptr;
        }
    }

    bool validate(const std::string& expression) {
        auto ast = parse(expression);
        return ast != nullptr;
    }
};

// DSLParser public interface implementation
DSLParser::DSLParser() : pImpl(std::make_unique<Impl>()) {}

DSLParser::~DSLParser() = default;

std::unique_ptr<ASTNode> DSLParser::parse(const std::string& expression) {
    return pImpl->parse(expression);
}

bool DSLParser::validate(const std::string& expression) const {
    return pImpl->validate(expression);
}

} // namespace nomic::dsl