#include <iostream>
#include <vector>
#include <string>
#include "nomic/core/ast_factory.h"

using namespace nomic::core;

void printUsage(const char* programName) {
    std::cout << "Nomic C Semantic Code Analyzer\n\n";
    std::cout << "Usage: " << programName << " [options] <source-files>\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help        Show this help message\n";
    std::cout << "  -o, --output      Output format (json, sarif)\n";
    std::cout << "  -r, --rules       Rules file (YAML)\n";
    std::cout << "  -q, --query       Execute DSL query\n";
    std::cout << "  -s, --sql         Execute SQL query\n";
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::vector<std::string> sourceFiles;
    std::string outputFormat = "json";
    std::string rulesFile;
    std::string dslQuery;
    std::string sqlQuery;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                outputFormat = argv[++i];
            }
        } else if (arg == "-r" || arg == "--rules") {
            if (i + 1 < argc) {
                rulesFile = argv[++i];
            }
        } else if (arg == "-q" || arg == "--query") {
            if (i + 1 < argc) {
                dslQuery = argv[++i];
            }
        } else if (arg == "-s" || arg == "--sql") {
            if (i + 1 < argc) {
                sqlQuery = argv[++i];
            }
        } else if (arg[0] != '-') {
            sourceFiles.push_back(arg);
        }
    }

    if (sourceFiles.empty()) {
        std::cerr << "Error: No source files specified\n";
        printUsage(argv[0]);
        return 1;
    }

    // Create factory and semantic model
    auto factory = std::make_unique<ASTFactory>();
    auto semanticModel = factory->createSemanticModel(sourceFiles);

    if (!semanticModel) {
        std::cerr << "Error: Failed to create semantic model\n";
        return 1;
    }

    std::cout << "Nomic C Semantic Code Analyzer\n";
    std::cout << "Analyzing " << sourceFiles.size() << " source file(s)...\n";

    // Basic analysis placeholder
    auto allNodes = semanticModel->getAllNodes();
    auto allSymbols = semanticModel->getAllSymbols();
    auto allScopes = semanticModel->getAllScopes();

    std::cout << "Analysis complete:\n";
    std::cout << "  - Nodes: " << allNodes.size() << "\n";
    std::cout << "  - Symbols: " << allSymbols.size() << "\n";
    std::cout << "  - Scopes: " << allScopes.size() << "\n";

    return 0;
}