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

#include "nomic/standards/coding_standards.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace nomic::standards {

// ==================== NAMING CONVENTIONS ====================

bool NamingConvention::validate(const std::string& name, std::string& error_message) const {
    // Check length
    if (min_length_.has_value() && name.length() < min_length_.value()) {
        error_message = "Name too short (min " + std::to_string(min_length_.value()) + " characters)";
        return false;
    }

    if (max_length_.has_value() && name.length() > max_length_.value()) {
        error_message = "Name too long (max " + std::to_string(max_length_.value()) + " characters)";
        return false;
    }

    // Check forbidden names
    if (std::find(forbidden_names_.begin(), forbidden_names_.end(), name) != forbidden_names_.end()) {
        error_message = "Name '" + name + "' is forbidden";
        return false;
    }

    // Check prefix
    if (prefix_.has_value() && !name.starts_with(prefix_.value())) {
        error_message = "Name must start with prefix '" + prefix_.value() + "'";
        return false;
    }

    // Check suffix
    if (suffix_.has_value() && !name.ends_with(suffix_.value())) {
        error_message = "Name must end with suffix '" + suffix_.value() + "'";
        return false;
    }

    // Check regex pattern
    if (regex_pattern_.has_value()) {
        try {
            std::regex pattern(regex_pattern_.value());
            if (!std::regex_match(name, pattern)) {
                error_message = "Name does not match pattern '" + regex_pattern_.value() + "'";
                return false;
            }
        } catch (const std::regex_error&) {
            error_message = "Invalid regex pattern";
            return false;
        }
    }

    // Check case style
    if (!matchesCaseStyle(name)) {
        error_message = "Name does not match required case style";
        return false;
    }

    return true;
}

bool NamingConvention::matchesCaseStyle(const std::string& name) const {
    return util::matchesCaseStyle(name, case_style_);
}

// ==================== NAMING CHECKER ====================

bool NamingChecker::checkFunctionName(const std::string& name, std::string& error) const {
    return function_conv_.validate(name, error);
}

bool NamingChecker::checkVariableName(const std::string& name, bool is_const,
                                     bool is_global, bool is_static, std::string& error) const {
    if (is_const) {
        return constant_conv_.validate(name, error);
    } else if (is_global) {
        return global_var_conv_.validate(name, error);
    } else if (is_static) {
        return static_var_conv_.validate(name, error);
    } else {
        return variable_conv_.validate(name, error);
    }
}

bool NamingChecker::checkParameterName(const std::string& name, std::string& error) const {
    return parameter_conv_.validate(name, error);
}

bool NamingChecker::checkTypeName(const std::string& name, std::string& error) const {
    return type_conv_.validate(name, error);
}

bool NamingChecker::checkMacroName(const std::string& name, std::string& error) const {
    return macro_conv_.validate(name, error);
}

bool NamingChecker::checkEnumConstantName(const std::string& name, std::string& error) const {
    return enum_const_conv_.validate(name, error);
}

bool NamingChecker::checkStructName(const std::string& name, std::string& error) const {
    return struct_conv_.validate(name, error);
}

// ==================== DOCUMENTATION ====================

void DocComment::addParam(const std::string& name, const std::string& desc) {
    param_docs_[name] = desc;
}

void DocComment::addTag(const std::string& tag, const std::string& value) {
    tags_[tag] = value;
}

bool DocComment::hasParamDoc(const std::string& param) const {
    return param_docs_.find(param) != param_docs_.end();
}

bool DocComment::hasTag(const std::string& tag) const {
    return tags_.find(tag) != tags_.end();
}

std::optional<DocComment> DocParser::parse(const std::string& comment_text) const {
    if (!isDocComment(comment_text)) {
        return std::nullopt;
    }

    switch (style_) {
        case DocStyle::DOXYGEN:
        case DocStyle::JAVADOC:
            return parseDoxygenComment(comment_text);
        case DocStyle::KERNEL_DOC:
            return parseKernelDocComment(comment_text);
        default:
            return parseDoxygenComment(comment_text);
    }
}

bool DocParser::isDocComment(const std::string& comment_text) const {
    // Doxygen: starts with /** or ///
    if (comment_text.starts_with("/**") || comment_text.starts_with("///")) {
        return true;
    }

    // Kernel-doc: starts with /**
    if (style_ == DocStyle::KERNEL_DOC && comment_text.starts_with("/**")) {
        return true;
    }

    return false;
}

DocComment DocParser::parseDoxygenComment(const std::string& text) const {
    DocComment doc;

    std::istringstream stream(text);
    std::string line;
    std::string current_tag;
    std::string current_content;

    while (std::getline(stream, line)) {
        // Remove comment markers
        size_t start = line.find_first_not_of("/* \t");
        if (start == std::string::npos) continue;

        size_t end = line.find_last_not_of("*/ \t");
        if (end == std::string::npos) continue;

        line = line.substr(start, end - start + 1);

        // Check for tags
        if (line.starts_with("@") || line.starts_with("\\")) {
            // Save previous tag
            if (!current_tag.empty()) {
                if (current_tag == "brief") {
                    doc.setBrief(current_content);
                } else if (current_tag.starts_with("param ")) {
                    size_t space = current_tag.find(' ', 6);
                    if (space != std::string::npos) {
                        std::string param_name = current_tag.substr(6, space - 6);
                        doc.addParam(param_name, current_content);
                    }
                } else if (current_tag == "return" || current_tag == "returns") {
                    doc.setReturn(current_content);
                } else {
                    doc.addTag(current_tag, current_content);
                }
                current_content.clear();
            }

            // Parse new tag
            size_t tag_end = line.find(' ', 1);
            if (tag_end == std::string::npos) {
                current_tag = line.substr(1);
            } else {
                current_tag = line.substr(1, tag_end - 1);
                current_content = line.substr(tag_end + 1);
            }
        } else {
            // Continue current tag or brief
            if (current_tag.empty() && !line.empty()) {
                if (doc.hasBrief()) {
                    current_content += " " + line;
                } else {
                    doc.setBrief(line);
                }
            } else {
                if (!current_content.empty()) current_content += " ";
                current_content += line;
            }
        }
    }

    // Save last tag
    if (!current_tag.empty()) {
        if (current_tag == "brief") {
            doc.setBrief(current_content);
        } else if (current_tag.starts_with("param ")) {
            size_t space = current_tag.find(' ', 6);
            if (space != std::string::npos) {
                std::string param_name = current_tag.substr(6, space - 6);
                doc.addParam(param_name, current_content);
            }
        } else if (current_tag == "return" || current_tag == "returns") {
            doc.setReturn(current_content);
        } else {
            doc.addTag(current_tag, current_content);
        }
    }

    return doc;
}

DocComment DocParser::parseKernelDocComment(const std::string& text) const {
    // Simplified kernel-doc parser
    // Format:
    // /**
    //  * function_name - Brief description
    //  * @param1: Description
    //  * @param2: Description
    //  *
    //  * Detailed description
    //  */

    DocComment doc;
    // Basic implementation - can be enhanced
    return parseDoxygenComment(text);
}

bool DocChecker::checkFunctionDoc(const std::string& comment,
                                 const std::vector<std::string>& param_names,
                                 bool has_return_value,
                                 std::vector<std::string>& violations) const {
    auto doc = parser_.parse(comment);

    if (!doc.has_value()) {
        if (requirements_.require_brief) {
            violations.push_back("Missing documentation comment");
        }
        return false;
    }

    bool valid = true;

    // Check brief
    if (requirements_.require_brief && !doc->hasBrief()) {
        violations.push_back("Missing @brief description");
        valid = false;
    }

    // Check parameters
    if (requirements_.require_param_docs) {
        for (const auto& param : param_names) {
            if (!doc->hasParamDoc(param)) {
                violations.push_back("Missing @param documentation for '" + param + "'");
                valid = false;
            }
        }
    }

    // Check return
    if (requirements_.require_return_doc && has_return_value && !doc->hasReturnDoc()) {
        violations.push_back("Missing @return documentation");
        valid = false;
    }

    // Check custom tags
    for (const auto& tag : requirements_.custom_tags) {
        if (!doc->hasTag(tag)) {
            violations.push_back("Missing @" + tag + " tag");
            valid = false;
        }
    }

    return valid;
}

bool DocChecker::checkTypeDoc(const std::string& comment, std::vector<std::string>& violations) const {
    auto doc = parser_.parse(comment);

    if (!doc.has_value()) {
        if (requirements_.require_brief) {
            violations.push_back("Missing documentation comment");
        }
        return false;
    }

    bool valid = true;

    if (requirements_.require_brief && !doc->hasBrief()) {
        violations.push_back("Missing @brief description");
        valid = false;
    }

    return valid;
}

// ==================== CODE ORGANIZATION ====================

bool OrganizationChecker::checkIncludeOrder(const std::vector<Include>& includes,
                                           std::vector<std::string>& violations) const {
    if (includes.size() < 2) return true;

    bool valid = true;

    switch (include_order_style_) {
        case IncludeOrderStyle::SYSTEM_FIRST: {
            bool seen_project = false;
            for (const auto& inc : includes) {
                if (!inc.is_system) {
                    seen_project = true;
                } else if (seen_project) {
                    violations.push_back("System include <" + inc.path + "> after project include at line " +
                                       std::to_string(inc.line_number));
                    valid = false;
                }
            }
            break;
        }

        case IncludeOrderStyle::PROJECT_FIRST: {
            bool seen_system = false;
            for (const auto& inc : includes) {
                if (inc.is_system) {
                    seen_system = true;
                } else if (seen_system) {
                    violations.push_back("Project include \"" + inc.path + "\" after system include at line " +
                                       std::to_string(inc.line_number));
                    valid = false;
                }
            }
            break;
        }

        case IncludeOrderStyle::ALPHABETICAL: {
            for (size_t i = 1; i < includes.size(); i++) {
                if (includes[i].path < includes[i-1].path) {
                    violations.push_back("Include not in alphabetical order: " + includes[i].path +
                                       " at line " + std::to_string(includes[i].line_number));
                    valid = false;
                }
            }
            break;
        }

        default:
            break;
    }

    return valid;
}

bool OrganizationChecker::checkHeaderGuard(const std::string& filename,
                                          const std::string& guard_name,
                                          std::string& violation) const {
    if (header_guard_style_ == HeaderGuardStyle::PRAGMA_ONCE) {
        // pragma once doesn't need checking
        return true;
    }

    if (header_guard_pattern_.has_value()) {
        std::string expected = util::generateHeaderGuard(filename, header_guard_pattern_.value());
        if (guard_name != expected) {
            violation = "Header guard '" + guard_name + "' does not match expected pattern '" + expected + "'";
            return false;
        }
    }

    return true;
}

bool OrganizationChecker::checkDeclarationOrder(const std::vector<std::string>& declarations,
                                               std::vector<std::string>& violations) const {
    // Check if declarations follow specified order
    // declaration_order_ might be: ["typedef", "struct", "enum", "function", "variable"]

    std::map<std::string, int> order_map;
    for (size_t i = 0; i < declaration_order_.size(); i++) {
        order_map[declaration_order_[i]] = i;
    }

    int last_order = -1;
    for (const auto& decl : declarations) {
        auto it = order_map.find(decl);
        if (it != order_map.end()) {
            if (it->second < last_order) {
                violations.push_back("Declaration '" + decl + "' appears out of order");
            }
            last_order = it->second;
        }
    }

    return violations.empty();
}

bool OrganizationChecker::checkStaticPlacement(const std::vector<std::pair<std::string, bool>>& functions,
                                              std::vector<std::string>& violations) const {
    if (!require_static_first_) return true;

    bool seen_non_static = false;
    for (const auto& [name, is_static] : functions) {
        if (!is_static) {
            seen_non_static = true;
        } else if (seen_non_static) {
            violations.push_back("Static function '" + name + "' appears after non-static functions");
        }
    }

    return violations.empty();
}

// ==================== UTILITY FUNCTIONS ====================

namespace util {

bool matchesCaseStyle(const std::string& str, CaseStyle style) {
    if (str.empty()) return false;

    switch (style) {
        case CaseStyle::SNAKE_CASE: {
            // lowercase with underscores
            for (char c : str) {
                if (!std::islower(c) && !std::isdigit(c) && c != '_') {
                    return false;
                }
            }
            return std::islower(str[0]);
        }

        case CaseStyle::UPPER_CASE: {
            // UPPERCASE with underscores
            for (char c : str) {
                if (!std::isupper(c) && !std::isdigit(c) && c != '_') {
                    return false;
                }
            }
            return std::isupper(str[0]);
        }

        case CaseStyle::CAMEL_CASE: {
            // camelCase - starts lowercase, has uppercase letters
            if (!std::islower(str[0])) return false;
            bool has_upper = false;
            for (char c : str) {
                if (std::isupper(c)) has_upper = true;
                if (!std::isalnum(c)) return false;
            }
            return has_upper;
        }

        case CaseStyle::PASCAL_CASE: {
            // PascalCase - starts uppercase
            if (!std::isupper(str[0])) return false;
            for (char c : str) {
                if (!std::isalnum(c)) return false;
            }
            return true;
        }

        case CaseStyle::ANY:
            return true;

        default:
            return false;
    }
}

CaseStyle detectCaseStyle(const std::string& str) {
    if (matchesCaseStyle(str, CaseStyle::SNAKE_CASE)) return CaseStyle::SNAKE_CASE;
    if (matchesCaseStyle(str, CaseStyle::UPPER_CASE)) return CaseStyle::UPPER_CASE;
    if (matchesCaseStyle(str, CaseStyle::PASCAL_CASE)) return CaseStyle::PASCAL_CASE;
    if (matchesCaseStyle(str, CaseStyle::CAMEL_CASE)) return CaseStyle::CAMEL_CASE;
    return CaseStyle::MIXED_CASE;
}

bool isValidIdentifier(const std::string& str) {
    if (str.empty()) return false;
    if (std::isdigit(str[0])) return false;

    for (char c : str) {
        if (!std::isalnum(c) && c != '_') {
            return false;
        }
    }

    return true;
}

std::string generateHeaderGuard(const std::string& filename, const std::string& pattern) {
    std::string guard = pattern;

    // Replace {FILE} with uppercase filename
    std::string upper_file = filename;
    std::transform(upper_file.begin(), upper_file.end(), upper_file.begin(), ::toupper);
    std::replace(upper_file.begin(), upper_file.end(), '.', '_');
    std::replace(upper_file.begin(), upper_file.end(), '-', '_');

    size_t pos = 0;
    while ((pos = guard.find("{FILE}", pos)) != std::string::npos) {
        guard.replace(pos, 6, upper_file);
        pos += upper_file.length();
    }

    return guard;
}

} // namespace util

} // namespace nomic::standards
