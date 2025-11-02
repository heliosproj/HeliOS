#ifndef NOMIC_CORE_SYMBOL_H
#define NOMIC_CORE_SYMBOL_H

#include "nomic/core/interfaces.h"
#include <vector>
#include <mutex>

namespace nomic {
namespace core {

/**
 * @brief Concrete implementation of ISymbol interface
 *
 * Represents symbols (variables, functions, types) in the semantic model.
 */
class Symbol : public ISymbol, public std::enable_shared_from_this<Symbol> {
private:
    SymbolKind kind_;
    std::string name_;
    std::string qualifiedName_;
    TypeInfoPtr type_;
    ScopePtr scope_;
    ASTNodePtr declaration_;
    std::vector<ASTNodePtr> references_;

    // Symbol attributes
    bool isGlobal_;
    bool isStatic_;
    bool isExtern_;
    bool isInline_;

    mutable std::mutex referencesMutex_;  // Thread safety for references

public:
    // Constructor
    explicit Symbol(SymbolKind kind);
    virtual ~Symbol() = default;

    // ISymbol interface implementation
    SymbolKind getKind() const override { return kind_; }
    std::string getName() const override { return name_; }
    std::string getQualifiedName() const override { return qualifiedName_; }
    TypeInfoPtr getType() const override { return type_; }
    ScopePtr getScope() const override { return scope_; }
    ASTNodePtr getDeclaration() const override { return declaration_; }

    std::vector<ASTNodePtr> getReferences() const override {
        std::lock_guard<std::mutex> lock(referencesMutex_);
        return references_;
    }

    bool isGlobal() const override { return isGlobal_; }
    bool isStatic() const override { return isStatic_; }
    bool isExtern() const override { return isExtern_; }
    bool isInline() const override { return isInline_; }

    // Setters for building symbol information
    void setName(const std::string& name) {
        name_ = name;
        // Update qualified name if no explicit qualified name is set
        if (qualifiedName_.empty()) {
            qualifiedName_ = name;
        }
    }

    void setQualifiedName(const std::string& qname) { qualifiedName_ = qname; }
    void setType(TypeInfoPtr type) { type_ = type; }
    void setScope(ScopePtr scope) { scope_ = scope; }
    void setDeclaration(ASTNodePtr decl) { declaration_ = decl; }
    void setGlobal(bool global) { isGlobal_ = global; }
    void setStatic(bool isStatic) { isStatic_ = isStatic; }
    void setExtern(bool isExtern) { isExtern_ = isExtern; }
    void setInline(bool isInline) { isInline_ = isInline; }

    void addReference(ASTNodePtr ref) {
        std::lock_guard<std::mutex> lock(referencesMutex_);
        references_.push_back(ref);
    }

    void clearReferences() {
        std::lock_guard<std::mutex> lock(referencesMutex_);
        references_.clear();
    }

private:
    std::string getKindName() const;
};

} // namespace core
} // namespace nomic

#endif // NOMIC_CORE_SYMBOL_H