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

#include "nomic/core/clang_tool.h"
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <clang/Frontend/FrontendActions.h>
#include <llvm/Support/CommandLine.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

namespace nomic {

// ClangAnalyzer implementation
ClangAnalyzer::ClangAnalyzer() {
    // Set default compiler arguments
    compiler_args_.push_back("-std=c11");
    compiler_args_.push_back("-Wall");
}

ClangAnalyzer::~ClangAnalyzer() {
    // Small delay to allow Clang's internal cleanup to complete
    // This prevents race conditions where Clang's background threads
    // are still accessing the SemanticDatabase during destruction
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

bool ClangAnalyzer::analyzeFile(const std::string& source_file,
                                const std::vector<std::string>& compile_args) {
    return analyzeFiles({source_file}, compile_args);
}

bool ClangAnalyzer::analyzeFiles(const std::vector<std::string>& source_files,
                                 const std::vector<std::string>& compile_args) {
    if (source_files.empty()) {
        spdlog::error("No source files provided");
        return false;
    }

    spdlog::info("Analyzing {} source file(s)", source_files.size());

    // Combine default args with provided args
    std::vector<std::string> all_args = compiler_args_;
    all_args.insert(all_args.end(), compile_args.begin(), compile_args.end());

    // Add standard if not already specified
    if (std::find_if(all_args.begin(), all_args.end(),
                     [](const std::string& arg) { return arg.find("-std=") == 0; }) == all_args.end()) {
        all_args.insert(all_args.begin(), "-std=" + standard_);
    }

    int result = runTool(source_files, all_args);
    return result == 0;
}

bool ClangAnalyzer::analyzeWithCompilationDatabase(const std::string& compilation_db_path,
                                                    const std::vector<std::string>& source_files) {
    spdlog::info("Loading compilation database: {}", compilation_db_path);

    std::string error_message;
    auto compilation_db = clang::tooling::JSONCompilationDatabase::loadFromFile(
        compilation_db_path, error_message, clang::tooling::JSONCommandLineSyntax::AutoDetect);

    if (!compilation_db) {
        spdlog::error("Failed to load compilation database: {}", error_message);
        return false;
    }

    // Get all files from database if none specified
    std::vector<std::string> files_to_analyze = source_files;
    if (files_to_analyze.empty()) {
        files_to_analyze = compilation_db->getAllFiles();
        spdlog::info("Found {} files in compilation database", files_to_analyze.size());
    }

    if (files_to_analyze.empty()) {
        spdlog::error("No files to analyze");
        return false;
    }

    // Create ClangTool with compilation database
    clang::tooling::ClangTool tool(*compilation_db, files_to_analyze);

    // Create action factory
    auto factory = std::make_unique<SemanticAnalysisFrontendActionFactory>(semantic_db_);

    // Run the tool
    spdlog::info("Running analysis on {} file(s)", files_to_analyze.size());
    int result = tool.run(factory.get());

    if (result != 0) {
        spdlog::error("Analysis failed with code {}", result);
        return false;
    }

    spdlog::info("Analysis completed successfully");
    spdlog::info("Analyzed {} functions", semantic_db_.getFunctions().size());
    spdlog::info("Analyzed {} global variables", semantic_db_.getGlobalVariables().size());

    return true;
}

void ClangAnalyzer::reset() {
    semantic_db_ = SemanticDatabase();
}

int ClangAnalyzer::runTool(const std::vector<std::string>& sources,
                           const std::vector<std::string>& args) {
    // Build command line arguments
    std::vector<std::string> cmd_args = buildCommandLineArgs();
    cmd_args.insert(cmd_args.end(), args.begin(), args.end());

    if (verbose_) {
        spdlog::debug("Compile arguments:");
        for (const auto& arg : cmd_args) {
            spdlog::debug("  {}", arg);
        }
    }

    // Create a fixed compilation database
    auto adjusting_compilations = std::make_unique<clang::tooling::FixedCompilationDatabase>(
        ".", cmd_args);

    // Create ClangTool
    clang::tooling::ClangTool tool(*adjusting_compilations, sources);

    // Create action factory
    auto factory = std::make_unique<SemanticAnalysisFrontendActionFactory>(semantic_db_);

    // Run the tool
    int result = tool.run(factory.get());

    if (result == 0) {
        spdlog::info("Successfully analyzed {} file(s)", sources.size());
        spdlog::info("  Functions: {}", semantic_db_.getFunctions().size());
        spdlog::info("  Global variables: {}", semantic_db_.getGlobalVariables().size());
    } else {
        spdlog::error("Analysis failed with error code {}", result);
    }

    return result;
}

std::vector<std::string> ClangAnalyzer::buildCommandLineArgs() const {
    std::vector<std::string> args;

    // Add standard library includes (platform-specific)
    #ifdef __linux__
    args.push_back("-I/usr/include");
    args.push_back("-I/usr/local/include");
    #elif __APPLE__
    args.push_back("-I/usr/include");
    args.push_back("-I/usr/local/include");
    #endif

    // Add resource directory for clang built-ins
    // This helps find compiler intrinsic headers
    args.push_back("-resource-dir");
    args.push_back("/usr/lib/clang/14.0.0");  // Adjust based on clang version

    return args;
}

// CompilationDatabaseBuilder implementation
void CompilationDatabaseBuilder::addCommand(const std::string& file,
                                           const std::string& directory,
                                           const std::vector<std::string>& args) {
    CompileCommand cmd;
    cmd.file = file;
    cmd.directory = directory;
    cmd.arguments = args;
    commands_.push_back(cmd);
}

std::string CompilationDatabaseBuilder::generateJSON() const {
    std::string json = "[\n";

    for (size_t i = 0; i < commands_.size(); ++i) {
        const auto& cmd = commands_[i];

        json += "  {\n";
        json += "    \"directory\": \"" + cmd.directory + "\",\n";
        json += "    \"file\": \"" + cmd.file + "\",\n";
        json += "    \"arguments\": [";

        for (size_t j = 0; j < cmd.arguments.size(); ++j) {
            json += "\"" + cmd.arguments[j] + "\"";
            if (j < cmd.arguments.size() - 1) {
                json += ", ";
            }
        }

        json += "]\n";
        json += "  }";

        if (i < commands_.size() - 1) {
            json += ",";
        }
        json += "\n";
    }

    json += "]\n";
    return json;
}

bool CompilationDatabaseBuilder::writeToFile(const std::string& path) const {
    std::ofstream file(path);
    if (!file) {
        spdlog::error("Failed to open {} for writing", path);
        return false;
    }

    file << generateJSON();
    file.close();

    spdlog::info("Wrote compilation database to {}", path);
    return true;
}

std::string CompilationDatabaseBuilder::createSimple(const std::vector<std::string>& files,
                                                    const std::vector<std::string>& compile_args,
                                                    const std::string& working_dir) {
    CompilationDatabaseBuilder builder;

    for (const auto& file : files) {
        std::vector<std::string> args = {"clang", "-std=c11"};
        args.insert(args.end(), compile_args.begin(), compile_args.end());
        args.push_back(file);

        builder.addCommand(file, working_dir, args);
    }

    return builder.generateJSON();
}

// Utility functions
namespace clang_utils {

bool isCSourceFile(const std::string& filename) {
    fs::path p(filename);
    std::string ext = p.extension().string();
    return ext == ".c" || ext == ".h";
}

bool isCppSourceFile(const std::string& filename) {
    fs::path p(filename);
    std::string ext = p.extension().string();
    return ext == ".cpp" || ext == ".cc" || ext == ".cxx" ||
           ext == ".hpp" || ext == ".hh" || ext == ".hxx";
}

std::vector<std::string> getDefaultCArgs() {
    return {
        "-std=c11",
        "-Wall",
        "-Wextra",
        "-pedantic"
    };
}

std::vector<std::string> getDefaultCppArgs() {
    return {
        "-std=c++17",
        "-Wall",
        "-Wextra",
        "-pedantic"
    };
}

std::string findCompilationDatabase(const std::string& start_dir) {
    fs::path current = fs::absolute(start_dir);

    while (current.has_parent_path()) {
        fs::path db_path = current / "compile_commands.json";
        if (fs::exists(db_path)) {
            return db_path.string();
        }

        fs::path build_db_path = current / "build" / "compile_commands.json";
        if (fs::exists(build_db_path)) {
            return build_db_path.string();
        }

        current = current.parent_path();
    }

    return "";
}

} // namespace clang_utils

} // namespace nomic
