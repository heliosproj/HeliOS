#ifndef NOMIC_CORE_AST_NODE_H
#define NOMIC_CORE_AST_NODE_H

#include "nomic/core/interfaces.h"
#include <unordered_map>
#include <mutex>
#include <algorithm>

namespace nomic {
namespace core {

/**
 * @brief Concrete implementation of IASTNode interface
 *
 * Provides a compiler-agnostic AST node implementation with full
 * support for tree navigation, semantic information, and properties.
 */
class ASTNode : public IASTNode, public std::enable_shared_from_this<ASTNode> {
private:
    NodeKind kind_;
    std::string kindName_;
    std::string sourceLocation_;
    std::pair<int, int> lineColumn_;
    std::string sourceText_;

    std::weak_ptr<IASTNode> parent_;
    std::vector<ASTNodePtr> children_;

    TypeInfoPtr type_;
    SymbolPtr symbol_;
    ScopePtr scope_;

    std::unordered_map<std::string, std::any> properties_;
    mutable std::mutex propertyMutex_;  // Thread safety for properties

public:
    // Constructor
    explicit ASTNode(NodeKind kind);
    virtual ~ASTNode() = default;

    // Core node properties
    NodeKind getKind() const override { return kind_; }
    std::string getKindName() const override { return kindName_; }
    std::string getSourceLocation() const override { return sourceLocation_; }
    std::pair<int, int> getLineColumn() const override { return lineColumn_; }
    std::string getSourceText() const override { return sourceText_; }

    // Tree navigation
    ASTNodePtr getParent() const override { return parent_.lock(); }
    std::vector<ASTNodePtr> getChildren() const override { return children_; }

    std::vector<ASTNodePtr> getAncestors() const override {
        std::vector<ASTNodePtr> ancestors;
        auto current = parent_.lock();
        while (current) {
            ancestors.push_back(current);
            current = current->getParent();
        }
        return ancestors;
    }

    std::vector<ASTNodePtr> getDescendants() const override {
        std::vector<ASTNodePtr> descendants;
        collectDescendants(descendants);
        return descendants;
    }

    std::vector<ASTNodePtr> getSiblings() const override {
        std::vector<ASTNodePtr> siblings;
        auto parentNode = parent_.lock();
        if (parentNode) {
            auto parentChildren = parentNode->getChildren();
            auto self = shared_from_this();
            for (auto& child : parentChildren) {
                if (child != self) {
                    siblings.push_back(child);
                }
            }
        }
        return siblings;
    }

    // Semantic information
    TypeInfoPtr getType() const override { return type_; }
    SymbolPtr getSymbol() const override { return symbol_; }
    ScopePtr getScope() const override { return scope_; }

    // Visitor pattern support
    void accept(IVisitor& visitor) override;

    // Property access
    bool hasProperty(const std::string& key) const override {
        if (key.empty()) return false;
        std::lock_guard<std::mutex> lock(propertyMutex_);
        return properties_.find(key) != properties_.end();
    }

    std::any getProperty(const std::string& key) const override {
        std::lock_guard<std::mutex> lock(propertyMutex_);
        auto it = properties_.find(key);
        if (it != properties_.end()) {
            return it->second;
        }
        return std::any();
    }

    void setProperty(const std::string& key, const std::any& value) override {
        std::lock_guard<std::mutex> lock(propertyMutex_);
        properties_[key] = value;
    }

    // Additional methods for building the tree
    void setParent(ASTNodePtr parent) { parent_ = parent; }
    void addChild(ASTNodePtr child) {
        if (child && std::find(children_.begin(), children_.end(), child) == children_.end()) {
            children_.push_back(child);
        }
    }
    void setSourceLocation(const std::string& loc) { sourceLocation_ = loc; }
    void setLineColumn(int line, int col) { lineColumn_ = {line, col}; }
    void setSourceText(const std::string& text) { sourceText_ = text; }
    void setType(TypeInfoPtr type) { type_ = type; }
    void setSymbol(SymbolPtr symbol) { symbol_ = symbol; }
    void setScope(ScopePtr scope) { scope_ = scope; }

private:
    void collectDescendants(std::vector<ASTNodePtr>& descendants) const {
        for (auto& child : children_) {
            descendants.push_back(child);
            if (auto childNode = std::dynamic_pointer_cast<ASTNode>(child)) {
                childNode->collectDescendants(descendants);
            }
        }
    }

    std::string getKindNameForKind(NodeKind kind) const;
};

} // namespace core
} // namespace nomic

#endif // NOMIC_CORE_AST_NODE_H