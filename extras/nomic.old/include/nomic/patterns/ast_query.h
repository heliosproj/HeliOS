/*
 * Nomic C Semantic Source Code Analyzer
 * (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 * This file is part of Nomic.
 *
 * Nomic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nomic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nomic. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef NOMIC_PATTERNS_AST_QUERY_H
#define NOMIC_PATTERNS_AST_QUERY_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <functional>

#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"

namespace nomic::patterns {

/**
 * @brief AST query node type
 */
enum class ASTQueryNodeType {
    // Declarations
    FUNCTION,           // function declaration
    VARIABLE,           // variable declaration
    PARAMETER,          // function parameter
    TYPE,               // type declaration
    STRUCT,             // struct declaration
    ENUM,               // enum declaration

    // Statements
    IF_STMT,            // if statement
    FOR_STMT,           // for loop
    WHILE_STMT,         // while loop
    DO_STMT,            // do-while loop
    SWITCH_STMT,        // switch statement
    CASE_STMT,          // case label
    RETURN_STMT,        // return statement
    BREAK_STMT,         // break statement
    CONTINUE_STMT,      // continue statement
    COMPOUND_STMT,      // { ... } block

    // Expressions
    CALL_EXPR,          // function call
    BINARY_OP,          // binary operator
    UNARY_OP,           // unary operator
    ASSIGNMENT,         // assignment
    ARRAY_SUBSCRIPT,    // array[index]
    MEMBER_ACCESS,      // struct.member
    PTR_MEMBER_ACCESS,  // ptr->member
    CAST_EXPR,          // type cast
    SIZEOF_EXPR,        // sizeof()
    LITERAL,            // literal value

    // Wildcards and combinators
    ANY,                // matches any node
    DESCENDANT,         // matches any descendant (...)
    CHILD,              // matches direct child (>)
    FOLLOWING_SIBLING,  // matches following sibling (~)
    PARENT,             // matches parent (<)
    ANCESTOR,           // matches any ancestor (^)

    // Logical
    AND,                // logical and
    OR,                 // logical or
    NOT                 // logical not
};

/**
 * @brief Operator types for filtering
 */
enum class ASTQueryOperator {
    EQUALS,             // ==
    NOT_EQUALS,         // !=
    CONTAINS,           // ~=
    MATCHES_REGEX,      // =~
    LESS_THAN,          // <
    GREATER_THAN,       // >
    LESS_EQUAL,         // <=
    GREATER_EQUAL,      // >=
    IN,                 // in [...]
    HAS_ATTRIBUTE       // has @attribute
};

/**
 * @brief Predicate for filtering AST nodes
 */
class ASTQueryPredicate {
public:
    ASTQueryPredicate(const std::string& attr, ASTQueryOperator op, const std::string& value)
        : attribute_(attr), operator_(op), value_(value) {}

    const std::string& getAttribute() const { return attribute_; }
    ASTQueryOperator getOperator() const { return operator_; }
    const std::string& getValue() const { return value_; }

    bool evaluate(const std::map<std::string, std::string>& attributes) const;

private:
    std::string attribute_;      // Attribute name (e.g., "name", "type")
    ASTQueryOperator operator_;  // Comparison operator
    std::string value_;          // Expected value
};

/**
 * @brief Capture binding for extracting matched nodes
 */
class ASTQueryCapture {
public:
    ASTQueryCapture(const std::string& name) : name_(name) {}

    const std::string& getName() const { return name_; }
    void addNode(const clang::Stmt* node) { captured_nodes_.push_back(node); }
    void addDecl(const clang::Decl* decl) { captured_decls_.push_back(decl); }

    const std::vector<const clang::Stmt*>& getNodes() const { return captured_nodes_; }
    const std::vector<const clang::Decl*>& getDecls() const { return captured_decls_; }

private:
    std::string name_;
    std::vector<const clang::Stmt*> captured_nodes_;
    std::vector<const clang::Decl*> captured_decls_;
};

/**
 * @brief AST query node in the query tree
 */
class ASTQueryNode {
public:
    ASTQueryNode(ASTQueryNodeType type) : type_(type) {}

    // Setters
    void setNodeType(ASTQueryNodeType type) { type_ = type; }
    void addPredicate(const ASTQueryPredicate& pred) { predicates_.push_back(pred); }
    void addChild(std::unique_ptr<ASTQueryNode> child) { children_.push_back(std::move(child)); }
    void setCombinator(ASTQueryNodeType comb) { combinator_ = comb; }
    void setCaptureName(const std::string& name) { capture_name_ = name; }
    void setOptional(bool opt) { optional_ = opt; }
    void setQuantifier(const std::string& quant) { quantifier_ = quant; } // *, +, ?

    // Getters
    ASTQueryNodeType getNodeType() const { return type_; }
    const std::vector<ASTQueryPredicate>& getPredicates() const { return predicates_; }
    const std::vector<std::unique_ptr<ASTQueryNode>>& getChildren() const { return children_; }
    std::optional<ASTQueryNodeType> getCombinator() const { return combinator_; }
    const std::optional<std::string>& getCaptureName() const { return capture_name_; }
    bool isOptional() const { return optional_; }
    const std::optional<std::string>& getQuantifier() const { return quantifier_; }

private:
    ASTQueryNodeType type_;
    std::vector<ASTQueryPredicate> predicates_;
    std::vector<std::unique_ptr<ASTQueryNode>> children_;
    std::optional<ASTQueryNodeType> combinator_;  // How to match children
    std::optional<std::string> capture_name_;     // Capture binding name
    bool optional_ = false;                        // Is this node optional?
    std::optional<std::string> quantifier_;       // *, +, ? for repetition
};

/**
 * @brief Result of an AST query match
 */
class ASTQueryMatch {
public:
    void addCapture(const std::string& name, const clang::Stmt* node);
    void addCapture(const std::string& name, const clang::Decl* decl);

    const clang::Stmt* getCapturedNode(const std::string& name) const;
    const clang::Decl* getCapturedDecl(const std::string& name) const;

    const std::map<std::string, std::vector<const clang::Stmt*>>& getAllNodeCaptures() const {
        return node_captures_;
    }

    const std::map<std::string, std::vector<const clang::Decl*>>& getAllDeclCaptures() const {
        return decl_captures_;
    }

private:
    std::map<std::string, std::vector<const clang::Stmt*>> node_captures_;
    std::map<std::string, std::vector<const clang::Decl*>> decl_captures_;
};

/**
 * @brief Parser for AST query language
 *
 * Query Syntax Examples:
 *
 * 1. Find all malloc calls without NULL checks:
 *    call_expr[@name="malloc"] ~ if_stmt[condition ~= "*NULL*"]
 *
 * 2. Find nested loops:
 *    for_stmt ... for_stmt ... for_stmt
 *
 * 3. Find functions with multiple returns:
 *    function > return_stmt{3,}
 *
 * 4. Find unsafe strcpy usage:
 *    call_expr[@name="strcpy"][@arg_count=2]
 *
 * 5. Find if without braces:
 *    if_stmt[!has_compound_stmt]
 *
 * 6. Capture pattern with bindings:
 *    call_expr[@name="malloc"] @alloc ~ (!call_expr[@name="free"]) ... return_stmt
 */
class ASTQueryParser {
public:
    ASTQueryParser() = default;

    /**
     * @brief Parse query string into query tree
     */
    std::unique_ptr<ASTQueryNode> parse(const std::string& query);

private:
    std::string query_;
    size_t pos_ = 0;

    // Parsing methods
    std::unique_ptr<ASTQueryNode> parseExpression();
    std::unique_ptr<ASTQueryNode> parseNode();
    std::vector<ASTQueryPredicate> parsePredicates();
    ASTQueryPredicate parsePredicate();
    std::string parseIdentifier();
    std::string parseString();
    ASTQueryNodeType parseNodeType(const std::string& name);
    ASTQueryOperator parseOperator();

    // Utility methods
    void skipWhitespace();
    char peek() const;
    char advance();
    bool match(char ch);
    bool isAtEnd() const;
};

/**
 * @brief Matcher for executing AST queries
 */
class ASTQueryMatcher : public clang::RecursiveASTVisitor<ASTQueryMatcher> {
public:
    ASTQueryMatcher(const ASTQueryNode& query) : query_(query) {}

    /**
     * @brief Execute query and return all matches
     */
    std::vector<ASTQueryMatch> match(clang::Decl* root);
    std::vector<ASTQueryMatch> match(clang::Stmt* root);

    // RecursiveASTVisitor overrides
    bool VisitStmt(clang::Stmt* stmt);
    bool VisitDecl(clang::Decl* decl);

private:
    const ASTQueryNode& query_;
    std::vector<ASTQueryMatch> matches_;

    // Matching methods
    bool matchNode(const ASTQueryNode& query_node, const clang::Stmt* stmt, ASTQueryMatch& match);
    bool matchNode(const ASTQueryNode& query_node, const clang::Decl* decl, ASTQueryMatch& match);
    bool matchPredicates(const std::vector<ASTQueryPredicate>& predicates,
                        const std::map<std::string, std::string>& attributes);
    std::map<std::string, std::string> extractAttributes(const clang::Stmt* stmt);
    std::map<std::string, std::string> extractAttributes(const clang::Decl* decl);
    bool matchChildren(const ASTQueryNode& query_node, const clang::Stmt* stmt, ASTQueryMatch& match);
};

/**
 * @brief High-level API for AST queries
 */
class ASTQueryEngine {
public:
    ASTQueryEngine() = default;

    /**
     * @brief Execute a query and return matches
     */
    std::vector<ASTQueryMatch> query(const std::string& query_str, clang::Decl* root);
    std::vector<ASTQueryMatch> query(const std::string& query_str, clang::Stmt* root);

    /**
     * @brief Check if query matches (returns true/false)
     */
    bool matches(const std::string& query_str, clang::Decl* root);
    bool matches(const std::string& query_str, clang::Stmt* root);

    /**
     * @brief Count matches
     */
    size_t count(const std::string& query_str, clang::Decl* root);
    size_t count(const std::string& query_str, clang::Stmt* root);

private:
    ASTQueryParser parser_;
};

/**
 * @brief Template-based pattern system
 */
class ASTTemplate {
public:
    ASTTemplate() = default;
    ASTTemplate(const std::string& name, const std::string& description)
        : name_(name), description_(description) {}

    void setQuery(const std::string& query) { query_ = query; }
    void addParameter(const std::string& name, const std::string& default_value = "");
    void setFixTemplate(const std::string& fix_template) { fix_template_ = fix_template; }

    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }
    const std::string& getQuery() const { return query_; }

    /**
     * @brief Instantiate template with parameters
     */
    std::string instantiate(const std::map<std::string, std::string>& params) const;

private:
    std::string name_;
    std::string description_;
    std::string query_;
    std::map<std::string, std::string> parameters_;
    std::optional<std::string> fix_template_;
};

/**
 * @brief Template library for common patterns
 */
class ASTTemplateLibrary {
public:
    static ASTTemplateLibrary& instance();

    void registerTemplate(const ASTTemplate& tmpl);
    const ASTTemplate* getTemplate(const std::string& name) const;
    std::vector<std::string> listTemplates() const;

    /**
     * @brief Load built-in templates
     */
    void loadBuiltinTemplates();

private:
    ASTTemplateLibrary() = default;
    std::map<std::string, ASTTemplate> templates_;
};

} // namespace nomic::patterns

#endif // NOMIC_PATTERNS_AST_QUERY_H
