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
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace nomic {

/**
 * @brief Helper function to convert string to Severity enum
 */
static Severity parseSeverity(const std::string& severity_str) {
    std::string lower = severity_str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "error") {
        return Severity::Error;
    } else if (lower == "warning") {
        return Severity::Warning;
    } else if (lower == "info") {
        return Severity::Info;
    } else {
        spdlog::warn("Unknown severity '{}', defaulting to Warning", severity_str);
        return Severity::Warning;
    }
}

/**
 * @brief Parse a single rule from YAML node
 */
static Rule parseRule(const YAML::Node& rule_node) {
    // Validate required fields
    if (!rule_node["id"]) {
        throw std::runtime_error("Rule missing required field: id");
    }
    if (!rule_node["description"]) {
        throw std::runtime_error("Rule missing required field: description");
    }
    if (!rule_node["severity"]) {
        throw std::runtime_error("Rule missing required field: severity");
    }

    // Parse required fields
    std::string id = rule_node["id"].as<std::string>();
    std::string description = rule_node["description"].as<std::string>();
    Severity severity = parseSeverity(rule_node["severity"].as<std::string>());

    // Create rule object
    Rule rule(id, description, severity);

    // Parse optional fields
    if (rule_node["scope"]) {
        rule.setScope(rule_node["scope"].as<std::string>());
    }

    if (rule_node["select"]) {
        rule.setSelectExpression(rule_node["select"].as<std::string>());
    }

    if (rule_node["where"]) {
        rule.setWhereClause(rule_node["where"].as<std::string>());
    }

    if (rule_node["assert"]) {
        rule.setAssertExpression(rule_node["assert"].as<std::string>());
    }

    if (rule_node["message"]) {
        rule.setMessage(rule_node["message"].as<std::string>());
    }

    if (rule_node["fixit"]) {
        rule.setFixit(rule_node["fixit"].as<std::string>());
    }

    // Parse tags array
    if (rule_node["tags"] && rule_node["tags"].IsSequence()) {
        for (const auto& tag : rule_node["tags"]) {
            rule.addTag(tag.as<std::string>());
        }
    }

    // Parse exceptions array
    if (rule_node["exceptions"] && rule_node["exceptions"].IsSequence()) {
        for (const auto& exception : rule_node["exceptions"]) {
            rule.addException(exception.as<std::string>());
        }
    }

    spdlog::debug("Loaded rule: {} - {}", id, description);
    return rule;
}

/**
 * @brief Load rules from a single YAML file
 */
static std::vector<Rule> loadRulesFromFile(const std::string& file_path) {
    std::vector<Rule> rules;

    try {
        spdlog::debug("Loading rules from: {}", file_path);

        // Load and parse YAML file
        YAML::Node doc = YAML::LoadFile(file_path);

        // Determine if rules are at top-level or under 'rules' key
        YAML::Node rules_node;

        if (doc.IsSequence()) {
            // Rules are at the top level as a sequence
            spdlog::debug("Detected top-level sequence format in {}", file_path);
            rules_node = doc;
        } else if (doc["rules"]) {
            // Rules are under 'rules' key
            spdlog::debug("Detected 'rules' key format in {}", file_path);
            rules_node = doc["rules"];
        } else {
            spdlog::warn("File {} does not contain rules in expected format", file_path);
            return rules;
        }

        // Validate rules_node is a sequence
        if (!rules_node.IsSequence()) {
            spdlog::error("Rules in {} are not a sequence", file_path);
            return rules;
        }

        // Parse each rule
        for (const auto& rule_node : rules_node) {
            try {
                Rule rule = parseRule(rule_node);
                rules.push_back(std::move(rule));
            } catch (const std::exception& e) {
                spdlog::error("Failed to parse rule in {}: {}", file_path, e.what());
                // Continue with next rule
            }
        }

        spdlog::info("Loaded {} rules from {}", rules.size(), file_path);

    } catch (const YAML::Exception& e) {
        spdlog::error("YAML parsing error in {}: {}", file_path, e.what());
    } catch (const std::exception& e) {
        spdlog::error("Error loading rules from {}: {}", file_path, e.what());
    }

    return rules;
}

/**
 * @brief Validate YAML file has correct schema
 */
static bool validateYamlSchema(const std::string& file_path) {
    try {
        YAML::Node doc = YAML::LoadFile(file_path);

        // Determine if rules are at top-level or under 'rules' key
        YAML::Node rules_node;

        if (doc.IsSequence()) {
            // Rules are at the top level as a sequence
            rules_node = doc;
        } else if (doc["rules"]) {
            // Rules are under 'rules' key
            rules_node = doc["rules"];
        } else {
            spdlog::debug("File {} missing rules", file_path);
            return false;
        }

        // Check that rules is a sequence
        if (!rules_node.IsSequence()) {
            spdlog::debug("Rules in {} are not a sequence", file_path);
            return false;
        }

        // Validate at least one rule has required fields
        if (rules_node.size() == 0) {
            spdlog::debug("File {} contains no rules", file_path);
            return false;
        }

        // Check first rule has required fields
        const YAML::Node& first_rule = rules_node[0];
        if (!first_rule["id"] || !first_rule["description"] || !first_rule["severity"]) {
            spdlog::debug("First rule in {} missing required fields", file_path);
            return false;
        }

        return true;

    } catch (const YAML::Exception& e) {
        spdlog::debug("YAML validation error in {}: {}", file_path, e.what());
        return false;
    } catch (const std::exception& e) {
        spdlog::debug("Validation error in {}: {}", file_path, e.what());
        return false;
    }
}

/**
 * @brief RuleLoader class implementation
 */
class RuleLoader {
public:
    static bool validateYamlSchema(const std::string& yaml_content) {
        try {
            YAML::Node doc = YAML::Load(yaml_content);
            return doc["rules"] && doc["rules"].IsSequence();
        } catch (const YAML::Exception& e) {
            spdlog::debug("YAML schema validation failed: {}", e.what());
            return false;
        }
    }

    static std::vector<Rule> loadFromDirectory(const std::string& dir_path) {
        std::vector<Rule> all_rules;

        try {
            // Check if directory exists
            if (!fs::exists(dir_path)) {
                spdlog::error("Rules directory does not exist: {}", dir_path);
                return all_rules;
            }

            if (!fs::is_directory(dir_path)) {
                spdlog::error("Path is not a directory: {}", dir_path);
                return all_rules;
            }

            spdlog::info("Loading rules from directory: {}", dir_path);

            // Iterate through all YAML files in directory
            int file_count = 0;
            for (const auto& entry : fs::directory_iterator(dir_path)) {
                if (!entry.is_regular_file()) {
                    continue;
                }

                const std::string& path = entry.path().string();
                const std::string ext = entry.path().extension().string();

                // Check for .yaml or .yml extension
                if (ext != ".yaml" && ext != ".yml") {
                    spdlog::debug("Skipping non-YAML file: {}", path);
                    continue;
                }

                // Validate schema before loading
                if (!nomic::validateYamlSchema(path)) {
                    spdlog::warn("Skipping invalid YAML file: {}", path);
                    continue;
                }

                // Load rules from file
                std::vector<Rule> file_rules = loadRulesFromFile(path);
                all_rules.insert(all_rules.end(),
                                std::make_move_iterator(file_rules.begin()),
                                std::make_move_iterator(file_rules.end()));
                file_count++;
            }

            spdlog::info("Loaded {} total rules from {} files", all_rules.size(), file_count);

        } catch (const std::exception& e) {
            spdlog::error("Error loading rules from directory: {}", e.what());
        }

        return all_rules;
    }

    /**
     * @brief Load rules from a specific file
     */
    static std::vector<Rule> loadFromFile(const std::string& file_path) {
        if (!fs::exists(file_path)) {
            spdlog::error("Rule file does not exist: {}", file_path);
            return {};
        }

        if (!nomic::validateYamlSchema(file_path)) {
            spdlog::error("Invalid YAML schema in file: {}", file_path);
            return {};
        }

        return loadRulesFromFile(file_path);
    }
};

} // namespace nomic
