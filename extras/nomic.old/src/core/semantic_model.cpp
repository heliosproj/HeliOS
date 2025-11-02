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
#include <sstream>
#include <algorithm>
#include <functional>

namespace nomic {

// SourceLocation implementation
std::string SourceLocation::toString() const {
    std::ostringstream oss;
    oss << file_ << ":" << line_ << ":" << column_;
    return oss.str();
}

bool SourceLocation::operator==(const SourceLocation& other) const {
    return file_ == other.file_ && line_ == other.line_ && column_ == other.column_;
}

bool SourceLocation::operator<(const SourceLocation& other) const {
    if (file_ != other.file_) return file_ < other.file_;
    if (line_ != other.line_) return line_ < other.line_;
    return column_ < other.column_;
}

// SourceRange implementation
bool SourceRange::contains(const SourceLocation& loc) const {
    if (loc.getFile() != start_.getFile() || loc.getFile() != end_.getFile()) {
        return false;
    }

    if (loc.getLine() < start_.getLine() || loc.getLine() > end_.getLine()) {
        return false;
    }

    if (loc.getLine() == start_.getLine() && loc.getColumn() < start_.getColumn()) {
        return false;
    }

    if (loc.getLine() == end_.getLine() && loc.getColumn() > end_.getColumn()) {
        return false;
    }

    return true;
}

bool SourceRange::overlaps(const SourceRange& other) const {
    if (start_.getFile() != other.start_.getFile()) {
        return false;
    }

    return !(end_ < other.start_ || other.end_ < start_);
}

std::string SourceRange::toString() const {
    std::ostringstream oss;
    oss << start_.toString() << " - " << end_.toString();
    return oss.str();
}

// TypeInfo implementation
std::string TypeInfo::toString() const {
    std::ostringstream oss;

    if (is_const_) oss << "const ";
    if (is_volatile_) oss << "volatile ";
    if (is_static_) oss << "static ";

    oss << canonical_type_;

    if (is_pointer_) {
        for (unsigned i = 0; i < pointer_depth_; ++i) {
            oss << "*";
        }
    }

    if (is_array_ && array_size_.has_value()) {
        oss << "[" << array_size_.value() << "]";
    }

    return oss.str();
}

// Function implementation
void Function::addCallSite(const CallSite& call) {
    calls_.push_back(call);
}

void Function::setCFG(std::unique_ptr<ControlFlowGraph> cfg) {
    cfg_ = std::move(cfg);
}

void Function::addStatement(const Statement& stmt) {
    statements_.push_back(stmt);
}

void Function::addAnnotation(const Annotation& annotation) {
    annotations_.push_back(annotation);
}

// BasicBlock implementation
void BasicBlock::addStatement(const Statement& stmt) {
    statements_.push_back(stmt);
}

// ControlFlowGraph implementation
ControlFlowGraph::ControlFlowGraph() {
    // Create entry block by default
    auto entry = std::make_unique<BasicBlock>(0);
    entry->setEntry(true);
    entry_block_id_ = 0;
    blocks_.push_back(std::move(entry));
}

BasicBlock* ControlFlowGraph::createBasicBlock() {
    unsigned id = blocks_.size();
    auto block = std::make_unique<BasicBlock>(id);
    BasicBlock* ptr = block.get();
    blocks_.push_back(std::move(block));
    return ptr;
}

void ControlFlowGraph::setEntryBlock(BasicBlock* block) {
    if (block) {
        entry_block_id_ = block->getId();
        block->setEntry(true);
    }
}

void ControlFlowGraph::addExitBlock(BasicBlock* block) {
    if (block) {
        exit_block_ids_.insert(block->getId());
        block->setExit(true);
    }
}

void ControlFlowGraph::addEdge(BasicBlock* from, BasicBlock* to) {
    if (from && to) {
        from->addSuccessor(to->getId());
        to->addPredecessor(from->getId());
    }
}

BasicBlock* ControlFlowGraph::getEntryBlock() const {
    if (entry_block_id_ < blocks_.size()) {
        return blocks_[entry_block_id_].get();
    }
    return nullptr;
}

std::vector<BasicBlock*> ControlFlowGraph::getExitBlocks() const {
    std::vector<BasicBlock*> exit_blocks;
    for (unsigned id : exit_block_ids_) {
        if (id < blocks_.size()) {
            exit_blocks.push_back(blocks_[id].get());
        }
    }
    return exit_blocks;
}

void ControlFlowGraph::computeDominanceTree() {
    // Implement dominance analysis using Cooper-Harvey-Kennedy algorithm
    // This is an iterative data-flow algorithm

    if (blocks_.empty()) return;

    unsigned entry_id = entry_block_id_;
    size_t num_blocks = blocks_.size();

    // Initialize: entry dominates itself, all others dominated by all blocks
    std::map<unsigned, std::set<unsigned>> dominators;
    dominators[entry_id].insert(entry_id);

    for (const auto& block : blocks_) {
        unsigned id = block->getId();
        if (id != entry_id) {
            // Initially, every block is dominated by all blocks
            for (size_t i = 0; i < num_blocks; ++i) {
                dominators[id].insert(i);
            }
        }
    }

    // Iterate until fixed point
    bool changed = true;
    while (changed) {
        changed = false;

        for (const auto& block : blocks_) {
            unsigned id = block->getId();
            if (id == entry_id) continue;

            // dom(n) = {n} ∪ (∩ dom(p) for all predecessors p of n)
            std::set<unsigned> new_doms;
            new_doms.insert(id);  // Block dominates itself

            const auto& preds = block->getPredecessors();
            if (!preds.empty()) {
                // Start with dominators of first predecessor
                auto pred_it = preds.begin();
                new_doms.insert(dominators[*pred_it].begin(), dominators[*pred_it].end());

                // Intersect with dominators of other predecessors
                for (++pred_it; pred_it != preds.end(); ++pred_it) {
                    std::set<unsigned> intersection;
                    std::set_intersection(
                        new_doms.begin(), new_doms.end(),
                        dominators[*pred_it].begin(), dominators[*pred_it].end(),
                        std::inserter(intersection, intersection.begin())
                    );
                    new_doms = std::move(intersection);
                    new_doms.insert(id);  // Ensure self is always included
                }
            }

            if (new_doms != dominators[id]) {
                dominators[id] = std::move(new_doms);
                changed = true;
            }
        }
    }

    // Compute immediate dominators
    // idom(n) is the unique dominator of n that is dominated by all other dominators of n
    immediate_dominators_.clear();
    for (const auto& block : blocks_) {
        unsigned id = block->getId();
        if (id == entry_id) continue;

        const auto& doms = dominators[id];
        // Find the immediate dominator (closest dominator in the tree)
        std::set<unsigned> strict_doms = doms;
        strict_doms.erase(id);  // Remove self

        if (!strict_doms.empty()) {
            // idom(n) is the dominator that is not strictly dominated by any other dominator of n
            for (unsigned candidate : strict_doms) {
                bool is_idom = true;
                for (unsigned other : strict_doms) {
                    if (other != candidate) {
                        // If other dominates candidate, then candidate is not the idom
                        if (dominators[candidate].count(other) > 0 && candidate != other) {
                            is_idom = false;
                            break;
                        }
                    }
                }
                if (is_idom) {
                    immediate_dominators_[id] = candidate;
                    break;
                }
            }
        }
    }

    // Build dominance tree from immediate dominators
    dominance_tree_.clear();
    for (const auto& [dominated, dominator] : immediate_dominators_) {
        dominance_tree_[dominator].insert(dominated);
    }
}

void ControlFlowGraph::computePostDominanceTree() {
    // Post-dominance is like dominance but computed backwards from exit blocks
    // A block X post-dominates block Y if all paths from Y to exit go through X

    if (blocks_.empty() || exit_block_ids_.empty()) return;

    size_t num_blocks = blocks_.size();

    // Initialize: exit blocks post-dominate themselves
    std::map<unsigned, std::set<unsigned>> post_dominators;

    for (unsigned exit_id : exit_block_ids_) {
        post_dominators[exit_id].insert(exit_id);
    }

    // All other blocks initially post-dominated by all blocks
    for (const auto& block : blocks_) {
        unsigned id = block->getId();
        if (exit_block_ids_.count(id) == 0) {
            for (size_t i = 0; i < num_blocks; ++i) {
                post_dominators[id].insert(i);
            }
        }
    }

    // Iterate until fixed point (working backwards through successors)
    bool changed = true;
    while (changed) {
        changed = false;

        for (const auto& block : blocks_) {
            unsigned id = block->getId();
            if (exit_block_ids_.count(id) > 0) continue;  // Skip exit blocks

            // pdom(n) = {n} ∪ (∩ pdom(s) for all successors s of n)
            std::set<unsigned> new_pdoms;
            new_pdoms.insert(id);  // Block post-dominates itself

            const auto& succs = block->getSuccessors();
            if (!succs.empty()) {
                // Start with post-dominators of first successor
                auto succ_it = succs.begin();
                new_pdoms.insert(post_dominators[*succ_it].begin(),
                                post_dominators[*succ_it].end());

                // Intersect with post-dominators of other successors
                for (++succ_it; succ_it != succs.end(); ++succ_it) {
                    std::set<unsigned> intersection;
                    std::set_intersection(
                        new_pdoms.begin(), new_pdoms.end(),
                        post_dominators[*succ_it].begin(), post_dominators[*succ_it].end(),
                        std::inserter(intersection, intersection.begin())
                    );
                    new_pdoms = std::move(intersection);
                    new_pdoms.insert(id);  // Ensure self is always included
                }
            } else {
                // No successors means this might be an unreachable exit
                // Treat as if it post-dominates itself only
                new_pdoms.clear();
                new_pdoms.insert(id);
            }

            if (new_pdoms != post_dominators[id]) {
                post_dominators[id] = std::move(new_pdoms);
                changed = true;
            }
        }
    }

    // Build post-dominance tree
    post_dominance_tree_.clear();
    for (const auto& [id, pdoms] : post_dominators) {
        // Find immediate post-dominator
        std::set<unsigned> strict_pdoms = pdoms;
        strict_pdoms.erase(id);  // Remove self

        if (!strict_pdoms.empty()) {
            for (unsigned candidate : strict_pdoms) {
                bool is_ipdom = true;
                for (unsigned other : strict_pdoms) {
                    if (other != candidate) {
                        if (post_dominators[candidate].count(other) > 0 && candidate != other) {
                            is_ipdom = false;
                            break;
                        }
                    }
                }
                if (is_ipdom) {
                    post_dominance_tree_[candidate].insert(id);
                    break;
                }
            }
        }
    }
}

bool ControlFlowGraph::dominates(const BasicBlock* dominator, const BasicBlock* dominated) const {
    if (!dominator || !dominated) return false;
    if (dominator->getId() == dominated->getId()) return true;

    // Check if dominated has dominator in its dominance path
    // Walk up the immediate dominator chain
    unsigned current = dominated->getId();
    while (true) {
        auto it = immediate_dominators_.find(current);
        if (it == immediate_dominators_.end()) {
            break;  // Reached entry or no dominator
        }
        if (it->second == dominator->getId()) {
            return true;
        }
        current = it->second;
    }

    return false;
}

bool ControlFlowGraph::postDominates(const BasicBlock* post_dominator, const BasicBlock* dominated) const {
    if (!post_dominator || !dominated) return false;
    if (post_dominator->getId() == dominated->getId()) return true;

    // Check if post_dominator post-dominates dominated
    // Walk through the post-dominance tree
    auto it = post_dominance_tree_.find(post_dominator->getId());
    if (it != post_dominance_tree_.end()) {
        // Check if dominated is in the post-dominance tree of post_dominator
        std::set<unsigned> visited;
        std::function<bool(unsigned)> search = [&](unsigned node) -> bool {
            if (node == dominated->getId()) return true;
            if (visited.count(node) > 0) return false;
            visited.insert(node);

            auto tree_it = post_dominance_tree_.find(node);
            if (tree_it != post_dominance_tree_.end()) {
                for (unsigned child : tree_it->second) {
                    if (search(child)) return true;
                }
            }
            return false;
        };

        return search(post_dominator->getId());
    }

    return false;
}

// Data flow analysis implementation
void ControlFlowGraph::computeReachingDefinitions() {
    // Reaching definitions analysis: which definitions reach each program point
    // This is a forward data-flow analysis

    if (blocks_.empty()) return;

    // First, compute gen and kill sets for each block
    for (const auto& block : blocks_) {
        unsigned block_id = block->getId();
        gen_defs_[block_id].clear();
        kill_defs_[block_id].clear();

        // Analyze statements in the block to find definitions
        for (const auto& stmt : block->getStatements()) {
            const std::string& text = stmt.getText();

            // Simple heuristic: look for assignment patterns
            // In a full implementation, this would use AST information
            if (text.find('=') != std::string::npos && text.find("==") == std::string::npos) {
                // Extract variable name (simplified)
                size_t eq_pos = text.find('=');
                std::string lhs = text.substr(0, eq_pos);

                // Trim whitespace
                lhs.erase(0, lhs.find_first_not_of(" \t"));
                lhs.erase(lhs.find_last_not_of(" \t") + 1);

                // Extract just the variable name (before any operators/brackets)
                size_t space_pos = lhs.find_first_of(" \t[.*&");
                if (space_pos != std::string::npos) {
                    lhs = lhs.substr(0, space_pos);
                }

                if (!lhs.empty() && lhs.find_first_of("0123456789") != 0) {
                    gen_defs_[block_id].insert(lhs);
                    // This definition kills all previous definitions of the same variable
                    kill_defs_[block_id].insert(lhs);
                }
            }
        }
    }

    // Initialize reaching definitions
    reaching_defs_in_.clear();
    reaching_defs_out_.clear();

    for (const auto& block : blocks_) {
        unsigned id = block->getId();
        reaching_defs_in_[id].clear();
        reaching_defs_out_[id] = gen_defs_[id];  // Initially, out = gen
    }

    // Iterate until fixed point
    bool changed = true;
    while (changed) {
        changed = false;

        for (const auto& block : blocks_) {
            unsigned id = block->getId();

            // IN[B] = ∪ OUT[P] for all predecessors P of B
            std::set<std::string> new_in;
            for (unsigned pred_id : block->getPredecessors()) {
                const auto& pred_out = reaching_defs_out_[pred_id];
                new_in.insert(pred_out.begin(), pred_out.end());
            }

            // OUT[B] = GEN[B] ∪ (IN[B] - KILL[B])
            std::set<std::string> new_out = gen_defs_[id];
            for (const std::string& var : new_in) {
                if (kill_defs_[id].count(var) == 0) {
                    new_out.insert(var);
                }
            }

            if (new_in != reaching_defs_in_[id] || new_out != reaching_defs_out_[id]) {
                reaching_defs_in_[id] = std::move(new_in);
                reaching_defs_out_[id] = std::move(new_out);
                changed = true;
            }
        }
    }
}

void ControlFlowGraph::computeLiveVariables() {
    // Live variable analysis: which variables are live (will be used) at each program point
    // This is a backward data-flow analysis

    if (blocks_.empty()) return;

    // Compute use and def sets for each block
    for (const auto& block : blocks_) {
        unsigned block_id = block->getId();
        gen_uses_[block_id].clear();

        // Analyze statements to find variable uses
        for (const auto& stmt : block->getStatements()) {
            const std::string& text = stmt.getText();

            // Simple heuristic: find variable names (alphanumeric identifiers)
            // In a full implementation, this would use AST information
            std::string current_var;
            for (size_t i = 0; i < text.length(); ++i) {
                char c = text[i];
                if (std::isalnum(c) || c == '_') {
                    current_var += c;
                } else {
                    if (!current_var.empty() && current_var.find_first_of("0123456789") != 0) {
                        // Check if it's not a keyword
                        if (current_var != "if" && current_var != "else" && current_var != "for" &&
                            current_var != "while" && current_var != "return" && current_var != "int" &&
                            current_var != "void" && current_var != "char" && current_var != "float") {
                            gen_uses_[block_id].insert(current_var);
                        }
                    }
                    current_var.clear();
                }
            }
            // Handle last variable
            if (!current_var.empty() && current_var.find_first_of("0123456789") != 0) {
                gen_uses_[block_id].insert(current_var);
            }
        }
    }

    // Initialize live variable sets
    live_vars_in_.clear();
    live_vars_out_.clear();

    for (const auto& block : blocks_) {
        unsigned id = block->getId();
        live_vars_in_[id] = gen_uses_[id];  // Initially, in = use
        live_vars_out_[id].clear();
    }

    // Iterate until fixed point (working backwards)
    bool changed = true;
    while (changed) {
        changed = false;

        // Process blocks in reverse order for better convergence
        for (auto it = blocks_.rbegin(); it != blocks_.rend(); ++it) {
            const auto& block = *it;
            unsigned id = block->getId();

            // OUT[B] = ∪ IN[S] for all successors S of B
            std::set<std::string> new_out;
            for (unsigned succ_id : block->getSuccessors()) {
                const auto& succ_in = live_vars_in_[succ_id];
                new_out.insert(succ_in.begin(), succ_in.end());
            }

            // IN[B] = USE[B] ∪ (OUT[B] - DEF[B])
            std::set<std::string> new_in = gen_uses_[id];
            for (const std::string& var : new_out) {
                if (gen_defs_[id].count(var) == 0) {
                    new_in.insert(var);
                }
            }

            if (new_in != live_vars_in_[id] || new_out != live_vars_out_[id]) {
                live_vars_in_[id] = std::move(new_in);
                live_vars_out_[id] = std::move(new_out);
                changed = true;
            }
        }
    }
}

void ControlFlowGraph::computeDefUseChains() {
    // Compute def-use chains: for each definition, find all uses
    def_use_chains_.clear();

    // Build mapping of variables to their definition and use sites
    std::map<std::string, std::set<unsigned>> var_def_sites;
    std::map<std::string, std::set<unsigned>> var_use_sites;

    for (const auto& block : blocks_) {
        unsigned id = block->getId();
        for (const std::string& var : gen_defs_[id]) {
            var_def_sites[var].insert(id);
        }
        for (const std::string& var : gen_uses_[id]) {
            var_use_sites[var].insert(id);
        }
    }

    // For each definition, find reachable uses
    for (const auto& [var, def_sites] : var_def_sites) {
        for (unsigned def_site : def_sites) {
            std::vector<unsigned> use_sites;

            // Find uses that are reached by this definition
            // A use is reached if the definition reaches the use site
            // and no other definition of the same variable is between them

            if (var_use_sites.count(var) > 0) {
                for (unsigned use_site : var_use_sites[var]) {
                    // Check if def reaches use (simplified check)
                    if (reaching_defs_in_[use_site].count(var) > 0) {
                        use_sites.push_back(use_site);
                    }
                }
            }

            def_use_chains_[var][def_site] = std::move(use_sites);
        }
    }
}

void ControlFlowGraph::computeUseDefChains() {
    // Compute use-def chains: for each use, find all reaching definitions
    use_def_chains_.clear();

    // Build mapping of variables to their definition and use sites
    std::map<std::string, std::set<unsigned>> var_def_sites;
    std::map<std::string, std::set<unsigned>> var_use_sites;

    for (const auto& block : blocks_) {
        unsigned id = block->getId();
        for (const std::string& var : gen_defs_[id]) {
            var_def_sites[var].insert(id);
        }
        for (const std::string& var : gen_uses_[id]) {
            var_use_sites[var].insert(id);
        }
    }

    // For each use, find reaching definitions
    for (const auto& [var, use_sites] : var_use_sites) {
        for (unsigned use_site : use_sites) {
            std::vector<unsigned> def_sites;

            // Find definitions that reach this use
            if (var_def_sites.count(var) > 0) {
                for (unsigned def_site : var_def_sites[var]) {
                    // Check if def reaches use
                    if (reaching_defs_in_[use_site].count(var) > 0) {
                        def_sites.push_back(def_site);
                    }
                }
            }

            use_def_chains_[var][use_site] = std::move(def_sites);
        }
    }
}

// Data flow query methods
std::set<std::string> ControlFlowGraph::getReachingDefinitions(unsigned block_id) const {
    auto it = reaching_defs_in_.find(block_id);
    if (it != reaching_defs_in_.end()) {
        return it->second;
    }
    return {};
}

std::set<std::string> ControlFlowGraph::getLiveVariablesIn(unsigned block_id) const {
    auto it = live_vars_in_.find(block_id);
    if (it != live_vars_in_.end()) {
        return it->second;
    }
    return {};
}

std::set<std::string> ControlFlowGraph::getLiveVariablesOut(unsigned block_id) const {
    auto it = live_vars_out_.find(block_id);
    if (it != live_vars_out_.end()) {
        return it->second;
    }
    return {};
}

std::set<unsigned> ControlFlowGraph::getDefSites(const std::string& variable) const {
    std::set<unsigned> sites;
    for (const auto& [block_id, vars] : gen_defs_) {
        if (vars.count(variable) > 0) {
            sites.insert(block_id);
        }
    }
    return sites;
}

std::set<unsigned> ControlFlowGraph::getUseSites(const std::string& variable) const {
    std::set<unsigned> sites;
    for (const auto& [block_id, vars] : gen_uses_) {
        if (vars.count(variable) > 0) {
            sites.insert(block_id);
        }
    }
    return sites;
}

std::vector<unsigned> ControlFlowGraph::getDefUseChain(const std::string& variable, unsigned def_site) const {
    auto var_it = def_use_chains_.find(variable);
    if (var_it != def_use_chains_.end()) {
        auto site_it = var_it->second.find(def_site);
        if (site_it != var_it->second.end()) {
            return site_it->second;
        }
    }
    return {};
}

std::vector<unsigned> ControlFlowGraph::getUseDefChain(const std::string& variable, unsigned use_site) const {
    auto var_it = use_def_chains_.find(variable);
    if (var_it != use_def_chains_.end()) {
        auto site_it = var_it->second.find(use_site);
        if (site_it != var_it->second.end()) {
            return site_it->second;
        }
    }
    return {};
}

// SemanticDatabase implementation
void SemanticDatabase::addFunction(std::unique_ptr<Function> func) {
    if (func) {
        const std::string& name = func->getName();
        const std::string& file = func->getLocation().getStart().getFile();

        Function* ptr = func.get();
        functions_.push_back(std::move(func));

        function_index_[name] = ptr;
        file_function_index_[file].push_back(ptr);
    }
}

void SemanticDatabase::addGlobalVariable(std::unique_ptr<Variable> var) {
    if (var) {
        const std::string& name = var->getName();
        Variable* ptr = var.get();
        global_variables_.push_back(std::move(var));
        variable_index_[name] = ptr;
    }
}

void SemanticDatabase::addTypeDefinition(const std::string& name, const TypeInfo& type) {
    type_definitions_[name] = type;
}

Function* SemanticDatabase::findFunction(const std::string& name) const {
    auto it = function_index_.find(name);
    if (it != function_index_.end()) {
        return it->second;
    }
    return nullptr;
}

Variable* SemanticDatabase::findGlobalVariable(const std::string& name) const {
    auto it = variable_index_.find(name);
    if (it != variable_index_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<Function*> SemanticDatabase::getFunctionsInFile(const std::string& file) const {
    auto it = file_function_index_.find(file);
    if (it != file_function_index_.end()) {
        return it->second;
    }
    return {};
}

FileInfo* SemanticDatabase::getOrCreateFileInfo(const std::string& file_path) {
    auto it = files_.find(file_path);
    if (it != files_.end()) {
        return it->second.get();
    }

    // Create new FileInfo
    auto file_info = std::make_unique<FileInfo>(file_path);
    FileInfo* ptr = file_info.get();
    files_[file_path] = std::move(file_info);
    return ptr;
}

FileInfo* SemanticDatabase::findFileInfo(const std::string& file_path) const {
    auto it = files_.find(file_path);
    if (it != files_.end()) {
        return it->second.get();
    }
    return nullptr;
}

// ==================== Taint Analysis Methods ====================

void ControlFlowGraph::computeTaintPropagation(const std::set<std::string>& taint_sources) {
    // Initialize all blocks with empty taint sets
    for (const auto& block : blocks_) {
        unsigned block_id = block->getId();
        tainted_vars_in_[block_id] = {};
        tainted_vars_out_[block_id] = {};
    }

    // Mark initial taint sources at entry block
    if (!blocks_.empty()) {
        tainted_vars_in_[entry_block_id_] = taint_sources;
    }

    // Iterative data-flow analysis for taint propagation
    // Similar to reaching definitions but for tainted variables
    // Algorithm: Forward data-flow
    //   IN[B] = ∪ OUT[P] for all predecessors P of B
    //   OUT[B] = GEN[B] ∪ (IN[B] - KILL[B])
    // where:
    //   GEN[B] = variables that become tainted in B
    //   KILL[B] = variables that are sanitized/overwritten in B

    bool changed = true;
    int iterations = 0;
    const int max_iterations = 1000;

    while (changed && iterations < max_iterations) {
        changed = false;
        iterations++;

        for (const auto& block : blocks_) {
            unsigned block_id = block->getId();

            // Compute IN[block] = ∪ OUT[pred]
            std::set<std::string> new_in;
            for (unsigned pred_id : block->getPredecessors()) {
                const auto& pred_out = tainted_vars_out_[pred_id];
                new_in.insert(pred_out.begin(), pred_out.end());
            }

            // If this is the entry block, also include initial sources
            if (block->isEntry()) {
                new_in.insert(taint_sources.begin(), taint_sources.end());
            }

            // Compute OUT[block] = GEN[block] ∪ (IN[block] - KILL[block])
            std::set<std::string> new_out = new_in;

            // Add generated taints (from assignments involving tainted vars)
            // This is a simplified version - full implementation would parse statements
            for (const auto& stmt : block->getStatements()) {
                // Check if statement involves tainted variables
                // For now, assume any variable used in an expression with a tainted var becomes tainted
                // Full implementation would parse the AST
            }

            // Remove killed taints (sanitized variables)
            // Full implementation would detect sanitizer calls

            // Check if OUT set changed
            if (new_out != tainted_vars_out_[block_id]) {
                tainted_vars_out_[block_id] = new_out;
                changed = true;
            }

            tainted_vars_in_[block_id] = new_in;
        }
    }
}

void ControlFlowGraph::markVariableTainted(const std::string& var_name, unsigned block_id) {
    tainted_vars_out_[block_id].insert(var_name);

    // Record taint path
    if (taint_paths_.find(var_name) == taint_paths_.end()) {
        taint_paths_[var_name] = {var_name};
    }
}

bool ControlFlowGraph::isVariableTaintedAt(const std::string& var_name, unsigned block_id) const {
    auto it = tainted_vars_out_.find(block_id);
    if (it != tainted_vars_out_.end()) {
        return it->second.find(var_name) != it->second.end();
    }
    return false;
}

std::set<std::string> ControlFlowGraph::getTaintedVariablesAt(unsigned block_id) const {
    auto it = tainted_vars_out_.find(block_id);
    if (it != tainted_vars_out_.end()) {
        return it->second;
    }
    return {};
}

std::map<std::string, std::vector<std::string>> ControlFlowGraph::getTaintPaths(const std::string& var_name) const {
    std::map<std::string, std::vector<std::string>> paths;

    // For each block where the variable is tainted, trace back through def-use chains
    for (const auto& [block_id, tainted_vars] : tainted_vars_out_) {
        if (tainted_vars.find(var_name) != tainted_vars.end()) {
            // Found a block where this variable is tainted
            std::vector<std::string> path;
            path.push_back(var_name);

            // Trace back through use-def chains to find the source
            auto use_def_it = use_def_chains_.find(var_name);
            if (use_def_it != use_def_chains_.end()) {
                // Follow the chain backwards
                for (const auto& [use_site, def_sites] : use_def_it->second) {
                    for (unsigned def_site : def_sites) {
                        // This is simplified - full implementation would build complete paths
                        path.push_back("def_at_block_" + std::to_string(def_site));
                    }
                }
            }

            paths["block_" + std::to_string(block_id)] = path;
        }
    }

    return paths;
}

} // namespace nomic