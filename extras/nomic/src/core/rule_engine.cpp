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

#include "nomic/core/rule_engine.h"
#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <filesystem>
#include <thread>
#include <mutex>
#include <future>
#include <algorithm>

namespace nomic {

// Helper function to convert std::any to string
static std::string anyToString(const std::any& value) {
    if (!value.has_value()) {
        return "[empty]";
    }

    try {
        if (value.type() == typeid(std::string)) {
            return std::any_cast<std::string>(value);
        } else if (value.type() == typeid(int)) {
            return std::to_string(std::any_cast<int>(value));
        } else if (value.type() == typeid(double)) {
            return std::to_string(std::any_cast<double>(value));
        } else if (value.type() == typeid(bool)) {
            return std::any_cast<bool>(value) ? "true" : "false";
        } else if (value.type() == typeid(const char*)) {
            return std::string(std::any_cast<const char*>(value));
        } else if (value.type() == typeid(long)) {
            return std::to_string(std::any_cast<long>(value));
        } else if (value.type() == typeid(unsigned)) {
            return std::to_string(std::any_cast<unsigned>(value));
        } else if (value.type() == typeid(float)) {
            return std::to_string(std::any_cast<float>(value));
        } else if (value.type() == typeid(size_t)) {
            return std::to_string(std::any_cast<size_t>(value));
        }
    } catch (const std::bad_any_cast& e) {
        spdlog::warn("Failed to cast any to string: {}", e.what());
        return "[bad cast]";
    }

    return "[unknown type: " + std::string(value.type().name()) + "]";
}

// RuleEngine implementation
RuleEngine::RuleEngine() : evaluator_(std::make_unique<RuleEvaluator>()) {
    spdlog::debug("RuleEngine initialized");
}

RuleEngine::~RuleEngine() = default;

RuleSet RuleEngine::loadRules(const std::string& yaml_file) {
    spdlog::info("Loading rules from: {}", yaml_file);

    // Check if it's a directory
    if (std::filesystem::is_directory(yaml_file)) {
        return loadRulesFromDirectory(yaml_file);
    }

    std::ifstream file(yaml_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open rules file: " + yaml_file);
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());

    return loadRulesFromString(content);
}

RuleSet RuleEngine::loadRulesFromDirectory(const std::string& directory_path) {
    RuleSet rule_set;

    try {
        // Check if directory exists
        if (!std::filesystem::exists(directory_path)) {
            throw std::runtime_error("Rules directory does not exist: " + directory_path);
        }

        if (!std::filesystem::is_directory(directory_path)) {
            throw std::runtime_error("Path is not a directory: " + directory_path);
        }

        spdlog::info("Loading rules from directory: {}", directory_path);

        int file_count = 0;
        int total_rules = 0;

        // Iterate through all YAML files in directory
        for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
            if (!entry.is_regular_file()) {
                continue;
            }

            const std::string ext = entry.path().extension().string();

            // Check for .yaml or .yml extension
            if (ext != ".yaml" && ext != ".yml") {
                spdlog::debug("Skipping non-YAML file: {}", entry.path().string());
                continue;
            }

            try {
                // Load rules from this file
                RuleSet file_rules = loadRules(entry.path().string());

                // Merge rules into main set
                rule_set.rules.insert(rule_set.rules.end(),
                                     file_rules.rules.begin(),
                                     file_rules.rules.end());

                // Merge global settings (later files can override)
                for (const auto& setting : file_rules.global_settings) {
                    rule_set.global_settings[setting.first] = setting.second;
                }

                total_rules += file_rules.rules.size();
                file_count++;

            } catch (const std::exception& e) {
                spdlog::error("Failed to load {}: {}", entry.path().string(), e.what());
                // Continue with next file
            }
        }

        spdlog::info("Loaded {} total rules from {} files", total_rules, file_count);

    } catch (const std::exception& e) {
        spdlog::error("Error loading rules from directory: {}", e.what());
        throw;
    }

    return rule_set;
}

RuleSet RuleEngine::loadRulesFromString(const std::string& yaml_content) {
    RuleSet rule_set;

    try {
        YAML::Node root = YAML::Load(yaml_content);

        // Determine if rules are at top-level or in structured format
        YAML::Node rules_node;
        bool has_rules = false;

        if (root.IsSequence()) {
            // Rules are at the top level as a sequence (e.g., - id: RULE-001)
            spdlog::debug("Detected top-level sequence format");
            rules_node = root;
            has_rules = true;
        } else if (root["rules"]) {
            // Rules are under 'rules' key
            spdlog::debug("Detected structured format with 'rules' key");
            rules_node = root["rules"];
            has_rules = true;

            // Load metadata if present
            if (root["metadata"]) {
                const auto& metadata = root["metadata"];
                if (metadata["version"]) {
                    rule_set.global_settings["version"] = metadata["version"].as<std::string>();
                }
                if (metadata["description"]) {
                    rule_set.global_settings["description"] = metadata["description"].as<std::string>();
                }
            }

            // Load global settings
            if (root["global_settings"]) {
                const auto& settings = root["global_settings"];
                for (const auto& setting : settings) {
                    rule_set.global_settings[setting.first.as<std::string>()] =
                        setting.second.as<std::string>();
                }
            }
        }

        // Load rules if found
        if (has_rules && rules_node.IsSequence()) {
            for (const auto& rule_node : rules_node) {
                try {
                    Rule rule = parseRule(rule_node);
                    if (validateRule(rule)) {
                        rule_set.rules.push_back(rule);
                    } else {
                        spdlog::warn("Invalid rule skipped: {}", rule.getId());
                    }
                } catch (const std::exception& e) {
                    spdlog::error("Failed to parse rule: {}", e.what());
                    // Continue with next rule
                }
            }
        }

        spdlog::info("Loaded {} rules", rule_set.rules.size());

    } catch (const YAML::Exception& e) {
        throw std::runtime_error("Failed to parse YAML: " + std::string(e.what()));
    }

    return rule_set;
}

Rule RuleEngine::parseRule(const YAML::Node& node) {
    // Parse required fields
    std::string id = node["id"].as<std::string>();
    std::string description = node["description"].as<std::string>();

    std::string severity_str = node["severity"].as<std::string>("warning");
    Severity severity = parseSeverity(severity_str);

    Rule rule(id, description, severity);

    // Parse optional fields
    if (node["scope"]) {
        rule.setScope(node["scope"].as<std::string>());
    }

    if (node["select"]) {
        rule.setSelectExpression(node["select"].as<std::string>());
    }

    if (node["where"]) {
        rule.setWhereClause(node["where"].as<std::string>());
    }

    if (node["assert"]) {
        rule.setAssertExpression(node["assert"].as<std::string>());
    }

    if (node["message"]) {
        rule.setMessage(node["message"].as<std::string>());
    }

    if (node["fixit"]) {
        rule.setFixit(node["fixit"].as<std::string>());
    }

    if (node["exceptions"]) {
        for (const auto& exception : node["exceptions"]) {
            rule.addException(exception.as<std::string>());
        }
    }

    if (node["tags"]) {
        for (const auto& tag : node["tags"]) {
            rule.addTag(tag.as<std::string>());
        }
    }

    return rule;
}

Severity RuleEngine::parseSeverity(const std::string& severity_str) {
    if (severity_str == "error") {
        return Severity::Error;
    } else if (severity_str == "warning") {
        return Severity::Warning;
    } else {
        return Severity::Info;
    }
}

bool RuleEngine::validateRule(const Rule& rule) {
    // Basic validation
    if (rule.getId().empty()) {
        spdlog::error("Rule has empty ID");
        return false;
    }

    if (rule.getDescription().empty()) {
        spdlog::warn("Rule {} has empty description", rule.getId());
    }

    if (rule.getScope().empty()) {
        spdlog::error("Rule {} has no scope", rule.getId());
        return false;
    }

    if (rule.getAssertExpression().empty()) {
        spdlog::error("Rule {} has no assert expression", rule.getId());
        return false;
    }

    // Note: DSL expression syntax validation is deferred to runtime evaluation
    // The parser doesn't throw exceptions for invalid syntax, it logs warnings
    // and may return partial results, so static validation is not reliable

    return true;
}

void RuleEngine::addRule(const Rule& rule) {
    if (validateRule(rule)) {
        rules_.push_back(rule);
    }
}

void RuleEngine::clearRules() {
    rules_.clear();
    rule_cache_.invalidate();
}

std::vector<Violation> RuleEngine::executeRules(const RuleSet& rules,
                                                 const SemanticDatabase& db,
                                                 const ExecutionOptions& options) {
    auto start_time = std::chrono::steady_clock::now();

    // Reset statistics
    last_stats_ = ExecutionStats();
    last_stats_.start_time = start_time;

    // Gather database statistics
    last_stats_.files_analyzed = db.getFiles().size();
    last_stats_.functions_analyzed = db.getFunctions().size();
    last_stats_.variables_analyzed = db.getGlobalVariables().size();

    // Filter rules based on enabled/disabled lists
    std::vector<Rule> filtered_rules = filterRules(rules);

    spdlog::info("Executing {} rules on {} files ({} functions, {} variables)",
                filtered_rules.size(),
                last_stats_.files_analyzed,
                last_stats_.functions_analyzed,
                last_stats_.variables_analyzed);

    std::vector<Violation> all_violations;

    if (options.parallel_execution && filtered_rules.size() > 1) {
        all_violations = executeRulesParallel(rules, db, options.max_threads);
    } else {
        all_violations = executeRulesSequential(rules, db);
    }

    // Update statistics
    auto end_time = std::chrono::steady_clock::now();
    last_stats_.end_time = end_time;
    last_stats_.total_execution_time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    last_stats_.total_violations_found = all_violations.size();

    // Compute per-rule and per-file statistics
    for (const auto& violation : all_violations) {
        last_stats_.violations_per_rule[violation.getRuleId()]++;
        last_stats_.violations_per_file[violation.getLocation().getStart().getFile()]++;
        last_stats_.violations_per_severity[violation.getSeverity()]++;
    }

    spdlog::info("Rule execution completed: {} violations found in {}ms",
                all_violations.size(),
                last_stats_.total_execution_time.count());

    return all_violations;
}

std::vector<Violation> RuleEngine::executeRule(const Rule& rule,
                                               const SemanticDatabase& db) {
    std::vector<Violation> violations;

    spdlog::debug("Executing rule: {}", rule.getId());
    auto start_time = std::chrono::steady_clock::now();

    // Set current database for file context access
    current_db_ = &db;

    try {
        // Select entities based on scope (case-insensitive comparison)
        std::string scope_lower = rule.getScope();
        std::transform(scope_lower.begin(), scope_lower.end(), scope_lower.begin(), ::tolower);

        if (scope_lower == "function") {
            for (const auto& func : db.getFunctions()) {
                if (evaluateRuleOnEntity(rule, *func)) {
                    violations.push_back(createViolation(rule, *func));
                }
            }
        } else if (scope_lower == "variable") {
            for (const auto& var : db.getGlobalVariables()) {
                if (evaluateRuleOnEntity(rule, *var)) {
                    violations.push_back(createViolation(rule, *var));
                }
            }
        }
        // Add more scopes as needed

        last_stats_.total_rules_executed++;

    } catch (const std::exception& e) {
        spdlog::error("Error executing rule {}: {}", rule.getId(), e.what());
    }

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    last_stats_.rule_execution_times[rule.getId()] = duration;

    return violations;
}

bool RuleEngine::evaluateRuleOnEntity(const Rule& rule, const Function& func) {
    // Check cache first
    std::string cache_key = rule.getId() + ":" + func.getName();
    auto cached = rule_cache_.getCachedEvaluation(cache_key);
    if (cached.has_value()) {
        return cached.value();
    }

    // Use RuleEvaluator to evaluate the assertion (pass database for file context)
    bool result = false;
    if (evaluator_) {
        result = evaluator_->evaluateAssertion(rule, func, current_db_);
    } else {
        spdlog::warn("RuleEvaluator not initialized");
    }

    // Cache result
    rule_cache_.cacheEvaluation(cache_key, result);

    last_stats_.total_entities_checked++;

    return result;
}

bool RuleEngine::evaluateRuleOnEntity(const Rule& rule, const Variable& var) {
    // Check cache first
    std::string cache_key = rule.getId() + ":" + var.getName();
    auto cached = rule_cache_.getCachedEvaluation(cache_key);
    if (cached.has_value()) {
        return cached.value();
    }

    // Use RuleEvaluator to evaluate the assertion (pass database for file context)
    bool result = false;
    if (evaluator_) {
        result = evaluator_->evaluateAssertion(rule, var, current_db_);
    } else {
        spdlog::warn("RuleEvaluator not initialized");
    }

    // Cache result
    rule_cache_.cacheEvaluation(cache_key, result);

    last_stats_.total_entities_checked++;

    return result;
}

Violation RuleEngine::createViolation(const Rule& rule, const Function& func) {
    Violation violation(rule, func.getLocation(),
                       formatMessage(rule.getMessage(), func));

    violation.setContext("Function", func.getName());

    // Add suggested fix if available
    if (rule.getFixit().has_value()) {
        std::string fix = formatMessage(rule.getFixit().value(), func);
        violation.setSuggestedFix(fix, "");  // Diff would be generated separately
    }

    return violation;
}

Violation RuleEngine::createViolation(const Rule& rule, const Variable& var) {
    Violation violation(rule, var.getLocation(),
                       formatMessage(rule.getMessage(), var));

    violation.setContext("Variable", var.getName());

    return violation;
}

std::string RuleEngine::formatMessage(const std::string& template_msg, const Function& func) {
    std::string message = template_msg;

    // Use DSL evaluator for proper template substitution
    if (!evaluator_) {
        return message;  // Return as-is if no evaluator
    }

    // Find all {{...}} patterns and evaluate them
    size_t start_pos = 0;
    while ((start_pos = message.find("{{", start_pos)) != std::string::npos) {
        size_t end_pos = message.find("}}", start_pos);
        if (end_pos == std::string::npos) {
            break;  // Malformed template
        }

        // Extract expression between {{ and }}
        std::string expr = message.substr(start_pos + 2, end_pos - start_pos - 2);

        // Evaluate expression using DSL evaluator
        try {
            dsl::DSLEvaluator::EvaluationContext context;
            context.current_function = &func;
            context.database = current_db_;

            // Set file context from function location if database available
            if (current_db_) {
                const std::string& file_path = func.getLocation().getStart().getFile();
                if (!file_path.empty()) {
                    context.current_file = current_db_->findFileInfo(file_path);
                }
            }

            auto result = evaluator_->evaluateExpression(expr, context);
            std::string value = anyToString(result);

            // Replace {{expr}} with the evaluated value
            message.replace(start_pos, end_pos - start_pos + 2, value);
            start_pos += value.length();
        } catch (const std::exception& e) {
            spdlog::warn("Failed to evaluate template expression '{}': {}", expr, e.what());
            start_pos = end_pos + 2;  // Skip this one
        }
    }

    return message;
}

std::string RuleEngine::formatMessage(const std::string& template_msg, const Variable& var) {
    std::string message = template_msg;

    // Use DSL evaluator for proper template substitution
    if (!evaluator_) {
        return message;  // Return as-is if no evaluator
    }

    // Find all {{...}} patterns and evaluate them
    size_t start_pos = 0;
    while ((start_pos = message.find("{{", start_pos)) != std::string::npos) {
        size_t end_pos = message.find("}}", start_pos);
        if (end_pos == std::string::npos) {
            break;  // Malformed template
        }

        // Extract expression between {{ and }}
        std::string expr = message.substr(start_pos + 2, end_pos - start_pos - 2);

        // Evaluate expression using DSL evaluator
        try {
            dsl::DSLEvaluator::EvaluationContext context;
            context.current_variable = &var;
            context.database = current_db_;

            // Set file context from variable location if database available
            if (current_db_) {
                const std::string& file_path = var.getLocation().getStart().getFile();
                if (!file_path.empty()) {
                    context.current_file = current_db_->findFileInfo(file_path);
                }
            }

            auto result = evaluator_->evaluateExpression(expr, context);
            std::string value = anyToString(result);

            // Replace {{expr}} with the evaluated value
            message.replace(start_pos, end_pos - start_pos + 2, value);
            start_pos += value.length();
        } catch (const std::exception& e) {
            spdlog::warn("Failed to evaluate template expression '{}': {}", expr, e.what());
            start_pos = end_pos + 2;  // Skip this one
        }
    }

    return message;
}

std::vector<Violation> RuleEngine::executeRulesParallel(const RuleSet& rules,
                                                        const SemanticDatabase& db,
                                                        unsigned num_threads) {
    if (num_threads == 0) {
        num_threads = std::thread::hardware_concurrency();
    }

    spdlog::info("Executing rules in parallel with {} threads", num_threads);

    std::vector<std::future<std::vector<Violation>>> futures;
    std::vector<Violation> all_violations;
    std::mutex violations_mutex;

    // Launch async tasks for each rule
    for (const auto& rule : rules.rules) {
        futures.push_back(std::async(std::launch::async, [this, &rule, &db]() {
            return executeRule(rule, db);
        }));
    }

    // Collect results
    for (auto& future : futures) {
        auto violations = future.get();
        std::lock_guard<std::mutex> lock(violations_mutex);
        all_violations.insert(all_violations.end(), violations.begin(), violations.end());
    }

    return all_violations;
}

std::vector<Violation> RuleEngine::executeRulesSequential(const RuleSet& rules,
                                                          const SemanticDatabase& db) {
    std::vector<Violation> all_violations;

    for (const auto& rule : rules.rules) {
        auto violations = executeRule(rule, db);
        all_violations.insert(all_violations.end(), violations.begin(), violations.end());

        // Call progress callback if set
        if (progress_callback_) {
            float progress = static_cast<float>(last_stats_.total_rules_executed) / rules.rules.size();
            progress_callback_(rule, progress);
        }
    }

    return all_violations;
}

std::vector<Rule> RuleEngine::filterRules(const RuleSet& rule_set) const {
    std::vector<Rule> filtered;

    for (const auto& rule : rule_set.rules) {
        // Check if rule is disabled
        bool disabled = false;
        for (const auto& disabled_id : rule_set.disabled_rules) {
            if (rule.getId() == disabled_id ||
                rule.getId().find(disabled_id) != std::string::npos) {
                disabled = true;
                break;
            }
        }

        if (!disabled) {
            filtered.push_back(rule);
        }
    }

    return filtered;
}

void RuleEngine::setMaxThreads(unsigned num_threads) {
    max_threads_ = num_threads;
}

void RuleEngine::enableCaching(bool enable) {
    caching_enabled_ = enable;
    if (!enable) {
        rule_cache_.invalidate();
    }
}

void RuleEngine::setProgressCallback(std::function<void(const Rule&, float)> callback) {
    progress_callback_ = callback;
}

void RuleEngine::printStatistics(std::ostream& out) const {
    const auto& stats = last_stats_;

    out << "\n========== Nomic Analysis Statistics ==========\n";
    out << "Execution Time: " << stats.total_execution_time.count() << " ms\n";
    out << "\n--- Input Statistics ---\n";
    out << "Files Analyzed:      " << stats.files_analyzed << "\n";
    out << "Functions Analyzed:  " << stats.functions_analyzed << "\n";
    out << "Variables Analyzed:  " << stats.variables_analyzed << "\n";
    out << "Total Entities:      " << stats.total_entities_checked << "\n";

    out << "\n--- Rule Execution ---\n";
    out << "Rules Executed:      " << stats.total_rules_executed << "\n";
    out << "Total Violations:    " << stats.total_violations_found << "\n";

    if (!stats.violations_per_severity.empty()) {
        out << "\n--- Violations by Severity ---\n";
        for (const auto& [severity, count] : stats.violations_per_severity) {
            std::string severity_str;
            switch (severity) {
                case Severity::Error:   severity_str = "Error";   break;
                case Severity::Warning: severity_str = "Warning"; break;
                case Severity::Info:    severity_str = "Info";    break;
            }
            out << severity_str << ": " << count << "\n";
        }
    }

    if (!stats.violations_per_rule.empty()) {
        out << "\n--- Top 10 Rules by Violations ---\n";

        // Sort by violation count (descending)
        std::vector<std::pair<std::string, size_t>> sorted_rules(
            stats.violations_per_rule.begin(),
            stats.violations_per_rule.end()
        );
        std::sort(sorted_rules.begin(), sorted_rules.end(),
                 [](const auto& a, const auto& b) { return a.second > b.second; });

        size_t display_count = std::min(size_t(10), sorted_rules.size());
        for (size_t i = 0; i < display_count; ++i) {
            out << "  " << sorted_rules[i].first << ": " << sorted_rules[i].second << "\n";
        }
    }

    if (!stats.violations_per_file.empty()) {
        out << "\n--- Top 10 Files by Violations ---\n";

        // Sort by violation count (descending)
        std::vector<std::pair<std::string, size_t>> sorted_files(
            stats.violations_per_file.begin(),
            stats.violations_per_file.end()
        );
        std::sort(sorted_files.begin(), sorted_files.end(),
                 [](const auto& a, const auto& b) { return a.second > b.second; });

        size_t display_count = std::min(size_t(10), sorted_files.size());
        for (size_t i = 0; i < display_count; ++i) {
            // Extract just the filename for cleaner output
            std::string filename = sorted_files[i].first;
            size_t last_slash = filename.find_last_of("/\\");
            if (last_slash != std::string::npos) {
                filename = filename.substr(last_slash + 1);
            }
            out << "  " << filename << ": " << sorted_files[i].second << "\n";
        }
    }

    if (!stats.rule_execution_times.empty()) {
        out << "\n--- Top 10 Slowest Rules ---\n";

        // Sort by execution time (descending)
        std::vector<std::pair<std::string, std::chrono::milliseconds>> sorted_times(
            stats.rule_execution_times.begin(),
            stats.rule_execution_times.end()
        );
        std::sort(sorted_times.begin(), sorted_times.end(),
                 [](const auto& a, const auto& b) { return a.second > b.second; });

        size_t display_count = std::min(size_t(10), sorted_times.size());
        for (size_t i = 0; i < display_count; ++i) {
            out << "  " << sorted_times[i].first << ": "
                << sorted_times[i].second.count() << " ms\n";
        }
    }

    out << "\n===============================================\n";
}

} // namespace nomic