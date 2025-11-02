#ifndef NOMIC_CORE_SEMANTIC_MODEL_H
#define NOMIC_CORE_SEMANTIC_MODEL_H

#include "nomic/core/interfaces.h"
#include "nomic/core/scope.h"
#include <unordered_map>
#include <set>
#include <mutex>
#include <algorithm>

namespace nomic {
namespace core {

/**
 * @brief Concrete implementation of ISemanticModel interface
 *
 * Provides high-level semantic analysis of C code with
 * AST access, symbol tables, scope management, and type information.
 */
class SemanticModel : public ISemanticModel, public std::enable_shared_from_this<SemanticModel> {
private:
    ASTNodePtr root_;
    ScopePtr globalScope_;

    std::vector<std::string> sourceFiles_;
    std::unordered_map<std::string, ASTNodePtr> fileASTs_;
    std::unordered_map<std::string, SymbolPtr> symbolTable_;
    std::unordered_map<std::string, TypeInfoPtr> typeTable_;
    std::vector<ASTNodePtr> allNodes_;
    std::vector<ScopePtr> allScopes_;

    mutable std::mutex nodesMutex_;
    mutable std::mutex symbolsMutex_;
    mutable std::mutex typesMutex_;
    mutable std::mutex scopesMutex_;

public:
    SemanticModel();
    virtual ~SemanticModel() = default;

    // AST access
    ASTNodePtr getRoot() const override { return root_; }

    std::vector<ASTNodePtr> getAllNodes() const override {
        std::lock_guard<std::mutex> lock(nodesMutex_);
        return allNodes_;
    }

    std::vector<ASTNodePtr> getNodesByKind(IASTNode::NodeKind kind) const override {
        std::lock_guard<std::mutex> lock(nodesMutex_);
        std::vector<ASTNodePtr> result;
        for (const auto& node : allNodes_) {
            if (node && node->getKind() == kind) {
                result.push_back(node);
            }
        }
        return result;
    }

    // Symbol table access
    std::vector<SymbolPtr> getAllSymbols() const override {
        std::lock_guard<std::mutex> lock(symbolsMutex_);
        std::vector<SymbolPtr> result;
        result.reserve(symbolTable_.size());
        for (const auto& [name, symbol] : symbolTable_) {
            result.push_back(symbol);
        }
        return result;
    }

    SymbolPtr findSymbol(const std::string& name) const override {
        if (name.empty()) return nullptr;

        std::lock_guard<std::mutex> lock(symbolsMutex_);
        auto it = symbolTable_.find(name);
        return (it != symbolTable_.end()) ? it->second : nullptr;
    }

    std::vector<SymbolPtr> findSymbolsByKind(ISymbol::SymbolKind kind) const override {
        std::lock_guard<std::mutex> lock(symbolsMutex_);
        std::vector<SymbolPtr> result;
        for (const auto& [name, symbol] : symbolTable_) {
            if (symbol && symbol->getKind() == kind) {
                result.push_back(symbol);
            }
        }
        return result;
    }

    // Scope access
    ScopePtr getGlobalScope() const override { return globalScope_; }

    std::vector<ScopePtr> getAllScopes() const override {
        std::lock_guard<std::mutex> lock(scopesMutex_);
        return allScopes_;
    }

    // Type information
    std::vector<TypeInfoPtr> getAllTypes() const override {
        std::lock_guard<std::mutex> lock(typesMutex_);
        std::vector<TypeInfoPtr> result;
        result.reserve(typeTable_.size());
        for (const auto& [name, type] : typeTable_) {
            result.push_back(type);
        }
        return result;
    }

    TypeInfoPtr findType(const std::string& name) const override {
        std::lock_guard<std::mutex> lock(typesMutex_);
        auto it = typeTable_.find(name);
        return (it != typeTable_.end()) ? it->second : nullptr;
    }

    // File information
    std::vector<std::string> getSourceFiles() const override {
        return sourceFiles_;
    }

    ASTNodePtr getFileAST(const std::string& filename) const override {
        auto it = fileASTs_.find(filename);
        return (it != fileASTs_.end()) ? it->second : nullptr;
    }

    // Analysis helpers
    std::vector<ASTNodePtr> findReferences(SymbolPtr symbol) const override {
        if (!symbol) return {};
        return symbol->getReferences();
    }

    std::vector<ASTNodePtr> findCallsTo(const std::string& functionName) const override {
        if (functionName.empty()) return {};

        std::lock_guard<std::mutex> lock(nodesMutex_);
        std::vector<ASTNodePtr> calls;

        for (const auto& node : allNodes_) {
            if (node && node->getKind() == IASTNode::NodeKind::CALL_EXPR) {
                // Check if this call matches the function name
                if (node->hasProperty("callee")) {
                    auto callee = std::any_cast<std::string>(node->getProperty("callee"));
                    if (callee == functionName) {
                        calls.push_back(node);
                    }
                }
            }
        }
        return calls;
    }

    bool isReachable(ASTNodePtr from, ASTNodePtr to) const override {
        if (!from || !to) return false;
        if (from == to) return true;

        // Simple reachability check through parent-child relationships
        // This is a basic implementation - a full CFG analysis would be more complex

        // Check if 'to' is a descendant of 'from'
        auto descendants = from->getDescendants();
        if (std::find(descendants.begin(), descendants.end(), to) != descendants.end()) {
            return true;
        }

        // Check if 'from' is a descendant of 'to'
        auto ancestors = from->getAncestors();
        if (std::find(ancestors.begin(), ancestors.end(), to) != ancestors.end()) {
            return true;
        }

        // For more complex reachability, we'd need control flow analysis
        return false;
    }

    // Builder methods for constructing the model
    void setRoot(ASTNodePtr root) {
        root_ = root;
        if (root) {
            addNode(root);
        }
    }

    void addSourceFile(const std::string& file) {
        sourceFiles_.push_back(file);
    }

    void setFileAST(const std::string& file, ASTNodePtr ast) {
        fileASTs_[file] = ast;
    }

    void addNode(ASTNodePtr node) {
        if (node) {
            std::lock_guard<std::mutex> lock(nodesMutex_);
            allNodes_.push_back(node);
        }
    }

    void addSymbol(SymbolPtr symbol) {
        if (symbol && !symbol->getName().empty()) {
            std::lock_guard<std::mutex> lock(symbolsMutex_);
            symbolTable_[symbol->getName()] = symbol;
        }
    }

    void addType(TypeInfoPtr type) {
        if (type && !type->getTypeName().empty()) {
            std::lock_guard<std::mutex> lock(typesMutex_);
            typeTable_[type->getTypeName()] = type;
        }
    }

    void addScope(ScopePtr scope) {
        if (scope) {
            std::lock_guard<std::mutex> lock(scopesMutex_);
            allScopes_.push_back(scope);
        }
    }

    void initializeGlobalScope() {
        globalScope_ = std::make_shared<Scope>(IScope::ScopeKind::GLOBAL);
        addScope(globalScope_);
    }
};

} // namespace core
} // namespace nomic

#endif // NOMIC_CORE_SEMANTIC_MODEL_H