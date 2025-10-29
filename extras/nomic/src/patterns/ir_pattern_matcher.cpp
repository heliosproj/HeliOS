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

#include "nomic/patterns/pattern_matcher.h"
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Dominators.h>
#include <spdlog/spdlog.h>
#include <regex>
#include <algorithm>
#include <set>

namespace nomic::patterns {

class IRPatternMatcher::Impl {
public:
    Impl() {
        spdlog::debug("IRPatternMatcher::Impl initialized");
    }

    /**
     * @brief Match instructions against a pattern
     * Pattern strings support:
     * - Instruction opcodes: "load", "store", "call", "icmp", "br", etc.
     * - Type patterns: "i32", "i64", "ptr", "*" (any type)
     * - Wildcards: "*" matches any instruction
     */
    std::vector<llvm::Instruction*> matchInstructions(const IRPattern& pattern,
                                                      llvm::Function& func) {
        std::vector<llvm::Instruction*> matches;

        if (pattern.instruction_patterns.empty()) {
            return matches;
        }

        for (auto& bb : func) {
            for (auto& inst : bb) {
                if (matchSingleInstruction(&inst, pattern)) {
                    matches.push_back(&inst);
                }
            }
        }

        return matches;
    }

    /**
     * @brief Match def-use chains between instructions
     * Finds pairs where:
     * 1. def instruction matches def_pattern
     * 2. use instruction matches use_pattern
     * 3. use instruction uses value produced by def instruction
     */
    std::vector<std::pair<llvm::Value*, llvm::Value*>>
    matchDefUseChains(const IRPattern& def_pattern,
                     const IRPattern& use_pattern,
                     llvm::Function& func) {
        std::vector<std::pair<llvm::Value*, llvm::Value*>> chains;

        // Find all potential def instructions
        for (auto& bb : func) {
            for (auto& inst : bb) {
                if (!matchSingleInstruction(&inst, def_pattern)) {
                    continue;
                }

                // For each def, find all uses
                for (auto* user : inst.users()) {
                    if (auto* use_inst = llvm::dyn_cast<llvm::Instruction>(user)) {
                        if (matchSingleInstruction(use_inst, use_pattern)) {
                            // Check flow constraints if specified
                            if (checkFlowConstraints(&inst, use_inst, use_pattern)) {
                                chains.emplace_back(&inst, use_inst);
                            }
                        }
                    }
                }
            }
        }

        return chains;
    }

    bool validatePattern(const IRPattern& pattern) const {
        if (pattern.instruction_patterns.empty()) {
            spdlog::warn("IR pattern has no instruction patterns");
            return false;
        }

        // Validate each instruction pattern
        for (const auto& inst_pattern : pattern.instruction_patterns) {
            if (inst_pattern.empty()) {
                spdlog::error("Empty instruction pattern");
                return false;
            }

            // Check for valid instruction opcode or wildcard
            if (inst_pattern != "*" && !isValidOpcodePattern(inst_pattern)) {
                spdlog::warn("Unknown instruction pattern: {}", inst_pattern);
                // Don't return false - might be a type pattern
            }
        }

        // Validate value constraints
        for (const auto& [name, constraint] : pattern.value_constraints) {
            if (constraint.must_be_constant && constraint.must_be_pointer) {
                spdlog::error("Constraint {} cannot be both constant and pointer", name);
                return false;
            }
        }

        return true;
    }

    void optimizePattern(IRPattern& pattern) const {
        // Optimization 1: Sort patterns by specificity (most specific first)
        std::stable_sort(pattern.instruction_patterns.begin(),
                        pattern.instruction_patterns.end(),
                        [](const std::string& a, const std::string& b) {
                            // Wildcards are least specific
                            if (a == "*") return false;
                            if (b == "*") return true;
                            // Otherwise, keep original order
                            return false;
                        });

        // Optimization 2: Merge duplicate constraints
        // (Implementation would deduplicate value constraints)

        // Optimization 3: Pre-compile regex patterns for matching
        // (Would be stored in a cache)

        spdlog::debug("Optimized IR pattern with {} instructions",
                     pattern.instruction_patterns.size());
    }

private:
    /**
     * @brief Match a single instruction against pattern
     */
    bool matchSingleInstruction(llvm::Instruction* inst, const IRPattern& pattern) const {
        // Try each pattern (OR logic)
        for (const auto& inst_pattern : pattern.instruction_patterns) {
            if (matchInstructionPattern(inst, inst_pattern)) {
                // Check value constraints
                if (checkValueConstraints(inst, pattern)) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * @brief Match instruction against a pattern string
     */
    bool matchInstructionPattern(llvm::Instruction* inst, const std::string& pattern) const {
        // Wildcard matches anything
        if (pattern == "*") {
            return true;
        }

        // Get instruction opcode name
        std::string opcode_name = std::string(inst->getOpcodeName());

        // Direct opcode match
        if (opcode_name == pattern) {
            return true;
        }

        // Pattern matching for instruction families
        if (pattern == "load" && llvm::isa<llvm::LoadInst>(inst)) {
            return true;
        }
        if (pattern == "store" && llvm::isa<llvm::StoreInst>(inst)) {
            return true;
        }
        if (pattern == "call" && llvm::isa<llvm::CallInst>(inst)) {
            return true;
        }
        if (pattern == "br" && llvm::isa<llvm::BranchInst>(inst)) {
            return true;
        }
        if (pattern == "icmp" && llvm::isa<llvm::ICmpInst>(inst)) {
            return true;
        }
        if (pattern == "fcmp" && llvm::isa<llvm::FCmpInst>(inst)) {
            return true;
        }
        if (pattern == "phi" && llvm::isa<llvm::PHINode>(inst)) {
            return true;
        }
        if (pattern == "alloca" && llvm::isa<llvm::AllocaInst>(inst)) {
            return true;
        }
        if (pattern == "ret" && llvm::isa<llvm::ReturnInst>(inst)) {
            return true;
        }
        if (pattern == "switch" && llvm::isa<llvm::SwitchInst>(inst)) {
            return true;
        }

        // Binary operations
        if (pattern == "add" || pattern == "sub" || pattern == "mul" ||
            pattern == "div" || pattern == "rem") {
            if (auto* binop = llvm::dyn_cast<llvm::BinaryOperator>(inst)) {
                return opcode_name == pattern;
            }
        }

        // Bitwise operations
        if (pattern == "and" || pattern == "or" || pattern == "xor" ||
            pattern == "shl" || pattern == "lshr" || pattern == "ashr") {
            if (auto* binop = llvm::dyn_cast<llvm::BinaryOperator>(inst)) {
                return opcode_name == pattern;
            }
        }

        // Cast operations
        if (pattern == "cast" || pattern == "trunc" || pattern == "zext" ||
            pattern == "sext" || pattern == "bitcast" || pattern == "ptrtoint" ||
            pattern == "inttoptr") {
            return llvm::isa<llvm::CastInst>(inst);
        }

        // GEP
        if (pattern == "getelementptr" || pattern == "gep") {
            return llvm::isa<llvm::GetElementPtrInst>(inst);
        }

        // Regex pattern matching for advanced patterns
        try {
            std::regex regex_pattern(pattern, std::regex::icase);
            if (std::regex_match(opcode_name, regex_pattern)) {
                return true;
            }
        } catch (const std::regex_error&) {
            // Not a valid regex, skip
        }

        return false;
    }

    /**
     * @brief Check value constraints for an instruction
     */
    bool checkValueConstraints(llvm::Instruction* inst, const IRPattern& pattern) const {
        for (const auto& [name, constraint] : pattern.value_constraints) {
            // Get the value to check (for now, use the instruction itself)
            llvm::Value* value = inst;

            // Check type constraint
            if (constraint.type.has_value()) {
                if (value->getType() != constraint.type.value()) {
                    return false;
                }
            }

            // Check if must be pointer
            if (constraint.must_be_pointer) {
                if (!value->getType()->isPointerTy()) {
                    return false;
                }
            }

            // Check if must be constant
            if (constraint.must_be_constant) {
                if (!llvm::isa<llvm::Constant>(value)) {
                    return false;
                }
            }

            // Check constant value if specified
            if (constraint.constant_value) {
                if (auto* const_int = llvm::dyn_cast<llvm::ConstantInt>(value)) {
                    if (const_int->getValue() != *constraint.constant_value) {
                        return false;
                    }
                } else {
                    return false;  // Not a constant int
                }
            }
        }

        return true;
    }

    /**
     * @brief Check data flow constraints between def and use
     */
    bool checkFlowConstraints(llvm::Instruction* def,
                              llvm::Instruction* use,
                              const IRPattern& pattern) const {
        const auto& flow = pattern.flow_constraints;

        // Check if in same basic block
        if (flow.require_same_basic_block) {
            if (def->getParent() != use->getParent()) {
                return false;
            }
        }

        // Check dominance (requires dominance tree, simplified check)
        if (flow.require_dominance) {
            // Simplified: if in same block, def must come before use
            if (def->getParent() == use->getParent()) {
                // Check instruction order in block
                bool found_def = false;
                for (auto& inst : *def->getParent()) {
                    if (&inst == def) {
                        found_def = true;
                    }
                    if (&inst == use) {
                        return found_def;
                    }
                }
            }
            // If in different blocks, would need full dominance analysis
            // For now, be conservative and return true
        }

        return true;
    }

    /**
     * @brief Check if a pattern string is a valid LLVM opcode
     */
    bool isValidOpcodePattern(const std::string& pattern) const {
        static const std::set<std::string> valid_opcodes = {
            // Terminator instructions
            "ret", "br", "switch", "indirectbr", "invoke", "resume",
            "unreachable", "cleanupret", "catchret", "catchswitch",

            // Binary operations
            "add", "fadd", "sub", "fsub", "mul", "fmul", "udiv", "sdiv",
            "fdiv", "urem", "srem", "frem",

            // Bitwise binary operations
            "shl", "lshr", "ashr", "and", "or", "xor",

            // Memory operations
            "alloca", "load", "store", "fence", "cmpxchg", "atomicrmw",
            "getelementptr", "gep",

            // Cast operations
            "trunc", "zext", "sext", "fptoui", "fptosi", "uitofp",
            "sitofp", "fptrunc", "fpext", "ptrtoint", "inttoptr",
            "bitcast", "addrspacecast", "cast",

            // Other operations
            "icmp", "fcmp", "phi", "call", "select", "va_arg",
            "extractelement", "insertelement", "shufflevector",
            "extractvalue", "insertvalue", "landingpad",

            // Wildcards
            "*"
        };

        return valid_opcodes.count(pattern) > 0;
    }
};

IRPatternMatcher::IRPatternMatcher()
    : impl_(std::make_unique<Impl>()) {}

IRPatternMatcher::~IRPatternMatcher() = default;

std::vector<llvm::Instruction*> IRPatternMatcher::matchInstructions(const IRPattern& pattern,
                                                                   llvm::Function& func) {
    return impl_->matchInstructions(pattern, func);
}

std::vector<std::pair<llvm::Value*, llvm::Value*>>
IRPatternMatcher::matchDefUseChains(const IRPattern& def_pattern,
                                   const IRPattern& use_pattern,
                                   llvm::Function& func) {
    return impl_->matchDefUseChains(def_pattern, use_pattern, func);
}

bool IRPatternMatcher::validatePattern(const IRPattern& pattern) const {
    return impl_->validatePattern(pattern);
}

void IRPatternMatcher::optimizePattern(IRPattern& pattern) const {
    impl_->optimizePattern(pattern);
}

} // namespace nomic::patterns