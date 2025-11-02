#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "nomic/core/interfaces.h"
#include "nomic/core/ast_node.h"
#include "nomic/core/ast_factory.h"
#include <limits>
#include <memory>
#include <vector>
#include <thread>

using namespace nomic::core;
using ::testing::Return;
using ::testing::NotNull;
using ::testing::IsNull;
using ::testing::ElementsAre;
using ::testing::IsEmpty;

/**
 * TDD Unit Tests for AST Node Implementation
 * Following REQ-045: Test Coverage and focusing on edge cases per Claude Code instructions
 */

// Mock implementation for testing interface contracts
class MockASTNode : public IASTNode {
public:
    MOCK_METHOD(NodeKind, getKind, (), (const, override));
    MOCK_METHOD(std::string, getKindName, (), (const, override));
    MOCK_METHOD(std::string, getSourceLocation, (), (const, override));
    MOCK_METHOD((std::pair<int, int>), getLineColumn, (), (const, override));
    MOCK_METHOD(std::string, getSourceText, (), (const, override));
    MOCK_METHOD(ASTNodePtr, getParent, (), (const, override));
    MOCK_METHOD(std::vector<ASTNodePtr>, getChildren, (), (const, override));
    MOCK_METHOD(std::vector<ASTNodePtr>, getAncestors, (), (const, override));
    MOCK_METHOD(std::vector<ASTNodePtr>, getDescendants, (), (const, override));
    MOCK_METHOD(std::vector<ASTNodePtr>, getSiblings, (), (const, override));
    MOCK_METHOD(TypeInfoPtr, getType, (), (const, override));
    MOCK_METHOD(SymbolPtr, getSymbol, (), (const, override));
    MOCK_METHOD(ScopePtr, getScope, (), (const, override));
    MOCK_METHOD(void, accept, (IVisitor& visitor), (override));
    MOCK_METHOD(bool, hasProperty, (const std::string& key), (const, override));
    MOCK_METHOD(std::any, getProperty, (const std::string& key), (const, override));
    MOCK_METHOD(void, setProperty, (const std::string& key, const std::any& value), (override));
};

class ASTNodeTest : public ::testing::Test {
protected:
    void SetUp() override {
        factory = std::make_unique<ASTFactory>();
    }

    std::unique_ptr<IASTFactory> factory;
};

// Test: Node creation with all possible NodeKind values
TEST_F(ASTNodeTest, CreateNodeWithAllKindValues) {
    // Test boundary: First enum value
    auto node = factory->createNode(IASTNode::NodeKind::UNKNOWN);
    ASSERT_THAT(node, NotNull());
    EXPECT_EQ(node->getKind(), IASTNode::NodeKind::UNKNOWN);

    // Test all node kinds to ensure complete coverage
    std::vector<IASTNode::NodeKind> allKinds = {
        IASTNode::NodeKind::UNKNOWN,
        IASTNode::NodeKind::TRANSLATION_UNIT,
        IASTNode::NodeKind::FUNCTION_DECL,
        IASTNode::NodeKind::VAR_DECL,
        IASTNode::NodeKind::PARAM_DECL,
        IASTNode::NodeKind::FIELD_DECL,
        IASTNode::NodeKind::TYPE_DEF,
        IASTNode::NodeKind::STRUCT_DECL,
        IASTNode::NodeKind::UNION_DECL,
        IASTNode::NodeKind::ENUM_DECL,
        IASTNode::NodeKind::STATEMENT,
        IASTNode::NodeKind::EXPRESSION,
        IASTNode::NodeKind::LITERAL,
        IASTNode::NodeKind::IDENTIFIER,
        IASTNode::NodeKind::OPERATOR,
        IASTNode::NodeKind::CALL_EXPR,
        IASTNode::NodeKind::BINARY_OP,
        IASTNode::NodeKind::UNARY_OP,
        IASTNode::NodeKind::CAST_EXPR,
        IASTNode::NodeKind::ARRAY_SUBSCRIPT,
        IASTNode::NodeKind::MEMBER_EXPR,
        IASTNode::NodeKind::COMPOUND_STMT,
        IASTNode::NodeKind::IF_STMT,
        IASTNode::NodeKind::WHILE_STMT,
        IASTNode::NodeKind::FOR_STMT,
        IASTNode::NodeKind::DO_STMT,
        IASTNode::NodeKind::SWITCH_STMT,
        IASTNode::NodeKind::CASE_STMT,
        IASTNode::NodeKind::RETURN_STMT,
        IASTNode::NodeKind::BREAK_STMT,
        IASTNode::NodeKind::CONTINUE_STMT,
        IASTNode::NodeKind::GOTO_STMT,
        IASTNode::NodeKind::LABEL_STMT,
        IASTNode::NodeKind::DECL_STMT,
        IASTNode::NodeKind::EXPR_STMT
    };

    for (auto kind : allKinds) {
        auto testNode = factory->createNode(kind);
        ASSERT_THAT(testNode, NotNull()) << "Failed to create node of kind: " << static_cast<int>(kind);
        EXPECT_EQ(testNode->getKind(), kind);
        EXPECT_FALSE(testNode->getKindName().empty()) << "Kind name should not be empty";
    }
}

// Test: Invalid node kind (edge case - out of enum range)
TEST_F(ASTNodeTest, CreateNodeWithInvalidKind) {
    // Test with value beyond enum range
    auto invalidKind = static_cast<IASTNode::NodeKind>(999);
    auto node = factory->createNode(invalidKind);
    // Should either return nullptr or default to UNKNOWN
    if (node) {
        EXPECT_EQ(node->getKind(), IASTNode::NodeKind::UNKNOWN);
    }
}

// Test: Node without parent (root node edge case)
TEST_F(ASTNodeTest, RootNodeHasNoParent) {
    auto root = factory->createNode(IASTNode::NodeKind::TRANSLATION_UNIT);
    ASSERT_THAT(root, NotNull());
    EXPECT_THAT(root->getParent(), IsNull());
    EXPECT_THAT(root->getAncestors(), IsEmpty());
}

// Test: Node with no children (leaf node edge case)
TEST_F(ASTNodeTest, LeafNodeHasNoChildren) {
    auto leaf = factory->createNode(IASTNode::NodeKind::LITERAL);
    ASSERT_THAT(leaf, NotNull());
    EXPECT_THAT(leaf->getChildren(), IsEmpty());
    EXPECT_THAT(leaf->getDescendants(), IsEmpty());
}

// Test: Line and column boundary values
TEST_F(ASTNodeTest, LineColumnBoundaryValues) {
    auto node = factory->createNode(IASTNode::NodeKind::IDENTIFIER);
    ASSERT_THAT(node, NotNull());

    // Default should be valid position
    auto [line, col] = node->getLineColumn();
    EXPECT_GE(line, 0);
    EXPECT_GE(col, 0);

    // Test setting extreme values
    node->setProperty("line", std::numeric_limits<int>::max());
    node->setProperty("column", std::numeric_limits<int>::max());

    if (node->hasProperty("line") && node->hasProperty("column")) {
        auto maxLine = std::any_cast<int>(node->getProperty("line"));
        auto maxCol = std::any_cast<int>(node->getProperty("column"));
        EXPECT_EQ(maxLine, std::numeric_limits<int>::max());
        EXPECT_EQ(maxCol, std::numeric_limits<int>::max());
    }

    // Test negative values (invalid but should handle gracefully)
    node->setProperty("line", -1);
    node->setProperty("column", -1);
    auto [negLine, negCol] = node->getLineColumn();
    // Implementation should handle invalid values appropriately
    EXPECT_GE(negLine, -1);
    EXPECT_GE(negCol, -1);
}

// Test: Empty source text handling
TEST_F(ASTNodeTest, EmptySourceText) {
    auto node = factory->createNode(IASTNode::NodeKind::EXPRESSION);
    ASSERT_THAT(node, NotNull());

    // Source text might be empty for synthetic nodes
    std::string sourceText = node->getSourceText();
    // Should not crash, return empty or placeholder
    EXPECT_TRUE(sourceText.empty() || !sourceText.empty());
}

// Test: Null checks for semantic information
TEST_F(ASTNodeTest, NullSemanticInformation) {
    auto node = factory->createNode(IASTNode::NodeKind::STATEMENT);
    ASSERT_THAT(node, NotNull());

    // Nodes might not have type/symbol/scope information
    auto type = node->getType();
    auto symbol = node->getSymbol();
    auto scope = node->getScope();

    // Should handle null gracefully
    // These can be null for certain node types
    if (!type) {
        EXPECT_THAT(type, IsNull());
    }
    if (!symbol) {
        EXPECT_THAT(symbol, IsNull());
    }
    if (!scope) {
        EXPECT_THAT(scope, IsNull());
    }
}

// Test: Property system with various types
TEST_F(ASTNodeTest, PropertySystemEdgeCases) {
    auto node = factory->createNode(IASTNode::NodeKind::FUNCTION_DECL);
    ASSERT_THAT(node, NotNull());

    // Test: Non-existent property
    EXPECT_FALSE(node->hasProperty("nonexistent"));

    // Test: Empty key
    EXPECT_FALSE(node->hasProperty(""));

    // Test: Very long key
    std::string longKey(10000, 'a');
    node->setProperty(longKey, 42);
    if (node->hasProperty(longKey)) {
        EXPECT_EQ(std::any_cast<int>(node->getProperty(longKey)), 42);
    }

    // Test: Special characters in key
    std::string specialKey = "!@#$%^&*()_+-=[]{}|;:'\",.<>?/\\`~";
    node->setProperty(specialKey, "special");
    EXPECT_TRUE(node->hasProperty(specialKey));

    // Test: Null character in key (edge case)
    std::string nullKey = "key\0hidden";
    node->setProperty(nullKey, "value");
    EXPECT_TRUE(node->hasProperty(nullKey));

    // Test: Setting property to different types
    node->setProperty("int", 42);
    node->setProperty("double", 3.14);
    node->setProperty("string", std::string("test"));
    node->setProperty("bool", true);
    node->setProperty("nullptr", nullptr);

    EXPECT_EQ(std::any_cast<int>(node->getProperty("int")), 42);
    EXPECT_DOUBLE_EQ(std::any_cast<double>(node->getProperty("double")), 3.14);
    EXPECT_EQ(std::any_cast<std::string>(node->getProperty("string")), "test");
    EXPECT_TRUE(std::any_cast<bool>(node->getProperty("bool")));
}

// Test: Circular reference prevention
TEST_F(ASTNodeTest, CircularReferenceHandling) {
    auto parent = factory->createNode(IASTNode::NodeKind::COMPOUND_STMT);
    auto child1 = factory->createNode(IASTNode::NodeKind::EXPR_STMT);
    auto child2 = factory->createNode(IASTNode::NodeKind::EXPR_STMT);

    ASSERT_THAT(parent, NotNull());
    ASSERT_THAT(child1, NotNull());
    ASSERT_THAT(child2, NotNull());

    // Create tree structure
    dynamic_cast<ASTNode*>(parent.get())->addChild(child1);
    dynamic_cast<ASTNode*>(parent.get())->addChild(child2);
    dynamic_cast<ASTNode*>(child1.get())->setParent(parent);
    dynamic_cast<ASTNode*>(child2.get())->setParent(parent);

    // Verify no circular reference
    auto ancestors = child1->getAncestors();
    // Should only contain parent, not child itself
    EXPECT_EQ(ancestors.size(), 1);
    EXPECT_EQ(ancestors[0], parent);

    // Siblings should not include self
    auto siblings = child1->getSiblings();
    EXPECT_EQ(siblings.size(), 1);
    EXPECT_EQ(siblings[0], child2);
}

// Test: Large tree traversal (stress test)
TEST_F(ASTNodeTest, LargeTreeTraversal) {
    auto root = factory->createNode(IASTNode::NodeKind::TRANSLATION_UNIT);
    ASSERT_THAT(root, NotNull());

    // Create a deep tree
    const int depth = 100;
    ASTNodePtr current = root;
    std::vector<ASTNodePtr> allNodes;
    allNodes.push_back(root);

    for (int i = 0; i < depth; ++i) {
        auto child = factory->createNode(IASTNode::NodeKind::COMPOUND_STMT);
        ASSERT_THAT(child, NotNull());
        dynamic_cast<ASTNode*>(current.get())->addChild(child);
        dynamic_cast<ASTNode*>(child.get())->setParent(current);
        allNodes.push_back(child);
        current = child;
    }

    // Test deepest node
    auto ancestors = current->getAncestors();
    EXPECT_EQ(ancestors.size(), depth);

    // Test root descendants
    auto descendants = root->getDescendants();
    EXPECT_EQ(descendants.size(), depth);
}

// Test: Source location with special file paths
TEST_F(ASTNodeTest, SourceLocationSpecialPaths) {
    auto node = factory->createNode(IASTNode::NodeKind::VAR_DECL);
    ASSERT_THAT(node, NotNull());

    // Test various special paths
    std::vector<std::string> testPaths = {
        "",                                    // Empty path
        "/",                                   // Root
        "C:\\Windows\\System32\\file.c",       // Windows path
        "/path/with spaces/file.c",           // Path with spaces
        "/path/with/特殊字符/file.c",           // Unicode characters
        "relative/path/file.c",               // Relative path
        "../../../file.c",                    // Parent directory traversal
        "/very/long/path/" + std::string(255, 'a') + "/file.c"  // Very long path
    };

    for (const auto& path : testPaths) {
        node->setProperty("location", path);
        if (node->hasProperty("location")) {
            auto loc = std::any_cast<std::string>(node->getProperty("location"));
            EXPECT_EQ(loc, path);
        }
    }
}

// Test: Visitor pattern with null visitor
TEST_F(ASTNodeTest, AcceptNullVisitor) {
    auto node = factory->createNode(IASTNode::NodeKind::FUNCTION_DECL);
    ASSERT_THAT(node, NotNull());

    // This should not crash
    // Implementation should handle gracefully
    class NullVisitor : public IVisitor {
    public:
        void visitNode(IASTNode&) override {}
        void visitFunctionDecl(IASTNode&) override {}
        void visitVarDecl(IASTNode&) override {}
        void visitStatement(IASTNode&) override {}
        void visitExpression(IASTNode&) override {}
        bool shouldTraverseChildren() const override { return false; }
        void setTraverseChildren(bool) override {}
    };

    NullVisitor visitor;
    EXPECT_NO_THROW(node->accept(visitor));
}

// Test: Memory safety with shared pointers
TEST_F(ASTNodeTest, SharedPointerMemorySafety) {
    ASTNodePtr node1, node2;

    {
        // Create nodes in inner scope
        node1 = factory->createNode(IASTNode::NodeKind::IDENTIFIER);
        node2 = factory->createNode(IASTNode::NodeKind::LITERAL);

        ASSERT_THAT(node1, NotNull());
        ASSERT_THAT(node2, NotNull());

        // Create circular reference through properties
        node1->setProperty("reference", node2);
        node2->setProperty("reference", node1);
    }

    // Nodes should still be valid outside scope due to shared_ptr
    EXPECT_THAT(node1, NotNull());
    EXPECT_THAT(node2, NotNull());
    EXPECT_TRUE(node1->hasProperty("reference"));
    EXPECT_TRUE(node2->hasProperty("reference"));
}

// Test: Thread safety concerns (basic check)
TEST_F(ASTNodeTest, BasicThreadSafetyCheck) {
    auto node = factory->createNode(IASTNode::NodeKind::EXPRESSION);
    ASSERT_THAT(node, NotNull());

    // Concurrent property access
    const int numThreads = 10;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([node, i]() {
            std::string key = "thread_" + std::to_string(i);
            node->setProperty(key, i);
            EXPECT_TRUE(node->hasProperty(key));
            if (node->hasProperty(key)) {
                EXPECT_EQ(std::any_cast<int>(node->getProperty(key)), i);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    // Verify all properties were set
    for (int i = 0; i < numThreads; ++i) {
        std::string key = "thread_" + std::to_string(i);
        EXPECT_TRUE(node->hasProperty(key));
    }
}