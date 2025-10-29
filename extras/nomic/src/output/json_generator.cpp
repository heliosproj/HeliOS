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

#include "nomic/output/json_generator.h"
#include <fstream>
#include <iomanip>
#include <spdlog/spdlog.h>

namespace nomic::output {

JSONOutputGenerator::JSONOutputGenerator() {
    spdlog::debug("JSONOutputGenerator initialized");
}

void JSONOutputGenerator::generateReport(const std::vector<Violation>& violations,
                                        const OutputOptions& options,
                                        std::ostream& output) {
    nlohmann::json report;

    // Add metadata
    report["metadata"] = {
        {"generator", "Nomic C"},
        {"version", "1.0.0"},
        {"timestamp", std::time(nullptr)},
        {"total_violations", violations.size()}
    };

    // Add summary by severity
    int error_count = 0, warning_count = 0, info_count = 0;
    for (const auto& violation : violations) {
        switch (violation.getSeverity()) {
            case Severity::Error:
                error_count++;
                break;
            case Severity::Warning:
                warning_count++;
                break;
            case Severity::Info:
                info_count++;
                break;
        }
    }

    report["summary"] = {
        {"errors", error_count},
        {"warnings", warning_count},
        {"info", info_count}
    };

    // Process violations
    nlohmann::json violations_json = nlohmann::json::array();

    if (options.group_by_rule) {
        // Group violations by rule
        std::map<std::string, std::vector<const Violation*>> grouped;
        for (const auto& violation : violations) {
            grouped[violation.getRuleId()].push_back(&violation);
        }

        for (const auto& [rule_id, rule_violations] : grouped) {
            nlohmann::json rule_group;
            rule_group["rule_id"] = rule_id;
            rule_group["count"] = rule_violations.size();
            rule_group["violations"] = nlohmann::json::array();

            for (const auto* violation : rule_violations) {
                rule_group["violations"].push_back(violationToJSON(*violation, options));
            }

            violations_json.push_back(rule_group);
        }
    } else if (options.group_by_file) {
        // Group violations by file
        std::map<std::string, std::vector<const Violation*>> grouped;
        for (const auto& violation : violations) {
            std::string file = violation.getLocation().getStart().getFile();
            grouped[file].push_back(&violation);
        }

        for (const auto& [file, file_violations] : grouped) {
            nlohmann::json file_group;
            file_group["file"] = file;
            file_group["count"] = file_violations.size();
            file_group["violations"] = nlohmann::json::array();

            for (const auto* violation : file_violations) {
                file_group["violations"].push_back(violationToJSON(*violation, options));
            }

            violations_json.push_back(file_group);
        }
    } else {
        // Flat list of violations
        for (const auto& violation : violations) {
            violations_json.push_back(violationToJSON(violation, options));
        }
    }

    report["violations"] = violations_json;

    // Add metrics if requested
    if (options.include_metrics) {
        report["metrics"] = {
            {"files_analyzed", 0},  // TODO: Track this
            {"rules_executed", 0},  // TODO: Track this
            {"execution_time_ms", 0}  // TODO: Track this
        };
    }

    // Output JSON
    if (options.pretty_print) {
        output << std::setw(2) << report << std::endl;
    } else {
        output << report << std::endl;
    }
}

void JSONOutputGenerator::generateReportToFile(const std::vector<Violation>& violations,
                                              const OutputOptions& options,
                                              const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open output file: " + filename);
    }

    generateReport(violations, options, file);
    file.close();

    spdlog::info("Report written to: {}", filename);
}

std::string JSONOutputGenerator::getViolationSchema() {
    return R"({
        "$schema": "http://json-schema.org/draft-07/schema#",
        "type": "object",
        "properties": {
            "rule_id": {"type": "string"},
            "severity": {"enum": ["error", "warning", "info"]},
            "message": {"type": "string"},
            "location": {
                "type": "object",
                "properties": {
                    "file": {"type": "string"},
                    "line_start": {"type": "integer"},
                    "col_start": {"type": "integer"},
                    "line_end": {"type": "integer"},
                    "col_end": {"type": "integer"}
                },
                "required": ["file", "line_start", "col_start"]
            },
            "context": {
                "type": "object",
                "properties": {
                    "scope": {"type": "string"},
                    "name": {"type": "string"},
                    "snippet": {"type": "string"}
                }
            },
            "suggested_fix": {
                "type": "object",
                "properties": {
                    "description": {"type": "string"},
                    "diff": {"type": "string"}
                }
            }
        },
        "required": ["rule_id", "severity", "message", "location"]
    })";
}

nlohmann::json JSONOutputGenerator::violationToJSON(const Violation& violation,
                                                   const OutputOptions& options) const {
    nlohmann::json json;

    // Basic fields
    json["rule_id"] = violation.getRuleId();
    json["severity"] = severityToString(violation.getSeverity());
    json["message"] = violation.getMessage();

    // Location
    const auto& location = violation.getLocation();
    json["location"] = {
        {"file", location.getStart().getFile()},
        {"line_start", location.getStart().getLine()},
        {"col_start", location.getStart().getColumn()},
        {"line_end", location.getEnd().getLine()},
        {"col_end", location.getEnd().getColumn()}
    };

    // Context (if available and requested)
    if (options.include_source_context) {
        nlohmann::json context;

        if (violation.getContextScope().has_value()) {
            context["scope"] = violation.getContextScope().value();
        }

        if (violation.getContextName().has_value()) {
            context["name"] = violation.getContextName().value();
        }

        if (violation.getSnippet().has_value()) {
            context["snippet"] = violation.getSnippet().value();
        } else {
            // Try to extract snippet
            std::string snippet = extractSourceSnippet(location.getStart());
            if (!snippet.empty()) {
                context["snippet"] = snippet;
            }
        }

        if (!context.empty()) {
            json["context"] = context;
        }
    }

    // Suggested fix (if available and requested)
    if (options.include_suggested_fixes) {
        if (violation.getSuggestedFixDescription().has_value()) {
            nlohmann::json fix;
            fix["description"] = violation.getSuggestedFixDescription().value();

            if (violation.getSuggestedFixDiff().has_value()) {
                fix["diff"] = violation.getSuggestedFixDiff().value();
            } else {
                // Generate diff if possible
                std::string diff = generateDiff(violation);
                if (!diff.empty()) {
                    fix["diff"] = diff;
                }
            }

            json["suggested_fix"] = fix;
        }
    }

    return json;
}

std::string JSONOutputGenerator::generateDiff(const Violation& violation) const {
    (void)violation;
    // TODO: Implement diff generation based on suggested fix
    // This would involve:
    // 1. Reading the original source
    // 2. Applying the fix
    // 3. Generating a unified diff

    return "";
}

std::string JSONOutputGenerator::extractSourceSnippet(const SourceLocation& loc,
                                                     unsigned context_lines) const {
    std::ifstream file(loc.getFile());
    if (!file.is_open()) {
        return "";
    }

    std::string line;
    unsigned current_line = 1;
    std::ostringstream snippet;

    // Find the target line
    while (std::getline(file, line)) {
        if (current_line >= loc.getLine() - context_lines &&
            current_line <= loc.getLine() + context_lines) {

            if (current_line == loc.getLine()) {
                snippet << ">>> ";
            } else {
                snippet << "    ";
            }

            snippet << std::setw(4) << current_line << " | " << line << "\n";
        }

        if (current_line > loc.getLine() + context_lines) {
            break;
        }

        current_line++;
    }

    return snippet.str();
}

std::string JSONOutputGenerator::severityToString(Severity severity) const {
    switch (severity) {
        case Severity::Error:
            return "error";
        case Severity::Warning:
            return "warning";
        case Severity::Info:
            return "info";
        default:
            return "unknown";
    }
}

// OutputManager implementation
OutputManager::OutputManager(const std::string& format)
    : format_(format), json_generator_(std::make_unique<JSONOutputGenerator>()) {
    spdlog::debug("OutputManager initialized with format: {}", format);
}

void OutputManager::setOptions(const OutputOptions& options) {
    options_ = options;
}

void OutputManager::write(const std::vector<Violation>& violations,
                         std::ostream& output) {
    if (format_ == "json") {
        json_generator_->generateReport(violations, options_, output);
    } else if (format_ == "sarif") {
        // TODO: Implement SARIF generator
        spdlog::warn("SARIF format not yet implemented, using JSON");
        json_generator_->generateReport(violations, options_, output);
    } else if (format_ == "text") {
        // Simple text output
        for (const auto& violation : violations) {
            output << violation.getLocation().toString() << ": "
                  << json_generator_->severityToString(violation.getSeverity()) << ": "
                  << violation.getMessage() << " [" << violation.getRuleId() << "]\n";
        }
    } else {
        throw std::runtime_error("Unknown output format: " + format_);
    }
}

void OutputManager::writeToFile(const std::vector<Violation>& violations,
                               const std::string& filename) {
    if (format_ == "json") {
        json_generator_->generateReportToFile(violations, options_, filename);
    } else {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open output file: " + filename);
        }
        write(violations, file);
        file.close();
    }
}

void OutputManager::setFormat(const std::string& format) {
    format_ = format;
}

} // namespace nomic::output