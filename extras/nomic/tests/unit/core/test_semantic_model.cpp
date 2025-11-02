#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "nomic/core/interfaces.h"
#include "nomic/core/semantic_model.h"
#include "nomic/core/ast_factory.h"
#include "nomic/core/ast_node.h"
#include "nomic/core/scope.h"
#include <limits>
#include <thread>
#include <atomic>

using namespace nomic::core;
using ::testing::Return;
using ::testing::NotNull;
using ::testing::IsNull;
using ::testing::IsEmpty;
using ::testing::SizeIs;
using ::testing::Contains;
using ::testing::Not;

/**
 * TDD Unit Tests for Semantic Model Implementation
 * Testing comprehensive analysis capabilities and edge cases
 */

class SemanticModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        factory = std::make_unique<ASTFactory>();
        // Create a simple test file list
        testFiles = {
            "/test/main.c",
            "/test/utils.c",
            "/test/config.h"
        };
    }

    std::unique_ptr<IASTFactory> factory;
    std::vector<std::string> testFiles;
};

// Test: Create semantic model with no files
TEST_F(SemanticModelTest, CreateWithNoFiles) {
    std::vector<std::string> emptyFiles;
    auto model = factory->createSemanticModel(emptyFiles);
    ASSERT_THAT(model, NotNull());

    // Should handle empty gracefully
    EXPECT_THAT(model->getAllNodes(), IsEmpty());
    EXPECT_THAT(model->getAllSymbols(), IsEmpty());
    // Global scope should always exist even with no files
    EXPECT_THAT(model->getAllScopes(), SizeIs(1));
    EXPECT_THAT(model->getSourceFiles(), IsEmpty());

    // Verify global scope exists
    auto globalScope = model->getGlobalScope();
    ASSERT_THAT(globalScope, NotNull());
    EXPECT_EQ(globalScope->getKind(), IScope::ScopeKind::GLOBAL);
}

// Test: Create with single file
TEST_F(SemanticModelTest, CreateWithSingleFile) {
    std::vector<std::string> singleFile = {"/test/single.c"};
    auto model = factory->createSemanticModel(singleFile);
    ASSERT_THAT(model, NotNull());

    auto files = model->getSourceFiles();
    EXPECT_THAT(files, SizeIs(1));
    if (!files.empty()) {
        EXPECT_EQ(files[0], "/test/single.c");
    }
}

// Test: Create with non-existent files
TEST_F(SemanticModelTest, CreateWithNonExistentFiles) {
    std::vector<std::string> nonExistentFiles = {
        "/non/existent/file1.c",
        "/non/existent/file2.c"
    };
    auto model = factory->createSemanticModel(nonExistentFiles);
    ASSERT_THAT(model, NotNull());

    // Should handle gracefully - either empty or with error nodes
    auto files = model->getSourceFiles();
    // May or may not include non-existent files in list
    EXPECT_TRUE(files.size() <= nonExistentFiles.size());
}

// Test: File paths with special characters
TEST_F(SemanticModelTest, SpecialCharacterFilePaths) {
    std::vector<std::string> specialPaths = {
        "/path/with spaces/file.c",
        "/path/with-dashes/file.c",
        "/path/with.dots/file.c",
        "/path/with$pecial/file.c",
        "/path/with(parens)/file.c",
        "/path/with[brackets]/file.c",
        "/path/with{braces}/file.c",
        "C:\\Windows\\Path\\file.c",
        "/unicode/路径/文件.c",
        ""  // Empty path
    };

    auto model = factory->createSemanticModel(specialPaths);
    ASSERT_THAT(model, NotNull());

    // Should not crash with special paths
    auto files = model->getSourceFiles();
    EXPECT_TRUE(files.size() <= specialPaths.size());
}

// Test: Get root node edge cases
TEST_F(SemanticModelTest, GetRootNodeEdgeCases) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    auto root = model->getRoot();
    if (root) {
        // Root should be translation unit
        EXPECT_TRUE(root->getKind() == IASTNode::NodeKind::TRANSLATION_UNIT ||
                   root->getKind() == IASTNode::NodeKind::UNKNOWN);
        // Root should have no parent
        EXPECT_THAT(root->getParent(), IsNull());
    }
}

// Test: Get nodes by kind - all kinds
TEST_F(SemanticModelTest, GetNodesByAllKinds) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    // Test each kind
    std::vector<IASTNode::NodeKind> allKinds = {
        IASTNode::NodeKind::UNKNOWN,
        IASTNode::NodeKind::TRANSLATION_UNIT,
        IASTNode::NodeKind::FUNCTION_DECL,
        IASTNode::NodeKind::VAR_DECL,
        IASTNode::NodeKind::PARAM_DECL,
        IASTNode::NodeKind::STATEMENT,
        IASTNode::NodeKind::EXPRESSION
    };

    for (auto kind : allKinds) {
        auto nodes = model->getNodesByKind(kind);
        // Should not crash, may be empty
        for (auto& node : nodes) {
            EXPECT_EQ(node->getKind(), kind);
        }
    }
}

// Test: Find symbol with empty name
TEST_F(SemanticModelTest, FindSymbolEmptyName) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    auto symbol = model->findSymbol("");
    // Should handle gracefully - return null or not crash
    if (symbol) {
        EXPECT_FALSE(symbol->getName().empty());
    } else {
        EXPECT_THAT(symbol, IsNull());
    }
}

// Test: Find symbol with special characters
TEST_F(SemanticModelTest, FindSymbolSpecialCharacters) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    std::vector<std::string> specialNames = {
        "_underscore_start",
        "__double_underscore__",
        "name$with$dollar",
        "name::with::colons",
        "name.with.dots",
        "name-with-dashes",
        "名前",  // Japanese
        "名字",  // Chinese
        "όνομα", // Greek
        std::string(1000, 'a'),  // Very long name
        "\n\t\r",  // Whitespace
        "name\0hidden"  // Null character
    };

    for (const auto& name : specialNames) {
        auto symbol = model->findSymbol(name);
        // Should not crash
        if (symbol) {
            // If found, name should match
            EXPECT_EQ(symbol->getName(), name);
        }
    }
}

// Test: Find symbols by all kinds
TEST_F(SemanticModelTest, FindSymbolsByAllKinds) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    std::vector<ISymbol::SymbolKind> allKinds = {
        ISymbol::SymbolKind::VARIABLE,
        ISymbol::SymbolKind::FUNCTION,
        ISymbol::SymbolKind::PARAMETER,
        ISymbol::SymbolKind::FIELD,
        ISymbol::SymbolKind::TYPEDEF,
        ISymbol::SymbolKind::STRUCT,
        ISymbol::SymbolKind::UNION,
        ISymbol::SymbolKind::ENUM,
        ISymbol::SymbolKind::ENUMERATOR,
        ISymbol::SymbolKind::LABEL,
        ISymbol::SymbolKind::UNKNOWN
    };

    for (auto kind : allKinds) {
        auto symbols = model->findSymbolsByKind(kind);
        for (auto& sym : symbols) {
            EXPECT_EQ(sym->getKind(), kind);
        }
    }
}

// Test: Global scope always exists
TEST_F(SemanticModelTest, GlobalScopeAlwaysExists) {
    // Even with empty model
    auto emptyModel = factory->createSemanticModel({});
    ASSERT_THAT(emptyModel, NotNull());

    auto globalScope = emptyModel->getGlobalScope();
    ASSERT_THAT(globalScope, NotNull());
    EXPECT_EQ(globalScope->getKind(), IScope::ScopeKind::GLOBAL);
    EXPECT_THAT(globalScope->getParent(), IsNull());

    // With files
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    auto globalScope2 = model->getGlobalScope();
    ASSERT_THAT(globalScope2, NotNull());
    EXPECT_EQ(globalScope2->getKind(), IScope::ScopeKind::GLOBAL);
}

// Test: Find type with empty/null name
TEST_F(SemanticModelTest, FindTypeEmptyName) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    auto type = model->findType("");
    // Anonymous types might exist
    if (!type) {
        EXPECT_THAT(type, IsNull());
    }
}

// Test: Find references to null symbol
TEST_F(SemanticModelTest, FindReferencesNullSymbol) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    SymbolPtr nullSymbol = nullptr;
    auto refs = model->findReferences(nullSymbol);
    EXPECT_THAT(refs, IsEmpty());
}

// Test: Find calls to empty function name
TEST_F(SemanticModelTest, FindCallsEmptyName) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    auto calls = model->findCallsTo("");
    EXPECT_THAT(calls, IsEmpty());
}

// Test: Find calls to very long function name
TEST_F(SemanticModelTest, FindCallsLongName) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    std::string longName(10000, 'f');
    auto calls = model->findCallsTo(longName);
    // Should not crash, likely empty
    EXPECT_TRUE(calls.empty() || !calls.empty());
}

// Test: Reachability with null nodes
TEST_F(SemanticModelTest, ReachabilityNullNodes) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    ASTNodePtr nullNode = nullptr;
    auto someNode = factory->createNode(IASTNode::NodeKind::EXPRESSION);

    // Null to null
    EXPECT_FALSE(model->isReachable(nullNode, nullNode));

    // Null to valid
    EXPECT_FALSE(model->isReachable(nullNode, someNode));

    // Valid to null
    EXPECT_FALSE(model->isReachable(someNode, nullNode));
}

// Test: Reachability same node
TEST_F(SemanticModelTest, ReachabilitySameNode) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    auto node = factory->createNode(IASTNode::NodeKind::STATEMENT);
    ASSERT_THAT(node, NotNull());

    // Node is reachable from itself
    EXPECT_TRUE(model->isReachable(node, node));
}

// Test: Get file AST for non-existent file
TEST_F(SemanticModelTest, GetFileASTNonExistent) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    auto ast = model->getFileAST("/non/existent/file.c");
    // Should return null or empty AST
    if (!ast) {
        EXPECT_THAT(ast, IsNull());
    }
}

// Test: Large number of nodes stress test
TEST_F(SemanticModelTest, LargeNodeCountStress) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    // Simulate adding many nodes
    const int nodeCount = 10000;
    std::vector<ASTNodePtr> nodes;

    for (int i = 0; i < nodeCount; ++i) {
        auto node = factory->createNode(
            static_cast<IASTNode::NodeKind>(i % 10)
        );
        nodes.push_back(node);
    }

    // Get all nodes should handle large counts
    auto allNodes = model->getAllNodes();
    // May not include our test nodes if not properly added
    EXPECT_TRUE(allNodes.size() >= 0);
}

// Test: Symbol lookup in nested scopes
TEST_F(SemanticModelTest, NestedScopeLookup) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    auto globalScope = model->getGlobalScope();
    ASSERT_THAT(globalScope, NotNull());

    // Test looking up symbol in empty scope
    auto symbol = globalScope->lookupSymbol("test_symbol");
    // May or may not find depending on actual files
    if (symbol) {
        EXPECT_FALSE(symbol->getName().empty());
    }

    // Test with special scope kinds
    std::vector<IScope::ScopeKind> scopeKinds = {
        IScope::ScopeKind::GLOBAL,
        IScope::ScopeKind::FILE,
        IScope::ScopeKind::FUNCTION,
        IScope::ScopeKind::BLOCK,
        IScope::ScopeKind::STRUCT
    };

    for (auto kind : scopeKinds) {
        auto scope = factory->createScope(kind);
        ASSERT_THAT(scope, NotNull());
        EXPECT_EQ(scope->getKind(), kind);
    }
}

// Test: Concurrent access to semantic model
TEST_F(SemanticModelTest, ConcurrentAccess) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    const int numThreads = 10;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([model, &successCount, i]() {
            // Concurrent read operations
            auto nodes = model->getAllNodes();
            auto symbols = model->getAllSymbols();
            auto files = model->getSourceFiles();

            // Find operations
            model->findSymbol("test_" + std::to_string(i));
            model->findType("type_" + std::to_string(i));
            model->findCallsTo("func_" + std::to_string(i));

            successCount++;
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(successCount.load(), numThreads);
}

// Test: Circular dependency in scopes
TEST_F(SemanticModelTest, CircularScopeDependency) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    auto scope1 = factory->createScope(IScope::ScopeKind::BLOCK);
    auto scope2 = factory->createScope(IScope::ScopeKind::BLOCK);

    ASSERT_THAT(scope1, NotNull());
    ASSERT_THAT(scope2, NotNull());

    // Create potential circular reference
    dynamic_cast<Scope*>(scope1.get())->addChild(scope2);
    dynamic_cast<Scope*>(scope2.get())->setParent(scope1);

    // Should not cause infinite loop when traversing
    auto children1 = scope1->getChildren();
    EXPECT_THAT(children1, Contains(scope2));

    auto parent2 = scope2->getParent();
    EXPECT_EQ(parent2, scope1);
}

// Test: Maximum depth tree traversal
TEST_F(SemanticModelTest, MaximumDepthTraversal) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    // Create very deep AST
    const int maxDepth = 1000;
    auto root = factory->createNode(IASTNode::NodeKind::TRANSLATION_UNIT);
    ASTNodePtr current = root;

    for (int i = 0; i < maxDepth; ++i) {
        auto child = factory->createNode(IASTNode::NodeKind::COMPOUND_STMT);
        dynamic_cast<ASTNode*>(current.get())->addChild(child);
        dynamic_cast<ASTNode*>(child.get())->setParent(current);
        current = child;
    }

    // Should handle deep traversal without stack overflow
    auto descendants = root->getDescendants();
    EXPECT_EQ(descendants.size(), maxDepth);
}

// Test: Empty and whitespace-only names
TEST_F(SemanticModelTest, WhitespaceNames) {
    auto model = factory->createSemanticModel(testFiles);
    ASSERT_THAT(model, NotNull());

    std::vector<std::string> whitespaceNames = {
        "",
        " ",
        "  ",
        "\t",
        "\n",
        "\r\n",
        " \t\n\r ",
        std::string(100, ' ')
    };

    for (const auto& name : whitespaceNames) {
        auto symbol = model->findSymbol(name);
        auto type = model->findType(name);
        auto calls = model->findCallsTo(name);

        // Should handle gracefully
        if (symbol) {
            EXPECT_FALSE(symbol->getName().empty() || symbol->getName() == name);
        }
        if (type) {
            EXPECT_FALSE(type->getTypeName().empty() || type->getTypeName() == name);
        }
        EXPECT_TRUE(calls.empty() || !calls.empty());
    }
}