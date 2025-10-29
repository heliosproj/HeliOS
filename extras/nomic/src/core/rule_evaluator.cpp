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

#include "nomic/core/rule_engine.h"
#include "nomic/dsl/dsl.h"
#include <spdlog/spdlog.h>
#include <memory>

namespace nomic {

// RuleEvaluator implementation

class RuleEvaluator::Impl {
public:
    Impl() : dsl_evaluator_() {
        spdlog::debug("RuleEvaluator initialized");
    }

    bool evaluateAssertion(const Rule& rule, const Function& func, const SemanticDatabase* db) {
        const std::string& assertion = rule.getAssertExpression();
        if (assertion.empty()) {
            spdlog::warn("Rule {} has no assertion expression", rule.getId());
            return false;
        }

        // Set up evaluation context
        dsl::DSLEvaluator::EvaluationContext context;
        context.current_function = &func;
        context.database = db;

        // Set file context from function location if database available
        if (db) {
            const std::string& file_path = func.getLocation().getStart().getFile();
            if (!file_path.empty()) {
                context.current_file = db->findFileInfo(file_path);
            }
        }

        // Populate collections from function
        populateFunctionCollections(func, context);

        // Evaluate the assertion
        try {
            bool result = dsl_evaluator_.evaluateAssertion(assertion, context);
            spdlog::debug("Rule {} on function {}: {}", rule.getId(), func.getName(),
                         result ? "FAIL (violation)" : "PASS");

            // Return true if assertion is FALSE (i.e., violation detected)
            // The assertion states what SHOULD be true; if it's false, it's a violation
            return !result;
        } catch (const std::exception& e) {
            spdlog::error("Error evaluating rule {} on function {}: {}",
                         rule.getId(), func.getName(), e.what());
            return false;
        }
    }

    bool evaluateAssertion(const Rule& rule, const Variable& var, const SemanticDatabase* db) {
        const std::string& assertion = rule.getAssertExpression();
        if (assertion.empty()) {
            spdlog::warn("Rule {} has no assertion expression", rule.getId());
            return false;
        }

        // Set up evaluation context
        dsl::DSLEvaluator::EvaluationContext context;
        context.current_variable = &var;
        context.database = db;

        // Set file context from variable location if database available
        if (db) {
            const std::string& file_path = var.getLocation().getStart().getFile();
            if (!file_path.empty()) {
                context.current_file = db->findFileInfo(file_path);
            }
        }

        // Evaluate the assertion
        try {
            bool result = dsl_evaluator_.evaluateAssertion(assertion, context);
            spdlog::debug("Rule {} on variable {}: {}", rule.getId(), var.getName(),
                         result ? "PASS" : "FAIL");

            // Return true if assertion FAILS (i.e., violation detected)
            return !result;
        } catch (const std::exception& e) {
            spdlog::error("Error evaluating rule {} on variable {}: {}",
                         rule.getId(), var.getName(), e.what());
            return false;
        }
    }

    std::any evaluateExpression(const std::string& expression, dsl::DSLEvaluator::EvaluationContext& context) {
        try {
            return dsl_evaluator_.evaluateExpression(expression, context);
        } catch (const std::exception& e) {
            spdlog::error("Error evaluating expression '{}': {}", expression, e.what());
            return std::any();
        }
    }

private:
    void populateFunctionCollections(const Function& func,
                                     dsl::DSLEvaluator::EvaluationContext& context) {
        // Populate parameters collection
        std::vector<std::any> params;
        for (const auto& param : func.getParameters()) {
            params.push_back(std::any(&param));
        }
        context.collections["parameters"] = params;

        // Populate local_vars collection
        std::vector<std::any> local_vars;
        for (const auto& var : func.getLocalVariables()) {
            local_vars.push_back(std::any(&var));
        }
        context.collections["local_vars"] = local_vars;

        // Populate calls collection
        std::vector<std::any> calls;
        for (const auto& call : func.getCallSites()) {
            calls.push_back(std::any(&call));
        }
        context.collections["calls"] = calls;

        // Populate statements collection
        std::vector<std::any> statements;
        for (const auto& stmt : func.getStatements()) {
            statements.push_back(std::any(&stmt));
        }
        context.collections["statements"] = statements;

        // TODO: Add more collections as needed
        // - array_accesses
        // - return_statements
        // - etc.
    }

    dsl::DSLEvaluator dsl_evaluator_;
};

// Public interface implementation

RuleEvaluator::RuleEvaluator() : pImpl(std::make_unique<Impl>()) {}

RuleEvaluator::~RuleEvaluator() = default;

bool RuleEvaluator::evaluateAssertion(const Rule& rule, const Function& func, const SemanticDatabase* db) {
    return pImpl->evaluateAssertion(rule, func, db);
}

bool RuleEvaluator::evaluateAssertion(const Rule& rule, const Variable& var, const SemanticDatabase* db) {
    return pImpl->evaluateAssertion(rule, var, db);
}

std::any RuleEvaluator::evaluateExpression(const std::string& expression, dsl::DSLEvaluator::EvaluationContext& context) {
    return pImpl->evaluateExpression(expression, context);
}

} // namespace nomic