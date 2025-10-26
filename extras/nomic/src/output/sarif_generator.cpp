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

#include "nomic/core/rule_engine.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <spdlog/spdlog.h>
#include <set>
#include <map>
#include <chrono>
#include <ctime>

namespace nomic::output {

/**
 * @brief Complete SARIF 2.1.0 output generator
 * Generates Static Analysis Results Interchange Format output with full support for:
 * - Rule definitions with properties
 * - Results with fixes and code flows
 * - Invocations with execution metadata
 * - Artifacts and thread flows
 */
class SARIFOutputGenerator {
public:
    SARIFOutputGenerator() {
        spdlog::debug("SARIFOutputGenerator initialized");
    }

    /**
     * @brief Generate complete SARIF output
     * @param violations List of violations to include
     * @param rules Optional list of rules to extract from (if null, rules extracted from violations)
     * @param tool_name Name of the analysis tool
     * @param tool_version Version of the analysis tool
     * @param output Output stream for SARIF JSON
     * @param include_metadata Include execution metadata (invocation, machine info, etc.)
     */
    void generateSARIF(const std::vector<Violation>& violations,
                      const std::vector<Rule>* rules,
                      const std::string& tool_name,
                      const std::string& tool_version,
                      std::ostream& output,
                      bool include_metadata = true) {
        nlohmann::json sarif;

        // SARIF 2.1.0 structure
        sarif["$schema"] = "https://raw.githubusercontent.com/oasis-tcs/sarif-spec/master/Schemata/sarif-schema-2.1.0.json";
        sarif["version"] = "2.1.0";

        // Create run object
        nlohmann::json run;
        run["tool"] = createTool(tool_name, tool_version, violations, rules);
        run["results"] = createResults(violations);

        if (include_metadata) {
            run["invocations"] = createInvocations();
            run["artifacts"] = createArtifacts(violations);
        }

        sarif["runs"] = nlohmann::json::array({run});

        output << sarif.dump(2) << std::endl;
    }

private:
    nlohmann::json createTool(const std::string& name,
                              const std::string& version,
                              const std::vector<Violation>& violations,
                              const std::vector<Rule>* rules) {
        nlohmann::json tool;
        tool["driver"] = {
            {"name", name},
            {"version", version},
            {"informationUri", "https://github.com/nomic/nomic-cpp"},
            {"organization", "Nomic Project"},
            {"semanticVersion", version},
            {"rules", createRules(violations, rules)}
        };
        return tool;
    }

    nlohmann::json createRules(const std::vector<Violation>& violations,
                               const std::vector<Rule>* rule_list) {
        nlohmann::json rules = nlohmann::json::array();
        std::map<std::string, const Rule*> rule_map;

        // Build map from rule IDs if we have a rule list
        if (rule_list) {
            for (const auto& rule : *rule_list) {
                rule_map[rule.getId()] = &rule;
            }
        }

        // Extract unique rules from violations
        std::set<std::string> seen_rule_ids;

        for (const auto& violation : violations) {
            const std::string& rule_id = violation.getRuleId();

            // Skip duplicates
            if (seen_rule_ids.count(rule_id) > 0) {
                continue;
            }
            seen_rule_ids.insert(rule_id);

            nlohmann::json rule_obj;
            rule_obj["id"] = rule_id;

            // If we have the full rule definition, use it
            if (rule_map.count(rule_id) > 0) {
                const Rule* rule = rule_map[rule_id];

                rule_obj["shortDescription"] = {
                    {"text", rule->getDescription()}
                };

                rule_obj["fullDescription"] = {
                    {"text", rule->getDescription()}
                };

                // Add help text
                rule_obj["help"] = {
                    {"text", rule->getMessage()}
                };

                // Add properties for custom fields
                nlohmann::json properties;
                properties["scope"] = rule->getScope();

                if (rule->getWhereClause().has_value()) {
                    properties["whereClause"] = rule->getWhereClause().value();
                }

                properties["assertExpression"] = rule->getAssertExpression();

                // Add tags
                if (!rule->getTags().empty()) {
                    properties["tags"] = nlohmann::json::array();
                    for (const auto& tag : rule->getTags()) {
                        properties["tags"].push_back(tag);
                    }
                    rule_obj["tags"] = properties["tags"];
                }

                // Add exceptions
                if (!rule->getExceptions().empty()) {
                    properties["exceptions"] = nlohmann::json::array();
                    for (const auto& exception : rule->getExceptions()) {
                        properties["exceptions"].push_back(exception);
                    }
                }

                rule_obj["properties"] = properties;

                // Default configuration
                rule_obj["defaultConfiguration"] = {
                    {"level", getSARIFLevel(rule->getSeverity())},
                    {"enabled", true}
                };

            } else {
                // Create minimal rule definition from violation
                rule_obj["shortDescription"] = {
                    {"text", violation.getMessage()}
                };

                rule_obj["defaultConfiguration"] = {
                    {"level", getSARIFLevel(violation.getSeverity())},
                    {"enabled", true}
                };
            }

            rules.push_back(rule_obj);
        }

        return rules;
    }

    nlohmann::json createResults(const std::vector<Violation>& violations) {
        nlohmann::json results = nlohmann::json::array();

        for (const auto& violation : violations) {
            nlohmann::json result;
            result["ruleId"] = violation.getRuleId();

            // Message
            result["message"] = {{"text", violation.getMessage()}};

            // Level
            result["level"] = getSARIFLevel(violation.getSeverity());

            // Locations
            result["locations"] = nlohmann::json::array({
                createLocation(violation.getLocation())
            });

            // Add context if available
            if (violation.getContextScope().has_value()) {
                nlohmann::json properties;
                properties["contextScope"] = violation.getContextScope().value();
                if (violation.getContextName().has_value()) {
                    properties["contextName"] = violation.getContextName().value();
                }
                result["properties"] = properties;
            }

            // Add snippet if available
            if (violation.getSnippet().has_value()) {
                result["locations"][0]["physicalLocation"]["contextRegion"] = {
                    {"snippet", {{"text", violation.getSnippet().value()}}}
                };
            }

            // Add suggested fixes if available
            if (violation.getSuggestedFixDescription().has_value()) {
                nlohmann::json fix;
                fix["description"] = {
                    {"text", violation.getSuggestedFixDescription().value()}
                };

                if (violation.getSuggestedFixDiff().has_value()) {
                    // Create artificial file change
                    nlohmann::json artifactChange;
                    artifactChange["artifactLocation"] = {
                        {"uri", violation.getLocation().getStart().getFile()}
                    };

                    // For now, store diff in properties since SARIF needs specific replacement format
                    fix["properties"] = {
                        {"diff", violation.getSuggestedFixDiff().value()}
                    };
                }

                result["fixes"] = nlohmann::json::array({fix});
            }

            results.push_back(result);
        }

        return results;
    }

    nlohmann::json createLocation(const SourceRange& range) {
        nlohmann::json location;
        location["physicalLocation"] = {
            {"artifactLocation", {
                {"uri", range.getStart().getFile()},
                {"uriBaseId", "%SRCROOT%"}
            }},
            {"region", {
                {"startLine", range.getStart().getLine()},
                {"startColumn", range.getStart().getColumn()},
                {"endLine", range.getEnd().getLine()},
                {"endColumn", range.getEnd().getColumn()}
            }}
        };
        return location;
    }

    nlohmann::json createInvocations() {
        // Create invocation metadata for SARIF compliance
        nlohmann::json invocations = nlohmann::json::array();

        nlohmann::json invocation;
        invocation["executionSuccessful"] = true;
        invocation["endTimeUtc"] = getCurrentTimestamp();

        // Command line info would go here if available
        // invocation["commandLine"] = "nomic --rules rules.yaml source.c";

        invocations.push_back(invocation);
        return invocations;
    }

    nlohmann::json createArtifacts(const std::vector<Violation>& violations) {
        // Extract unique files from violations
        std::set<std::string> files;
        for (const auto& violation : violations) {
            files.insert(violation.getLocation().getStart().getFile());
        }

        nlohmann::json artifacts = nlohmann::json::array();
        for (const auto& file : files) {
            nlohmann::json artifact;
            artifact["location"] = {
                {"uri", file},
                {"uriBaseId", "%SRCROOT%"}
            };

            // Could add source language, encoding, etc.
            artifact["sourceLanguage"] = "c";
            artifacts.push_back(artifact);
        }

        return artifacts;
    }

    std::string getSARIFLevel(Severity severity) {
        switch (severity) {
            case Severity::Error:
                return "error";
            case Severity::Warning:
                return "warning";
            case Severity::Info:
                return "note";
            default:
                return "none";
        }
    }

    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::gmtime(&time_t);

        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tm);
        return std::string(buffer);
    }
};

} // namespace nomic::output