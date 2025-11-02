#ifndef NOMIC_CORE_CLANG_PARSER_H
#define NOMIC_CORE_CLANG_PARSER_H

#include "nomic/core/interfaces.h"
#include <string>
#include <vector>
#include <memory>

namespace nomic {
namespace core {

/**
 * @brief Clang-based C source code parser
 *
 * This class integrates with Clang/LLVM to parse C source files and
 * convert them into Nomic's AST representation.
 */
class ClangParser {
public:
    /**
     * @brief Parse options for Clang parser
     */
    struct ParseOptions {
        std::vector<std::string> includePaths;       // -I paths
        std::vector<std::string> defines;            // -D defines
        std::vector<std::string> compilerFlags;      // Additional flags
        std::string compilationDatabase;             // Path to compile_commands.json
        bool parsePreprocessor = true;               // Include preprocessor info
        bool parseComments = true;                   // Include comments
        bool verbose = false;                        // Verbose output
    };

    /**
     * @brief Parse result containing AST and semantic model
     */
    struct ParseResult {
        ASTNodePtr ast;                              // Root AST node
        SemanticModelPtr semanticModel;              // Semantic model
        std::vector<std::string> errors;             // Parse errors
        std::vector<std::string> warnings;           // Parse warnings
        bool success = false;                        // Overall success
    };

    ClangParser();
    ~ClangParser();

    /**
     * @brief Parse a single C source file
     * @param filePath Path to C source file
     * @param options Parse options
     * @return Parse result with AST and semantic model
     */
    ParseResult parseFile(const std::string& filePath, const ParseOptions& options = ParseOptions());

    /**
     * @brief Parse multiple C source files
     * @param filePaths Vector of file paths
     * @param options Parse options
     * @return Parse result with combined AST
     */
    ParseResult parseFiles(const std::vector<std::string>& filePaths, const ParseOptions& options = ParseOptions());

    /**
     * @brief Parse C source code from string
     * @param source Source code string
     * @param filename Virtual filename for diagnostics
     * @param options Parse options
     * @return Parse result
     */
    ParseResult parseString(const std::string& source, const std::string& filename = "input.c", const ParseOptions& options = ParseOptions());

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace core
} // namespace nomic

#endif // NOMIC_CORE_CLANG_PARSER_H
