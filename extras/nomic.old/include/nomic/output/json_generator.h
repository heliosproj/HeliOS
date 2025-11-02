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

#ifndef NOMIC_OUTPUT_JSON_GENERATOR_H
#define NOMIC_OUTPUT_JSON_GENERATOR_H

#include "nomic/core/rule_engine.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

namespace nomic::output {

/**
 * @brief Output options for JSON generation
 */
struct OutputOptions {
    bool pretty_print = true;
    bool include_source_context = true;
    bool include_suggested_fixes = true;
    bool include_metrics = true;
    bool group_by_rule = false;
    bool group_by_file = false;
};

/**
 * @brief JSON output generator for violations
 */
class JSONOutputGenerator {
public:
    JSONOutputGenerator();
    ~JSONOutputGenerator() = default;

    /**
     * @brief Generate JSON report
     * @param violations List of violations to report
     * @param options Output options
     * @param output Output stream
     */
    void generateReport(const std::vector<Violation>& violations,
                       const OutputOptions& options,
                       std::ostream& output);

    /**
     * @brief Generate JSON report to file
     * @param violations List of violations to report
     * @param options Output options
     * @param filename Output filename
     */
    void generateReportToFile(const std::vector<Violation>& violations,
                             const OutputOptions& options,
                             const std::string& filename);

    /**
     * @brief Get JSON schema for violation format
     * @return JSON schema string
     */
    static std::string getViolationSchema();

    /**
     * @brief Convert severity to string
     * @param severity Severity level
     * @return String representation
     */
    std::string severityToString(Severity severity) const;

private:
    /**
     * @brief Convert violation to JSON
     * @param violation Violation to convert
     * @param options Output options
     * @return JSON object
     */
    nlohmann::json violationToJSON(const Violation& violation,
                                   const OutputOptions& options) const;

    /**
     * @brief Generate diff for suggested fix
     * @param violation Violation with fix
     * @return Diff string
     */
    std::string generateDiff(const Violation& violation) const;

    /**
     * @brief Extract source code snippet
     * @param loc Source location
     * @param context_lines Number of context lines
     * @return Source snippet
     */
    std::string extractSourceSnippet(const SourceLocation& loc,
                                     unsigned context_lines = 3) const;
};

/**
 * @brief Unified output manager supporting multiple formats
 */
class OutputManager {
public:
    explicit OutputManager(const std::string& format = "json");
    ~OutputManager() = default;

    /**
     * @brief Set output options
     * @param options Output options
     */
    void setOptions(const OutputOptions& options);

    /**
     * @brief Write violations to output stream
     * @param violations List of violations
     * @param output Output stream
     */
    void write(const std::vector<Violation>& violations,
              std::ostream& output);

    /**
     * @brief Write violations to file
     * @param violations List of violations
     * @param filename Output filename
     */
    void writeToFile(const std::vector<Violation>& violations,
                    const std::string& filename);

    /**
     * @brief Set output format
     * @param format Output format (json, sarif, text)
     */
    void setFormat(const std::string& format);

private:
    std::string format_;
    OutputOptions options_;
    std::unique_ptr<JSONOutputGenerator> json_generator_;
    // Future: Add SARIF and text generators
};

} // namespace nomic::output

#endif // NOMIC_OUTPUT_JSON_GENERATOR_H