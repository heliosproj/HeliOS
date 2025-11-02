#ifndef NOMIC_CORE_INTERFACES_H
#define NOMIC_CORE_INTERFACES_H

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <any>

namespace nomic {
namespace core {

// Forward declarations
class IASTNode;
class ISemanticModel;
class ITypeInfo;
class ISymbol;
class IScope;
class IVisitor;

// Smart pointer types for interface ownership
using ASTNodePtr = std::shared_ptr<IASTNode>;
using SemanticModelPtr = std::shared_ptr<ISemanticModel>;
using TypeInfoPtr = std::shared_ptr<ITypeInfo>;
using SymbolPtr = std::shared_ptr<ISymbol>;
using ScopePtr = std::shared_ptr<IScope>;

/**
 * @brief Abstract base interface for all AST nodes
 *
 * This interface provides a compiler-agnostic abstraction over AST nodes,
 * ensuring no direct coupling to Clang/LLVM types in public APIs (REQ-002).
 */
class IASTNode {
public:
    virtual ~IASTNode() = default;

    // Node type information
    enum class NodeKind {
        UNKNOWN,
        TRANSLATION_UNIT,
        FUNCTION_DECL,
        VAR_DECL,
        PARAM_DECL,
        FIELD_DECL,
        TYPE_DEF,
        STRUCT_DECL,
        UNION_DECL,
        ENUM_DECL,
        STATEMENT,
        EXPRESSION,
        LITERAL,
        IDENTIFIER,
        OPERATOR,
        CALL_EXPR,
        BINARY_OP,
        UNARY_OP,
        CAST_EXPR,
        ARRAY_SUBSCRIPT,
        MEMBER_EXPR,
        COMPOUND_STMT,
        IF_STMT,
        WHILE_STMT,
        FOR_STMT,
        DO_STMT,
        SWITCH_STMT,
        CASE_STMT,
        RETURN_STMT,
        BREAK_STMT,
        CONTINUE_STMT,
        GOTO_STMT,
        LABEL_STMT,
        DECL_STMT,
        EXPR_STMT
    };

    // Core node properties
    virtual NodeKind getKind() const = 0;
    virtual std::string getKindName() const = 0;
    virtual std::string getSourceLocation() const = 0;
    virtual std::pair<int, int> getLineColumn() const = 0;
    virtual std::string getSourceText() const = 0;

    // Tree navigation
    virtual ASTNodePtr getParent() const = 0;
    virtual std::vector<ASTNodePtr> getChildren() const = 0;
    virtual std::vector<ASTNodePtr> getAncestors() const = 0;
    virtual std::vector<ASTNodePtr> getDescendants() const = 0;
    virtual std::vector<ASTNodePtr> getSiblings() const = 0;

    // Semantic information
    virtual TypeInfoPtr getType() const = 0;
    virtual SymbolPtr getSymbol() const = 0;
    virtual ScopePtr getScope() const = 0;

    // Visitor pattern support
    virtual void accept(IVisitor& visitor) = 0;

    // Property access for extensibility
    virtual bool hasProperty(const std::string& key) const = 0;
    virtual std::any getProperty(const std::string& key) const = 0;
    virtual void setProperty(const std::string& key, const std::any& value) = 0;
};

/**
 * @brief Type information interface
 */
class ITypeInfo {
public:
    virtual ~ITypeInfo() = default;

    enum class TypeKind {
        VOID,
        BOOL,
        CHAR,
        INT,
        FLOAT,
        DOUBLE,
        POINTER,
        ARRAY,
        FUNCTION,
        STRUCT,
        UNION,
        ENUM,
        TYPEDEF,
        QUALIFIED,
        UNKNOWN
    };

    virtual TypeKind getKind() const = 0;
    virtual std::string getTypeName() const = 0;
    virtual size_t getSize() const = 0;
    virtual size_t getAlignment() const = 0;
    virtual bool isConst() const = 0;
    virtual bool isVolatile() const = 0;
    virtual bool isPointer() const = 0;
    virtual bool isArray() const = 0;
    virtual bool isFunction() const = 0;
    virtual bool isComplete() const = 0;
    virtual TypeInfoPtr getCanonicalType() const = 0;
    virtual TypeInfoPtr getPointeeType() const = 0;
    virtual TypeInfoPtr getReturnType() const = 0;
    virtual std::vector<TypeInfoPtr> getParameterTypes() const = 0;
};

/**
 * @brief Symbol information interface
 */
class ISymbol {
public:
    virtual ~ISymbol() = default;

    enum class SymbolKind {
        VARIABLE,
        FUNCTION,
        PARAMETER,
        FIELD,
        TYPEDEF,
        STRUCT,
        UNION,
        ENUM,
        ENUMERATOR,
        LABEL,
        UNKNOWN
    };

    virtual SymbolKind getKind() const = 0;
    virtual std::string getName() const = 0;
    virtual std::string getQualifiedName() const = 0;
    virtual TypeInfoPtr getType() const = 0;
    virtual ScopePtr getScope() const = 0;
    virtual ASTNodePtr getDeclaration() const = 0;
    virtual std::vector<ASTNodePtr> getReferences() const = 0;
    virtual bool isGlobal() const = 0;
    virtual bool isStatic() const = 0;
    virtual bool isExtern() const = 0;
    virtual bool isInline() const = 0;
};

/**
 * @brief Scope information interface
 */
class IScope {
public:
    virtual ~IScope() = default;

    enum class ScopeKind {
        GLOBAL,
        FILE,
        FUNCTION,
        BLOCK,
        STRUCT,
        UNION,
        ENUM,
        NAMESPACE,
        UNKNOWN
    };

    virtual ScopeKind getKind() const = 0;
    virtual ScopePtr getParent() const = 0;
    virtual std::vector<ScopePtr> getChildren() const = 0;
    virtual std::vector<SymbolPtr> getSymbols() const = 0;
    virtual SymbolPtr lookupSymbol(const std::string& name) const = 0;
    virtual ASTNodePtr getASTNode() const = 0;
};

/**
 * @brief Visitor pattern interface for AST traversal
 */
class IVisitor {
public:
    virtual ~IVisitor() = default;

    // Visit methods for different node types
    virtual void visitNode(IASTNode& node) = 0;
    virtual void visitFunctionDecl(IASTNode& node) = 0;
    virtual void visitVarDecl(IASTNode& node) = 0;
    virtual void visitStatement(IASTNode& node) = 0;
    virtual void visitExpression(IASTNode& node) = 0;

    // Control traversal
    virtual bool shouldTraverseChildren() const = 0;
    virtual void setTraverseChildren(bool traverse) = 0;
};

/**
 * @brief Semantic model interface providing high-level code analysis
 */
class ISemanticModel {
public:
    virtual ~ISemanticModel() = default;

    // AST access
    virtual ASTNodePtr getRoot() const = 0;
    virtual std::vector<ASTNodePtr> getAllNodes() const = 0;
    virtual std::vector<ASTNodePtr> getNodesByKind(IASTNode::NodeKind kind) const = 0;

    // Symbol table access
    virtual std::vector<SymbolPtr> getAllSymbols() const = 0;
    virtual SymbolPtr findSymbol(const std::string& name) const = 0;
    virtual std::vector<SymbolPtr> findSymbolsByKind(ISymbol::SymbolKind kind) const = 0;

    // Scope access
    virtual ScopePtr getGlobalScope() const = 0;
    virtual std::vector<ScopePtr> getAllScopes() const = 0;

    // Type information
    virtual std::vector<TypeInfoPtr> getAllTypes() const = 0;
    virtual TypeInfoPtr findType(const std::string& name) const = 0;

    // File information
    virtual std::vector<std::string> getSourceFiles() const = 0;
    virtual ASTNodePtr getFileAST(const std::string& filename) const = 0;

    // Analysis helpers
    virtual std::vector<ASTNodePtr> findReferences(SymbolPtr symbol) const = 0;
    virtual std::vector<ASTNodePtr> findCallsTo(const std::string& functionName) const = 0;
    virtual bool isReachable(ASTNodePtr from, ASTNodePtr to) const = 0;
};

/**
 * @brief Factory interface for creating AST nodes and semantic models
 * Ensures proper abstraction and loose coupling
 */
class IASTFactory {
public:
    virtual ~IASTFactory() = default;

    virtual SemanticModelPtr createSemanticModel(const std::vector<std::string>& sourceFiles) = 0;
    virtual ASTNodePtr createNode(IASTNode::NodeKind kind) = 0;
    virtual TypeInfoPtr createType(ITypeInfo::TypeKind kind) = 0;
    virtual SymbolPtr createSymbol(ISymbol::SymbolKind kind) = 0;
    virtual ScopePtr createScope(IScope::ScopeKind kind) = 0;
};

} // namespace core
} // namespace nomic

#endif // NOMIC_CORE_INTERFACES_H