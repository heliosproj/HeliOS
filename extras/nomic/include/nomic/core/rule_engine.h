/*
 * Nomic C Semantic Source Code Analyzer
 * (C) 2020-2026 Manny Peterson <manny@heliosproject.org>
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

#ifndef NOMIC_CORE_RULE_ENGINE_H
#define NOMIC_CORE_RULE_ENGINE_H

#include "nomic/core/semantic_model.h"
#include "nomic/dsl/dsl.h"
#include <string>
#include <vector>
#include <map>
#include <any>
#include <functional>
#include <thread>
#include <chrono>
#include <iostream>

// Forward declaration for YAML
namespace YAML {
    class Node;
}

namespace nomic {

// Forward declarations
class Rule;
class Violation;

/**
 * @brief Rule evaluator for DSL expressions
 * Note: Defined here because RuleEngine uses unique_ptr which needs complete type
 */
class RuleEvaluator {
public:
    RuleEvaluator();
    ~RuleEvaluator();

    bool evaluateAssertion(const Rule& rule, const Function& func, const SemanticDatabase* db = nullptr);
    bool evaluateAssertion(const Rule& rule, const Variable& var, const SemanticDatabase* db = nullptr);
    std::any evaluateExpression(const std::string& expression, dsl::DSLEvaluator::EvaluationContext& context);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

/**
 * @brief Severity levels for rule violations
 */
enum class Severity {
    Info,
    Warning,
    Error
};

/**
 * @brief Rule definition
 */
class Rule {
public:
    Rule(const std::string& id, const std::string& description, Severity severity)
        : id_(id), description_(description), severity_(severity) {}

    // Setters
    void setScope(const std::string& scope) { scope_ = scope; }
    void setSelectExpression(const std::string& expr) { select_expression_ = expr; }
    void setWhereClause(const std::string& clause) { where_clause_ = clause; }
    void setAssertExpression(const std::string& expr) { assert_expression_ = expr; }
    void setMessage(const std::string& msg) { message_ = msg; }
    void setFixit(const std::string& fixit) { fixit_ = fixit; }
    void addException(const std::string& exception) { exceptions_.push_back(exception); }
    void addTag(const std::string& tag) { tags_.push_back(tag); }

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getDescription() const { return description_; }
    Severity getSeverity() const { return severity_; }
    const std::string& getScope() const { return scope_; }
    const std::string& getSelectExpression() const { return select_expression_; }
    const std::optional<std::string>& getWhereClause() const { return where_clause_; }
    const std::string& getAssertExpression() const { return assert_expression_; }
    const std::string& getMessage() const { return message_; }
    const std::optional<std::string>& getFixit() const { return fixit_; }
    const std::vector<std::string>& getExceptions() const { return exceptions_; }
    const std::vector<std::string>& getTags() const { return tags_; }

private:
    std::string id_;
    std::string description_;
    Severity severity_;
    std::string scope_;
    std::string select_expression_;
    std::optional<std::string> where_clause_;
    std::string assert_expression_;
    std::string message_;
    std::optional<std::string> fixit_;
    std::vector<std::string> exceptions_;
    std::vector<std::string> tags_;
};

/**
 * @brief Represents a rule violation
 */
class Violation {
public:
    Violation(const Rule& rule, const SourceRange& location, const std::string& message)
        : rule_id_(rule.getId()),
          severity_(rule.getSeverity()),
          location_(location),
          message_(message) {}

    // Setters
    void setContext(const std::string& scope, const std::string& name) {
        context_scope_ = scope;
        context_name_ = name;
    }
    void setSnippet(const std::string& snippet) { snippet_ = snippet; }
    void setSuggestedFix(const std::string& fix, const std::string& diff) {
        suggested_fix_description_ = fix;
        suggested_fix_diff_ = diff;
    }

    // Getters
    const std::string& getRuleId() const { return rule_id_; }
    Severity getSeverity() const { return severity_; }
    const SourceRange& getLocation() const { return location_; }
    const std::string& getMessage() const { return message_; }
    const std::optional<std::string>& getContextScope() const { return context_scope_; }
    const std::optional<std::string>& getContextName() const { return context_name_; }
    const std::optional<std::string>& getSnippet() const { return snippet_; }
    const std::optional<std::string>& getSuggestedFixDescription() const { return suggested_fix_description_; }
    const std::optional<std::string>& getSuggestedFixDiff() const { return suggested_fix_diff_; }

private:
    std::string rule_id_;
    Severity severity_;
    SourceRange location_;
    std::string message_;
    std::optional<std::string> context_scope_;
    std::optional<std::string> context_name_;
    std::optional<std::string> snippet_;
    std::optional<std::string> suggested_fix_description_;
    std::optional<std::string> suggested_fix_diff_;
};

/**
 * @brief Rule set configuration
 */
struct RuleSet {
    std::vector<Rule> rules;
    std::map<std::string, std::string> global_settings;
    std::vector<std::string> enabled_categories;
    std::vector<std::string> disabled_rules;
};

/**
 * @brief Execution options for rule engine
 */
struct ExecutionOptions {
    bool parallel_execution = true;
    unsigned max_threads = std::thread::hardware_concurrency();
    bool stop_on_first_error = false;
    bool enable_fixits = true;
    std::vector<std::string> file_filter;
    std::function<void(const Rule&, float)> progress_callback;
};

/**
 * @brief Cache for rule evaluation optimization
 */
class RuleCache {
public:
    void invalidate() {
        entity_cache_.clear();
        evaluation_cache_.clear();
        last_invalidation_ = std::chrono::steady_clock::now();
    }

    template<typename T>
    void cacheEntity(const std::string& key, T* entity) {
        // Implementation in source file
    }

    template<typename T>
    T* getCachedEntity(const std::string& key) const {
        // Implementation in source file
        return nullptr;
    }

    void cacheEvaluation(const std::string& key, bool result) {
        evaluation_cache_[key] = result;
    }

    std::optional<bool> getCachedEvaluation(const std::string& key) const {
        auto it = evaluation_cache_.find(key);
        if (it != evaluation_cache_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

private:
    std::map<std::string, void*> entity_cache_;
    std::map<std::string, bool> evaluation_cache_;
    std::chrono::steady_clock::time_point last_invalidation_;
};

/**
 * @brief Main rule engine for executing rules
 */
class RuleEngine {
public:
    RuleEngine();
    ~RuleEngine();

    // Rule loading and validation
    RuleSet loadRules(const std::string& yaml_file);
    RuleSet loadRulesFromString(const std::string& yaml_content);
    RuleSet loadRulesFromDirectory(const std::string& directory_path);
    bool validateRule(const Rule& rule);
    void addRule(const Rule& rule);
    void clearRules();

    // Rule execution
    std::vector<Violation> executeRules(const RuleSet& rules,
                                        const SemanticDatabase& db,
                                        const ExecutionOptions& options = {});

    std::vector<Violation> executeRule(const Rule& rule,
                                       const SemanticDatabase& db);

    // Configuration
    void setMaxThreads(unsigned num_threads);
    void enableCaching(bool enable);
    void setProgressCallback(std::function<void(const Rule&, float)> callback);

    // Statistics
    struct ExecutionStats {
        size_t total_rules_executed = 0;
        size_t total_entities_checked = 0;
        size_t total_violations_found = 0;
        size_t files_analyzed = 0;
        size_t functions_analyzed = 0;
        size_t variables_analyzed = 0;
        std::chrono::milliseconds total_execution_time{0};
        std::map<std::string, std::chrono::milliseconds> rule_execution_times;
        std::map<std::string, size_t> violations_per_rule;
        std::map<std::string, size_t> violations_per_file;
        std::map<Severity, size_t> violations_per_severity;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;
    };

    const ExecutionStats& getLastExecutionStats() const { return last_stats_; }
    void printStatistics(std::ostream& out = std::cout) const;

private:
    // Parallel execution support
    std::vector<Violation> executeRulesParallel(const RuleSet& rules,
                                                 const SemanticDatabase& db,
                                                 unsigned num_threads);

    // Sequential execution
    std::vector<Violation> executeRulesSequential(const RuleSet& rules,
                                                  const SemanticDatabase& db);

    // Rule filtering
    std::vector<Rule> filterRules(const RuleSet& rule_set) const;

    // Entity selection
    template<typename T>
    std::vector<T*> selectEntities(const std::string& scope,
                                   const SemanticDatabase& db) const;

    // Message formatting helpers
    std::string formatMessage(const std::string& template_msg, const Function& func);
    std::string formatMessage(const std::string& template_msg, const Variable& var);

    // Rule parsing helpers
    Rule parseRule(const YAML::Node& node);
    Severity parseSeverity(const std::string& severity_str);

    // Rule evaluation helpers
    bool evaluateRuleOnEntity(const Rule& rule, const Function& func);
    bool evaluateRuleOnEntity(const Rule& rule, const Variable& var);

    // Violation creation helpers
    Violation createViolation(const Rule& rule, const Function& func);
    Violation createViolation(const Rule& rule, const Variable& var);

private:
    std::vector<Rule> rules_;
    RuleCache rule_cache_;
    std::unique_ptr<RuleEvaluator> evaluator_;
    ExecutionStats last_stats_;
    const SemanticDatabase* current_db_ = nullptr;

    // Configuration
    unsigned max_threads_ = std::thread::hardware_concurrency();
    bool caching_enabled_ = true;
    std::function<void(const Rule&, float)> progress_callback_;
};

} // namespace nomic

#endif // NOMIC_CORE_RULE_ENGINE_H