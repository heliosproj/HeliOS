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

#include "nomic/core/semantic_model.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <regex>
#include <queue>
#include <map>

namespace nomic::dsl {

class BuiltinFunctions {
public:
    // Pattern matching functions
    static bool pattern_matches(const std::string& text, const std::string& pattern) {
        try {
            std::regex re(pattern);
            return std::regex_search(text, re);
        } catch (const std::regex_error& e) {
            spdlog::error("Invalid regex pattern: {}", e.what());
            return false;
        }
    }

    static bool regex_matches(const std::string& text, const std::string& regex) {
        return pattern_matches(text, regex);
    }

    // Collection functions
    template<typename Container>
    static size_t len(const Container& container) {
        return container.size();
    }

    template<typename Container, typename Predicate>
    static bool all(const Container& container, Predicate pred) {
        return std::all_of(container.begin(), container.end(), pred);
    }

    template<typename Container, typename Predicate>
    static bool any(const Container& container, Predicate pred) {
        return std::any_of(container.begin(), container.end(), pred);
    }

    template<typename Container, typename Predicate>
    static bool none(const Container& container, Predicate pred) {
        return std::none_of(container.begin(), container.end(), pred);
    }

    template<typename Container, typename Predicate>
    static size_t count(const Container& container, Predicate pred) {
        return std::count_if(container.begin(), container.end(), pred);
    }

    // Type checking functions
    static bool is_pointer_type(const TypeInfo& type) {
        return type.isPointer();
    }

    static bool is_array_type(const TypeInfo& type) {
        return type.isArray();
    }

    static bool is_integral_type(const TypeInfo& type) {
        const std::string& canonical = type.getCanonicalType();
        return canonical == "int" || canonical == "long" ||
               canonical == "short" || canonical == "char" ||
               canonical == "unsigned int" || canonical == "unsigned long" ||
               canonical == "unsigned short" || canonical == "unsigned char";
    }

    static bool is_floating_type(const TypeInfo& type) {
        const std::string& canonical = type.getCanonicalType();
        return canonical == "float" || canonical == "double" ||
               canonical == "long double";
    }

    // Control flow functions
    static bool dominates(const ControlFlowGraph* cfg, const BasicBlock* b1, const BasicBlock* b2) {
        if (!cfg || !b1 || !b2) {
            return false;
        }
        return cfg->dominates(b1, b2);
    }

    static bool post_dominates(const ControlFlowGraph* cfg, const BasicBlock* b1, const BasicBlock* b2) {
        if (!cfg || !b1 || !b2) {
            return false;
        }
        return cfg->postDominates(b1, b2);
    }

    static std::vector<BasicBlock*> path_between(const ControlFlowGraph* cfg, const BasicBlock* from, const BasicBlock* to) {
        // BFS-based path finding between blocks in CFG
        if (!cfg || !from || !to) {
            return {};
        }

        // Handle trivial case
        if (from->getId() == to->getId()) {
            return {const_cast<BasicBlock*>(from)};
        }

        // BFS to find shortest path
        std::map<unsigned, unsigned> parent;  // child -> parent mapping
        std::set<unsigned> visited;
        std::queue<const BasicBlock*> queue;

        queue.push(from);
        visited.insert(from->getId());
        parent[from->getId()] = from->getId();  // Mark start

        bool found = false;
        while (!queue.empty() && !found) {
            const BasicBlock* current = queue.front();
            queue.pop();

            // Check all successors
            for (unsigned succ_id : current->getSuccessors()) {
                if (visited.count(succ_id) > 0) {
                    continue;
                }

                // Find the successor block
                const BasicBlock* succ_block = nullptr;
                for (const auto& block : cfg->getBlocks()) {
                    if (block->getId() == succ_id) {
                        succ_block = block.get();
                        break;
                    }
                }

                if (!succ_block) {
                    continue;
                }

                visited.insert(succ_id);
                parent[succ_id] = current->getId();

                if (succ_id == to->getId()) {
                    found = true;
                    break;
                }

                queue.push(succ_block);
            }
        }

        // If no path found
        if (!found) {
            return {};
        }

        // Reconstruct path from parent map
        std::vector<BasicBlock*> path;
        unsigned current_id = to->getId();

        while (current_id != from->getId()) {
            // Find block with this ID
            for (const auto& block : cfg->getBlocks()) {
                if (block->getId() == current_id) {
                    path.push_back(block.get());
                    break;
                }
            }
            current_id = parent[current_id];
        }

        // Add starting block
        path.push_back(const_cast<BasicBlock*>(from));

        // Reverse to get path from->to
        std::reverse(path.begin(), path.end());

        return path;
    }

    // Semantic functions
    static bool calls_function(const Function& caller, const std::string& callee_name) {
        for (const auto& call : caller.getCallSites()) {
            if (call.getName() == callee_name) {
                return true;
            }
        }
        return false;
    }

    static bool modifies_variable(const Function& func, const Variable& var) {
        // Check if function modifies the given variable
        const SourceRange& func_range = func.getLocation();

        for (const auto& write_site : var.getWriteSites()) {
            // Check if write site is within this function's source range
            if (func_range.contains(write_site)) {
                return true;
            }
        }

        // Also check local variables - if this variable is a local in this function, it's modified
        for (const auto& local_var : func.getLocalVariables()) {
            if (local_var.getName() == var.getName()) {
                return !local_var.getWriteSites().empty();
            }
        }

        return false;
    }

    static bool reads_variable(const Function& func, const Variable& var) {
        // Check if function reads the given variable
        const SourceRange& func_range = func.getLocation();

        for (const auto& read_site : var.getReadSites()) {
            // Check if read site is within this function's source range
            if (func_range.contains(read_site)) {
                return true;
            }
        }

        // Also check local variables
        for (const auto& local_var : func.getLocalVariables()) {
            if (local_var.getName() == var.getName()) {
                return !local_var.getReadSites().empty();
            }
        }

        return false;
    }

    static bool returns_value(const Function& func, const std::string& value_pattern) {
        // Check if function returns a value matching the pattern
        const auto& return_stmts = func.getReturnStatements();

        if (return_stmts.empty()) {
            return false;
        }

        // Use regex to match return expressions
        try {
            std::regex pattern_regex(value_pattern);

            for (const auto& ret_stmt : return_stmts) {
                const std::string& ret_expr = ret_stmt.getReturnExpression();
                if (std::regex_search(ret_expr, pattern_regex)) {
                    return true;
                }
            }
        } catch (const std::regex_error& e) {
            spdlog::error("Invalid regex pattern in returns_value: {}", e.what());
            // Fall back to simple string matching
            for (const auto& ret_stmt : return_stmts) {
                if (ret_stmt.getReturnExpression().find(value_pattern) != std::string::npos) {
                    return true;
                }
            }
        }

        return false;
    }

    // Data flow functions
    static bool reaches(const ControlFlowGraph* cfg, const std::string& var_name,
                       unsigned def_block_id, unsigned use_block_id) {
        // Check if definition of var_name in def_block reaches use_block
        if (!cfg) {
            return false;
        }

        // Use the reaching definitions analysis
        const auto& reaching_defs = cfg->getReachingDefinitions(use_block_id);
        return reaching_defs.count(var_name) > 0;
    }

    static bool is_live(const ControlFlowGraph* cfg, const std::string& var_name, unsigned block_id) {
        // Check if variable is live (will be used) at the given block
        if (!cfg) {
            return false;
        }

        // Check both live-in and live-out sets
        const auto& live_in = cfg->getLiveVariablesIn(block_id);
        const auto& live_out = cfg->getLiveVariablesOut(block_id);

        return live_in.count(var_name) > 0 || live_out.count(var_name) > 0;
    }

    static std::set<std::string> aliased_variables(const ControlFlowGraph* cfg, const std::string& var_name) {
        // Find variables that may alias with the given variable
        // This is a simplified alias analysis based on pointer operations
        std::set<std::string> aliases;

        if (!cfg) {
            return aliases;
        }

        // For a comprehensive alias analysis, we would need to:
        // 1. Track pointer assignments (e.g., p = &var)
        // 2. Track pointer dereferences
        // 3. Perform flow-sensitive pointer analysis
        //
        // For now, we use a conservative approach:
        // - Variables are aliased if they appear together in pointer operations
        // - This is determined by analyzing statements in the CFG

        for (const auto& block : cfg->getBlocks()) {
            for (const auto& stmt : block->getStatements()) {
                const std::string& text = stmt.getText();

                // Check for pointer operations involving var_name
                // Pattern: ptr = &var_name or var_name = *ptr
                if (text.find(var_name) != std::string::npos) {
                    // Look for address-of operator
                    if (text.find('&') != std::string::npos) {
                        // Extract variable names from the statement
                        // This is simplified - full implementation would parse AST
                        std::regex var_regex("[a-zA-Z_][a-zA-Z0-9_]*");
                        std::smatch match;
                        std::string search_text = text;

                        while (std::regex_search(search_text, match, var_regex)) {
                            std::string candidate = match.str();
                            if (candidate != var_name && candidate != "if" &&
                                candidate != "while" && candidate != "for" &&
                                candidate != "return") {
                                aliases.insert(candidate);
                            }
                            search_text = match.suffix();
                        }
                    }
                }
            }
        }

        return aliases;
    }
};

// Helper class for tracking statements
class Statement {
public:
    const SourceRange& getLocation() const { return location_; }
    const std::string& getText() const { return text_; }
private:
    SourceRange location_;
    std::string text_;
};

// Helper class for tracking call sites
class CallSite {
public:
    const std::string& getName() const { return name_; }
private:
    std::string name_;
};

} // namespace nomic::dsl