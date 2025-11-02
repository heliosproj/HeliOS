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

#ifndef NOMIC_CLI_CLI_PARSER_H
#define NOMIC_CLI_CLI_PARSER_H

#include <string>
#include <vector>
#include <iostream>

namespace nomic::cli {

/**
 * @brief Command-line options for Nomic
 */
struct CLIOptions {
    // Input/Output
    std::vector<std::string> source_files;
    std::string rules_file;
    std::string output_file;
    std::string output_format = "json";  // json, sarif, text

    // Analysis options
    bool recursive = true;
    std::vector<std::string> include_patterns;
    std::vector<std::string> exclude_patterns;
    std::string compilation_database;

    // Rule options
    std::vector<std::string> enabled_rules;
    std::vector<std::string> disabled_rules;
    std::string severity_threshold = "info";

    // Performance options
    unsigned num_threads = 0;  // 0 = auto
    bool enable_cache = true;
    std::string cache_directory;
    bool no_parallel = false;

    // Debug options
    bool verbose = false;
    bool debug = false;
    bool profile = false;
    bool dump_ast = false;
    bool dump_ir = false;

    // Display options
    bool show_progress = true;
    bool color_output = true;
    bool quiet = false;
    bool compact_output = false;
    bool show_stats = false;

    // Output options
    bool include_context = true;
    bool enable_fixits = true;
    bool show_metrics = false;
    bool group_by_rule = false;
    bool group_by_file = false;

    // Error handling
    bool fail_on_error = true;
    bool fail_on_warning = false;

    // Special flags
    bool show_help = false;
    bool show_version = false;

    // File filters
    std::vector<std::string> file_filter;
};

/**
 * @brief Command-line parser for Nomic
 */
class CLIParser {
public:
    CLIParser();
    ~CLIParser() = default;

    /**
     * @brief Parse command-line arguments
     * @param argc Argument count
     * @param argv Argument values
     * @return Parsed options
     * @throws std::runtime_error on parse errors
     */
    CLIOptions parse(int argc, char* argv[]);

    /**
     * @brief Print help message
     * @param out Output stream
     */
    void printHelp(std::ostream& out) const;

    /**
     * @brief Print version information
     * @param out Output stream
     */
    void printVersion(std::ostream& out) const;

private:
    /**
     * @brief Validate parsed options
     * @param opts Options to validate
     * @throws std::runtime_error if options are invalid
     */
    void validateOptions(const CLIOptions& opts) const;

    /**
     * @brief Expand file patterns (glob)
     * @param pattern File pattern
     * @return List of matching files
     */
    std::vector<std::string> expandFilePattern(const std::string& pattern) const;

    /**
     * @brief Parse severity level
     * @param level Severity string
     * @return True if valid
     */
    bool isValidSeverityLevel(const std::string& level) const;

    /**
     * @brief Parse output format
     * @param format Format string
     * @return True if valid
     */
    bool isValidOutputFormat(const std::string& format) const;
};

} // namespace nomic::cli

#endif // NOMIC_CLI_CLI_PARSER_H