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

#ifndef NOMIC_CORE_CLANG_TOOL_H
#define NOMIC_CORE_CLANG_TOOL_H

#include "nomic/core/semantic_model.h"
#include "nomic/core/ast_visitor.h"
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <string>
#include <vector>
#include <memory>

namespace nomic {

/**
 * @brief Factory for creating AST consumers for semantic analysis
 */
class SemanticAnalysisFrontendActionFactory : public clang::tooling::FrontendActionFactory {
public:
    explicit SemanticAnalysisFrontendActionFactory(SemanticDatabase& db)
        : semantic_db_(db) {}

    std::unique_ptr<clang::FrontendAction> create() override {
        return std::make_unique<SemanticAnalysisAction>(semantic_db_);
    }

private:
    SemanticDatabase& semantic_db_;
};

/**
 * @brief Main interface for analyzing C/C++ files using Clang
 */
class ClangAnalyzer {
public:
    ClangAnalyzer();
    ~ClangAnalyzer();

    /**
     * @brief Analyze a single source file
     * @param source_file Path to source file
     * @param compile_args Compilation arguments (e.g., -I/path/to/includes)
     * @return true on success, false on failure
     */
    bool analyzeFile(const std::string& source_file,
                     const std::vector<std::string>& compile_args = {});

    /**
     * @brief Analyze multiple source files
     * @param source_files List of source file paths
     * @param compile_args Compilation arguments
     * @return true on success, false on failure
     */
    bool analyzeFiles(const std::vector<std::string>& source_files,
                      const std::vector<std::string>& compile_args = {});

    /**
     * @brief Analyze files using a compilation database
     * @param compilation_db_path Path to compile_commands.json
     * @param source_files List of source files (empty = analyze all)
     * @return true on success, false on failure
     */
    bool analyzeWithCompilationDatabase(const std::string& compilation_db_path,
                                        const std::vector<std::string>& source_files = {});

    /**
     * @brief Get the populated semantic database
     * @return Reference to semantic database
     */
    SemanticDatabase& getSemanticDatabase() { return semantic_db_; }
    const SemanticDatabase& getSemanticDatabase() const { return semantic_db_; }

    /**
     * @brief Reset the semantic database
     */
    void reset();

    /**
     * @brief Set additional compiler arguments
     */
    void setCompilerArgs(const std::vector<std::string>& args) {
        compiler_args_ = args;
    }

    /**
     * @brief Add include directory
     */
    void addIncludeDirectory(const std::string& dir) {
        compiler_args_.push_back("-I" + dir);
    }

    /**
     * @brief Add preprocessor definition
     */
    void addDefinition(const std::string& def) {
        compiler_args_.push_back("-D" + def);
    }

    /**
     * @brief Set language standard (c99, c11, c17, etc.)
     */
    void setStandard(const std::string& std) {
        standard_ = std;
    }

    /**
     * @brief Enable/disable verbose output
     */
    void setVerbose(bool verbose) { verbose_ = verbose; }

private:
    /**
     * @brief Run ClangTool on given sources with arguments
     */
    int runTool(const std::vector<std::string>& sources,
                const std::vector<std::string>& args);

    /**
     * @brief Build command line arguments for clang
     */
    std::vector<std::string> buildCommandLineArgs() const;

private:
    SemanticDatabase semantic_db_;
    std::vector<std::string> compiler_args_;
    std::string standard_ = "c11";
    bool verbose_ = false;
};

/**
 * @brief Helper class to build compilation database programmatically
 */
class CompilationDatabaseBuilder {
public:
    struct CompileCommand {
        std::string file;
        std::string directory;
        std::vector<std::string> arguments;
    };

    CompilationDatabaseBuilder() = default;

    /**
     * @brief Add a compile command
     */
    void addCommand(const std::string& file,
                    const std::string& directory,
                    const std::vector<std::string>& args);

    /**
     * @brief Generate compilation database JSON
     */
    std::string generateJSON() const;

    /**
     * @brief Write compilation database to file
     */
    bool writeToFile(const std::string& path) const;

    /**
     * @brief Create simple compilation database for files
     */
    static std::string createSimple(const std::vector<std::string>& files,
                                    const std::vector<std::string>& compile_args,
                                    const std::string& working_dir = ".");

private:
    std::vector<CompileCommand> commands_;
};

/**
 * @brief Utility functions for Clang analysis
 */
namespace clang_utils {

/**
 * @brief Check if file is a C source file
 */
bool isCSourceFile(const std::string& filename);

/**
 * @brief Check if file is a C++ source file
 */
bool isCppSourceFile(const std::string& filename);

/**
 * @brief Get default compile arguments for C
 */
std::vector<std::string> getDefaultCArgs();

/**
 * @brief Get default compile arguments for C++
 */
std::vector<std::string> getDefaultCppArgs();

/**
 * @brief Find compilation database in directory tree
 */
std::string findCompilationDatabase(const std::string& start_dir);

} // namespace clang_utils

} // namespace nomic

#endif // NOMIC_CORE_CLANG_TOOL_H
