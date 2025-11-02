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

#include "nomic/cli/cli_parser.h"
#include <algorithm>
#include <filesystem>
#include <glob.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace nomic::cli {

CLIParser::CLIParser() {}

CLIOptions CLIParser::parse(int argc, char* argv[]) {
    CLIOptions options;

    if (argc == 1) {
        options.show_help = true;
        return options;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // Help and version
        if (arg == "--help" || arg == "-h") {
            options.show_help = true;
            return options;
        }
        if (arg == "--version" || arg == "-v") {
            options.show_version = true;
            return options;
        }

        // Input/Output options
        if (arg == "--rules" || arg == "-r") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--rules requires an argument");
            }
            options.rules_file = argv[++i];
        }
        else if (arg == "--output" || arg == "-o") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--output requires an argument");
            }
            options.output_file = argv[++i];
        }
        else if (arg == "--format" || arg == "-f") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--format requires an argument");
            }
            options.output_format = argv[++i];
            if (!isValidOutputFormat(options.output_format)) {
                throw std::runtime_error("Invalid output format: " + options.output_format);
            }
        }
        else if (arg == "--compilation-database") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--compilation-database requires an argument");
            }
            options.compilation_database = argv[++i];
        }

        // Analysis options
        else if (arg == "--no-recursive") {
            options.recursive = false;
        }
        else if (arg == "--include") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--include requires an argument");
            }
            options.include_patterns.push_back(argv[++i]);
        }
        else if (arg == "--exclude") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--exclude requires an argument");
            }
            options.exclude_patterns.push_back(argv[++i]);
        }

        // Rule options
        else if (arg == "--enable-rules") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--enable-rules requires an argument");
            }
            std::string rules = argv[++i];
            std::stringstream ss(rules);
            std::string rule;
            while (std::getline(ss, rule, ',')) {
                options.enabled_rules.push_back(rule);
            }
        }
        else if (arg == "--disable-rules") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--disable-rules requires an argument");
            }
            std::string rules = argv[++i];
            std::stringstream ss(rules);
            std::string rule;
            while (std::getline(ss, rule, ',')) {
                options.disabled_rules.push_back(rule);
            }
        }
        else if (arg == "--severity") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--severity requires an argument");
            }
            options.severity_threshold = argv[++i];
            if (!isValidSeverityLevel(options.severity_threshold)) {
                throw std::runtime_error("Invalid severity level: " + options.severity_threshold);
            }
        }

        // Performance options
        else if (arg == "--threads" || arg == "-j") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--threads requires an argument");
            }
            options.num_threads = std::stoul(argv[++i]);
        }
        else if (arg == "--no-cache") {
            options.enable_cache = false;
        }
        else if (arg == "--cache-dir") {
            if (i + 1 >= argc) {
                throw std::runtime_error("--cache-dir requires an argument");
            }
            options.cache_directory = argv[++i];
        }
        else if (arg == "--no-parallel") {
            options.no_parallel = true;
        }

        // Debug options
        else if (arg == "--verbose") {
            options.verbose = true;
        }
        else if (arg == "--debug") {
            options.debug = true;
            options.verbose = true;  // Debug implies verbose
        }
        else if (arg == "--profile") {
            options.profile = true;
        }
        else if (arg == "--dump-ast") {
            options.dump_ast = true;
        }
        else if (arg == "--dump-ir") {
            options.dump_ir = true;
        }

        // Display options
        else if (arg == "--quiet" || arg == "-q") {
            options.quiet = true;
            options.show_progress = false;
        }
        else if (arg == "--no-progress") {
            options.show_progress = false;
        }
        else if (arg == "--no-color") {
            options.color_output = false;
        }
        else if (arg == "--compact") {
            options.compact_output = true;
        }
        else if (arg == "--stats") {
            options.show_stats = true;
        }

        // Output options
        else if (arg == "--no-context") {
            options.include_context = false;
        }
        else if (arg == "--no-fixits") {
            options.enable_fixits = false;
        }
        else if (arg == "--metrics") {
            options.show_metrics = true;
        }
        else if (arg == "--group-by-rule") {
            options.group_by_rule = true;
        }
        else if (arg == "--group-by-file") {
            options.group_by_file = true;
        }

        // Error handling options
        else if (arg == "--no-fail-on-error") {
            options.fail_on_error = false;
        }
        else if (arg == "--fail-on-warning") {
            options.fail_on_warning = true;
        }

        // Positional arguments (source files)
        else if (arg[0] != '-') {
            // Check if it's a file or directory
            if (std::filesystem::exists(arg)) {
                if (std::filesystem::is_directory(arg)) {
                    // If directory, find all .c files
                    if (options.recursive) {
                        for (const auto& entry :
                             std::filesystem::recursive_directory_iterator(arg)) {
                            if (entry.is_regular_file() &&
                                entry.path().extension() == ".c") {
                                options.source_files.push_back(entry.path().string());
                            }
                        }
                    } else {
                        for (const auto& entry :
                             std::filesystem::directory_iterator(arg)) {
                            if (entry.is_regular_file() &&
                                entry.path().extension() == ".c") {
                                options.source_files.push_back(entry.path().string());
                            }
                        }
                    }
                } else {
                    options.source_files.push_back(arg);
                }
            } else {
                // Try as glob pattern
                auto expanded = expandFilePattern(arg);
                options.source_files.insert(options.source_files.end(),
                                           expanded.begin(), expanded.end());
            }
        }
        else {
            throw std::runtime_error("Unknown option: " + arg);
        }
    }

    // Apply file filters
    if (!options.include_patterns.empty() || !options.exclude_patterns.empty()) {
        std::vector<std::string> filtered_files;

        for (const auto& file : options.source_files) {
            bool include = options.include_patterns.empty();

            // Check include patterns
            for (const auto& pattern : options.include_patterns) {
                if (file.find(pattern) != std::string::npos) {
                    include = true;
                    break;
                }
            }

            // Check exclude patterns
            if (include) {
                for (const auto& pattern : options.exclude_patterns) {
                    if (file.find(pattern) != std::string::npos) {
                        include = false;
                        break;
                    }
                }
            }

            if (include) {
                filtered_files.push_back(file);
            }
        }

        options.source_files = filtered_files;
        options.file_filter = filtered_files;
    }

    // Validate options
    validateOptions(options);

    return options;
}

void CLIParser::printHelp(std::ostream& out) const {
    out << "Usage: nomic [options] <source-files...>\n";
    out << "\n";
    out << "Nomic C Semantic Source Code Analyzer\n";
    out << "\n";
    out << "Input/Output Options:\n";
    out << "  -r, --rules <file>           Path to YAML rules file (required)\n";
    out << "  -o, --output <file>          Output file (default: stdout)\n";
    out << "  -f, --format <fmt>           Output format: json, sarif, text (default: json)\n";
    out << "  --compilation-database <dir> Path to compilation database\n";
    out << "\n";
    out << "Analysis Options:\n";
    out << "  --no-recursive               Don't recursively analyze directories\n";
    out << "  --include <pattern>          Include files matching pattern\n";
    out << "  --exclude <pattern>          Exclude files matching pattern\n";
    out << "\n";
    out << "Rule Options:\n";
    out << "  --enable-rules <patterns>    Enable only rules matching patterns (comma-separated)\n";
    out << "  --disable-rules <patterns>   Disable rules matching patterns (comma-separated)\n";
    out << "  --severity <level>           Minimum severity: info, warning, error\n";
    out << "\n";
    out << "Performance Options:\n";
    out << "  -j, --threads <n>            Number of threads (0=auto, default: auto)\n";
    out << "  --no-cache                   Disable caching\n";
    out << "  --cache-dir <dir>            Cache directory\n";
    out << "  --no-parallel                Disable parallel execution\n";
    out << "\n";
    out << "Debug Options:\n";
    out << "  --verbose                    Enable verbose output\n";
    out << "  --debug                      Enable debug mode\n";
    out << "  --profile                    Enable performance profiling\n";
    out << "  --dump-ast                   Dump AST for debugging\n";
    out << "  --dump-ir                    Dump LLVM IR for debugging\n";
    out << "\n";
    out << "Display Options:\n";
    out << "  -q, --quiet                  Suppress informational output\n";
    out << "  --no-progress                Disable progress reporting\n";
    out << "  --no-color                   Disable colored output\n";
    out << "  --compact                    Use compact output format\n";
    out << "  --stats                      Show analysis statistics\n";
    out << "\n";
    out << "Output Options:\n";
    out << "  --no-context                 Don't include source context in output\n";
    out << "  --no-fixits                  Don't include fix suggestions\n";
    out << "  --metrics                    Include code metrics in output\n";
    out << "  --group-by-rule              Group violations by rule\n";
    out << "  --group-by-file              Group violations by file\n";
    out << "\n";
    out << "Error Handling:\n";
    out << "  --no-fail-on-error           Don't exit with error code on violations\n";
    out << "  --fail-on-warning            Exit with error code on warnings\n";
    out << "\n";
    out << "Help:\n";
    out << "  -h, --help                   Show this help message\n";
    out << "  -v, --version                Show version information\n";
    out << "\n";
    out << "Examples:\n";
    out << "  # Analyze a single file\n";
    out << "  nomic --rules rules.yaml --output violations.json src/main.c\n";
    out << "\n";
    out << "  # Analyze entire project with compilation database\n";
    out << "  nomic --rules rules.yaml --compilation-database build/ --output report.json\n";
    out << "\n";
    out << "  # Analyze with specific rules enabled\n";
    out << "  nomic --rules rules.yaml --enable-rules \"SECURITY-*,MEMORY-*\" src/\n";
    out << "\n";
    out << "For more information, visit: https://github.com/nomic/nomic-cpp\n";
}

void CLIParser::printVersion(std::ostream& out) const {
    out << "Nomic C Semantic Source Code Analyzer 1.0.0\n";
    out << "Copyright (c) 2024 Nomic Project Contributors\n";
    out << "Built with LLVM/Clang 15.0\n";
    out << "License: MIT\n";
}

void CLIParser::validateOptions(const CLIOptions& opts) const {
    // Rules file is required unless help or version is requested
    if (!opts.show_help && !opts.show_version) {
        if (opts.rules_file.empty()) {
            throw std::runtime_error("Rules file is required (--rules)");
        }

        if (!std::filesystem::exists(opts.rules_file)) {
            throw std::runtime_error("Rules file does not exist: " + opts.rules_file);
        }

        // Source files are required
        if (opts.source_files.empty() && opts.compilation_database.empty()) {
            throw std::runtime_error("No source files specified");
        }
    }

    // Check for conflicting options
    if (opts.group_by_rule && opts.group_by_file) {
        throw std::runtime_error("Cannot group by both rule and file");
    }

    if (opts.quiet && opts.verbose) {
        throw std::runtime_error("Cannot use both --quiet and --verbose");
    }

    // Validate compilation database if specified
    if (!opts.compilation_database.empty()) {
        std::filesystem::path db_path = opts.compilation_database;
        if (!std::filesystem::exists(db_path / "compile_commands.json")) {
            throw std::runtime_error("compile_commands.json not found in: " +
                                   opts.compilation_database);
        }
    }
}

std::vector<std::string> CLIParser::expandFilePattern(const std::string& pattern) const {
    std::vector<std::string> files;

    glob_t glob_result;
    int ret = glob(pattern.c_str(), GLOB_TILDE | GLOB_BRACE, nullptr, &glob_result);

    if (ret == 0) {
        for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
            files.push_back(glob_result.gl_pathv[i]);
        }
        globfree(&glob_result);
    }

    return files;
}

bool CLIParser::isValidSeverityLevel(const std::string& level) const {
    return level == "info" || level == "warning" || level == "error";
}

bool CLIParser::isValidOutputFormat(const std::string& format) const {
    return format == "json" || format == "sarif" || format == "text";
}

} // namespace nomic::cli