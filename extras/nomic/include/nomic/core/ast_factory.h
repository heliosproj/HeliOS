#ifndef NOMIC_CORE_AST_FACTORY_H
#define NOMIC_CORE_AST_FACTORY_H

#include "nomic/core/interfaces.h"
#include "nomic/core/ast_node.h"
#include "nomic/core/type_info.h"
#include "nomic/core/symbol.h"
#include "nomic/core/scope.h"
#include "nomic/core/semantic_model.h"

namespace nomic {
namespace core {

/**
 * @brief Concrete implementation of IASTFactory
 *
 * Factory for creating AST nodes, types, symbols, scopes, and semantic models.
 * Ensures proper abstraction and loose coupling as required.
 */
class ASTFactory : public IASTFactory {
public:
    ASTFactory() = default;
    virtual ~ASTFactory() = default;

    // Create a semantic model from source files
    SemanticModelPtr createSemanticModel(const std::vector<std::string>& sourceFiles) override {
        auto model = std::make_shared<SemanticModel>();

        // Add source files to the model
        for (const auto& file : sourceFiles) {
            model->addSourceFile(file);

            // In a real implementation, we would parse the file here
            // For now, create a placeholder AST
            if (!file.empty()) {
                auto fileAST = createNode(IASTNode::NodeKind::TRANSLATION_UNIT);
                model->setFileAST(file, fileAST);

                // Set root to first file's AST
                if (!model->getRoot()) {
                    model->setRoot(fileAST);
                }
            }
        }

        return model;
    }

    // Create AST nodes
    ASTNodePtr createNode(IASTNode::NodeKind kind) override {
        // Handle invalid kind by defaulting to UNKNOWN
        if (static_cast<int>(kind) < 0 || static_cast<int>(kind) > static_cast<int>(IASTNode::NodeKind::EXPR_STMT)) {
            kind = IASTNode::NodeKind::UNKNOWN;
        }
        return std::make_shared<ASTNode>(kind);
    }

    // Create types
    TypeInfoPtr createType(ITypeInfo::TypeKind kind) override {
        // Handle invalid kind by defaulting to UNKNOWN
        if (static_cast<int>(kind) < 0 || static_cast<int>(kind) > static_cast<int>(ITypeInfo::TypeKind::UNKNOWN)) {
            kind = ITypeInfo::TypeKind::UNKNOWN;
        }
        return std::make_shared<TypeInfo>(kind);
    }

    // Create symbols
    SymbolPtr createSymbol(ISymbol::SymbolKind kind) override {
        // Handle invalid kind by defaulting to UNKNOWN
        if (static_cast<int>(kind) < 0 || static_cast<int>(kind) > static_cast<int>(ISymbol::SymbolKind::UNKNOWN)) {
            kind = ISymbol::SymbolKind::UNKNOWN;
        }
        return std::make_shared<Symbol>(kind);
    }

    // Create scopes
    ScopePtr createScope(IScope::ScopeKind kind) override {
        // Handle invalid kind by defaulting to UNKNOWN
        if (static_cast<int>(kind) < 0 || static_cast<int>(kind) > static_cast<int>(IScope::ScopeKind::UNKNOWN)) {
            kind = IScope::ScopeKind::UNKNOWN;
        }
        return std::make_shared<Scope>(kind);
    }

    // Extended factory methods for specific node types
    ASTNodePtr createFunctionDecl(const std::string& name) {
        auto node = createNode(IASTNode::NodeKind::FUNCTION_DECL);
        node->setProperty("name", name);
        return node;
    }

    ASTNodePtr createVariableDecl(const std::string& name, TypeInfoPtr type) {
        auto node = createNode(IASTNode::NodeKind::VAR_DECL);
        node->setProperty("name", name);
        if (auto astNode = std::dynamic_pointer_cast<ASTNode>(node)) {
            astNode->setType(type);
        }
        return node;
    }

    ASTNodePtr createCallExpr(const std::string& callee) {
        auto node = createNode(IASTNode::NodeKind::CALL_EXPR);
        node->setProperty("callee", callee);
        return node;
    }
};

} // namespace core
} // namespace nomic

#endif // NOMIC_CORE_AST_FACTORY_H