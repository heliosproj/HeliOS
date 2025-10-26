/*
 * Nomic C Semantic Source Code Analyzer
 * Copyright (C) 2025 Manny Peterson <me@mannypeterson.com>
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

#ifndef NOMIC_DSL_DSL_H
#define NOMIC_DSL_DSL_H

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <any>
#include "nomic/core/semantic_model.h"
#include "nomic/dsl/ast_nodes.h"

namespace nomic::dsl {

// Forward declarations
class DSLParser;
class DSLEvaluator;

// Parser interface
class DSLParser {
public:
    DSLParser();
    ~DSLParser();

    std::unique_ptr<ASTNode> parse(const std::string& expression);
    bool validate(const std::string& expression) const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

// Evaluator interface
class DSLEvaluator {
public:
    struct EvaluationContext {
        const Function* current_function = nullptr;
        const Variable* current_variable = nullptr;
        const Statement* current_statement = nullptr;
        const FileInfo* current_file = nullptr;
        const SemanticDatabase* database = nullptr;
        std::map<std::string, std::any> bindings;
        std::map<std::string, std::vector<std::any>> collections;
    };

    DSLEvaluator();
    ~DSLEvaluator();

    bool evaluateAssertion(const std::string& expression, EvaluationContext& context);
    std::any evaluateExpression(const std::string& expression, EvaluationContext& context);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace nomic::dsl

#endif // NOMIC_DSL_DSL_H