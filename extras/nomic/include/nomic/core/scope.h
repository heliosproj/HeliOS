#ifndef NOMIC_CORE_SCOPE_H
#define NOMIC_CORE_SCOPE_H

#include "nomic/core/interfaces.h"
#include <unordered_map>
#include <mutex>

namespace nomic {
namespace core {

/**
 * @brief Concrete implementation of IScope interface
 *
 * Represents lexical scopes in the program with symbol lookup capabilities.
 */
class Scope : public IScope, public std::enable_shared_from_this<Scope> {
private:
    ScopeKind kind_;
    std::weak_ptr<IScope> parent_;
    std::vector<ScopePtr> children_;
    std::unordered_map<std::string, SymbolPtr> symbols_;
    ASTNodePtr astNode_;

    mutable std::mutex symbolsMutex_;  // Thread safety for symbol table

public:
    // Constructor
    explicit Scope(ScopeKind kind);
    virtual ~Scope() = default;

    // IScope interface implementation
    ScopeKind getKind() const override { return kind_; }

    ScopePtr getParent() const override { return parent_.lock(); }

    std::vector<ScopePtr> getChildren() const override { return children_; }

    std::vector<SymbolPtr> getSymbols() const override {
        std::lock_guard<std::mutex> lock(symbolsMutex_);
        std::vector<SymbolPtr> result;
        result.reserve(symbols_.size());
        for (const auto& [name, symbol] : symbols_) {
            result.push_back(symbol);
        }
        return result;
    }

    SymbolPtr lookupSymbol(const std::string& name) const override {
        // First look in current scope
        {
            std::lock_guard<std::mutex> lock(symbolsMutex_);
            auto it = symbols_.find(name);
            if (it != symbols_.end()) {
                return it->second;
            }
        }

        // Then look in parent scope (lexical scoping)
        auto parentScope = parent_.lock();
        if (parentScope) {
            return parentScope->lookupSymbol(name);
        }

        return nullptr;
    }

    ASTNodePtr getASTNode() const override { return astNode_; }

    // Setters for building scope information
    void setParent(ScopePtr parent) { parent_ = parent; }

    void addChild(ScopePtr child) {
        if (child && std::find(children_.begin(), children_.end(), child) == children_.end()) {
            children_.push_back(child);
        }
    }

    void addSymbol(SymbolPtr symbol) {
        if (symbol && !symbol->getName().empty()) {
            std::lock_guard<std::mutex> lock(symbolsMutex_);
            symbols_[symbol->getName()] = symbol;
        }
    }

    void removeSymbol(const std::string& name) {
        std::lock_guard<std::mutex> lock(symbolsMutex_);
        symbols_.erase(name);
    }

    void setASTNode(ASTNodePtr node) { astNode_ = node; }

    // Extended functionality
    bool containsSymbol(const std::string& name) const {
        std::lock_guard<std::mutex> lock(symbolsMutex_);
        return symbols_.find(name) != symbols_.end();
    }

    void clearSymbols() {
        std::lock_guard<std::mutex> lock(symbolsMutex_);
        symbols_.clear();
    }

private:
    std::string getKindName() const;
};

} // namespace core
} // namespace nomic

#endif // NOMIC_CORE_SCOPE_H