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

#ifndef NOMIC_STANDARDS_CODING_STANDARDS_H
#define NOMIC_STANDARDS_CODING_STANDARDS_H

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <regex>

namespace nomic::standards {

// ==================== NAMING CONVENTIONS ====================

/**
 * @brief Supported naming case styles
 */
enum class CaseStyle {
    SNAKE_CASE,        // lowercase_with_underscores
    UPPER_CASE,        // UPPERCASE_WITH_UNDERSCORES
    CAMEL_CASE,        // camelCaseStartingLower
    PASCAL_CASE,       // PascalCaseStartingUpper
    KEBAB_CASE,        // lowercase-with-hyphens
    MIXED_CASE,        // Any mixed case
    ANY                // No restriction
};

/**
 * @brief Naming convention rule
 */
class NamingConvention {
public:
    NamingConvention() = default;
    NamingConvention(CaseStyle style) : case_style_(style) {}

    void setCaseStyle(CaseStyle style) { case_style_ = style; }
    void setPrefix(const std::string& prefix) { prefix_ = prefix; }
    void setSuffix(const std::string& suffix) { suffix_ = suffix; }
    void setPattern(const std::string& pattern) { regex_pattern_ = pattern; }
    void setMinLength(size_t len) { min_length_ = len; }
    void setMaxLength(size_t len) { max_length_ = len; }
    void setForbiddenNames(const std::vector<std::string>& names) { forbidden_names_ = names; }

    bool validate(const std::string& name, std::string& error_message) const;
    bool matchesCaseStyle(const std::string& name) const;

    CaseStyle getCaseStyle() const { return case_style_; }
    const std::optional<std::string>& getPrefix() const { return prefix_; }
    const std::optional<std::string>& getSuffix() const { return suffix_; }

private:
    CaseStyle case_style_ = CaseStyle::ANY;
    std::optional<std::string> prefix_;
    std::optional<std::string> suffix_;
    std::optional<std::string> regex_pattern_;
    std::optional<size_t> min_length_;
    std::optional<size_t> max_length_;
    std::vector<std::string> forbidden_names_;
};

/**
 * @brief Naming convention checker
 */
class NamingChecker {
public:
    // Set conventions for different entity types
    void setFunctionConvention(const NamingConvention& conv) { function_conv_ = conv; }
    void setVariableConvention(const NamingConvention& conv) { variable_conv_ = conv; }
    void setParameterConvention(const NamingConvention& conv) { parameter_conv_ = conv; }
    void setTypeConvention(const NamingConvention& conv) { type_conv_ = conv; }
    void setConstantConvention(const NamingConvention& conv) { constant_conv_ = conv; }
    void setMacroConvention(const NamingConvention& conv) { macro_conv_ = conv; }
    void setGlobalVariableConvention(const NamingConvention& conv) { global_var_conv_ = conv; }
    void setStaticVariableConvention(const NamingConvention& conv) { static_var_conv_ = conv; }
    void setEnumConstantConvention(const NamingConvention& conv) { enum_const_conv_ = conv; }
    void setStructConvention(const NamingConvention& conv) { struct_conv_ = conv; }

    // Check functions
    bool checkFunctionName(const std::string& name, std::string& error) const;
    bool checkVariableName(const std::string& name, bool is_const, bool is_global, bool is_static, std::string& error) const;
    bool checkParameterName(const std::string& name, std::string& error) const;
    bool checkTypeName(const std::string& name, std::string& error) const;
    bool checkMacroName(const std::string& name, std::string& error) const;
    bool checkEnumConstantName(const std::string& name, std::string& error) const;
    bool checkStructName(const std::string& name, std::string& error) const;

private:
    NamingConvention function_conv_;
    NamingConvention variable_conv_;
    NamingConvention parameter_conv_;
    NamingConvention type_conv_;
    NamingConvention constant_conv_;
    NamingConvention macro_conv_;
    NamingConvention global_var_conv_;
    NamingConvention static_var_conv_;
    NamingConvention enum_const_conv_;
    NamingConvention struct_conv_;
};

// ==================== DOCUMENTATION REQUIREMENTS ====================

/**
 * @brief Documentation style
 */
enum class DocStyle {
    DOXYGEN,           // /** @brief ... */
    KERNEL_DOC,        // /* ... */
    JAVADOC,           // /** ... */
    CUSTOM             // Custom format
};

/**
 * @brief Required documentation fields
 */
struct DocRequirements {
    bool require_brief = true;
    bool require_param_docs = true;
    bool require_return_doc = true;
    bool require_example = false;
    bool require_author = false;
    bool require_since = false;
    std::vector<std::string> custom_tags;
};

/**
 * @brief Parsed documentation comment
 */
class DocComment {
public:
    void setBrief(const std::string& brief) { brief_ = brief; }
    void setDetailed(const std::string& detailed) { detailed_ = detailed; }
    void addParam(const std::string& name, const std::string& desc);
    void setReturn(const std::string& desc) { return_desc_ = desc; }
    void addTag(const std::string& tag, const std::string& value);

    bool hasBrief() const { return brief_.has_value(); }
    bool hasParamDoc(const std::string& param) const;
    bool hasReturnDoc() const { return return_desc_.has_value(); }
    bool hasTag(const std::string& tag) const;

    const std::optional<std::string>& getBrief() const { return brief_; }
    const std::optional<std::string>& getDetailed() const { return detailed_; }
    const std::map<std::string, std::string>& getParams() const { return param_docs_; }
    const std::optional<std::string>& getReturn() const { return return_desc_; }

private:
    std::optional<std::string> brief_;
    std::optional<std::string> detailed_;
    std::map<std::string, std::string> param_docs_;
    std::optional<std::string> return_desc_;
    std::map<std::string, std::string> tags_;
};

/**
 * @brief Documentation parser
 */
class DocParser {
public:
    DocParser(DocStyle style = DocStyle::DOXYGEN) : style_(style) {}

    std::optional<DocComment> parse(const std::string& comment_text) const;
    bool isDocComment(const std::string& comment_text) const;

private:
    DocStyle style_;

    DocComment parseDoxygenComment(const std::string& text) const;
    DocComment parseKernelDocComment(const std::string& text) const;
};

/**
 * @brief Documentation checker
 */
class DocChecker {
public:
    DocChecker(DocStyle style = DocStyle::DOXYGEN) : parser_(style) {}

    void setRequirements(const DocRequirements& reqs) { requirements_ = reqs; }
    void setStyle(DocStyle style) { parser_ = DocParser(style); }

    bool checkFunctionDoc(const std::string& comment,
                         const std::vector<std::string>& param_names,
                         bool has_return_value,
                         std::vector<std::string>& violations) const;

    bool checkTypeDoc(const std::string& comment, std::vector<std::string>& violations) const;

private:
    DocParser parser_;
    DocRequirements requirements_;
};

// ==================== CODE ORGANIZATION ====================

/**
 * @brief Include directive
 */
struct Include {
    std::string path;
    bool is_system;         // <header> vs "header"
    bool is_angle_bracket;  // Same as is_system
    size_t line_number;
    std::string module;     // Extracted module name
};

/**
 * @brief Include ordering style
 */
enum class IncludeOrderStyle {
    SYSTEM_FIRST,      // <system> then "project"
    PROJECT_FIRST,     // "project" then <system>
    ALPHABETICAL,      // Strict alphabetical
    GROUPED,           // Groups: system, external, project
    CUSTOM             // Custom order
};

/**
 * @brief Header guard style
 */
enum class HeaderGuardStyle {
    IFNDEF_DEFINE,     // #ifndef FOO_H / #define FOO_H
    PRAGMA_ONCE,       // #pragma once
    BOTH,              // Both allowed
    CUSTOM             // Custom pattern
};

/**
 * @brief Code organization checker
 */
class OrganizationChecker {
public:
    // Include order checking
    void setIncludeOrderStyle(IncludeOrderStyle style) { include_order_style_ = style; }
    void setIncludeGroups(const std::vector<std::string>& groups) { include_groups_ = groups; }
    bool checkIncludeOrder(const std::vector<Include>& includes, std::vector<std::string>& violations) const;

    // Header guard checking
    void setHeaderGuardStyle(HeaderGuardStyle style) { header_guard_style_ = style; }
    void setHeaderGuardPattern(const std::string& pattern) { header_guard_pattern_ = pattern; }
    bool checkHeaderGuard(const std::string& filename,
                         const std::string& guard_name,
                         std::string& violation) const;

    // Declaration order checking
    void setDeclarationOrder(const std::vector<std::string>& order) { declaration_order_ = order; }
    bool checkDeclarationOrder(const std::vector<std::string>& declarations,
                              std::vector<std::string>& violations) const;

    // Static function placement
    void setRequireStaticFirst(bool required) { require_static_first_ = required; }
    bool checkStaticPlacement(const std::vector<std::pair<std::string, bool>>& functions,
                             std::vector<std::string>& violations) const;

private:
    IncludeOrderStyle include_order_style_ = IncludeOrderStyle::SYSTEM_FIRST;
    std::vector<std::string> include_groups_;

    HeaderGuardStyle header_guard_style_ = HeaderGuardStyle::IFNDEF_DEFINE;
    std::optional<std::string> header_guard_pattern_;

    std::vector<std::string> declaration_order_;
    bool require_static_first_ = true;

    std::string extractModule(const std::string& include_path) const;
    bool isSystemInclude(const Include& inc) const;
    bool compareIncludes(const Include& a, const Include& b) const;
};

// ==================== UTILITY FUNCTIONS ====================

namespace util {

/**
 * @brief Check if string matches case style
 */
bool matchesCaseStyle(const std::string& str, CaseStyle style);

/**
 * @brief Convert string to specified case style
 */
std::string convertToCase(const std::string& str, CaseStyle target);

/**
 * @brief Detect case style of string
 */
CaseStyle detectCaseStyle(const std::string& str);

/**
 * @brief Check if identifier is valid C identifier
 */
bool isValidIdentifier(const std::string& str);

/**
 * @brief Extract header guard name from filename
 */
std::string generateHeaderGuard(const std::string& filename, const std::string& pattern);

} // namespace util

} // namespace nomic::standards

#endif // NOMIC_STANDARDS_CODING_STANDARDS_H
