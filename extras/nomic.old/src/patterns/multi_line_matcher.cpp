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

#include "nomic/patterns/pattern_matcher.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <sstream>
#include <set>
#include <iterator>

namespace nomic::patterns {

MultiLinePatternMatcher::MultiLinePatternMatcher() {
    spdlog::debug("MultiLinePatternMatcher initialized");
}

MultiLinePatternMatcher::~MultiLinePatternMatcher() = default;

std::vector<MatchResult> MultiLinePatternMatcher::findMatches(
    const MultiLinePattern& pattern,
    const std::vector<Statement>& statements) {

    std::vector<MatchResult> results;

    if (!validatePattern(pattern)) {
        spdlog::error("Invalid pattern");
        return results;
    }

    // Compile the pattern if not already compiled
    if (!compiled_pattern_.is_compiled) {
        compilePattern(pattern);
    }

    // Iterate through statements looking for matches
    for (size_t i = 0; i < statements.size(); ++i) {
        MatchResult match;
        std::map<std::string, std::string> captures;

        // Try to match starting from this position
        bool all_lines_matched = true;
        size_t current_stmt = i;
        size_t pattern_line = 0;

        while (pattern_line < pattern.lines.size() && current_stmt < statements.size()) {
            const auto& pattern_text = pattern.lines[pattern_line];
            const auto& stmt_text = statements[current_stmt].getText();
            (void)pattern_text; // Reserved for future logging/debugging

            if (matchLine(stmt_text, compiled_pattern_.line_regexes[pattern_line], captures)) {
                match.matched_lines.push_back(statements[current_stmt].getLocation());
                pattern_line++;
                current_stmt++;

                // Handle interleaving
                if (pattern.allow_interleaving && pattern_line < pattern.lines.size()) {
                    // Look ahead for next pattern match
                    while (current_stmt < statements.size()) {
                        if (matchLine(statements[current_stmt].getText(),
                                    compiled_pattern_.line_regexes[pattern_line],
                                    captures)) {
                            break;
                        }
                        current_stmt++;
                    }
                }
            } else {
                all_lines_matched = false;
                break;
            }
        }

        if (all_lines_matched && pattern_line == pattern.lines.size()) {
            // Complete match found
            match.captured_values = captures;

            // Set matched range
            if (!match.matched_lines.empty()) {
                match.matched_range = SourceRange(
                    match.matched_lines.front().getStart(),
                    match.matched_lines.back().getEnd()
                );
            }

            // Set similarity score
            match.similarity_score = 1.0f;  // Exact match
            results.push_back(match);
        }
    }

    return results;
}

std::vector<MatchResult> MultiLinePatternMatcher::findMatchesInFunction(
    const MultiLinePattern& pattern,
    const Function& func) {

    // Get statements from function
    const auto& statements = func.getStatements();
    return findMatches(pattern, statements);
}

std::vector<MatchResult> MultiLinePatternMatcher::findSemanticMatches(
    const MultiLinePattern& pattern,
    const Function& func,
    const SemanticConstraints& constraints) {

    // First find text matches
    auto matches = findMatchesInFunction(pattern, func);

    // Filter by semantic constraints
    std::vector<MatchResult> filtered;
    for (const auto& match : matches) {
        if (checkSemanticConstraints(func.getStatements(), constraints, func)) {
            filtered.push_back(match);
        }
    }

    return filtered;
}

bool MultiLinePatternMatcher::validatePattern(const MultiLinePattern& pattern) const {
    if (pattern.lines.empty()) {
        spdlog::error("Pattern has no lines");
        return false;
    }

    // Validate placeholders
    for (const auto& [placeholder, regex_str] : pattern.placeholders) {
        if (placeholder.empty() || regex_str.empty()) {
            spdlog::error("Invalid placeholder: {}", placeholder);
            return false;
        }

        // Try to compile regex
        try {
            std::regex test_regex(regex_str);
        } catch (const std::regex_error& e) {
            spdlog::error("Invalid regex for placeholder {}: {}", placeholder, e.what());
            return false;
        }
    }

    return true;
}

void MultiLinePatternMatcher::compilePattern(const MultiLinePattern& pattern) {
    compiled_pattern_.line_regexes.clear();
    compiled_pattern_.placeholder_indices.clear();

    size_t placeholder_index = 0;

    for (const auto& line : pattern.lines) {
        std::string regex_pattern = line;

        // Escape special regex characters except for placeholders
        std::string escaped;
        for (char c : regex_pattern) {
            if (std::string("^$.*+?()[]{}|\\").find(c) != std::string::npos) {
                escaped += '\\';
            }
            escaped += c;
        }
        regex_pattern = escaped;

        // Replace placeholders with their regex patterns
        for (const auto& [placeholder, placeholder_regex] : pattern.placeholders) {
            size_t pos = 0;
            while ((pos = regex_pattern.find(placeholder, pos)) != std::string::npos) {
                // Create a capturing group
                std::string capture_group = "(" + placeholder_regex + ")";
                regex_pattern.replace(pos, placeholder.length(), capture_group);

                // Track placeholder index
                compiled_pattern_.placeholder_indices[placeholder] = placeholder_index++;

                pos += capture_group.length();
            }
        }

        // Handle whitespace flexibility
        if (!pattern.preserve_whitespace) {
            // Replace whitespace sequences with \s+
            std::string flexible;
            bool in_whitespace = false;
            for (char c : regex_pattern) {
                if (std::isspace(c)) {
                    if (!in_whitespace) {
                        flexible += "\\s+";
                        in_whitespace = true;
                    }
                } else {
                    flexible += c;
                    in_whitespace = false;
                }
            }
            regex_pattern = flexible;
        }

        // Compile regex with case sensitivity option
        auto flags = std::regex::ECMAScript | std::regex::optimize;
        if (!pattern.case_sensitive) {
            flags |= std::regex::icase;
        }

        compiled_pattern_.line_regexes.emplace_back(regex_pattern, flags);
    }

    compiled_pattern_.is_compiled = true;
}

bool MultiLinePatternMatcher::matchLine(
    const std::string& line,
    const std::regex& pattern_regex,
    std::map<std::string, std::string>& captures) const {

    std::smatch match;
    if (std::regex_search(line, match, pattern_regex)) {
        // Extract captures
        for (size_t i = 1; i < match.size(); ++i) {
            // Find placeholder name for this capture index
            for (const auto& [placeholder, index] : compiled_pattern_.placeholder_indices) {
                if (index == i - 1) {
                    captures[placeholder] = match[i].str();
                    break;
                }
            }
        }
        return true;
    }
    return false;
}

bool MultiLinePatternMatcher::checkSemanticConstraints(
    const std::vector<Statement>& matched_statements,
    const SemanticConstraints& constraints,
    const Function& func) const {

    // Check basic block constraint
    if (constraints.must_be_in_same_basic_block) {
        // Check if all statements are in the same basic block
        // This would require CFG analysis
        // For now, return true as placeholder
    }

    // Check loop constraint
    if (constraints.must_be_in_loop) {
        // Check if statements are within a loop construct
        // This would require AST analysis
    }

    // Check execution order
    if (constraints.preserve_execution_order) {
        // Verify that statements execute in the order they appear
        // This would require CFG path analysis
    }

    // Check statement distance
    if (constraints.max_statements_between < UINT_MAX) {
        if (matched_statements.size() > 1) {
            // Check distance between first and last statement
            // This would require counting intermediate statements
        }
    }

    // Additional semantic checks would go here

    return true;  // Placeholder: accept all for now
}

float MultiLinePatternMatcher::calculateSimilarity(
    const std::string& text1,
    const std::string& text2) const {

    // Simple Levenshtein distance-based similarity
    size_t len1 = text1.length();
    size_t len2 = text2.length();

    if (len1 == 0) return len2 == 0 ? 1.0f : 0.0f;
    if (len2 == 0) return 0.0f;

    std::vector<std::vector<size_t>> dp(len1 + 1, std::vector<size_t>(len2 + 1));

    for (size_t i = 0; i <= len1; ++i) dp[i][0] = i;
    for (size_t j = 0; j <= len2; ++j) dp[0][j] = j;

    for (size_t i = 1; i <= len1; ++i) {
        for (size_t j = 1; j <= len2; ++j) {
            size_t cost = text1[i - 1] == text2[j - 1] ? 0 : 1;
            dp[i][j] = std::min({
                dp[i - 1][j] + 1,      // deletion
                dp[i][j - 1] + 1,      // insertion
                dp[i - 1][j - 1] + cost // substitution
            });
        }
    }

    size_t distance = dp[len1][len2];
    size_t max_len = std::max(len1, len2);

    return 1.0f - (static_cast<float>(distance) / static_cast<float>(max_len));
}

// ==================== ADVANCED MATCHING MODES ====================

std::vector<MatchResult> MultiLinePatternMatcher::findFuzzyMatches(
    const MultiLinePattern& pattern,
    const std::vector<Statement>& statements,
    float min_similarity) {

    std::vector<MatchResult> results;

    for (size_t i = 0; i < statements.size(); ++i) {
        MatchResult match;
        std::map<std::string, std::string> captures;
        bool all_lines_fuzzy_matched = true;
        size_t current_stmt = i;

        for (size_t pattern_line = 0; pattern_line < pattern.lines.size() && current_stmt < statements.size(); ++pattern_line) {
            const auto& pattern_text = pattern.lines[pattern_line];
            const auto& stmt_text = statements[current_stmt].getText();

            if (matchLineFuzzy(stmt_text, pattern_text, min_similarity)) {
                float sim = calculateSimilarity(stmt_text, pattern_text);
                match.matched_lines.push_back(statements[current_stmt].getLocation());
                match.similarity_score = std::min(match.similarity_score, sim);
                current_stmt++;
            } else {
                all_lines_fuzzy_matched = false;
                break;
            }
        }

        if (all_lines_fuzzy_matched && !match.matched_lines.empty()) {
            match.captured_values = captures;
            if (!match.matched_lines.empty()) {
                match.matched_range = SourceRange(
                    match.matched_lines.front().getStart(),
                    match.matched_lines.back().getEnd()
                );
            }
            results.push_back(match);
        }
    }

    return results;
}

std::vector<MatchResult> MultiLinePatternMatcher::findWildcardMatches(
    const MultiLinePattern& pattern,
    const std::vector<Statement>& statements) {

    std::vector<MatchResult> results;

    for (size_t i = 0; i < statements.size(); ++i) {
        MatchResult match;
        std::map<std::string, std::string> captures;
        bool all_matched = true;
        size_t current_stmt = i;

        for (size_t pattern_line = 0; pattern_line < pattern.lines.size() && current_stmt < statements.size(); ++pattern_line) {
            const auto& pattern_text = pattern.lines[pattern_line];
            const auto& stmt_text = statements[current_stmt].getText();

            if (matchLineWildcard(stmt_text, pattern_text, captures)) {
                match.matched_lines.push_back(statements[current_stmt].getLocation());
                current_stmt++;
            } else {
                all_matched = false;
                break;
            }
        }

        if (all_matched && !match.matched_lines.empty()) {
            match.captured_values = captures;
            match.matched_range = SourceRange(
                match.matched_lines.front().getStart(),
                match.matched_lines.back().getEnd()
            );
            results.push_back(match);
        }
    }

    return results;
}

std::vector<MatchResult> MultiLinePatternMatcher::findUnorderedMatches(
    const MultiLinePattern& pattern,
    const std::vector<Statement>& statements) {

    std::vector<MatchResult> results;

    // Track which pattern lines have been matched
    std::vector<bool> pattern_matched(pattern.lines.size(), false);

    for (size_t window_start = 0; window_start < statements.size(); ++window_start) {
        std::fill(pattern_matched.begin(), pattern_matched.end(), false);
        std::vector<size_t> matched_stmt_indices;

        // Try to match all pattern lines within a reasonable window
        size_t window_end = std::min(window_start + pattern.lines.size() * 3, statements.size());

        for (size_t stmt_idx = window_start; stmt_idx < window_end; ++stmt_idx) {
            const auto& stmt_text = statements[stmt_idx].getText();

            // Try to match against any unmatched pattern line
            for (size_t pattern_line = 0; pattern_line < pattern.lines.size(); ++pattern_line) {
                if (pattern_matched[pattern_line]) continue;

                if (!compiled_pattern_.is_compiled) {
                    compilePattern(pattern);
                }

                std::map<std::string, std::string> captures;
                if (matchLine(stmt_text, compiled_pattern_.line_regexes[pattern_line], captures)) {
                    pattern_matched[pattern_line] = true;
                    matched_stmt_indices.push_back(stmt_idx);
                    break;
                }
            }
        }

        // Check if all pattern lines matched
        if (std::all_of(pattern_matched.begin(), pattern_matched.end(), [](bool b) { return b; })) {
            MatchResult match;
            for (size_t idx : matched_stmt_indices) {
                match.matched_lines.push_back(statements[idx].getLocation());
            }
            if (!match.matched_lines.empty()) {
                match.matched_range = SourceRange(
                    match.matched_lines.front().getStart(),
                    match.matched_lines.back().getEnd()
                );
                match.similarity_score = 1.0f;
                results.push_back(match);
            }
        }
    }

    return results;
}

std::vector<MatchResult> MultiLinePatternMatcher::findGapTolerantMatches(
    const MultiLinePattern& pattern,
    const std::vector<Statement>& statements,
    unsigned max_gap) {

    std::vector<MatchResult> results;

    for (size_t i = 0; i < statements.size(); ++i) {
        MatchResult match;
        std::map<std::string, std::string> captures;
        bool all_matched = true;
        size_t current_stmt = i;
        size_t pattern_line = 0;
        size_t total_gaps = 0;

        while (pattern_line < pattern.lines.size() && current_stmt < statements.size()) {
            const auto& pattern_text = pattern.lines[pattern_line];
            bool found_match = false;

            // Search forward within gap tolerance
            for (size_t gap = 0; gap <= max_gap && current_stmt + gap < statements.size(); ++gap) {
                const auto& stmt_text = statements[current_stmt + gap].getText();

                if (!compiled_pattern_.is_compiled) {
                    compilePattern(pattern);
                }

                if (matchLine(stmt_text, compiled_pattern_.line_regexes[pattern_line], captures)) {
                    match.matched_lines.push_back(statements[current_stmt + gap].getLocation());
                    total_gaps += gap;
                    current_stmt += gap + 1;
                    pattern_line++;
                    found_match = true;
                    break;
                }
            }

            if (!found_match) {
                all_matched = false;
                break;
            }
        }

        if (all_matched && pattern_line == pattern.lines.size()) {
            match.captured_values = captures;
            match.total_gap_lines = total_gaps;
            if (!match.matched_lines.empty()) {
                match.matched_range = SourceRange(
                    match.matched_lines.front().getStart(),
                    match.matched_lines.back().getEnd()
                );
            }
            results.push_back(match);
        }
    }

    return results;
}

// ==================== HELPER IMPLEMENTATIONS ====================

bool MultiLinePatternMatcher::matchLineWildcard(
    const std::string& line,
    const std::string& wildcard_pattern,
    std::map<std::string, std::string>& captures) const {

    return wildcardMatch(line.c_str(), wildcard_pattern.c_str());
}

bool MultiLinePatternMatcher::matchLineFuzzy(
    const std::string& line,
    const std::string& pattern,
    float min_similarity) const {

    float sim = calculateSimilarity(line, pattern);
    return sim >= min_similarity;
}

bool MultiLinePatternMatcher::matchLineStructural(
    const std::string& line,
    const std::string& pattern) const {

    std::string norm_line = normalizeForStructural(line);
    std::string norm_pattern = normalizeForStructural(pattern);
    return norm_line.find(norm_pattern) != std::string::npos;
}

bool MultiLinePatternMatcher::wildcardMatch(const char* text, const char* pattern) const {
    // Classic wildcard matching algorithm
    if (*pattern == '\0') return *text == '\0';
    if (*pattern == '*') {
        while (*text) {
            if (wildcardMatch(text, pattern + 1)) return true;
            text++;
        }
        return wildcardMatch(text, pattern + 1);
    }
    if (*text && (*pattern == '?' || *pattern == *text)) {
        return wildcardMatch(text + 1, pattern + 1);
    }
    return false;
}

std::string MultiLinePatternMatcher::normalizeForStructural(const std::string& code) const {
    std::string result;
    bool in_string = false;
    bool in_char = false;

    for (size_t i = 0; i < code.length(); ++i) {
        char c = code[i];

        if (c == '"' && (i == 0 || code[i-1] != '\\')) {
            in_string = !in_string;
            result += c;
        } else if (c == '\'' && (i == 0 || code[i-1] != '\\')) {
            in_char = !in_char;
            result += c;
        } else if (in_string || in_char) {
            result += '_';  // Replace string/char contents with underscore
        } else if (std::isalnum(c)) {
            result += std::isdigit(c) ? '0' : c;  // Normalize numbers
        } else if (std::isspace(c)) {
            if (!result.empty() && result.back() != ' ') {
                result += ' ';  // Normalize whitespace
            }
        } else {
            result += c;
        }
    }

    return result;
}

std::string MultiLinePatternMatcher::normalizeForSemantic(const std::string& code) const {
    // Remove variable names, keep structure
    std::string result = normalizeForStructural(code);

    // Additional semantic normalization could go here
    // e.g., treating "i++" and "i = i + 1" as equivalent

    return result;
}

std::vector<std::string> MultiLinePatternMatcher::tokenize(const std::string& code) const {
    std::vector<std::string> tokens;
    std::string current;

    for (char c : code) {
        if (std::isalnum(c) || c == '_') {
            current += c;
        } else {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            if (!std::isspace(c)) {
                tokens.push_back(std::string(1, c));
            }
        }
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

float MultiLinePatternMatcher::calculateTokenSimilarity(
    const std::string& text1,
    const std::string& text2) const {

    auto tokens1 = tokenize(text1);
    auto tokens2 = tokenize(text2);

    if (tokens1.empty() && tokens2.empty()) return 1.0f;
    if (tokens1.empty() || tokens2.empty()) return 0.0f;

    // Jaccard similarity on token sets
    std::set<std::string> set1(tokens1.begin(), tokens1.end());
    std::set<std::string> set2(tokens2.begin(), tokens2.end());

    std::vector<std::string> intersection;
    std::set_intersection(set1.begin(), set1.end(),
                         set2.begin(), set2.end(),
                         std::back_inserter(intersection));

    std::vector<std::string> union_set;
    std::set_union(set1.begin(), set1.end(),
                  set2.begin(), set2.end(),
                  std::back_inserter(union_set));

    return static_cast<float>(intersection.size()) / static_cast<float>(union_set.size());
}

// ==================== PATTERN BUILDER HELPERS ====================

MultiLinePattern MultiLinePatternMatcher::fromLines(const std::vector<std::string>& lines) {
    MultiLinePattern pattern;
    pattern.lines = lines;
    return pattern;
}

MultiLinePattern MultiLinePatternMatcher::fromRegex(const std::string& regex_pattern) {
    MultiLinePattern pattern;
    pattern.lines = {regex_pattern};
    pattern.match_mode = MultiLinePattern::MatchMode::REGEX_FULL;
    return pattern;
}

MultiLinePattern MultiLinePatternMatcher::fromWildcard(const std::string& wildcard_pattern) {
    MultiLinePattern pattern;
    pattern.lines = {wildcard_pattern};
    pattern.enable_wildcards = true;
    pattern.match_mode = MultiLinePattern::MatchMode::WILDCARD;
    return pattern;
}

// ==================== PATTERN STATISTICS ====================

MultiLinePatternMatcher::PatternStats MultiLinePatternMatcher::analyzePattern(
    const MultiLinePattern& pattern) const {

    PatternStats stats;
    stats.num_lines = pattern.lines.size();
    stats.num_placeholders = pattern.placeholders.size();
    stats.num_quantifiers = pattern.line_quantifiers.size();
    stats.num_alternatives = 0;

    for (const auto& group : pattern.line_groups) {
        if (group.is_alternative) {
            stats.num_alternatives++;
        }
    }

    // Estimate complexity
    stats.estimated_complexity =
        static_cast<float>(stats.num_lines) * 1.0f +
        static_cast<float>(stats.num_placeholders) * 0.5f +
        static_cast<float>(stats.num_quantifiers) * 2.0f +
        static_cast<float>(stats.num_alternatives) * 1.5f;

    return stats;
}

std::vector<MatchResult> MultiLinePatternMatcher::findStructuralMatches(
    const MultiLinePattern& pattern,
    const std::vector<Statement>& statements) {

    // Structural matching ignores variable names and literals
    std::vector<MatchResult> results;

    if (pattern.lines.empty() || statements.empty()) {
        return results;
    }

    // For each starting position
    for (size_t i = 0; i <= statements.size() - pattern.lines.size(); ++i) {
        bool all_match = true;
        MatchResult match;
        match.matched_range = statements[i].getLocation();

        // Try to match all pattern lines starting from position i
        for (size_t j = 0; j < pattern.lines.size(); ++j) {
            const std::string& pattern_line = pattern.lines[j];
            const std::string& stmt_text = statements[i + j].getText();

            // Normalize both for structural comparison
            std::string norm_pattern = normalizeForStructural(pattern_line);
            std::string norm_stmt = normalizeForStructural(stmt_text);

            if (norm_pattern != norm_stmt) {
                all_match = false;
                break;
            }

            match.matched_lines.push_back(statements[i + j].getLocation());
        }

        if (all_match) {
            match.structural_similarity = 1.0f;
            results.push_back(match);
        }
    }

    return results;
}

std::vector<std::string> MultiLinePatternMatcher::extractPatternInstances(
    const MultiLinePattern& pattern,
    const std::vector<Statement>& statements) {

    std::vector<std::string> instances;
    auto matches = findMatches(pattern, statements);

    for (const auto& match : matches) {
        std::string instance;
        for (const auto& line_range : match.matched_lines) {
            // Find the statement with this location
            for (const auto& stmt : statements) {
                if (stmt.getLocation().getStart() == line_range.getStart()) {
                    if (!instance.empty()) {
                        instance += "\n";
                    }
                    instance += stmt.getText();
                    break;
                }
            }
        }
        if (!instance.empty()) {
            instances.push_back(instance);
        }
    }

    return instances;
}

} // namespace nomic::patterns