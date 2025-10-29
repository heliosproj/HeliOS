/*
 * Nomic C Semantic Source Code Analyzer
 * (C) 2020-2026 Manny Peterson <manny@heliosproject.org>
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

#ifndef NOMIC_PATTERNS_PATTERN_MATCHER_H
#define NOMIC_PATTERNS_PATTERN_MATCHER_H

#include "nomic/core/semantic_model.h"
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <optional>

// Forward declarations for LLVM types
namespace llvm {
    class Instruction;
    class Function;
    class Value;
    class Type;
    class APInt;
}

// Forward declarations for Clang types
namespace clang {
    class Stmt;
    class ASTContext;
}

namespace nomic::patterns {

/**
 * @brief Multi-line pattern specification with advanced features
 */
struct MultiLinePattern {
    std::vector<std::string> lines;
    std::map<std::string, std::string> placeholders;

    // Flexibility options
    bool allow_interleaving = false;
    bool preserve_whitespace = false;
    bool case_sensitive = true;
    bool allow_partial_match = false;
    bool fuzzy_matching = false;
    float min_similarity_threshold = 0.8f;

    // Advanced matching modes
    enum class MatchMode {
        EXACT,          // Exact line-by-line match
        FUZZY,          // Allows minor variations
        STRUCTURAL,     // Matches structure, ignores details
        SEMANTIC,       // Semantic equivalence
        WILDCARD,       // Supports * and ? wildcards
        REGEX_FULL      // Full regex mode
    } match_mode = MatchMode::EXACT;

    // Wildcard patterns (using * and ? without full regex)
    bool enable_wildcards = false;

    // Gap constraints
    unsigned min_gap_lines = 0;  // Minimum lines between pattern matches
    unsigned max_gap_lines = 0;  // Maximum lines between pattern matches (0 = no limit)

    // Ordering constraints
    enum class OrderMode {
        STRICT,         // Must appear in exact order
        RELAXED,        // Can appear in any order
        PARTIAL_ORDER   // Some lines must precede others
    } order_mode = OrderMode::STRICT;

    std::map<size_t, std::vector<size_t>> ordering_constraints;  // line_index -> must_precede_indices

    // Context requirements
    bool require_contiguous = false;  // All lines must be consecutive
    bool allow_duplicates = false;    // Can match same line multiple times

    // Negation patterns (must NOT match)
    std::vector<std::string> negative_patterns;

    // Quantifiers for lines
    struct LineQuantifier {
        size_t min_count = 1;
        size_t max_count = 1;
        bool greedy = true;
    };
    std::map<size_t, LineQuantifier> line_quantifiers;  // line_index -> quantifier

    // Grouping and alternation
    struct LineGroup {
        std::vector<size_t> line_indices;
        bool is_alternative = false;  // If true, match any ONE from group
        bool is_optional = false;      // If true, group is optional
    };
    std::vector<LineGroup> line_groups;
};

/**
 * @brief Result of pattern matching with detailed information
 */
struct MatchResult {
    SourceRange matched_range;
    std::map<std::string, std::string> captured_values;
    std::vector<SourceRange> matched_lines;
    float similarity_score = 1.0f;

    // Detailed match information
    std::vector<size_t> matched_statement_indices;
    std::map<std::string, std::vector<std::string>> multi_captures;  // For quantified captures

    // Match quality metrics
    float structural_similarity = 1.0f;
    float semantic_similarity = 1.0f;
    float lexical_similarity = 1.0f;

    // Pattern coverage
    std::vector<bool> pattern_lines_matched;  // Which pattern lines matched
    size_t total_gap_lines = 0;                // Total gap between matches

    // Context information
    std::string matched_context;  // Surrounding code context
    bool is_partial_match = false;
    bool has_negative_match = false;  // True if negative pattern matched
};

/**
 * @brief Semantic constraints for pattern matching
 */
struct SemanticConstraints {
    // Data flow constraints
    bool must_be_in_same_basic_block = false;
    bool must_have_data_dependency = false;
    bool must_be_in_control_dependency = false;

    // Scope constraints
    bool must_be_in_same_scope = false;
    bool must_be_in_loop = false;
    bool must_be_in_conditional = false;

    // Type constraints
    std::optional<TypeInfo> variable_type_filter;
    std::optional<std::string> function_name_filter;

    // Temporal constraints
    bool preserve_execution_order = true;
    unsigned max_statements_between = UINT_MAX;
};

/**
 * @brief Multi-line pattern matcher with semantic awareness and advanced features
 */
class MultiLinePatternMatcher {
public:
    MultiLinePatternMatcher();
    ~MultiLinePatternMatcher();

    // ==================== BASIC PATTERN MATCHING ====================

    // Pattern matching
    std::vector<MatchResult> findMatches(const MultiLinePattern& pattern,
                                         const std::vector<Statement>& statements);

    std::vector<MatchResult> findMatchesInFunction(const MultiLinePattern& pattern,
                                                   const Function& func);

    // Semantic-aware matching
    std::vector<MatchResult> findSemanticMatches(const MultiLinePattern& pattern,
                                                 const Function& func,
                                                 const SemanticConstraints& constraints);

    // ==================== ADVANCED MATCHING MODES ====================

    // Fuzzy matching with configurable threshold
    std::vector<MatchResult> findFuzzyMatches(const MultiLinePattern& pattern,
                                              const std::vector<Statement>& statements,
                                              float min_similarity = 0.7f);

    // Structural matching (ignores variable names, literals)
    std::vector<MatchResult> findStructuralMatches(const MultiLinePattern& pattern,
                                                   const std::vector<Statement>& statements);

    // Wildcard matching (supports * and ?)
    std::vector<MatchResult> findWildcardMatches(const MultiLinePattern& pattern,
                                                 const std::vector<Statement>& statements);

    // Unordered matching (finds pattern lines in any order)
    std::vector<MatchResult> findUnorderedMatches(const MultiLinePattern& pattern,
                                                  const std::vector<Statement>& statements);

    // Gap-tolerant matching (allows gaps between pattern lines)
    std::vector<MatchResult> findGapTolerantMatches(const MultiLinePattern& pattern,
                                                    const std::vector<Statement>& statements,
                                                    unsigned max_gap = 5);

    // ==================== QUANTIFIED MATCHING ====================

    // Match with line quantifiers (e.g., "if (...)*" matches multiple ifs)
    std::vector<MatchResult> findQuantifiedMatches(const MultiLinePattern& pattern,
                                                   const std::vector<Statement>& statements);

    // ==================== CONTEXT-AWARE MATCHING ====================

    // Match with before/after context requirements
    std::vector<MatchResult> findContextualMatches(
        const MultiLinePattern& pattern,
        const std::vector<Statement>& statements,
        const MultiLinePattern& before_context,
        const MultiLinePattern& after_context);

    // Match within specific scope (loop, if block, etc.)
    std::vector<MatchResult> findScopedMatches(
        const MultiLinePattern& pattern,
        const Function& func,
        const std::string& scope_type);  // "loop", "if", "switch", etc.

    // ==================== PATTERN TRANSFORMATION ====================

    // Extract all unique matches of a pattern
    std::vector<std::string> extractPatternInstances(
        const MultiLinePattern& pattern,
        const std::vector<Statement>& statements);

    // Replace matched patterns with new code
    std::string replaceMatches(
        const MultiLinePattern& pattern,
        const std::vector<Statement>& statements,
        const std::string& replacement_template);

    // ==================== PATTERN COMPOSITION ====================

    // Combine multiple patterns with AND logic
    MultiLinePattern combineAND(const std::vector<MultiLinePattern>& patterns);

    // Combine multiple patterns with OR logic
    MultiLinePattern combineOR(const std::vector<MultiLinePattern>& patterns);

    // Negate a pattern (match when pattern does NOT occur)
    MultiLinePattern negate(const MultiLinePattern& pattern);

    // ==================== PATTERN UTILITIES ====================

    // Pattern validation
    bool validatePattern(const MultiLinePattern& pattern) const;

    // Pattern compilation for optimization
    void compilePattern(const MultiLinePattern& pattern);

    // Pattern statistics
    struct PatternStats {
        size_t num_lines;
        size_t num_placeholders;
        size_t num_quantifiers;
        size_t num_alternatives;
        float estimated_complexity;
    };
    PatternStats analyzePattern(const MultiLinePattern& pattern) const;

    // Pattern builder helpers
    static MultiLinePattern fromLines(const std::vector<std::string>& lines);
    static MultiLinePattern fromRegex(const std::string& regex_pattern);
    static MultiLinePattern fromWildcard(const std::string& wildcard_pattern);

private:
    // Internal pattern representation
    struct CompiledPattern {
        std::vector<std::regex> line_regexes;
        std::map<std::string, size_t> placeholder_indices;
        std::vector<std::string> wildcard_patterns;
        std::map<size_t, MultiLinePattern::LineQuantifier> quantifiers;
        bool is_compiled = false;
    };

    // Core matching implementation
    bool matchLine(const std::string& line, const std::regex& pattern_regex,
                  std::map<std::string, std::string>& captures) const;

    bool matchLineWildcard(const std::string& line, const std::string& wildcard_pattern,
                          std::map<std::string, std::string>& captures) const;

    bool matchLineFuzzy(const std::string& line, const std::string& pattern,
                       float min_similarity) const;

    bool matchLineStructural(const std::string& line, const std::string& pattern) const;

    // Semantic constraint checking
    bool checkSemanticConstraints(const std::vector<Statement>& matched_statements,
                                  const SemanticConstraints& constraints,
                                  const Function& func) const;

    bool checkDataDependency(const Statement& s1, const Statement& s2) const;
    bool checkControlDependency(const Statement& s1, const Statement& s2) const;
    bool isInSameScope(const Statement& s1, const Statement& s2) const;
    bool isInLoop(const Statement& stmt) const;
    bool isInConditional(const Statement& stmt) const;

    // Similarity metrics
    float calculateSimilarity(const std::string& text1, const std::string& text2) const;
    float calculateStructuralSimilarity(const std::string& text1, const std::string& text2) const;
    float calculateSemanticSimilarity(const std::string& text1, const std::string& text2) const;
    float calculateTokenSimilarity(const std::string& text1, const std::string& text2) const;

    // Pattern normalization
    std::string normalizeForStructural(const std::string& code) const;
    std::string normalizeForSemantic(const std::string& code) const;
    std::vector<std::string> tokenize(const std::string& code) const;

    // Wildcard helpers
    bool wildcardMatch(const char* text, const char* pattern) const;
    std::string wildcardToRegex(const std::string& wildcard) const;

    // Gap analysis
    bool isGapValid(size_t gap_size, unsigned min_gap, unsigned max_gap) const;
    size_t countGapStatements(const std::vector<Statement>& statements,
                             size_t start_idx, size_t end_idx) const;

    // Quantifier matching
    struct QuantifierMatch {
        size_t start_index;
        size_t count;
        std::vector<std::map<std::string, std::string>> captures;
    };
    std::vector<QuantifierMatch> matchWithQuantifier(
        const std::vector<Statement>& statements,
        size_t start_idx,
        const std::regex& pattern,
        const MultiLinePattern::LineQuantifier& quantifier) const;

    // Group matching
    bool matchLineGroup(
        const std::vector<Statement>& statements,
        size_t& current_idx,
        const MultiLinePattern::LineGroup& group,
        const MultiLinePattern& pattern,
        std::map<std::string, std::string>& captures) const;

    // Ordering validation
    bool validateOrdering(
        const std::vector<size_t>& matched_indices,
        const std::map<size_t, std::vector<size_t>>& ordering_constraints) const;

    // Context extraction
    std::string extractContext(
        const std::vector<Statement>& statements,
        size_t match_start,
        size_t match_end,
        size_t context_lines = 3) const;

    // Pattern compilation helpers
    void compileQuantifiers(const MultiLinePattern& pattern);
    void compileGroups(const MultiLinePattern& pattern);
    void compileWildcards(const MultiLinePattern& pattern);

private:
    CompiledPattern compiled_pattern_;
    mutable std::map<std::string, float> similarity_cache_;  // Cache for expensive similarity calculations
};

// Forward declarations for AST pattern matching are already at the top of the file

/**
 * @brief AST pattern specification
 */
struct ASTPattern {
    std::string pattern_template;  // e.g., "if ($cond) { return $val; }"

    struct NodeConstraint {
        std::optional<std::string> node_type;
        std::optional<std::string> value_pattern;
        std::optional<TypeInfo> type_constraint;
        std::function<bool(const clang::Stmt*)> custom_predicate;
    };

    std::map<std::string, NodeConstraint> constraints;
};

/**
 * @brief AST-based pattern matcher
 */
class ASTPatternMatcher {
public:
    explicit ASTPatternMatcher(clang::ASTContext& context);
    ~ASTPatternMatcher();

    // Pattern matching
    std::vector<clang::Stmt*> matchPattern(const ASTPattern& pattern,
                                          clang::Stmt* root);

    // Pattern building
    ASTPattern buildPatternFromExample(const std::string& code_example);

    // Pattern composition
    enum class CombineMode {
        SEQUENCE,    // p1 followed by p2
        EITHER,      // p1 or p2
        NESTED,      // p2 within p1
        INTERLEAVED  // p1 and p2 can be interleaved
    };

    ASTPattern combinePatterns(const ASTPattern& p1,
                              const ASTPattern& p2,
                              CombineMode mode);

    // Pattern validation
    bool validatePattern(const ASTPattern& pattern) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Forward declarations for IR pattern matching are already at the top of the file

/**
 * @brief IR pattern specification
 */
struct IRPattern {
    std::vector<std::string> instruction_patterns;

    struct ValueConstraint {
        std::optional<llvm::Type*> type;
        std::shared_ptr<llvm::APInt> constant_value;
        bool must_be_pointer = false;
        bool must_be_constant = false;
    };

    std::map<std::string, ValueConstraint> value_constraints;

    struct DataFlowConstraints {
        bool require_def_use_chain = false;
        bool require_same_basic_block = false;
        bool require_dominance = false;
    } flow_constraints;
};

/**
 * @brief IR-based pattern matcher
 */
class IRPatternMatcher {
public:
    IRPatternMatcher();
    ~IRPatternMatcher();

    // Pattern matching
    std::vector<llvm::Instruction*> matchInstructions(const IRPattern& pattern,
                                                      llvm::Function& func);

    // SSA-aware matching
    std::vector<std::pair<llvm::Value*, llvm::Value*>>
    matchDefUseChains(const IRPattern& def_pattern,
                     const IRPattern& use_pattern,
                     llvm::Function& func);

    // Pattern validation
    bool validatePattern(const IRPattern& pattern) const;

    // Pattern optimization
    void optimizePattern(IRPattern& pattern) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Unified pattern matcher that combines multiple matching strategies
 */
class UnifiedPatternMatcher {
public:
    UnifiedPatternMatcher(clang::ASTContext* ast_context = nullptr);
    ~UnifiedPatternMatcher();

    // Set contexts
    void setASTContext(clang::ASTContext* context);
    void setIRContext(llvm::Function* func);

    // Multi-strategy matching
    struct UnifiedPattern {
        std::optional<MultiLinePattern> text_pattern;
        std::optional<ASTPattern> ast_pattern;
        std::optional<IRPattern> ir_pattern;
        SemanticConstraints semantic_constraints;
    };

    struct UnifiedMatchResult {
        std::vector<MatchResult> text_matches;
        std::vector<clang::Stmt*> ast_matches;
        std::vector<llvm::Instruction*> ir_matches;
        float overall_confidence = 1.0f;
    };

    UnifiedMatchResult match(const UnifiedPattern& pattern,
                            const Function& func);

    // Pattern learning
    UnifiedPattern learnPatternFromExamples(const std::vector<std::string>& examples);

private:
    std::unique_ptr<MultiLinePatternMatcher> text_matcher_;
    std::unique_ptr<ASTPatternMatcher> ast_matcher_;
    std::unique_ptr<IRPatternMatcher> ir_matcher_;
};

} // namespace nomic::patterns

#endif // NOMIC_PATTERNS_PATTERN_MATCHER_H