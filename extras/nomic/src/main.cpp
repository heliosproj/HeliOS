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

#include "nomic/core/semantic_model.h"
#include "nomic/core/rule_engine.h"
#include "nomic/core/clang_tool.h"
#include "nomic/cli/cli_parser.h"
#include "nomic/output/json_generator.h"
#include <iostream>
#include <memory>
#include <exception>
#include <chrono>
#include <spdlog/spdlog.h>
#include <llvm/Support/CommandLine.h>

using namespace nomic;

/**
 * @brief Main entry point for Nomic C++ analyzer
 */
int main(int argc, char* argv[]) {
    try {
        // Reset LLVM command line parser to prevent "option registered more than once" errors
        // This is necessary because both LLVM and Clang libraries register command line options
        llvm::cl::ResetCommandLineParser();

        auto start_time = std::chrono::steady_clock::now();

        // Parse command-line arguments
        cli::CLIParser parser;
        auto options = parser.parse(argc, argv);

        // Handle special cases
        if (options.show_help) {
            parser.printHelp(std::cout);
            return 0;
        }

        if (options.show_version) {
            parser.printVersion(std::cout);
            return 0;
        }

        // Configure logging
        if (options.verbose) {
            spdlog::set_level(spdlog::level::debug);
        } else if (options.quiet) {
            spdlog::set_level(spdlog::level::err);
        } else {
            spdlog::set_level(spdlog::level::info);
        }

        // Validate options
        if (options.source_files.empty()) {
            std::cerr << "Error: No source files specified\n";
            std::cerr << "Use --help for usage information\n";
            return 1;
        }

        if (options.rules_file.empty()) {
            std::cerr << "Error: No rules file specified\n";
            std::cerr << "Use --help for usage information\n";
            return 1;
        }

        spdlog::info("Nomic C++ Semantic Analyzer v1.0.0");
        spdlog::info("Loading rules from: {}", options.rules_file);

        // Initialize the Clang analyzer
        ClangAnalyzer clang_analyzer;
        clang_analyzer.setVerbose(options.verbose);

        // Initialize the rule engine
        RuleEngine rule_engine;
        if (options.enable_cache) {
            rule_engine.enableCaching(true);
        }
        if (options.num_threads > 0) {
            rule_engine.setMaxThreads(options.num_threads);
        }

        // Load rules
        RuleSet rule_set;
        try {
            rule_set = rule_engine.loadRules(options.rules_file);
            spdlog::info("Loaded {} rules", rule_set.rules.size());
        } catch (const std::exception& e) {
            spdlog::error("Failed to load rules: {}", e.what());
            return 1;
        }

        // Apply rule filters
        if (!options.enabled_rules.empty()) {
            // Filter to only enabled rules
            std::vector<Rule> filtered_rules;
            for (const auto& rule : rule_set.rules) {
                bool enabled = false;
                for (const auto& pattern : options.enabled_rules) {
                    if (rule.getId().find(pattern) != std::string::npos) {
                        enabled = true;
                        break;
                    }
                }
                if (enabled) {
                    filtered_rules.push_back(rule);
                }
            }
            rule_set.rules = std::move(filtered_rules);
            spdlog::info("After filtering: {} rules enabled", rule_set.rules.size());
        }

        // Disable specific rules
        for (const auto& disabled : options.disabled_rules) {
            rule_set.disabled_rules.push_back(disabled);
        }

        // Analyze source files using Clang
        spdlog::info("Analyzing {} source files", options.source_files.size());

        bool analysis_success = false;
        if (!options.compilation_database.empty()) {
            // Use compilation database if provided
            analysis_success = clang_analyzer.analyzeWithCompilationDatabase(
                options.compilation_database,
                options.source_files
            );
        } else {
            // Analyze files directly with default compile flags
            std::vector<std::string> compile_args;
            // Use default C11 standard
            analysis_success = clang_analyzer.analyzeFiles(options.source_files, compile_args);
        }

        if (!analysis_success) {
            spdlog::error("Failed to analyze source files");
            return 1;
        }

        // Get the populated semantic database
        const auto& semantic_db = clang_analyzer.getSemanticDatabase();

        // Execute rules
        spdlog::info("Executing rules...");
        ExecutionOptions exec_options;
        exec_options.parallel_execution = !options.no_parallel;
        exec_options.max_threads = options.num_threads;
        exec_options.enable_fixits = options.enable_fixits;
        exec_options.file_filter = options.file_filter;

        if (options.show_progress) {
            exec_options.progress_callback = [](const Rule& rule, float progress) {
                spdlog::debug("Processing rule {}: {:.1f}%", rule.getId(), progress * 100);
            };
        }

        auto violations = rule_engine.executeRules(rule_set, semantic_db, exec_options);

        // Get execution statistics
        auto stats = rule_engine.getLastExecutionStats();
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        spdlog::info("Analysis complete in {:.2f}s", duration.count() / 1000.0);
        spdlog::info("Found {} violations", violations.size());
        spdlog::info("Checked {} entities", stats.total_entities_checked);

        // Generate output
        if (!violations.empty() || !options.quiet) {
            output::OutputManager output_manager(options.output_format);
            output_manager.setOptions({
                .pretty_print = !options.compact_output,
                .include_source_context = options.include_context,
                .include_suggested_fixes = options.enable_fixits,
                .include_metrics = options.show_metrics,
                .group_by_rule = options.group_by_rule,
                .group_by_file = options.group_by_file
            });

            if (options.output_file.empty()) {
                output_manager.write(violations, std::cout);
            } else {
                output_manager.writeToFile(violations, options.output_file);
                spdlog::info("Results written to: {}", options.output_file);
            }
        }

        // Print summary statistics if requested
        if (options.show_stats && !options.quiet) {
            std::cout << "\n=== Analysis Statistics ===\n";
            std::cout << "Total execution time: " << duration.count() << "ms\n";
            std::cout << "Rules executed: " << stats.total_rules_executed << "\n";
            std::cout << "Entities checked: " << stats.total_entities_checked << "\n";
            std::cout << "Violations found: " << stats.total_violations_found << "\n";

            if (options.verbose) {
                std::cout << "\nRule execution times:\n";
                for (const auto& [rule_id, time] : stats.rule_execution_times) {
                    std::cout << "  " << rule_id << ": " << time.count() << "ms\n";
                }
            }
        }

        // Return exit code based on violations and severity threshold
        if (!violations.empty()) {
            // Check if any violations meet the severity threshold
            bool has_error = false;
            for (const auto& violation : violations) {
                if (violation.getSeverity() == Severity::Error) {
                    has_error = true;
                    break;
                }
            }

            if (has_error && options.fail_on_error) {
                return 1;
            } else if (!violations.empty() && options.fail_on_warning) {
                return 1;
            }
        }

        return 0;

    } catch (const std::exception& e) {
        spdlog::error("Fatal error: {}", e.what());
        return 2;
    } catch (...) {
        spdlog::error("Unknown fatal error occurred");
        return 2;
    }
}