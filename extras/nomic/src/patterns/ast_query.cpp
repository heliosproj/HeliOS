/*
 * Nomic C Semantic Source Code Analyzer
 * Copyright (C) 2025 Manny Peterson <me@mannypeterson.com>
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

#include "nomic/patterns/ast_query.h"
#include <spdlog/spdlog.h>
#include <regex>
#include <algorithm>
#include <cctype>

namespace nomic::patterns {

// ==================== ASTQueryPredicate ====================

bool ASTQueryPredicate::evaluate(const std::map<std::string, std::string>& attributes) const {
    auto it = attributes.find(attribute_);
    if (it == attributes.end()) {
        return false;
    }

    const std::string& attr_value = it->second;

    switch (operator_) {
        case ASTQueryOperator::EQUALS:
            return attr_value == value_;

        case ASTQueryOperator::NOT_EQUALS:
            return attr_value != value_;

        case ASTQueryOperator::CONTAINS:
            return attr_value.find(value_) != std::string::npos;

        case ASTQueryOperator::MATCHES_REGEX:
            try {
                std::regex pattern(value_);
                return std::regex_search(attr_value, pattern);
            } catch (const std::regex_error& e) {
                spdlog::error("Invalid regex pattern '{}': {}", value_, e.what());
                return false;
            }

        case ASTQueryOperator::LESS_THAN:
            try {
                return std::stoi(attr_value) < std::stoi(value_);
            } catch (...) {
                return attr_value < value_;
            }

        case ASTQueryOperator::GREATER_THAN:
            try {
                return std::stoi(attr_value) > std::stoi(value_);
            } catch (...) {
                return attr_value > value_;
            }

        case ASTQueryOperator::LESS_EQUAL:
            try {
                return std::stoi(attr_value) <= std::stoi(value_);
            } catch (...) {
                return attr_value <= value_;
            }

        case ASTQueryOperator::GREATER_EQUAL:
            try {
                return std::stoi(attr_value) >= std::stoi(value_);
            } catch (...) {
                return attr_value >= value_;
            }

        default:
            return false;
    }
}

// ==================== ASTQueryMatch ====================

void ASTQueryMatch::addCapture(const std::string& name, const clang::Stmt* node) {
    node_captures_[name].push_back(node);
}

void ASTQueryMatch::addCapture(const std::string& name, const clang::Decl* decl) {
    decl_captures_[name].push_back(decl);
}

const clang::Stmt* ASTQueryMatch::getCapturedNode(const std::string& name) const {
    auto it = node_captures_.find(name);
    if (it != node_captures_.end() && !it->second.empty()) {
        return it->second[0];
    }
    return nullptr;
}

const clang::Decl* ASTQueryMatch::getCapturedDecl(const std::string& name) const {
    auto it = decl_captures_.find(name);
    if (it != decl_captures_.end() && !it->second.empty()) {
        return it->second[0];
    }
    return nullptr;
}

// ==================== ASTQueryParser ====================

std::unique_ptr<ASTQueryNode> ASTQueryParser::parse(const std::string& query) {
    query_ = query;
    pos_ = 0;

    skipWhitespace();
    return parseExpression();
}

std::unique_ptr<ASTQueryNode> ASTQueryParser::parseExpression() {
    auto node = parseNode();

    skipWhitespace();

    // Check for combinators
    if (!isAtEnd()) {
        if (match('>')) {
            // Direct child combinator
            node->setCombinator(ASTQueryNodeType::CHILD);
            skipWhitespace();
            node->addChild(parseExpression());
        } else if (peek() == '.') {
            // Descendant combinator (...)
            if (pos_ + 2 < query_.size() && query_[pos_ + 1] == '.' && query_[pos_ + 2] == '.') {
                pos_ += 3;
                node->setCombinator(ASTQueryNodeType::DESCENDANT);
                skipWhitespace();
                node->addChild(parseExpression());
            }
        } else if (match('~')) {
            // Following sibling combinator
            node->setCombinator(ASTQueryNodeType::FOLLOWING_SIBLING);
            skipWhitespace();
            node->addChild(parseExpression());
        }
    }

    return node;
}

std::unique_ptr<ASTQueryNode> ASTQueryParser::parseNode() {
    skipWhitespace();

    // Check for capture binding (@name)
    std::string capture_name;
    if (match('@')) {
        capture_name = parseIdentifier();
        skipWhitespace();
    }

    // Parse node type
    std::string type_name = parseIdentifier();
    ASTQueryNodeType type = parseNodeType(type_name);

    auto node = std::make_unique<ASTQueryNode>(type);

    if (!capture_name.empty()) {
        node->setCaptureName(capture_name);
    }

    // Parse predicates [...]
    if (match('[')) {
        auto predicates = parsePredicates();
        for (const auto& pred : predicates) {
            node->addPredicate(pred);
        }

        if (!match(']')) {
            spdlog::error("Expected ']' to close predicates");
        }
    }

    // Parse quantifier {n,m}, *, +, ?
    skipWhitespace();
    if (match('{')) {
        std::string quantifier;
        while (!isAtEnd() && peek() != '}') {
            quantifier += advance();
        }
        match('}');
        node->setQuantifier(quantifier);
    } else if (match('*')) {
        node->setQuantifier("*");
    } else if (match('+')) {
        node->setQuantifier("+");
    } else if (match('?')) {
        node->setQuantifier("?");
        node->setOptional(true);
    }

    return node;
}

std::vector<ASTQueryPredicate> ASTQueryParser::parsePredicates() {
    std::vector<ASTQueryPredicate> predicates;

    skipWhitespace();

    while (!isAtEnd() && peek() != ']') {
        predicates.push_back(parsePredicate());
        skipWhitespace();

        // Skip comma separator
        if (peek() == ',') {
            advance();
            skipWhitespace();
        }
    }

    return predicates;
}

ASTQueryPredicate ASTQueryParser::parsePredicate() {
    skipWhitespace();

    // Parse attribute name
    if (!match('@')) {
        spdlog::error("Expected '@' before attribute name");
    }

    std::string attr_name = parseIdentifier();
    skipWhitespace();

    // Parse operator
    ASTQueryOperator op = parseOperator();
    skipWhitespace();

    // Parse value
    std::string value = parseString();

    return ASTQueryPredicate(attr_name, op, value);
}

std::string ASTQueryParser::parseIdentifier() {
    std::string id;

    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
        id += advance();
    }

    return id;
}

std::string ASTQueryParser::parseString() {
    std::string str;

    if (match('"')) {
        // Quoted string
        while (!isAtEnd() && peek() != '"') {
            if (peek() == '\\') {
                advance();
                if (!isAtEnd()) {
                    str += advance();
                }
            } else {
                str += advance();
            }
        }
        match('"');
    } else {
        // Unquoted identifier/value
        while (!isAtEnd() && !std::isspace(peek()) && peek() != ']' && peek() != ',') {
            str += advance();
        }
    }

    return str;
}

ASTQueryNodeType ASTQueryParser::parseNodeType(const std::string& name) {
    static const std::map<std::string, ASTQueryNodeType> type_map = {
        {"function", ASTQueryNodeType::FUNCTION},
        {"variable", ASTQueryNodeType::VARIABLE},
        {"parameter", ASTQueryNodeType::PARAMETER},
        {"if_stmt", ASTQueryNodeType::IF_STMT},
        {"for_stmt", ASTQueryNodeType::FOR_STMT},
        {"while_stmt", ASTQueryNodeType::WHILE_STMT},
        {"do_stmt", ASTQueryNodeType::DO_STMT},
        {"switch_stmt", ASTQueryNodeType::SWITCH_STMT},
        {"return_stmt", ASTQueryNodeType::RETURN_STMT},
        {"call_expr", ASTQueryNodeType::CALL_EXPR},
        {"binary_op", ASTQueryNodeType::BINARY_OP},
        {"unary_op", ASTQueryNodeType::UNARY_OP},
        {"assignment", ASTQueryNodeType::ASSIGNMENT},
        {"array_subscript", ASTQueryNodeType::ARRAY_SUBSCRIPT},
        {"any", ASTQueryNodeType::ANY},
    };

    auto it = type_map.find(name);
    if (it != type_map.end()) {
        return it->second;
    }

    spdlog::warn("Unknown AST node type: {}, using ANY", name);
    return ASTQueryNodeType::ANY;
}

ASTQueryOperator ASTQueryParser::parseOperator() {
    if (match('=')) {
        if (match('=')) {
            return ASTQueryOperator::EQUALS;
        } else if (match('~')) {
            return ASTQueryOperator::MATCHES_REGEX;
        }
        return ASTQueryOperator::EQUALS;
    } else if (match('!')) {
        if (match('=')) {
            return ASTQueryOperator::NOT_EQUALS;
        }
    } else if (match('~')) {
        if (match('=')) {
            return ASTQueryOperator::CONTAINS;
        }
    } else if (match('<')) {
        if (match('=')) {
            return ASTQueryOperator::LESS_EQUAL;
        }
        return ASTQueryOperator::LESS_THAN;
    } else if (match('>')) {
        if (match('=')) {
            return ASTQueryOperator::GREATER_EQUAL;
        }
        return ASTQueryOperator::GREATER_THAN;
    }

    spdlog::error("Unknown operator at position {}", pos_);
    return ASTQueryOperator::EQUALS;
}

void ASTQueryParser::skipWhitespace() {
    while (!isAtEnd() && std::isspace(peek())) {
        advance();
    }
}

char ASTQueryParser::peek() const {
    if (isAtEnd()) {
        return '\0';
    }
    return query_[pos_];
}

char ASTQueryParser::advance() {
    if (isAtEnd()) {
        return '\0';
    }
    return query_[pos_++];
}

bool ASTQueryParser::match(char ch) {
    if (peek() == ch) {
        advance();
        return true;
    }
    return false;
}

bool ASTQueryParser::isAtEnd() const {
    return pos_ >= query_.size();
}

// ==================== ASTQueryMatcher ====================

std::vector<ASTQueryMatch> ASTQueryMatcher::match(clang::Decl* root) {
    matches_.clear();
    TraverseDecl(root);
    return matches_;
}

std::vector<ASTQueryMatch> ASTQueryMatcher::match(clang::Stmt* root) {
    matches_.clear();
    TraverseStmt(root);
    return matches_;
}

bool ASTQueryMatcher::VisitStmt(clang::Stmt* stmt) {
    ASTQueryMatch match;
    if (matchNode(query_, stmt, match)) {
        matches_.push_back(match);
    }
    return true;
}

bool ASTQueryMatcher::VisitDecl(clang::Decl* decl) {
    ASTQueryMatch match;
    if (matchNode(query_, decl, match)) {
        matches_.push_back(match);
    }
    return true;
}

bool ASTQueryMatcher::matchNode(const ASTQueryNode& query_node, const clang::Stmt* stmt, ASTQueryMatch& match) {
    if (!stmt) return false;

    // Match node type
    ASTQueryNodeType query_type = query_node.getNodeType();

    if (query_type == ASTQueryNodeType::ANY) {
        // ANY matches everything
    } else if (query_type == ASTQueryNodeType::CALL_EXPR) {
        if (!llvm::isa<clang::CallExpr>(stmt)) {
            return false;
        }
    } else if (query_type == ASTQueryNodeType::IF_STMT) {
        if (!llvm::isa<clang::IfStmt>(stmt)) {
            return false;
        }
    } else if (query_type == ASTQueryNodeType::FOR_STMT) {
        if (!llvm::isa<clang::ForStmt>(stmt)) {
            return false;
        }
    } else if (query_type == ASTQueryNodeType::WHILE_STMT) {
        if (!llvm::isa<clang::WhileStmt>(stmt)) {
            return false;
        }
    } else if (query_type == ASTQueryNodeType::RETURN_STMT) {
        if (!llvm::isa<clang::ReturnStmt>(stmt)) {
            return false;
        }
    } else if (query_type == ASTQueryNodeType::BINARY_OP) {
        if (!llvm::isa<clang::BinaryOperator>(stmt)) {
            return false;
        }
    }

    // Extract attributes and match predicates
    auto attributes = extractAttributes(stmt);
    if (!matchPredicates(query_node.getPredicates(), attributes)) {
        return false;
    }

    // Add capture if specified
    if (query_node.getCaptureName().has_value()) {
        match.addCapture(query_node.getCaptureName().value(), stmt);
    }

    // Match children if specified
    if (!query_node.getChildren().empty()) {
        return matchChildren(query_node, stmt, match);
    }

    return true;
}

bool ASTQueryMatcher::matchNode(const ASTQueryNode& query_node, const clang::Decl* decl, ASTQueryMatch& match) {
    if (!decl) return false;

    // Match node type
    ASTQueryNodeType query_type = query_node.getNodeType();

    if (query_type == ASTQueryNodeType::ANY) {
        // ANY matches everything
    } else if (query_type == ASTQueryNodeType::FUNCTION) {
        if (!llvm::isa<clang::FunctionDecl>(decl)) {
            return false;
        }
    } else if (query_type == ASTQueryNodeType::VARIABLE) {
        if (!llvm::isa<clang::VarDecl>(decl)) {
            return false;
        }
    }

    // Extract attributes and match predicates
    auto attributes = extractAttributes(decl);
    if (!matchPredicates(query_node.getPredicates(), attributes)) {
        return false;
    }

    // Add capture if specified
    if (query_node.getCaptureName().has_value()) {
        match.addCapture(query_node.getCaptureName().value(), decl);
    }

    return true;
}

bool ASTQueryMatcher::matchPredicates(const std::vector<ASTQueryPredicate>& predicates,
                                     const std::map<std::string, std::string>& attributes) {
    for (const auto& pred : predicates) {
        if (!pred.evaluate(attributes)) {
            return false;
        }
    }
    return true;
}

std::map<std::string, std::string> ASTQueryMatcher::extractAttributes(const clang::Stmt* stmt) {
    std::map<std::string, std::string> attributes;

    if (auto* call = llvm::dyn_cast<clang::CallExpr>(stmt)) {
        if (auto* callee_decl = call->getDirectCallee()) {
            attributes["name"] = callee_decl->getNameAsString();

            // Add qualified name if available
            if (auto* named_decl = llvm::dyn_cast<clang::NamedDecl>(callee_decl)) {
                attributes["qualified_name"] = named_decl->getQualifiedNameAsString();
            }
        }
        attributes["arg_count"] = std::to_string(call->getNumArgs());

        // Add type information for return type
        clang::QualType return_type = call->getType();
        attributes["return_type"] = return_type.getAsString();

        // Check if call is in const context
        attributes["is_const"] = return_type.isConstQualified() ? "true" : "false";

    } else if (auto* binop = llvm::dyn_cast<clang::BinaryOperator>(stmt)) {
        attributes["operator"] = binop->getOpcodeStr().str();
        attributes["is_assignment"] = binop->isAssignmentOp() ? "true" : "false";
        attributes["is_comparison"] = binop->isComparisonOp() ? "true" : "false";

        // Add type information
        attributes["type"] = binop->getType().getAsString();

    } else if (auto* unop = llvm::dyn_cast<clang::UnaryOperator>(stmt)) {
        attributes["operator"] = clang::UnaryOperator::getOpcodeStr(unop->getOpcode()).str();
        attributes["is_postfix"] = unop->isPostfix() ? "true" : "false";
        attributes["is_prefix"] = unop->isPrefix() ? "true" : "false";

    } else if (auto* array_sub = llvm::dyn_cast<clang::ArraySubscriptExpr>(stmt)) {
        attributes["type"] = array_sub->getType().getAsString();

    } else if (auto* member = llvm::dyn_cast<clang::MemberExpr>(stmt)) {
        if (auto* member_decl = member->getMemberDecl()) {
            attributes["member_name"] = member_decl->getNameAsString();
        }
        attributes["is_arrow"] = member->isArrow() ? "true" : "false";
        attributes["type"] = member->getType().getAsString();

    } else if (auto* cast = llvm::dyn_cast<clang::CastExpr>(stmt)) {
        attributes["cast_kind"] = cast->getCastKindName();
        attributes["type"] = cast->getType().getAsString();

    } else if (auto* literal = llvm::dyn_cast<clang::IntegerLiteral>(stmt)) {
        attributes["value"] = std::to_string(literal->getValue().getLimitedValue());
        attributes["type"] = literal->getType().getAsString();

    } else if (auto* str_literal = llvm::dyn_cast<clang::StringLiteral>(stmt)) {
        attributes["value"] = str_literal->getString().str();
        attributes["length"] = std::to_string(str_literal->getLength());
    }

    // Add common attributes for all statements
    attributes["class"] = stmt->getStmtClassName();

    return attributes;
}

std::map<std::string, std::string> ASTQueryMatcher::extractAttributes(const clang::Decl* decl) {
    std::map<std::string, std::string> attributes;

    if (auto* func = llvm::dyn_cast<clang::FunctionDecl>(decl)) {
        attributes["name"] = func->getNameAsString();
        attributes["qualified_name"] = func->getQualifiedNameAsString();
        attributes["param_count"] = std::to_string(func->getNumParams());

        // Return type
        attributes["return_type"] = func->getReturnType().getAsString();

        // Storage class and linkage
        attributes["is_static"] = func->getStorageClass() == clang::SC_Static ? "true" : "false";
        attributes["is_inline"] = func->isInlineSpecified() ? "true" : "false";
        attributes["is_extern"] = func->getStorageClass() == clang::SC_Extern ? "true" : "false";
        attributes["linkage"] = clang::VarDecl::getStorageClassSpecifierString(func->getStorageClass());

        // Function properties
        attributes["is_variadic"] = func->isVariadic() ? "true" : "false";
        attributes["has_body"] = func->hasBody() ? "true" : "false";
        attributes["is_main"] = func->isMain() ? "true" : "false";

    } else if (auto* var = llvm::dyn_cast<clang::VarDecl>(decl)) {
        attributes["name"] = var->getNameAsString();
        attributes["qualified_name"] = var->getQualifiedNameAsString();
        attributes["type"] = var->getType().getAsString();

        // Storage and qualifiers
        attributes["is_static"] = var->getStorageClass() == clang::SC_Static ? "true" : "false";
        attributes["is_extern"] = var->getStorageClass() == clang::SC_Extern ? "true" : "false";
        attributes["is_const"] = var->getType().isConstQualified() ? "true" : "false";
        attributes["is_volatile"] = var->getType().isVolatileQualified() ? "true" : "false";

        // Scope
        attributes["is_local"] = var->isLocalVarDecl() ? "true" : "false";
        attributes["is_global"] = var->hasGlobalStorage() ? "true" : "false";
        attributes["has_init"] = var->hasInit() ? "true" : "false";

    } else if (auto* parm = llvm::dyn_cast<clang::ParmVarDecl>(decl)) {
        attributes["name"] = parm->getNameAsString();
        attributes["type"] = parm->getType().getAsString();
        attributes["has_default"] = parm->hasDefaultArg() ? "true" : "false";

    } else if (auto* record = llvm::dyn_cast<clang::RecordDecl>(decl)) {
        attributes["name"] = record->getNameAsString();
        attributes["is_struct"] = record->isStruct() ? "true" : "false";
        attributes["is_union"] = record->isUnion() ? "true" : "false";
        attributes["is_class"] = record->isClass() ? "true" : "false";
        attributes["is_complete"] = record->isCompleteDefinition() ? "true" : "false";

    } else if (auto* enum_decl = llvm::dyn_cast<clang::EnumDecl>(decl)) {
        attributes["name"] = enum_decl->getNameAsString();
        attributes["is_scoped"] = enum_decl->isScoped() ? "true" : "false";
    }

    // Add common attributes
    attributes["class"] = decl->getDeclKindName();

    return attributes;
}

bool ASTQueryMatcher::matchChildren(const ASTQueryNode& query_node, const clang::Stmt* stmt, ASTQueryMatch& match) {
    // Simplified child matching - full implementation would handle combinators
    return true;
}

// ==================== ASTQueryEngine ====================

std::vector<ASTQueryMatch> ASTQueryEngine::query(const std::string& query_str, clang::Decl* root) {
    auto query_tree = parser_.parse(query_str);
    if (!query_tree) {
        spdlog::error("Failed to parse query: {}", query_str);
        return {};
    }

    ASTQueryMatcher matcher(*query_tree);
    return matcher.match(root);
}

std::vector<ASTQueryMatch> ASTQueryEngine::query(const std::string& query_str, clang::Stmt* root) {
    auto query_tree = parser_.parse(query_str);
    if (!query_tree) {
        spdlog::error("Failed to parse query: {}", query_str);
        return {};
    }

    ASTQueryMatcher matcher(*query_tree);
    return matcher.match(root);
}

bool ASTQueryEngine::matches(const std::string& query_str, clang::Decl* root) {
    return !query(query_str, root).empty();
}

bool ASTQueryEngine::matches(const std::string& query_str, clang::Stmt* root) {
    return !query(query_str, root).empty();
}

size_t ASTQueryEngine::count(const std::string& query_str, clang::Decl* root) {
    return query(query_str, root).size();
}

size_t ASTQueryEngine::count(const std::string& query_str, clang::Stmt* root) {
    return query(query_str, root).size();
}

// ==================== ASTTemplate ====================

void ASTTemplate::addParameter(const std::string& name, const std::string& default_value) {
    parameters_[name] = default_value;
}

std::string ASTTemplate::instantiate(const std::map<std::string, std::string>& params) const {
    std::string result = query_;

    // Replace ${param} with actual values
    for (const auto& [name, default_val] : parameters_) {
        std::string placeholder = "${" + name + "}";
        std::string value = default_val;

        auto it = params.find(name);
        if (it != params.end()) {
            value = it->second;
        }

        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), value);
            pos += value.length();
        }
    }

    return result;
}

// ==================== ASTTemplateLibrary ====================

ASTTemplateLibrary& ASTTemplateLibrary::instance() {
    static ASTTemplateLibrary lib;
    return lib;
}

void ASTTemplateLibrary::registerTemplate(const ASTTemplate& tmpl) {
    templates_[tmpl.getName()] = tmpl;
}

const ASTTemplate* ASTTemplateLibrary::getTemplate(const std::string& name) const {
    auto it = templates_.find(name);
    if (it != templates_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> ASTTemplateLibrary::listTemplates() const {
    std::vector<std::string> names;
    for (const auto& [name, _] : templates_) {
        names.push_back(name);
    }
    return names;
}

void ASTTemplateLibrary::loadBuiltinTemplates() {
    // Template: Unchecked malloc
    {
        ASTTemplate tmpl("unchecked_malloc", "Detect malloc without NULL check");
        tmpl.setQuery("call_expr[@name=\"malloc\"] ~ if_stmt[@condition~=\"NULL\"]");
        registerTemplate(tmpl);
    }

    // Template: Nested loops
    {
        ASTTemplate tmpl("nested_loops", "Detect deeply nested loops");
        tmpl.addParameter("depth", "3");
        tmpl.setQuery("for_stmt ... for_stmt ... for_stmt");
        registerTemplate(tmpl);
    }

    // Template: Multiple returns
    {
        ASTTemplate tmpl("multiple_returns", "Functions with many return statements");
        tmpl.addParameter("count", "5");
        tmpl.setQuery("function > return_stmt{${count},}");
        registerTemplate(tmpl);
    }

    // Template: Unsafe strcpy
    {
        ASTTemplate tmpl("unsafe_strcpy", "Unsafe strcpy usage");
        tmpl.setQuery("call_expr[@name=\"strcpy\"][@arg_count=2]");
        registerTemplate(tmpl);
    }
}

} // namespace nomic::patterns
