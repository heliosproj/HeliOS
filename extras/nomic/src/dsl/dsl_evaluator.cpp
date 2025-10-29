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

#include "nomic/dsl/dsl.h"
#include "nomic/dsl/tokenizer.h"
#include "nomic/dsl/parser.h"
#include "nomic/dsl/ast_nodes.h"
#include "nomic/patterns/pattern_matcher.h"
#include "nomic/analysis/taint_analysis.h"
#include "nomic/patterns/ast_query.h"
#include "nomic/standards/coding_standards.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vector>
#include <cmath>
#include <regex>
#include <algorithm>
#include <set>
#include <numeric>

namespace nomic::dsl {

// Helper function to convert std::any to double
static double anyToDouble(const std::any& value) {
    if (value.type() == typeid(int)) {
        return static_cast<double>(std::any_cast<int>(value));
    } else if (value.type() == typeid(double)) {
        return std::any_cast<double>(value);
    } else if (value.type() == typeid(bool)) {
        return std::any_cast<bool>(value) ? 1.0 : 0.0;
    } else if (value.type() == typeid(std::string)) {
        try {
            return std::stod(std::any_cast<std::string>(value));
        } catch (...) {
            return 0.0;
        }
    }
    return 0.0;
}

// Helper function to convert std::any to bool
static bool anyToBool(const std::any& value) {
    if (value.type() == typeid(bool)) {
        return std::any_cast<bool>(value);
    } else if (value.type() == typeid(int)) {
        return std::any_cast<int>(value) != 0;
    } else if (value.type() == typeid(double)) {
        return std::any_cast<double>(value) != 0.0;
    } else if (value.type() == typeid(std::string)) {
        return !std::any_cast<std::string>(value).empty();
    }
    return false;
}

// Helper function to convert std::any to string
static std::string anyToString(const std::any& value) {
    if (value.type() == typeid(std::string)) {
        return std::any_cast<std::string>(value);
    } else if (value.type() == typeid(int)) {
        return std::to_string(std::any_cast<int>(value));
    } else if (value.type() == typeid(double)) {
        return std::to_string(std::any_cast<double>(value));
    } else if (value.type() == typeid(bool)) {
        return std::any_cast<bool>(value) ? "true" : "false";
    }
    return "";
}

// Helper function to convert std::any to int
static int anyToInt(const std::any& value) {
    if (value.type() == typeid(int)) {
        return std::any_cast<int>(value);
    } else if (value.type() == typeid(double)) {
        return static_cast<int>(std::any_cast<double>(value));
    } else if (value.type() == typeid(bool)) {
        return std::any_cast<bool>(value) ? 1 : 0;
    } else if (value.type() == typeid(std::string)) {
        try {
            return std::stoi(std::any_cast<std::string>(value));
        } catch (...) {
            return 0;
        }
    }
    return 0;
}

// Forward declaration
class DSLEvaluatorImpl;

// Built-in function implementations
static std::any callBuiltinFunction(const std::string& name,
                                   const std::vector<std::any>& args,
                                   const DSLEvaluator::EvaluationContext& context);

class DSLEvaluatorImpl {
public:
    DSLEvaluatorImpl() {
        spdlog::debug("DSLEvaluator initialized");
    }

    std::any evaluate(const ASTNode* node, DSLEvaluator::EvaluationContext& context) {
        if (!node) {
            return std::any();
        }

        switch (node->getType()) {
            case ASTNode::IDENTIFIER:
                return evaluateIdentifier(static_cast<const IdentifierNode*>(node), context);

            case ASTNode::LITERAL:
                return evaluateLiteral(static_cast<const LiteralNode*>(node));

            case ASTNode::BINARY_OP:
                return evaluateBinaryOp(static_cast<const BinaryOpNode*>(node), context);

            case ASTNode::UNARY_OP:
                return evaluateUnaryOp(static_cast<const UnaryOpNode*>(node), context);

            case ASTNode::FUNCTION_CALL:
                return evaluateFunctionCall(static_cast<const FunctionCallNode*>(node), context);

            case ASTNode::QUANTIFIER:
                return evaluateQuantifier(static_cast<const QuantifierNode*>(node), context);

            case ASTNode::MEMBER_ACCESS:
                return evaluateMemberAccess(static_cast<const MemberAccessNode*>(node), context);

            case ASTNode::LIST_LITERAL:
                return evaluateListLiteral(static_cast<const ListLiteralNode*>(node), context);

            default:
                spdlog::error("Unknown AST node type");
                return std::any();
        }
    }

private:
    std::any evaluateIdentifier(const IdentifierNode* node, DSLEvaluator::EvaluationContext& context) {
        const std::string& name = node->getName();

        // Check bindings first
        auto it = context.bindings.find(name);
        if (it != context.bindings.end()) {
            return it->second;
        }

        // Check for collections
        auto coll_it = context.collections.find(name);
        if (coll_it != context.collections.end()) {
            return coll_it->second;
        }

        // Check for special identifiers
        if (name == "fn" && context.current_function) {
            return std::any(context.current_function);
        }
        if (name == "var" && context.current_variable) {
            return std::any(context.current_variable);
        }
        if (name == "file" && context.current_file) {
            return std::any(context.current_file);
        }

        spdlog::warn("Undefined identifier: {}", name);
        return std::any();
    }

    std::any evaluateLiteral(const LiteralNode* node) {
        const std::string& value = node->getValue();

        // Boolean
        if (value == "true") {
            return true;
        }
        if (value == "false") {
            return false;
        }

        // Try to parse as number
        try {
            if (value.find('.') != std::string::npos) {
                return std::stod(value);
            } else {
                return std::stoi(value);
            }
        } catch (...) {
            // If not a number, return as string
            return value;
        }
    }

    std::any evaluateBinaryOp(const BinaryOpNode* node, DSLEvaluator::EvaluationContext& context) {
        const std::string& op = node->getOp();

        // Logical operators - short circuit evaluation
        if (op == "and" || op == "&&") {
            auto left = evaluate(node->getLeft(), context);
            if (!anyToBool(left)) {
                return false;
            }
            auto right = evaluate(node->getRight(), context);
            return anyToBool(right);
        }

        if (op == "or" || op == "||") {
            auto left = evaluate(node->getLeft(), context);
            if (anyToBool(left)) {
                return true;
            }
            auto right = evaluate(node->getRight(), context);
            return anyToBool(right);
        }

        // Evaluate both operands for other operators
        auto left = evaluate(node->getLeft(), context);
        auto right = evaluate(node->getRight(), context);

        // Comparison operators
        if (op == "==") {
            return anyToString(left) == anyToString(right);
        }
        if (op == "!=") {
            return anyToString(left) != anyToString(right);
        }
        if (op == "<") {
            return anyToDouble(left) < anyToDouble(right);
        }
        if (op == "<=") {
            return anyToDouble(left) <= anyToDouble(right);
        }
        if (op == ">") {
            return anyToDouble(left) > anyToDouble(right);
        }
        if (op == ">=") {
            return anyToDouble(left) >= anyToDouble(right);
        }

        // Membership operator
        if (op == "in") {
            std::string left_str = anyToString(left);

            // Check if right is a list
            if (right.type() == typeid(std::vector<std::any>)) {
                auto list = std::any_cast<std::vector<std::any>>(right);
                for (const auto& elem : list) {
                    if (anyToString(elem) == left_str) {
                        return true;
                    }
                }
                return false;
            }

            spdlog::warn("'in' operator requires list on right side");
            return false;
        }

        // Arithmetic operators
        if (op == "+") {
            return anyToDouble(left) + anyToDouble(right);
        }
        if (op == "-") {
            return anyToDouble(left) - anyToDouble(right);
        }
        if (op == "*") {
            return anyToDouble(left) * anyToDouble(right);
        }
        if (op == "/") {
            double divisor = anyToDouble(right);
            if (divisor == 0.0) {
                spdlog::error("Division by zero");
                return 0.0;
            }
            return anyToDouble(left) / divisor;
        }

        spdlog::error("Unknown binary operator: {}", op);
        return std::any();
    }

    std::any evaluateUnaryOp(const UnaryOpNode* node, DSLEvaluator::EvaluationContext& context) {
        const std::string& op = node->getOp();
        auto operand = evaluate(node->getOperand(), context);

        if (op == "not") {
            return !anyToBool(operand);
        }
        if (op == "-") {
            return -anyToDouble(operand);
        }

        spdlog::error("Unknown unary operator: {}", op);
        return std::any();
    }

    std::any evaluateFunctionCall(const FunctionCallNode* node, DSLEvaluator::EvaluationContext& context) {
        const std::string& name = node->getName();
        const auto& arg_nodes = node->getArgs();

        // Evaluate arguments
        std::vector<std::any> args;
        for (const auto& arg_node : arg_nodes) {
            args.push_back(evaluate(arg_node.get(), context));
        }

        // Call built-in function
        return callBuiltinFunction(name, args, context);
    }

    std::any evaluateQuantifier(const QuantifierNode* node, DSLEvaluator::EvaluationContext& context) {
        auto qtype = node->getQuantifierType();
        const std::string& var = node->getVar();

        // Evaluate collection
        auto collection_value = evaluate(node->getCollection(), context);

        // Convert collection to vector
        std::vector<std::any> collection;
        if (collection_value.type() == typeid(std::vector<std::any>)) {
            collection = std::any_cast<std::vector<std::any>>(collection_value);
        } else {
            spdlog::warn("Collection is not a vector for quantifier");
            return false;
        }

        // Iterate over collection
        int count = 0;
        for (const auto& item : collection) {
            // Bind variable
            context.bindings[var] = item;

            // Check where clause if present
            bool passes_where = true;
            if (node->getWhereClause()) {
                auto where_result = evaluate(node->getWhereClause(), context);
                passes_where = anyToBool(where_result);
            }

            if (!passes_where) {
                continue;
            }

            // Evaluate satisfies clause
            auto satisfies_result = evaluate(node->getSatisfiesClause(), context);
            bool satisfies = anyToBool(satisfies_result);

            if (satisfies) {
                count++;
            }

            // Early exit for some quantifiers
            if (qtype == QuantifierNode::EXISTS && satisfies) {
                context.bindings.erase(var);
                return true;
            }
            if (qtype == QuantifierNode::ALL && !satisfies) {
                context.bindings.erase(var);
                return false;
            }
        }

        // Remove binding
        context.bindings.erase(var);

        // Return result based on quantifier type
        switch (qtype) {
            case QuantifierNode::ALL:
                return count == static_cast<int>(collection.size());
            case QuantifierNode::ANY:
                return count > 0;
            case QuantifierNode::EXISTS:
                return false; // Would have returned true early if found
            case QuantifierNode::NONE:
                return count == 0;
            default:
                return false;
        }
    }

    std::any evaluateMemberAccess(const MemberAccessNode* node, DSLEvaluator::EvaluationContext& context) {
        auto object = evaluate(node->getObject(), context);
        const std::string& member = node->getMember();

        // Check if object is a Function pointer
        if (object.type() == typeid(const Function*)) {
            auto func = std::any_cast<const Function*>(object);
            return getFunctionProperty(func, member, context);
        }

        // Check if object is a Variable pointer
        if (object.type() == typeid(const Variable*)) {
            auto var = std::any_cast<const Variable*>(object);
            return getVariableProperty(var, member);
        }

        // Check if object is a TypeInfo
        if (object.type() == typeid(TypeInfo)) {
            auto type = std::any_cast<TypeInfo>(object);
            return getTypeProperty(type, member);
        }

        // Check if object is a FileInfo pointer
        if (object.type() == typeid(const FileInfo*)) {
            auto file = std::any_cast<const FileInfo*>(object);
            return getFileProperty(file, member);
        }

        // Check if object is a ControlFlowGraph pointer
        if (object.type() == typeid(ControlFlowGraph*)) {
            auto cfg = std::any_cast<ControlFlowGraph*>(object);
            return getCFGProperty(cfg, member, context);
        }

        // Check if object is a PreprocessorContext pointer
        if (object.type() == typeid(const PreprocessorContext*)) {
            auto preprocessor = std::any_cast<const PreprocessorContext*>(object);
            return getPreprocessorProperty(preprocessor, member);
        }

        // Check if object is a collection (vector<any>)
        if (object.type() == typeid(std::vector<std::any>)) {
            auto collection = std::any_cast<std::vector<std::any>>(object);

            if (member == "size") {
                return static_cast<int>(collection.size());
            }

            if (member == "empty") {
                return collection.empty();
            }
        }

        // Check if object is a string (for nested access like fn.location.file)
        if (object.type() == typeid(std::string)) {
            std::string obj_str = std::any_cast<std::string>(object);

            // String methods
            if (member == "length" || member == "size") {
                return static_cast<int>(obj_str.length());
            }

            // Handle location.* properties
            if (member == "file" || member == "filename") {
                // Extract filename from "file:line:col" format
                size_t colon_pos = obj_str.find(':');
                if (colon_pos != std::string::npos) {
                    return obj_str.substr(0, colon_pos);
                }
                return obj_str;
            }

            if (member == "line") {
                // Extract line number from "file:line:col" format
                size_t first_colon = obj_str.find(':');
                if (first_colon != std::string::npos) {
                    size_t second_colon = obj_str.find(':', first_colon + 1);
                    if (second_colon != std::string::npos) {
                        std::string line_str = obj_str.substr(first_colon + 1, second_colon - first_colon - 1);
                        try {
                            return std::stoi(line_str);
                        } catch (...) {
                            return 0;
                        }
                    }
                }
                return 0;
            }

            if (member == "column" || member == "col") {
                // Extract column from "file:line:col" format
                size_t first_colon = obj_str.find(':');
                if (first_colon != std::string::npos) {
                    size_t second_colon = obj_str.find(':', first_colon + 1);
                    if (second_colon != std::string::npos) {
                        std::string col_str = obj_str.substr(second_colon + 1);
                        try {
                            return std::stoi(col_str);
                        } catch (...) {
                            return 0;
                        }
                    }
                }
                return 0;
            }
        }

        spdlog::warn("Member access on unknown type: {}", member);
        return std::any();
    }

    std::any evaluateListLiteral(const ListLiteralNode* node, DSLEvaluator::EvaluationContext& context) {
        std::vector<std::any> elements;
        for (const auto& elem_node : node->getElements()) {
            elements.push_back(evaluate(elem_node.get(), context));
        }
        return elements;
    }

    std::any getFunctionProperty(const Function* func, const std::string& member,
                                DSLEvaluator::EvaluationContext& context) {
        // ==================== EXISTING PROPERTIES ====================
        if (member == "name") {
            return func->getName();
        }
        if (member == "cyclomatic_complexity") {
            return static_cast<int>(func->getCyclomaticComplexity());
        }
        if (member == "parameter_count") {
            return static_cast<int>(func->getParameters().size());
        }
        if (member == "is_static") {
            return func->isStatic();
        }
        if (member == "return_type") {
            return func->getReturnType();
        }

        // ==================== TIER 1: BASIC PROPERTIES ====================

        if (member == "signature") {
            return func->getSignature();
        }

        if (member == "is_inline") {
            return func->isInline();
        }

        if (member == "linkage") {
            return func->getLinkage();
        }

        if (member == "location") {
            // Return SourceRange as string (file:line:col)
            const auto& loc = func->getLocation();
            const auto& start = loc.getStart();
            return start.getFile() + ":" + std::to_string(start.getLine()) + ":" + std::to_string(start.getColumn());
        }

        if (member == "line_count") {
            const auto& loc = func->getLocation();
            return static_cast<int>(loc.getEnd().getLine() - loc.getStart().getLine());
        }

        if (member == "statement_count") {
            return static_cast<int>(func->getStatements().size());
        }

        if (member == "return_statements_count") {
            return static_cast<int>(func->getReturnStatements().size());
        }

        if (member == "local_var_count") {
            return static_cast<int>(func->getLocalVariables().size());
        }

        if (member == "call_count") {
            return static_cast<int>(func->getCallSites().size());
        }

        if (member == "return_count") {
            return static_cast<int>(func->getReturnStatements().size());
        }

        if (member == "has_body") {
            return !func->getStatements().empty();
        }

        // ==================== TIER 2: COLLECTIONS (FIXED!) ====================

        if (member == "parameters") {
            // Return ACTUAL parameter collection
            std::vector<std::any> params;
            for (const auto& param : func->getParameters()) {
                params.push_back(static_cast<const Variable*>(&param));
            }
            return params;
        }

        if (member == "local_vars") {
            // Return ACTUAL local variables collection
            std::vector<std::any> vars;
            for (const auto& var : func->getLocalVariables()) {
                vars.push_back(static_cast<const Variable*>(&var));
            }
            return vars;
        }

        if (member == "calls") {
            // Return ACTUAL function calls collection
            std::vector<std::any> calls;
            for (const auto& call : func->getCallSites()) {
                // Store call name as string for now
                calls.push_back(call.getName());
            }
            return calls;
        }

        if (member == "statements") {
            // Return ACTUAL statements collection
            std::vector<std::any> stmts;
            for (const auto& stmt : func->getStatements()) {
                // Store statement text for now
                stmts.push_back(stmt.getText());
            }
            return stmts;
        }

        if (member == "return_statements") {
            // Return return statements collection
            std::vector<std::any> rets;
            for (const auto& ret : func->getReturnStatements()) {
                rets.push_back(ret.getReturnExpression());
            }
            return rets;
        }

        if (member == "annotations") {
            // Return annotations collection
            std::vector<std::any> annots;
            for (const auto& annot : func->getAnnotations()) {
                annots.push_back(annot.getName());
            }
            return annots;
        }

        if (member == "array_accesses") {
            // Return array accesses collection
            std::vector<std::any> accesses;
            for (const auto& access : func->getArrayAccesses()) {
                accesses.push_back(access.getArrayName());
            }
            return accesses;
        }

        // ==================== TIER 2: COMPUTED PROPERTIES ====================

        if (member == "nesting_depth") {
            // Calculate max nesting depth from statements
            // This is a simplified implementation - would need CFG analysis for accurate depth
            // For now, just count compound statements as a proxy
            int count = 0;
            for (const auto& stmt : func->getStatements()) {
                if (stmt.getType() == Statement::COMPOUND_STMT) {
                    count++;
                }
            }
            return count;
        }

        if (member == "conditional_count") {
            // Count if statements (switch not yet in enum)
            int count = 0;
            for (const auto& stmt : func->getStatements()) {
                auto type = stmt.getType();
                if (type == Statement::IF_STMT) {
                    count++;
                }
            }
            return count;
        }

        if (member == "loop_count") {
            // Count loops
            int count = 0;
            for (const auto& stmt : func->getStatements()) {
                auto type = stmt.getType();
                if (type == Statement::FOR_STMT ||
                    type == Statement::WHILE_STMT) {
                    count++;
                }
            }
            return count;
        }

        if (member == "goto_count") {
            // Count goto statements
            // Goto not yet tracked in Statement enum, return 0 for now
            return 0;
        }

        // ==================== TIER 4: TAINT ANALYSIS PROPERTIES ====================

        if (member == "has_taint_flow") {
            // Check if any parameters or local variables are tainted
            for (const auto& param : func->getParameters()) {
                if (param.isTainted()) {
                    return true;
                }
            }
            for (const auto& var : func->getLocalVariables()) {
                if (var.isTainted()) {
                    return true;
                }
            }
            return false;
        }

        if (member == "taint_sources") {
            // Return list of taint sources from parameters and locals
            std::vector<std::any> sources;
            for (const auto& param : func->getParameters()) {
                if (param.isTainted() && param.getTaintSource().has_value()) {
                    sources.push_back(param.getTaintSource().value());
                }
            }
            for (const auto& var : func->getLocalVariables()) {
                if (var.isTainted() && var.getTaintSource().has_value()) {
                    sources.push_back(var.getTaintSource().value());
                }
            }
            return sources;
        }

        // Return CFG object for nested access (fn.cfg.*)
        if (member == "cfg") {
            return func->getCFG();
        }

        // Return PreprocessorContext object for nested access (fn.preprocessor.*)
        if (member == "preprocessor") {
            return &func->getPreprocessorContext();
        }

        // ==================== API CONSISTENCY PROPERTIES ====================

        if (member == "returns_error_code") {
            // Check if function returns an error code (int, long, etc.)
            const std::string& ret_type = func->getReturnType().getCanonicalType();
            return ret_type == "int" || ret_type == "long" ||
                   ret_type == "unsigned int" || ret_type == "unsigned long" ||
                   ret_type == "short" || ret_type == "unsigned short";
        }

        if (member == "returns_pointer") {
            // Check if function returns a pointer
            return func->getReturnType().isPointer();
        }

        if (member == "returns_void") {
            // Check if function returns void
            return func->getReturnType().getCanonicalType() == "void";
        }

        if (member == "has_output_params") {
            // Check if function has output parameters (non-const pointers)
            for (const auto& param : func->getParameters()) {
                if (param.getType().isPointer() && !param.getType().isConst()) {
                    return true;
                }
            }
            return false;
        }

        if (member == "output_params_last") {
            // Check if all output parameters come after input parameters
            bool seen_output = false;
            for (const auto& param : func->getParameters()) {
                bool is_output = param.getType().isPointer() && !param.getType().isConst();
                bool is_input = !is_output;

                if (seen_output && is_input) {
                    // Found input param after output param - order violated
                    return false;
                }
                if (is_output) {
                    seen_output = true;
                }
            }
            return true;
        }

        if (member == "const_input_params") {
            // Check if all input pointer parameters are const
            // A stricter check: count non-const pointers
            int non_const_pointers = 0;
            for (const auto& param : func->getParameters()) {
                if (param.getType().isPointer() && !param.getType().isConst()) {
                    non_const_pointers++;
                }
            }
            // Allow some non-const pointers (output params), but flag if too many
            // For now, return true (this is a heuristic-based check)
            return true;
        }

        if (member == "has_size_before_buffer") {
            // Check if size parameters come before buffer parameters
            // Pattern: func(size_t size, char* buffer) is good
            // Pattern: func(char* buffer, size_t size) is bad
            const auto& params = func->getParameters();
            for (size_t i = 0; i < params.size(); i++) {
                const auto& param = params[i];
                const std::string& type = param.getType().getCanonicalType();

                // Check if this is a buffer (pointer to char/byte/void)
                if (param.getType().isPointer() &&
                    (type.find("char") != std::string::npos ||
                     type.find("unsigned char") != std::string::npos ||
                     type.find("void") != std::string::npos ||
                     type.find("byte") != std::string::npos)) {

                    // Look for a size parameter before this buffer
                    bool has_size_before = false;
                    for (size_t j = 0; j < i; j++) {
                        const std::string& prev_type = params[j].getType().getCanonicalType();
                        if (prev_type.find("size") != std::string::npos ||
                            prev_type == "int" || prev_type == "unsigned int" ||
                            prev_type == "long" || prev_type == "unsigned long") {
                            has_size_before = true;
                            break;
                        }
                    }

                    if (!has_size_before && i < params.size() - 1) {
                        // Buffer found without size before it (and not last param)
                        return false;
                    }
                }
            }
            return true;
        }

        if (member == "follows_naming_convention") {
            // Check if function follows common naming conventions
            // For OS APIs: lowercase with underscores or CamelCase
            const std::string& name = func->getName();

            // Check for snake_case (lowercase with underscores)
            bool is_snake_case = true;
            for (char c : name) {
                if (std::isupper(c)) {
                    is_snake_case = false;
                    break;
                }
            }

            // Check for CamelCase
            bool is_camel_case = !name.empty() && std::isupper(name[0]);

            return is_snake_case || is_camel_case;
        }

        spdlog::warn("Unknown function property: {}", member);
        return std::any();
    }

    std::any getVariableProperty(const Variable* var, const std::string& member) {
        // ==================== EXISTING PROPERTIES ====================
        if (member == "name") {
            return var->getName();
        }
        if (member == "type") {
            return var->getType();
        }
        if (member == "is_initialized") {
            return var->isInitialized();
        }

        // ==================== TIER 1: BASIC PROPERTIES ====================

        if (member == "is_global") {
            return var->isGlobal();
        }

        if (member == "is_static") {
            return var->isStatic();
        }

        if (member == "is_parameter") {
            return var->isParameter();
        }

        if (member == "is_const") {
            return var->getType().isConst();
        }

        if (member == "location") {
            const auto& loc = var->getLocation();
            const auto& start = loc.getStart();
            return start.getFile() + ":" + std::to_string(start.getLine()) + ":" + std::to_string(start.getColumn());
        }

        // ==================== TIER 2: COMPUTED PROPERTIES ====================

        if (member == "read_count") {
            return static_cast<int>(var->getReadSites().size());
        }

        if (member == "write_count") {
            return static_cast<int>(var->getWriteSites().size());
        }

        if (member == "initial_value") {
            const auto& val = var->getInitialValue();
            return val.has_value() ? val.value() : std::string("");
        }

        // ==================== TIER 2: TAINT PROPERTIES ====================

        if (member == "is_tainted") {
            return var->isTainted();
        }

        if (member == "taint_source") {
            const auto& src = var->getTaintSource();
            return src.has_value() ? src.value() : std::string("");
        }

        if (member == "taint_confidence") {
            return static_cast<double>(var->getTaintConfidence());
        }

        spdlog::warn("Unknown variable property: {}", member);
        return std::any();
    }

    std::any getTypeProperty(const TypeInfo& type, const std::string& member) {
        // ==================== TIER 2: TYPE PROPERTIES ====================

        if (member == "is_pointer") {
            return type.isPointer();
        }

        if (member == "is_array") {
            return type.isArray();
        }

        if (member == "is_const") {
            return type.isConst();
        }

        if (member == "is_volatile") {
            return type.isVolatile();
        }

        if (member == "is_static") {
            return type.isStatic();
        }

        if (member == "pointer_depth") {
            return static_cast<int>(type.getPointerDepth());
        }

        if (member == "array_size") {
            const auto& size = type.getArraySize();
            return size.has_value() ? static_cast<int>(size.value()) : 0;
        }

        if (member == "canonical_type") {
            return type.getCanonicalType();
        }

        if (member == "desugared_type") {
            return type.getDesugaredType();
        }

        // ==================== TIER 4: TYPE SYSTEM PROPERTIES ====================

        if (member == "canonical_name") {
            return type.getCanonicalType();
        }

        if (member == "desugared_name") {
            return type.getDesugaredType();
        }

        if (member == "is_struct") {
            const std::string& canonical = type.getCanonicalType();
            return canonical.find("struct ") != std::string::npos;
        }

        if (member == "is_union") {
            const std::string& canonical = type.getCanonicalType();
            return canonical.find("union ") != std::string::npos;
        }

        if (member == "is_enum") {
            const std::string& canonical = type.getCanonicalType();
            return canonical.find("enum ") != std::string::npos;
        }

        if (member == "is_typedef") {
            const std::string& canonical = type.getCanonicalType();
            const std::string& desugared = type.getDesugaredType();
            return canonical != desugared;
        }

        if (member == "is_function_pointer") {
            const std::string& canonical = type.getCanonicalType();
            return type.isPointer() && (canonical.find("(*)") != std::string::npos || canonical.find("(*)(") != std::string::npos);
        }

        if (member == "is_void") {
            const std::string& canonical = type.getCanonicalType();
            return canonical == "void" || canonical == "const void" || canonical == "volatile void";
        }

        if (member == "is_arithmetic") {
            const std::string& canonical = type.getCanonicalType();
            // Check for common arithmetic types
            return canonical == "int" || canonical == "unsigned int" || canonical == "short" ||
                   canonical == "unsigned short" || canonical == "long" || canonical == "unsigned long" ||
                   canonical == "long long" || canonical == "unsigned long long" ||
                   canonical == "float" || canonical == "double" || canonical == "long double" ||
                   canonical == "char" || canonical == "unsigned char" || canonical == "signed char";
        }

        if (member == "is_signed") {
            const std::string& canonical = type.getCanonicalType();
            // Check if it's a signed type (default for int, long, etc.)
            if (canonical.find("unsigned") != std::string::npos) {
                return false;
            }
            return canonical == "int" || canonical == "short" || canonical == "long" ||
                   canonical == "long long" || canonical == "char" || canonical == "signed char" ||
                   canonical == "float" || canonical == "double" || canonical == "long double";
        }

        // Note: size and alignment would require Clang ASTContext, not implemented here
        if (member == "size") {
            return 0; // Placeholder - would need Clang type system
        }

        if (member == "alignment") {
            return 0; // Placeholder - would need Clang type system
        }

        spdlog::warn("Unknown type property: {}", member);
        return std::any();
    }

    std::any getFileProperty(const FileInfo* file, const std::string& member) {
        // ==================== TIER 3: FILE PROPERTIES ====================

        if (member == "path") {
            return file->getPath();
        }

        if (member == "name") {
            return file->getName();
        }

        if (member == "extension") {
            return file->getExtension();
        }

        if (member == "line_count") {
            return static_cast<int>(file->getLineCount());
        }

        if (member == "function_count") {
            return static_cast<int>(file->getFunctionCount());
        }

        if (member == "global_var_count") {
            return static_cast<int>(file->getGlobalVarCount());
        }

        if (member == "include_count") {
            return static_cast<int>(file->getIncludeCount());
        }

        if (member == "has_header_guard") {
            return file->hasHeaderGuard();
        }

        if (member == "is_header") {
            return file->isHeader();
        }

        // ==================== TIER 4: FILE PREPROCESSOR PROPERTIES ====================

        if (member == "has_include_guards") {
            return file->hasHeaderGuard();
        }

        if (member == "system_includes") {
            // Return list of system includes (e.g., <stdio.h>)
            std::vector<std::any> includes;
            for (const auto& include : file->getSystemIncludes()) {
                includes.push_back(include);
            }
            return includes;
        }

        spdlog::warn("Unknown file property: {}", member);
        return std::any();
    }

    std::any getCFGProperty(ControlFlowGraph* cfg, const std::string& member,
                            DSLEvaluator::EvaluationContext& context) {
        // ==================== TIER 4: CFG PROPERTIES ====================

        if (!cfg) {
            spdlog::warn("Attempting to access CFG property '{}' on null CFG", member);
            return std::any();
        }

        if (member == "entry_block") {
            // Return the entry basic block ID
            BasicBlock* entry = cfg->getEntryBlock();
            return entry ? static_cast<int>(entry->getId()) : -1;
        }

        if (member == "exit_block") {
            // Return the first exit block ID (or -1 if none)
            auto exits = cfg->getExitBlocks();
            return exits.empty() ? -1 : static_cast<int>(exits[0]->getId());
        }

        if (member == "block_count") {
            // Return number of basic blocks
            return static_cast<int>(cfg->getBlocks().size());
        }

        if (member == "edge_count") {
            // Count all edges by iterating through blocks and their successors
            int edge_count = 0;
            for (const auto& block : cfg->getBlocks()) {
                edge_count += static_cast<int>(block->getSuccessors().size());
            }
            return edge_count;
        }

        if (member == "has_unreachable_code") {
            // Check if there are blocks not reachable from entry
            // A block is unreachable if it's not the entry and has no predecessors
            BasicBlock* entry = cfg->getEntryBlock();
            if (!entry) return false;

            for (const auto& block : cfg->getBlocks()) {
                if (block.get() != entry && block->getPredecessors().empty()) {
                    return true;
                }
            }
            return false;
        }

        if (member == "has_infinite_loop") {
            // Detect infinite loops: a block that dominates itself and has no exit
            // This is a simplified check - a block with a back-edge to itself
            for (const auto& block : cfg->getBlocks()) {
                const auto& successors = block->getSuccessors();
                // Check if block has itself as a successor (self-loop)
                if (successors.find(block->getId()) != successors.end()) {
                    // Check if this block has no path to exit
                    // For now, simple heuristic: if it only points to itself, it's infinite
                    if (successors.size() == 1) {
                        return true;
                    }
                }
            }
            return false;
        }

        if (member == "dominance_frontier") {
            // Return dominance frontier as a map
            // This would require computing dominance frontier first
            // For now, return empty map as placeholder
            std::map<int, std::vector<int>> frontier;
            // TODO: Implement dominance frontier computation
            return frontier;
        }

        if (member == "post_dominance") {
            // Return post-dominance tree
            // This requires computing post-dominance
            const_cast<ControlFlowGraph*>(cfg)->computePostDominanceTree();
            std::map<int, int> post_dom_map;
            // TODO: Convert internal representation to map
            return post_dom_map;
        }

        spdlog::warn("Unknown CFG property: {}", member);
        return std::any();
    }

    std::any getPreprocessorProperty(const PreprocessorContext* preprocessor, const std::string& member) {
        // ==================== TIER 4: PREPROCESSOR PROPERTIES ====================

        if (!preprocessor) {
            spdlog::warn("Attempting to access preprocessor property '{}' on null preprocessor", member);
            return std::any();
        }

        if (member == "macros") {
            // Return list of active macro definitions
            std::vector<std::any> macros;
            for (const auto& define : preprocessor->getActiveDefines()) {
                macros.push_back(define);
            }
            return macros;
        }

        if (member == "ifdef_depth") {
            // Return the number of active #ifdef conditions
            // This is approximated by the number of active defines
            return static_cast<int>(preprocessor->getActiveDefines().size());
        }

        if (member == "includes") {
            // Return list of includes
            // Note: This information is not currently tracked in PreprocessorContext
            // Would need to extend PreprocessorContext to track includes
            std::vector<std::any> includes;
            return includes;
        }

        spdlog::warn("Unknown preprocessor property: {}", member);
        return std::any();
    }
};

// Built-in function implementations
static std::any callBuiltinFunction(const std::string& name,
                                   const std::vector<std::any>& args,
                                   const DSLEvaluator::EvaluationContext& context) {
    // ==================== TYPE CHECKING FUNCTIONS ====================

    // is_pointer_type(type)
    if (name == "is_pointer_type") {
        if (args.empty()) return false;
        std::string type = anyToString(args[0]);
        return type.find('*') != std::string::npos;
    }

    // is_array_type(type)
    if (name == "is_array_type") {
        if (args.empty()) return false;
        std::string type = anyToString(args[0]);
        return type.find('[') != std::string::npos || type.find("array") != std::string::npos;
    }

    // is_const(type)
    if (name == "is_const") {
        if (args.empty()) return false;
        std::string type = anyToString(args[0]);
        return type.find("const") != std::string::npos;
    }

    // is_volatile(type)
    if (name == "is_volatile") {
        if (args.empty()) return false;
        std::string type = anyToString(args[0]);
        return type.find("volatile") != std::string::npos;
    }

    // is_static(type)
    if (name == "is_static") {
        if (args.empty()) return false;
        std::string type = anyToString(args[0]);
        return type.find("static") != std::string::npos;
    }

    // is_unsigned(type)
    if (name == "is_unsigned") {
        if (args.empty()) return false;
        std::string type = anyToString(args[0]);
        return type.find("unsigned") != std::string::npos;
    }

    // is_signed(type)
    if (name == "is_signed") {
        if (args.empty()) return false;
        std::string type = anyToString(args[0]);
        std::string t = type;
        return t.find("signed") != std::string::npos ||
               (t.find("int") != std::string::npos && t.find("unsigned") == std::string::npos);
    }

    // is_integral(type)
    if (name == "is_integral") {
        if (args.empty()) return false;
        std::string type = anyToString(args[0]);
        return type.find("int") != std::string::npos ||
               type.find("char") != std::string::npos ||
               type.find("short") != std::string::npos ||
               type.find("long") != std::string::npos;
    }

    // is_floating(type)
    if (name == "is_floating") {
        if (args.empty()) return false;
        std::string type = anyToString(args[0]);
        return type.find("float") != std::string::npos ||
               type.find("double") != std::string::npos;
    }

    // is_void(type)
    if (name == "is_void") {
        if (args.empty()) return false;
        std::string type = anyToString(args[0]);
        return type == "void" || type.find("void") != std::string::npos;
    }

    // pointer_depth(type) - returns depth of pointer indirection
    if (name == "pointer_depth") {
        if (args.empty()) return 0;
        std::string type = anyToString(args[0]);
        return static_cast<int>(std::count(type.begin(), type.end(), '*'));
    }

    // ==================== STRING FUNCTIONS ====================

    // pattern_matches(text, pattern) - regex matching
    if (name == "pattern_matches") {
        if (args.size() < 2) return false;
        std::string text = anyToString(args[0]);
        std::string pattern = anyToString(args[1]);
        try {
            std::regex re(pattern);
            return std::regex_search(text, re);
        } catch (...) {
            // Fall back to substring matching
            return text.find(pattern) != std::string::npos;
        }
    }

    // contains(text, substring) - also works on collections
    if (name == "contains") {
        if (args.size() < 2) return false;

        // Check if first argument is a collection
        if (args[0].type() == typeid(std::vector<std::any>)) {
            auto collection = std::any_cast<std::vector<std::any>>(args[0]);
            std::string search_str = anyToString(args[1]);
            for (const auto& elem : collection) {
                if (anyToString(elem) == search_str) {
                    return true;
                }
            }
            return false;
        }

        // String contains
        std::string text = anyToString(args[0]);
        std::string substring = anyToString(args[1]);
        return text.find(substring) != std::string::npos;
    }

    // matches(text, pattern) - regex matching
    if (name == "matches") {
        if (args.size() < 2) return false;
        std::string text = anyToString(args[0]);
        std::string pattern = anyToString(args[1]);
        try {
            std::regex re(pattern);
            return std::regex_search(text, re);
        } catch (const std::regex_error& e) {
            spdlog::warn("Invalid regex pattern '{}': {}", pattern, e.what());
            return false;
        }
    }

    // size(collection_or_string) - works on both collections and strings
    if (name == "size") {
        if (args.empty()) return 0;

        // Check if it's a collection
        if (args[0].type() == typeid(std::vector<std::any>)) {
            auto collection = std::any_cast<std::vector<std::any>>(args[0]);
            return static_cast<int>(collection.size());
        }

        // Check if it's a string
        std::string text = anyToString(args[0]);
        return static_cast<int>(text.length());
    }

    // empty(collection_or_string) - checks if empty
    if (name == "empty") {
        if (args.empty()) return true;

        // Check if it's a collection
        if (args[0].type() == typeid(std::vector<std::any>)) {
            auto collection = std::any_cast<std::vector<std::any>>(args[0]);
            return collection.empty();
        }

        // Check if it's a string
        std::string text = anyToString(args[0]);
        return text.empty();
    }

    // starts_with(text, prefix)
    if (name == "starts_with" || name == "startsWith") {
        if (args.size() < 2) return false;
        std::string text = anyToString(args[0]);
        std::string prefix = anyToString(args[1]);
        return text.find(prefix) == 0;
    }

    // ends_with(text, suffix)
    if (name == "ends_with" || name == "endsWith") {
        if (args.size() < 2) return false;
        std::string text = anyToString(args[0]);
        std::string suffix = anyToString(args[1]);
        if (suffix.length() > text.length()) return false;
        return text.compare(text.length() - suffix.length(), suffix.length(), suffix) == 0;
    }

    // length(text)
    if (name == "length") {
        if (args.empty()) return 0;
        std::string text = anyToString(args[0]);
        return static_cast<int>(text.length());
    }

    // to_upper(text)
    if (name == "to_upper") {
        if (args.empty()) return std::string();
        std::string text = anyToString(args[0]);
        std::transform(text.begin(), text.end(), text.begin(), ::toupper);
        return text;
    }

    // to_lower(text)
    if (name == "to_lower") {
        if (args.empty()) return std::string();
        std::string text = anyToString(args[0]);
        std::transform(text.begin(), text.end(), text.begin(), ::tolower);
        return text;
    }

    // trim(text)
    if (name == "trim") {
        if (args.empty()) return std::string();
        std::string text = anyToString(args[0]);
        size_t start = text.find_first_not_of(" \t\n\r");
        size_t end = text.find_last_not_of(" \t\n\r");
        if (start == std::string::npos) return std::string();
        return text.substr(start, end - start + 1);
    }

    // replace(text, from, to)
    if (name == "replace") {
        if (args.size() < 3) return std::string();
        std::string text = anyToString(args[0]);
        std::string from = anyToString(args[1]);
        std::string to = anyToString(args[2]);
        size_t pos = 0;
        while ((pos = text.find(from, pos)) != std::string::npos) {
            text.replace(pos, from.length(), to);
            pos += to.length();
        }
        return text;
    }

    // split(text, delimiter) - returns count of parts
    if (name == "split") {
        if (args.size() < 2) return 0;
        std::string text = anyToString(args[0]);
        std::string delim = anyToString(args[1]);
        int count = 1;
        size_t pos = 0;
        while ((pos = text.find(delim, pos)) != std::string::npos) {
            count++;
            pos += delim.length();
        }
        return count;
    }

    // ==================== MATH FUNCTIONS ====================

    // abs(number)
    if (name == "abs") {
        if (args.empty()) return 0;
        int num = anyToInt(args[0]);
        return std::abs(num);
    }

    // min(a, b)
    if (name == "min") {
        if (args.size() < 2) return 0;
        int a = anyToInt(args[0]);
        int b = anyToInt(args[1]);
        return std::min(a, b);
    }

    // max(a, b)
    if (name == "max") {
        if (args.size() < 2) return 0;
        int a = anyToInt(args[0]);
        int b = anyToInt(args[1]);
        return std::max(a, b);
    }

    // clamp(value, min, max)
    if (name == "clamp") {
        if (args.size() < 3) return 0;
        int value = anyToInt(args[0]);
        int min_val = anyToInt(args[1]);
        int max_val = anyToInt(args[2]);
        return std::max(min_val, std::min(value, max_val));
    }

    // pow(base, exp)
    if (name == "pow") {
        if (args.size() < 2) return 0;
        int base = anyToInt(args[0]);
        int exp = anyToInt(args[1]);
        return static_cast<int>(std::pow(base, exp));
    }

    // ==================== COLLECTION FUNCTIONS ====================

    // count(collection)
    if (name == "count") {
        if (args.empty()) return 0;
        if (args[0].type() == typeid(std::vector<std::any>)) {
            auto vec = std::any_cast<std::vector<std::any>>(args[0]);
            return static_cast<int>(vec.size());
        }
        return 0;
    }

    // is_empty(collection)
    if (name == "is_empty") {
        if (args.empty()) return true;
        if (args[0].type() == typeid(std::vector<std::any>)) {
            auto vec = std::any_cast<std::vector<std::any>>(args[0]);
            return vec.empty();
        }
        return true;
    }

    // first(collection)
    if (name == "first") {
        if (args.empty()) return std::any();
        if (args[0].type() == typeid(std::vector<std::any>)) {
            auto vec = std::any_cast<std::vector<std::any>>(args[0]);
            if (!vec.empty()) return vec.front();
        }
        return std::any();
    }

    // last(collection)
    if (name == "last") {
        if (args.empty()) return std::any();
        if (args[0].type() == typeid(std::vector<std::any>)) {
            auto vec = std::any_cast<std::vector<std::any>>(args[0]);
            if (!vec.empty()) return vec.back();
        }
        return std::any();
    }

    // ==================== FUNCTION-SPECIFIC FUNCTIONS ====================

    // calls(function_name) - check if current function calls given function
    if (name == "calls") {
        if (args.empty() || !context.current_function) return false;
        std::string target = anyToString(args[0]);
        for (const auto& call : context.current_function->getCallSites()) {
            if (call.getName() == target || call.getQualifiedName() == target) {
                return true;
            }
        }
        return false;
    }

    // calls_function(pattern) - check if function calls any function matching regex pattern
    if (name == "calls_function") {
        if (args.empty() || !context.current_function) return false;
        std::string pattern = anyToString(args[0]);

        try {
            std::regex re(pattern);
            for (const auto& call : context.current_function->getCallSites()) {
                if (std::regex_match(call.getName(), re) ||
                    std::regex_match(call.getQualifiedName(), re)) {
                    return true;
                }
            }
        } catch (const std::regex_error&) {
            // Fall back to simple string matching if regex is invalid
            for (const auto& call : context.current_function->getCallSites()) {
                if (call.getName().find(pattern) != std::string::npos ||
                    call.getQualifiedName().find(pattern) != std::string::npos) {
                    return true;
                }
            }
        }
        return false;
    }

    // calls_any(list_of_names) - check if calls any function in list
    if (name == "calls_any") {
        if (args.empty() || !context.current_function) return false;
        if (args[0].type() == typeid(std::vector<std::any>)) {
            auto names = std::any_cast<std::vector<std::any>>(args[0]);
            for (const auto& call : context.current_function->getCallSites()) {
                for (const auto& name_any : names) {
                    std::string target = anyToString(name_any);
                    if (call.getName() == target || call.getQualifiedName() == target) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    // call_count(function_name) - count calls to specific function
    if (name == "call_count") {
        if (args.empty() || !context.current_function) return 0;
        std::string target = anyToString(args[0]);
        int count = 0;
        for (const auto& call : context.current_function->getCallSites()) {
            if (call.getName() == target || call.getQualifiedName() == target) {
                count++;
            }
        }
        return count;
    }

    // has_params() - check if function has parameters
    if (name == "has_params") {
        if (!context.current_function) return false;
        return !context.current_function->getParameters().empty();
    }

    // param_count() - get parameter count
    if (name == "param_count") {
        if (!context.current_function) return 0;
        return static_cast<int>(context.current_function->getParameters().size());
    }

    // has_return() - check if function has return statements
    if (name == "has_return") {
        if (!context.current_function) return false;
        return !context.current_function->getReturnStatements().empty();
    }

    // return_count() - count return statements
    if (name == "return_count") {
        if (!context.current_function) return 0;
        return static_cast<int>(context.current_function->getReturnStatements().size());
    }

    // returns(pattern) - check if any return matches pattern
    if (name == "returns") {
        if (args.empty() || !context.current_function) return false;
        std::string pattern = anyToString(args[0]);
        for (const auto& ret : context.current_function->getReturnStatements()) {
            if (ret.hasValue() && ret.getReturnExpression().find(pattern) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    // returns_null() - check if function returns NULL/nullptr
    if (name == "returns_null") {
        if (!context.current_function) return false;
        for (const auto& ret : context.current_function->getReturnStatements()) {
            if (ret.hasValue()) {
                const std::string& expr = ret.getReturnExpression();
                if (expr == "NULL" || expr == "nullptr" || expr == "0") {
                    return true;
                }
            }
        }
        return false;
    }

    // accesses_array(array_name) - check if function accesses specific array
    if (name == "accesses_array") {
        if (args.empty() || !context.current_function) return false;
        std::string arr_name = anyToString(args[0]);
        for (const auto& access : context.current_function->getArrayAccesses()) {
            if (access.getArrayName() == arr_name) {
                return true;
            }
        }
        return false;
    }

    // array_access_count(array_name) - count accesses to specific array
    if (name == "array_access_count") {
        if (args.empty() || !context.current_function) return 0;
        std::string arr_name = anyToString(args[0]);
        int count = 0;
        for (const auto& access : context.current_function->getArrayAccesses()) {
            if (access.getArrayName() == arr_name) {
                count++;
            }
        }
        return count;
    }

    // has_local_vars() - check if function has local variables
    if (name == "has_local_vars") {
        if (!context.current_function) return false;
        return !context.current_function->getLocalVariables().empty();
    }

    // local_var_count() - count local variables
    if (name == "local_var_count") {
        if (!context.current_function) return 0;
        return static_cast<int>(context.current_function->getLocalVariables().size());
    }

    // statement_count() - count statements in function
    if (name == "statement_count") {
        if (!context.current_function) return 0;
        return static_cast<int>(context.current_function->getStatements().size());
    }

    // ==================== NAMING CONVENTION FUNCTIONS ====================

    // is_snake_case(name)
    if (name == "is_snake_case") {
        if (args.empty()) return false;
        std::string text = anyToString(args[0]);
        return std::regex_match(text, std::regex("^[a-z][a-z0-9_]*$"));
    }

    // is_camel_case(name)
    if (name == "is_camel_case") {
        if (args.empty()) return false;
        std::string text = anyToString(args[0]);
        return std::regex_match(text, std::regex("^[a-z][a-zA-Z0-9]*$"));
    }

    // is_pascal_case(name)
    if (name == "is_pascal_case") {
        if (args.empty()) return false;
        std::string text = anyToString(args[0]);
        return std::regex_match(text, std::regex("^[A-Z][a-zA-Z0-9]*$"));
    }

    // is_upper_case(name)
    if (name == "is_upper_case") {
        if (args.empty()) return false;
        std::string text = anyToString(args[0]);
        return std::regex_match(text, std::regex("^[A-Z][A-Z0-9_]*$"));
    }

    // is_kebab_case(name)
    if (name == "is_kebab_case") {
        if (args.empty()) return false;
        std::string text = anyToString(args[0]);
        return std::regex_match(text, std::regex("^[a-z][a-z0-9-]*$"));
    }

    // ==================== LOGICAL/UTILITY FUNCTIONS ====================

    // between(value, min, max)
    if (name == "between") {
        if (args.size() < 3) return false;
        int val = anyToInt(args[0]);
        int min_val = anyToInt(args[1]);
        int max_val = anyToInt(args[2]);
        return val >= min_val && val <= max_val;
    }

    // one_of(value, list)
    if (name == "one_of") {
        if (args.size() < 2) return false;
        std::string val = anyToString(args[0]);
        if (args[1].type() == typeid(std::vector<std::any>)) {
            auto list = std::any_cast<std::vector<std::any>>(args[1]);
            for (const auto& item : list) {
                if (anyToString(item) == val) {
                    return true;
                }
            }
        }
        return false;
    }

    // not(value)
    if (name == "not") {
        if (args.empty()) return true;
        return !anyToBool(args[0]);
    }

    // ==================== MORE STRING FUNCTIONS ====================

    // substring(text, start, length)
    if (name == "substring") {
        if (args.size() < 2) return std::string();
        std::string text = anyToString(args[0]);
        int start = anyToInt(args[1]);
        if (start < 0 || start >= static_cast<int>(text.length())) return std::string();
        if (args.size() >= 3) {
            int length = anyToInt(args[2]);
            return text.substr(start, length);
        }
        return text.substr(start);
    }

    // index_of(text, substring)
    if (name == "index_of") {
        if (args.size() < 2) return -1;
        std::string text = anyToString(args[0]);
        std::string substring = anyToString(args[1]);
        size_t pos = text.find(substring);
        return pos != std::string::npos ? static_cast<int>(pos) : -1;
    }

    // last_index_of(text, substring)
    if (name == "last_index_of") {
        if (args.size() < 2) return -1;
        std::string text = anyToString(args[0]);
        std::string substring = anyToString(args[1]);
        size_t pos = text.rfind(substring);
        return pos != std::string::npos ? static_cast<int>(pos) : -1;
    }

    // char_at(text, index)
    if (name == "char_at") {
        if (args.size() < 2) return std::string();
        std::string text = anyToString(args[0]);
        int index = anyToInt(args[1]);
        if (index < 0 || index >= static_cast<int>(text.length())) return std::string();
        return std::string(1, text[index]);
    }

    // reverse(text)
    if (name == "reverse") {
        if (args.empty()) return std::string();
        std::string text = anyToString(args[0]);
        std::reverse(text.begin(), text.end());
        return text;
    }

    // repeat(text, count)
    if (name == "repeat") {
        if (args.size() < 2) return std::string();
        std::string text = anyToString(args[0]);
        int count = anyToInt(args[1]);
        std::string result;
        for (int i = 0; i < count; i++) {
            result += text;
        }
        return result;
    }

    // count_occurrences(text, substring)
    if (name == "count_occurrences") {
        if (args.size() < 2) return 0;
        std::string text = anyToString(args[0]);
        std::string substring = anyToString(args[1]);
        int count = 0;
        size_t pos = 0;
        while ((pos = text.find(substring, pos)) != std::string::npos) {
            count++;
            pos += substring.length();
        }
        return count;
    }

    // is_empty_or_whitespace(text)
    if (name == "is_empty_or_whitespace") {
        if (args.empty()) return true;
        std::string text = anyToString(args[0]);
        return text.find_first_not_of(" \t\n\r") == std::string::npos;
    }

    // ==================== MORE MATH FUNCTIONS ====================

    // sqrt(number)
    if (name == "sqrt") {
        if (args.empty()) return 0;
        double num = anyToDouble(args[0]);
        return static_cast<int>(std::sqrt(num));
    }

    // floor(number)
    if (name == "floor") {
        if (args.empty()) return 0;
        double num = anyToDouble(args[0]);
        return static_cast<int>(std::floor(num));
    }

    // ceil(number)
    if (name == "ceil") {
        if (args.empty()) return 0;
        double num = anyToDouble(args[0]);
        return static_cast<int>(std::ceil(num));
    }

    // round(number)
    if (name == "round") {
        if (args.empty()) return 0;
        double num = anyToDouble(args[0]);
        return static_cast<int>(std::round(num));
    }

    // mod(a, b)
    if (name == "mod") {
        if (args.size() < 2) return 0;
        int a = anyToInt(args[0]);
        int b = anyToInt(args[1]);
        if (b == 0) return 0;
        return a % b;
    }

    // gcd(a, b) - greatest common divisor
    if (name == "gcd") {
        if (args.size() < 2) return 0;
        int a = anyToInt(args[0]);
        int b = anyToInt(args[1]);
        while (b != 0) {
            int temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }

    // lcm(a, b) - least common multiple
    if (name == "lcm") {
        if (args.size() < 2) return 0;
        int a = anyToInt(args[0]);
        int b = anyToInt(args[1]);
        if (a == 0 || b == 0) return 0;
        return std::abs(a * b) / std::abs(std::__gcd(a, b));
    }

    // is_even(number)
    if (name == "is_even") {
        if (args.empty()) return false;
        int num = anyToInt(args[0]);
        return num % 2 == 0;
    }

    // is_odd(number)
    if (name == "is_odd") {
        if (args.empty()) return false;
        int num = anyToInt(args[0]);
        return num % 2 != 0;
    }

    // is_prime(number)
    if (name == "is_prime") {
        if (args.empty()) return false;
        int num = anyToInt(args[0]);
        if (num < 2) return false;
        if (num == 2) return true;
        if (num % 2 == 0) return false;
        for (int i = 3; i * i <= num; i += 2) {
            if (num % i == 0) return false;
        }
        return true;
    }

    // is_power_of_two(number)
    if (name == "is_power_of_two") {
        if (args.empty()) return false;
        int num = anyToInt(args[0]);
        return num > 0 && (num & (num - 1)) == 0;
    }

    // factorial(n)
    if (name == "factorial") {
        if (args.empty()) return 0;
        int n = anyToInt(args[0]);
        if (n < 0 || n > 20) return 0; // Prevent overflow
        int result = 1;
        for (int i = 2; i <= n; i++) {
            result *= i;
        }
        return result;
    }

    // ==================== ADVANCED FUNCTION ANALYSIS ====================

    // has_recursion() - check if function is recursive
    if (name == "has_recursion") {
        if (!context.current_function) return false;
        const std::string& func_name = context.current_function->getName();
        for (const auto& call : context.current_function->getCallSites()) {
            if (call.getName() == func_name || call.getQualifiedName().find(func_name) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    // has_goto() - check if function uses goto
    if (name == "has_goto") {
        if (!context.current_function) return false;
        for (const auto& stmt : context.current_function->getStatements()) {
            if (stmt.getText().find("goto") != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    // has_switch() - check if function uses switch
    if (name == "has_switch") {
        if (!context.current_function) return false;
        for (const auto& stmt : context.current_function->getStatements()) {
            if (stmt.getText().find("switch") != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    // has_loop() - check if function has any loop
    if (name == "has_loop") {
        if (!context.current_function) return false;
        for (const auto& stmt : context.current_function->getStatements()) {
            const std::string& text = stmt.getText();
            if (text.find("for") != std::string::npos ||
                text.find("while") != std::string::npos ||
                text.find("do") != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    // loop_count() - count loops in function
    if (name == "loop_count") {
        if (!context.current_function) return 0;
        int count = 0;
        for (const auto& stmt : context.current_function->getStatements()) {
            if (stmt.getType() == Statement::FOR_STMT || stmt.getType() == Statement::WHILE_STMT) {
                count++;
            }
        }
        return count;
    }

    // if_count() - count if statements
    if (name == "if_count") {
        if (!context.current_function) return 0;
        int count = 0;
        for (const auto& stmt : context.current_function->getStatements()) {
            if (stmt.getType() == Statement::IF_STMT) {
                count++;
            }
        }
        return count;
    }

    // max_nesting_depth() - estimate maximum nesting depth
    if (name == "max_nesting_depth") {
        if (!context.current_function) return 0;
        int max_depth = 0;
        int current_depth = 0;
        for (const auto& stmt : context.current_function->getStatements()) {
            const std::string& text = stmt.getText();
            current_depth += std::count(text.begin(), text.end(), '{');
            current_depth -= std::count(text.begin(), text.end(), '}');
            max_depth = std::max(max_depth, current_depth);
        }
        return max_depth;
    }

    // uses_malloc() - check if function uses malloc/calloc/realloc
    if (name == "uses_malloc") {
        if (!context.current_function) return false;
        for (const auto& call : context.current_function->getCallSites()) {
            const std::string& name = call.getName();
            if (name == "malloc" || name == "calloc" || name == "realloc") {
                return true;
            }
        }
        return false;
    }

    // uses_free() - check if function uses free
    if (name == "uses_free") {
        if (!context.current_function) return false;
        for (const auto& call : context.current_function->getCallSites()) {
            if (call.getName() == "free") {
                return true;
            }
        }
        return false;
    }

    // has_memory_leak_risk() - check malloc without free
    if (name == "has_memory_leak_risk") {
        if (!context.current_function) return false;
        bool has_malloc = false;
        bool has_free = false;
        for (const auto& call : context.current_function->getCallSites()) {
            const std::string& name = call.getName();
            if (name == "malloc" || name == "calloc" || name == "realloc") {
                has_malloc = true;
            }
            if (name == "free") {
                has_free = true;
            }
        }
        return has_malloc && !has_free;
    }

    // ==================== PARAMETER ANALYSIS ====================

    // has_pointer_param() - check if any parameter is a pointer
    if (name == "has_pointer_param") {
        if (!context.current_function) return false;
        for (const auto& param : context.current_function->getParameters()) {
            if (param.getType().isPointer()) {
                return true;
            }
        }
        return false;
    }

    // pointer_param_count() - count pointer parameters
    if (name == "pointer_param_count") {
        if (!context.current_function) return 0;
        int count = 0;
        for (const auto& param : context.current_function->getParameters()) {
            if (param.getType().isPointer()) {
                count++;
            }
        }
        return count;
    }

    // has_const_param() - check if any parameter is const
    if (name == "has_const_param") {
        if (!context.current_function) return false;
        for (const auto& param : context.current_function->getParameters()) {
            if (param.getType().isConst()) {
                return true;
            }
        }
        return false;
    }

    // all_params_const() - check if all parameters are const
    if (name == "all_params_const") {
        if (!context.current_function) return false;
        const auto& params = context.current_function->getParameters();
        if (params.empty()) return false;
        for (const auto& param : params) {
            if (!param.getType().isConst()) {
                return false;
            }
        }
        return true;
    }

    // ==================== VARIABLE ANALYSIS ====================

    // has_uninitialized_var() - check for uninitialized variables
    if (name == "has_uninitialized_var") {
        if (!context.current_function) return false;
        for (const auto& var : context.current_function->getLocalVariables()) {
            if (!var.isInitialized()) {
                return true;
            }
        }
        return false;
    }

    // uninitialized_var_count() - count uninitialized variables
    if (name == "uninitialized_var_count") {
        if (!context.current_function) return 0;
        int count = 0;
        for (const auto& var : context.current_function->getLocalVariables()) {
            if (!var.isInitialized()) {
                count++;
            }
        }
        return count;
    }

    // has_static_var() - check for static local variables
    if (name == "has_static_var") {
        if (!context.current_function) return false;
        for (const auto& var : context.current_function->getLocalVariables()) {
            if (var.isStatic()) {
                return true;
            }
        }
        return false;
    }

    // static_var_count() - count static local variables
    if (name == "static_var_count") {
        if (!context.current_function) return 0;
        int count = 0;
        for (const auto& var : context.current_function->getLocalVariables()) {
            if (var.isStatic()) {
                count++;
            }
        }
        return count;
    }

    // ==================== CODE SMELL DETECTION ====================

    // has_magic_number() - check for magic numbers (numbers > 1 that aren't common)
    if (name == "has_magic_number") {
        if (!context.current_function) return false;
        for (const auto& stmt : context.current_function->getStatements()) {
            const std::string& text = stmt.getText();
            // Simple heuristic: look for numbers that aren't 0, 1, -1
            std::regex num_regex("\\b([2-9]\\d+)\\b");
            if (std::regex_search(text, num_regex)) {
                return true;
            }
        }
        return false;
    }

    // has_long_line() - check if any statement is very long
    if (name == "has_long_line") {
        if (!context.current_function) return false;
        int threshold = args.empty() ? 80 : anyToInt(args[0]);
        for (const auto& stmt : context.current_function->getStatements()) {
            if (static_cast<int>(stmt.getText().length()) > threshold) {
                return true;
            }
        }
        return false;
    }

    // max_line_length() - get maximum line length
    if (name == "max_line_length") {
        if (!context.current_function) return 0;
        int max_len = 0;
        for (const auto& stmt : context.current_function->getStatements()) {
            max_len = std::max(max_len, static_cast<int>(stmt.getText().length()));
        }
        return max_len;
    }

    // has_todo_comment() - check for TODO comments
    if (name == "has_todo_comment") {
        if (!context.current_function) return false;
        for (const auto& stmt : context.current_function->getStatements()) {
            const std::string& text = stmt.getText();
            if (text.find("TODO") != std::string::npos || text.find("FIXME") != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    // ==================== RETURN VALUE ANALYSIS ====================

    // all_returns_same_type() - check if all returns have consistent type
    if (name == "all_returns_same_type") {
        if (!context.current_function) return true;
        const auto& rets = context.current_function->getReturnStatements();
        if (rets.empty()) return true;
        bool first_has_value = rets[0].hasValue();
        for (size_t i = 1; i < rets.size(); i++) {
            if (rets[i].hasValue() != first_has_value) {
                return false;
            }
        }
        return true;
    }

    // has_early_return() - check for early returns (before last statement)
    if (name == "has_early_return") {
        if (!context.current_function) return false;
        return context.current_function->getReturnStatements().size() > 1;
    }

    // returns_constant() - check if all returns are constant values
    if (name == "returns_constant") {
        if (!context.current_function) return false;
        for (const auto& ret : context.current_function->getReturnStatements()) {
            if (!ret.hasValue()) continue;
            const std::string& expr = ret.getReturnExpression();
            // Simple check: if it contains operators or function calls, it's not constant
            if (expr.find('(') != std::string::npos ||
                expr.find('+') != std::string::npos ||
                expr.find('-') != std::string::npos ||
                expr.find('*') != std::string::npos) {
                return false;
            }
        }
        return true;
    }

    // ==================== CALL ANALYSIS ====================

    // total_call_count() - total number of function calls
    if (name == "total_call_count") {
        if (!context.current_function) return 0;
        return static_cast<int>(context.current_function->getCallSites().size());
    }

    // unique_call_count() - count of unique functions called
    if (name == "unique_call_count") {
        if (!context.current_function) return 0;
        std::set<std::string> unique_names;
        for (const auto& call : context.current_function->getCallSites()) {
            unique_names.insert(call.getName());
        }
        return static_cast<int>(unique_names.size());
    }

    // calls_printf_family() - check for printf/fprintf/sprintf calls
    if (name == "calls_printf_family") {
        if (!context.current_function) return false;
        for (const auto& call : context.current_function->getCallSites()) {
            const std::string& name = call.getName();
            if (name.find("printf") != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    // calls_unsafe_functions() - check for unsafe C functions
    if (name == "calls_unsafe_functions") {
        if (!context.current_function) return false;
        const std::vector<std::string> unsafe = {"strcpy", "strcat", "sprintf", "gets", "scanf"};
        for (const auto& call : context.current_function->getCallSites()) {
            for (const auto& unsafe_name : unsafe) {
                if (call.getName() == unsafe_name) {
                    return true;
                }
            }
        }
        return false;
    }

    // ==================== UTILITY FUNCTIONS ====================

    // default(value, default_value) - return default if value is empty/null
    if (name == "default") {
        if (args.size() < 2) return std::any();
        const auto& val = args[0];
        if (val.type() == typeid(std::string)) {
            std::string s = std::any_cast<std::string>(val);
            if (s.empty()) return args[1];
        }
        return val;
    }

    // coalesce(...) - return first non-empty value
    if (name == "coalesce") {
        for (const auto& arg : args) {
            if (arg.type() == typeid(std::string)) {
                std::string s = std::any_cast<std::string>(arg);
                if (!s.empty()) return s;
            } else if (arg.type() == typeid(int)) {
                int i = std::any_cast<int>(arg);
                if (i != 0) return i;
            } else if (arg.type() == typeid(bool)) {
                bool b = std::any_cast<bool>(arg);
                if (b) return b;
            }
        }
        return std::any();
    }

    // if_else(condition, true_value, false_value)
    if (name == "if_else") {
        if (args.size() < 3) return std::any();
        bool condition = anyToBool(args[0]);
        return condition ? args[1] : args[2];
    }

    // ==================== CONTROL FLOW & DATA FLOW ANALYSIS ====================

    // has_cfg() - check if function has control flow graph
    if (name == "has_cfg") {
        if (!context.current_function) return false;
        return context.current_function->getCFG() != nullptr;
    }

    // cfg_block_count() - get number of basic blocks in CFG
    if (name == "cfg_block_count") {
        if (!context.current_function) return 0;
        auto* cfg = context.current_function->getCFG();
        if (!cfg) return 0;
        return static_cast<int>(cfg->getBlocks().size());
    }

    // cfg_has_loops() - check if CFG contains loops (back edges)
    if (name == "cfg_has_loops") {
        if (!context.current_function) return false;
        auto* cfg = context.current_function->getCFG();
        if (!cfg) return false;

        // A loop exists if there's a back edge (edge to a dominator)
        for (const auto& block : cfg->getBlocks()) {
            for (unsigned succ_id : block->getSuccessors()) {
                if (succ_id < cfg->getBlocks().size()) {
                    const auto* succ = cfg->getBlocks()[succ_id].get();
                    if (cfg->dominates(succ, block.get())) {
                        return true;  // Back edge found
                    }
                }
            }
        }
        return false;
    }

    // cfg_loop_count() - estimate number of loops
    if (name == "cfg_loop_count") {
        if (!context.current_function) return 0;
        auto* cfg = context.current_function->getCFG();
        if (!cfg) return 0;

        int loop_count = 0;
        for (const auto& block : cfg->getBlocks()) {
            for (unsigned succ_id : block->getSuccessors()) {
                if (succ_id < cfg->getBlocks().size()) {
                    const auto* succ = cfg->getBlocks()[succ_id].get();
                    if (cfg->dominates(succ, block.get())) {
                        loop_count++;
                    }
                }
            }
        }
        return loop_count;
    }

    // cfg_max_nesting() - estimate maximum nesting depth
    if (name == "cfg_max_nesting") {
        if (!context.current_function) return 0;
        auto* cfg = context.current_function->getCFG();
        if (!cfg) return 0;

        // Use dominator tree depth as approximation
        const auto& idom = cfg->getImmediateDominators();
        int max_depth = 0;

        for (const auto& block : cfg->getBlocks()) {
            int depth = 0;
            unsigned current = block->getId();
            while (idom.find(current) != idom.end()) {
                depth++;
                current = idom.at(current);
            }
            max_depth = std::max(max_depth, depth);
        }
        return max_depth;
    }

    // variable_is_defined(var_name) - check if variable has definition
    if (name == "variable_is_defined") {
        if (args.empty() || !context.current_function) return false;
        std::string var_name = anyToString(args[0]);
        auto* cfg = context.current_function->getCFG();
        if (!cfg) return false;

        auto def_sites = cfg->getDefSites(var_name);
        return !def_sites.empty();
    }

    // variable_is_used(var_name) - check if variable is used
    if (name == "variable_is_used") {
        if (args.empty() || !context.current_function) return false;
        std::string var_name = anyToString(args[0]);
        auto* cfg = context.current_function->getCFG();
        if (!cfg) return false;

        auto use_sites = cfg->getUseSites(var_name);
        return !use_sites.empty();
    }

    // variable_def_count(var_name) - count definitions of variable
    if (name == "variable_def_count") {
        if (args.empty() || !context.current_function) return 0;
        std::string var_name = anyToString(args[0]);
        auto* cfg = context.current_function->getCFG();
        if (!cfg) return 0;

        auto def_sites = cfg->getDefSites(var_name);
        return static_cast<int>(def_sites.size());
    }

    // variable_use_count(var_name) - count uses of variable
    if (name == "variable_use_count") {
        if (args.empty() || !context.current_function) return 0;
        std::string var_name = anyToString(args[0]);
        auto* cfg = context.current_function->getCFG();
        if (!cfg) return 0;

        auto use_sites = cfg->getUseSites(var_name);
        return static_cast<int>(use_sites.size());
    }

    // has_dead_code() - check for unreachable code
    if (name == "has_dead_code") {
        if (!context.current_function) return false;
        auto* cfg = context.current_function->getCFG();
        if (!cfg) return false;

        // A block is dead code if it's not reachable from entry
        // Simple check: block has no predecessors and is not entry
        for (const auto& block : cfg->getBlocks()) {
            if (!block->isEntry() && block->getPredecessors().empty()) {
                return true;
            }
        }
        return false;
    }

    // has_unreachable_return() - check for return statements in dead code
    if (name == "has_unreachable_return") {
        if (!context.current_function) return false;
        auto* cfg = context.current_function->getCFG();
        if (!cfg) return false;

        for (const auto& block : cfg->getBlocks()) {
            if (!block->isEntry() && block->getPredecessors().empty()) {
                // Check if this dead block has a return
                for (const auto& stmt : block->getStatements()) {
                    if (stmt.getText().find("return") != std::string::npos) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    // has_undefined_variable() - check for variables used before definition
    if (name == "has_undefined_variable") {
        if (!context.current_function) return false;
        auto* cfg = context.current_function->getCFG();
        if (!cfg) return false;

        // Compute reaching definitions first
        const_cast<ControlFlowGraph*>(cfg)->computeReachingDefinitions();

        // Check each block for uses of undefined variables
        for (const auto& block : cfg->getBlocks()) {
            auto reaching = cfg->getReachingDefinitions(block->getId());
            for (const auto& stmt : block->getStatements()) {
                // Extract variable uses (simplified)
                std::string text = stmt.getText();
                std::string var;
                for (char c : text) {
                    if (std::isalnum(c) || c == '_') {
                        var += c;
                    } else {
                        if (!var.empty() && reaching.count(var) == 0) {
                            // Variable used but no definition reaches here
                            return true;
                        }
                        var.clear();
                    }
                }
            }
        }
        return false;
    }

    // has_live_variable(var_name) - check if variable is live at exit
    if (name == "has_live_variable") {
        if (args.empty() || !context.current_function) return false;
        std::string var_name = anyToString(args[0]);
        auto* cfg = context.current_function->getCFG();
        if (!cfg) return false;

        // Compute live variables
        const_cast<ControlFlowGraph*>(cfg)->computeLiveVariables();

        // Check if variable is live at any exit block
        for (const auto* exit_block : cfg->getExitBlocks()) {
            auto live_vars = cfg->getLiveVariablesOut(exit_block->getId());
            if (live_vars.count(var_name) > 0) {
                return true;
            }
        }
        return false;
    }

    // ==================== PATTERN MATCHING ====================

    // matches_pattern(pattern_lines...) - check if function matches multi-line pattern
    if (name == "matches_pattern") {
        if (args.empty() || !context.current_function) return false;

        using namespace nomic::patterns;

        // Build pattern from arguments
        MultiLinePattern pattern;
        for (const auto& arg : args) {
            pattern.lines.push_back(anyToString(arg));
        }

        // Get function statements
        const auto& statements = context.current_function->getStatements();

        // Create pattern matcher and find matches
        MultiLinePatternMatcher matcher;
        auto matches = matcher.findMatches(pattern, statements);

        return !matches.empty();
    }

    // fuzzy_match(min_similarity, pattern_lines...) - fuzzy pattern matching
    if (name == "fuzzy_match") {
        if (args.size() < 2 || !context.current_function) return false;

        using namespace nomic::patterns;

        float min_similarity = 0.7f;
        if (args[0].type() == typeid(double)) {
            min_similarity = static_cast<float>(std::any_cast<double>(args[0]));
        } else if (args[0].type() == typeid(int)) {
            min_similarity = static_cast<float>(std::any_cast<int>(args[0])) / 100.0f;
        }

        // Build pattern from remaining arguments
        MultiLinePattern pattern;
        for (size_t i = 1; i < args.size(); ++i) {
            pattern.lines.push_back(anyToString(args[i]));
        }

        const auto& statements = context.current_function->getStatements();
        MultiLinePatternMatcher matcher;
        auto matches = matcher.findFuzzyMatches(pattern, statements, min_similarity);

        return !matches.empty();
    }

    // wildcard_match(pattern_lines...) - wildcard pattern matching (* and ?)
    if (name == "wildcard_match") {
        if (args.empty() || !context.current_function) return false;

        using namespace nomic::patterns;

        MultiLinePattern pattern;
        pattern.enable_wildcards = true;
        for (const auto& arg : args) {
            pattern.lines.push_back(anyToString(arg));
        }

        const auto& statements = context.current_function->getStatements();
        MultiLinePatternMatcher matcher;
        auto matches = matcher.findWildcardMatches(pattern, statements);

        return !matches.empty();
    }

    // structural_match(pattern_lines...) - structural matching (ignores variable names)
    if (name == "structural_match") {
        if (args.empty() || !context.current_function) return false;

        using namespace nomic::patterns;

        MultiLinePattern pattern;
        for (const auto& arg : args) {
            pattern.lines.push_back(anyToString(arg));
        }

        const auto& statements = context.current_function->getStatements();
        MultiLinePatternMatcher matcher;
        auto matches = matcher.findStructuralMatches(pattern, statements);

        return !matches.empty();
    }

    // unordered_match(pattern_lines...) - match lines in any order
    if (name == "unordered_match") {
        if (args.empty() || !context.current_function) return false;

        using namespace nomic::patterns;

        MultiLinePattern pattern;
        for (const auto& arg : args) {
            pattern.lines.push_back(anyToString(arg));
        }

        const auto& statements = context.current_function->getStatements();
        MultiLinePatternMatcher matcher;
        auto matches = matcher.findUnorderedMatches(pattern, statements);

        return !matches.empty();
    }

    // gap_tolerant_match(max_gap, pattern_lines...) - allow gaps between pattern lines
    if (name == "gap_tolerant_match") {
        if (args.size() < 2 || !context.current_function) return false;

        using namespace nomic::patterns;

        unsigned max_gap = 5;
        if (args[0].type() == typeid(int)) {
            max_gap = static_cast<unsigned>(std::any_cast<int>(args[0]));
        }

        MultiLinePattern pattern;
        for (size_t i = 1; i < args.size(); ++i) {
            pattern.lines.push_back(anyToString(args[i]));
        }

        const auto& statements = context.current_function->getStatements();
        MultiLinePatternMatcher matcher;
        auto matches = matcher.findGapTolerantMatches(pattern, statements, max_gap);

        return !matches.empty();
    }

    // pattern_match_count(pattern_lines...) - count pattern matches
    if (name == "pattern_match_count") {
        if (args.empty() || !context.current_function) return 0;

        using namespace nomic::patterns;

        MultiLinePattern pattern;
        for (const auto& arg : args) {
            pattern.lines.push_back(anyToString(arg));
        }

        const auto& statements = context.current_function->getStatements();
        MultiLinePatternMatcher matcher;
        auto matches = matcher.findMatches(pattern, statements);

        return static_cast<int>(matches.size());
    }

    // has_code_smell(smell_type) - detect common code smells using patterns
    if (name == "has_code_smell") {
        if (args.empty() || !context.current_function) return false;

        using namespace nomic::patterns;

        std::string smell_type = anyToString(args[0]);
        const auto& statements = context.current_function->getStatements();
        MultiLinePatternMatcher matcher;

        if (smell_type == "god_class" || smell_type == "long_method") {
            // Long method: more than 50 statements
            return statements.size() > 50;
        }
        else if (smell_type == "duplicate_code") {
            // Look for repeated patterns
            std::map<std::string, int> pattern_counts;
            for (size_t i = 0; i < statements.size() - 2; ++i) {
                std::string pattern = statements[i].getText() + "|" +
                                    statements[i+1].getText() + "|" +
                                    statements[i+2].getText();
                pattern_counts[pattern]++;
            }
            for (const auto& [pattern, count] : pattern_counts) {
                if (count > 1) return true;
            }
            return false;
        }
        else if (smell_type == "arrow_code") {
            // Deep nesting - look for multiple levels of indentation
            MultiLinePattern pattern;
            pattern.lines = {"if (*) {", "if (*) {", "if (*) {"};
            pattern.enable_wildcards = true;
            auto matches = matcher.findWildcardMatches(pattern, statements);
            return !matches.empty();
        }
        else if (smell_type == "magic_numbers") {
            // Numbers in code (not 0, 1, -1)
            for (const auto& stmt : statements) {
                std::string text = stmt.getText();
                std::regex num_regex(R"(\b[2-9]\d*\b)");
                if (std::regex_search(text, num_regex)) {
                    return true;
                }
            }
            return false;
        }

        return false;
    }

    // extract_pattern_instances(pattern_lines...) - extract all matching code
    if (name == "extract_pattern_instances") {
        if (args.empty() || !context.current_function) return std::vector<std::string>();

        using namespace nomic::patterns;

        MultiLinePattern pattern;
        for (const auto& arg : args) {
            pattern.lines.push_back(anyToString(arg));
        }

        const auto& statements = context.current_function->getStatements();
        MultiLinePatternMatcher matcher;
        auto instances = matcher.extractPatternInstances(pattern, statements);

        // Return count for now (could extend to return actual instances)
        return static_cast<int>(instances.size());
    }

    // ==================== TAINT ANALYSIS FUNCTIONS ====================

    if (name == "is_taint_source") {
        // Check if a function is a known taint source
        // Usage: is_taint_source("scanf")
        if (args.empty()) return false;
        std::string func_name = anyToString(args[0]);
        return nomic::analysis::taint::isKnownSource(func_name);
    }

    if (name == "is_taint_sink") {
        // Check if a function is a known taint sink
        // Usage: is_taint_sink("system")
        if (args.empty()) return false;
        std::string func_name = anyToString(args[0]);
        return nomic::analysis::taint::isKnownSink(func_name);
    }

    if (name == "is_sanitizer") {
        // Check if a function is a known sanitizer
        // Usage: is_sanitizer("escape_html")
        if (args.empty()) return false;
        std::string func_name = anyToString(args[0]);
        return nomic::analysis::taint::isKnownSanitizer(func_name);
    }

    if (name == "has_taint_source") {
        // Check if function calls any taint source
        // Usage: has_taint_source()
        if (!context.current_function) return false;

        for (const auto& call : context.current_function->getCallSites()) {
            if (nomic::analysis::taint::isKnownSource(call.getName())) {
                return true;
            }
        }
        return false;
    }

    if (name == "has_taint_sink") {
        // Check if function calls any taint sink
        // Usage: has_taint_sink()
        if (!context.current_function) return false;

        for (const auto& call : context.current_function->getCallSites()) {
            if (nomic::analysis::taint::isKnownSink(call.getName())) {
                return true;
            }
        }
        return false;
    }

    if (name == "taint_source_count") {
        // Count taint sources in function
        // Usage: taint_source_count()
        if (!context.current_function) return 0;

        int count = 0;
        for (const auto& call : context.current_function->getCallSites()) {
            if (nomic::analysis::taint::isKnownSource(call.getName())) {
                count++;
            }
        }
        return count;
    }

    if (name == "taint_sink_count") {
        // Count taint sinks in function
        // Usage: taint_sink_count()
        if (!context.current_function) return 0;

        int count = 0;
        for (const auto& call : context.current_function->getCallSites()) {
            if (nomic::analysis::taint::isKnownSink(call.getName())) {
                count++;
            }
        }
        return count;
    }

    if (name == "has_taint_flow") {
        // Check if function has taint flow from source to sink
        // Usage: has_taint_flow()
        if (!context.current_function) return false;

        using namespace nomic::analysis;

        TaintAnalyzer analyzer;
        analyzer.analyzeFunction(*context.current_function);
        auto flows = analyzer.getTaintFlows();

        return !flows.empty();
    }

    if (name == "taint_flow_count") {
        // Count taint flows in function
        // Usage: taint_flow_count()
        if (!context.current_function) return 0;

        using namespace nomic::analysis;

        TaintAnalyzer analyzer;
        analyzer.analyzeFunction(*context.current_function);
        auto flows = analyzer.getTaintFlows();

        return static_cast<int>(flows.size());
    }

    if (name == "has_unsanitized_taint") {
        // Check if function has unsanitized taint flow
        // Usage: has_unsanitized_taint()
        if (!context.current_function) return false;

        using namespace nomic::analysis;

        TaintAnalyzer analyzer;
        analyzer.analyzeFunction(*context.current_function);
        auto flows = analyzer.getTaintFlows();

        for (const auto& flow : flows) {
            if (!flow.isSanitized()) {
                return true;
            }
        }
        return false;
    }

    if (name == "get_taint_sources") {
        // Get list of taint source types in function
        // Usage: get_taint_sources()
        if (!context.current_function) return std::vector<std::string>();

        using namespace nomic::analysis;

        TaintAnalyzer analyzer;
        analyzer.analyzeFunction(*context.current_function);
        auto sources = analyzer.getTaintSources();

        std::vector<std::string> source_types;
        for (const auto& source : sources) {
            source_types.push_back(taint::sourceTypeToString(source.getSourceType()));
        }
        return source_types;
    }

    if (name == "get_taint_sinks") {
        // Get list of taint sink types in function
        // Usage: get_taint_sinks()
        if (!context.current_function) return std::vector<std::string>();

        using namespace nomic::analysis;

        TaintAnalyzer analyzer;
        analyzer.analyzeFunction(*context.current_function);
        auto sinks = analyzer.getTaintSinks();

        std::vector<std::string> sink_types;
        for (const auto& sink : sinks) {
            sink_types.push_back(taint::sinkTypeToString(sink.getSinkType()));
        }
        return sink_types;
    }

    if (name == "has_command_injection_risk") {
        // Check if function has command injection risk
        // Usage: has_command_injection_risk()
        if (!context.current_function) return false;

        using namespace nomic::analysis;

        TaintAnalyzer analyzer;
        analyzer.analyzeFunction(*context.current_function);
        auto flows = analyzer.getTaintFlows();

        for (const auto& flow : flows) {
            if (flow.getSink().getSinkType() == TaintSinkType::COMMAND_EXECUTION &&
                !flow.isSanitized()) {
                return true;
            }
        }
        return false;
    }

    if (name == "has_sql_injection_risk") {
        // Check if function has SQL injection risk
        // Usage: has_sql_injection_risk()
        if (!context.current_function) return false;

        using namespace nomic::analysis;

        TaintAnalyzer analyzer;
        analyzer.analyzeFunction(*context.current_function);
        auto flows = analyzer.getTaintFlows();

        for (const auto& flow : flows) {
            if (flow.getSink().getSinkType() == TaintSinkType::SQL_QUERY &&
                !flow.isSanitized()) {
                return true;
            }
        }
        return false;
    }

    if (name == "has_format_string_risk") {
        // Check if function has format string vulnerability risk
        // Usage: has_format_string_risk()
        if (!context.current_function) return false;

        using namespace nomic::analysis;

        TaintAnalyzer analyzer;
        analyzer.analyzeFunction(*context.current_function);
        auto flows = analyzer.getTaintFlows();

        for (const auto& flow : flows) {
            if (flow.getSink().getSinkType() == TaintSinkType::FORMAT_STRING &&
                !flow.isSanitized()) {
                return true;
            }
        }
        return false;
    }

    // ==================== AST QUERY FUNCTIONS ====================

    if (name == "ast_query") {
        // Simplified AST query implementation using pattern matching
        // Usage: ast_query("macro_expansion[@name='FUNCTION_ENTER']")
        if (args.empty() || !context.current_function) return false;

        std::string query_str = anyToString(args[0]);

        // Pattern matching for common HeliOS coding standard queries
        const auto& stmts = context.current_function->getStatements();

        // Check for macro expansions
        if (query_str.find("macro_expansion[@name='FUNCTION_ENTER']") != std::string::npos) {
            for (const auto& stmt : stmts) {
                if (stmt.getText().find("FUNCTION_ENTER") != std::string::npos) {
                    return true;
                }
            }
            return false;
        }

        if (query_str.find("macro_expansion[@name='FUNCTION_EXIT']") != std::string::npos) {
            for (const auto& stmt : stmts) {
                if (stmt.getText().find("FUNCTION_EXIT") != std::string::npos) {
                    return true;
                }
            }
            return false;
        }

        if (query_str.find("macro_expansion[@name='__ReturnOk__']") != std::string::npos) {
            for (const auto& stmt : stmts) {
                if (stmt.getText().find("__ReturnOk__") != std::string::npos) {
                    return true;
                }
            }
            return false;
        }

        // Check for binary operators (assignments)
        if (query_str.find("binary_operator") != std::string::npos) {
            for (const auto& stmt : stmts) {
                const std::string& text = stmt.getText();

                // ret = assignments
                if (query_str.find("left[@name='ret']") != std::string::npos) {
                    if (text.find("ret") != std::string::npos && text.find("=") != std::string::npos) {
                        // Check if it's not in a macro
                        if (query_str.find("!ancestor::macro_expansion") != std::string::npos) {
                            // If text doesn't contain macro indicators, it's a direct assignment
                            if (text.find("FUNCTION_") == std::string::npos &&
                                text.find("__Return") == std::string::npos) {
                                return true;
                            }
                        } else {
                            return true;
                        }
                    }
                }

                // Check for ret = ReturnOK
                if (query_str.find("right[@name='ReturnOK']") != std::string::npos ||
                    query_str.find("right[@name='ReturnError']") != std::string::npos) {
                    if ((text.find("ret") != std::string::npos && text.find("ReturnOK") != std::string::npos) ||
                        (text.find("ret") != std::string::npos && text.find("ReturnError") != std::string::npos)) {
                        return true;
                    }
                }

                // Check for valid field assignments
                if (query_str.find("member_access[@member='valid']") != std::string::npos) {
                    if (text.find("->valid") != std::string::npos || text.find(".valid") != std::string::npos) {
                        if (query_str.find("right[@name='VALID']") != std::string::npos && text.find("VALID") != std::string::npos) {
                            return true;
                        }
                        if (query_str.find("right[@name='INVALID']") != std::string::npos && text.find("INVALID") != std::string::npos) {
                            return true;
                        }
                    }
                }
            }
        }

        // Check for identifiers
        if (query_str.find("identifier[@name='NULL']") != std::string::npos) {
            for (const auto& stmt : stmts) {
                if (stmt.getText().find("NULL") != std::string::npos) {
                    return true;
                }
            }
            return false;
        }

        // Check for for statements with cursor
        if (query_str.find("for_stmt") != std::string::npos) {
            for (const auto& stmt : stmts) {
                if (stmt.getType() == Statement::FOR_STMT) {
                    if (query_str.find("cursor") != std::string::npos) {
                        if (stmt.getText().find("cursor") != std::string::npos) {
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        // Check for variable declarations
        if (query_str.find("var_decl") != std::string::npos) {
            for (const auto& var : context.current_function->getLocalVariables()) {
                if (query_str.find("[@name='cursor']") != std::string::npos) {
                    if (var.getName() == "cursor") {
                        // Check initialization
                        if (query_str.find("[@initializer='null']") != std::string::npos) {
                            // Check if initialized to null
                            const auto& init_val = var.getInitialValue();
                            if (init_val.has_value() &&
                                (init_val.value().find("null") != std::string::npos ||
                                 init_val.value().find("NULL") != std::string::npos)) {
                                return true;
                            }
                        }
                    }
                }
            }
            return false;
        }

        // Check for call expressions
        if (query_str.find("call_expr") != std::string::npos) {
            for (const auto& call : context.current_function->getCallSites()) {
                if (query_str.find("callee='OK'") != std::string::npos && call.getName() == "OK") {
                    return true;
                }
                if (query_str.find("callee='ERROR'") != std::string::npos && call.getName() == "ERROR") {
                    return true;
                }
                if (query_str.find("FreeMemory") != std::string::npos && call.getName().find("FreeMemory") != std::string::npos) {
                    return true;
                }
            }
        }

        // Default: return false for unsupported queries
        return false;
    }

    if (name == "ast_query_count") {
        // Count AST query matches using simplified pattern matching
        // Usage: ast_query_count("macro_expansion[@name='FUNCTION_ENTER']")
        if (args.empty() || !context.current_function) return 0;

        std::string query_str = anyToString(args[0]);
        int count = 0;

        const auto& stmts = context.current_function->getStatements();

        // Count macro expansions
        if (query_str.find("macro_expansion[@name='FUNCTION_ENTER']") != std::string::npos) {
            for (const auto& stmt : stmts) {
                if (stmt.getText().find("FUNCTION_ENTER") != std::string::npos) {
                    count++;
                }
            }
            return count;
        }

        if (query_str.find("macro_expansion[@name='FUNCTION_EXIT']") != std::string::npos) {
            for (const auto& stmt : stmts) {
                if (stmt.getText().find("FUNCTION_EXIT") != std::string::npos) {
                    count++;
                }
            }
            return count;
        }

        // Count nested for statements
        if (query_str.find("for_stmt") != std::string::npos) {
            for (const auto& stmt : stmts) {
                if (stmt.getType() == Statement::FOR_STMT) {
                    count++;
                }
            }
            return count;
        }

        return count;
    }

    if (name == "ast_matches") {
        // Check if AST query matches (boolean)
        // Usage: ast_matches("call_expr[@name=\"malloc\"]")
        if (args.empty() || !context.current_function) return false;

        std::string query_str = anyToString(args[0]);

        // Would execute query and return true/false
        spdlog::warn("ast_matches requires Clang AST access - not yet implemented in DSL");
        return false;
    }

    if (name == "has_nested_loops") {
        // Check for nested loops of specified depth
        // Usage: has_nested_loops(3)
        if (!context.current_function) return false;

        int depth = 3;
        if (!args.empty()) {
            depth = anyToInt(args[0]);
        }

        // Use AST query template
        using namespace nomic::patterns;
        ASTTemplateLibrary::instance().loadBuiltinTemplates();

        spdlog::warn("has_nested_loops requires Clang AST access - not yet implemented in DSL");
        return false;
    }

    if (name == "has_unchecked_allocation") {
        // Check for malloc/calloc/realloc without NULL check
        // Usage: has_unchecked_allocation()
        if (!context.current_function) return false;

        // This can be approximated using existing call tracking
        bool has_alloc = false;
        bool has_null_check = false;

        for (const auto& call : context.current_function->getCallSites()) {
            std::string name = call.getName();
            if (name == "malloc" || name == "calloc" || name == "realloc") {
                has_alloc = true;
            }
        }

        // Check for NULL checks in statements (simplified)
        for (const auto& stmt : context.current_function->getStatements()) {
            if (stmt.getText().find("NULL") != std::string::npos ||
                stmt.getText().find("nullptr") != std::string::npos) {
                has_null_check = true;
            }
        }

        return has_alloc && !has_null_check;
    }

    if (name == "has_unsafe_cast") {
        // Check for potentially unsafe type casts
        // Usage: has_unsafe_cast()
        if (!context.current_function) return false;

        // Look for C-style casts in statements
        for (const auto& stmt : context.current_function->getStatements()) {
            std::string text = stmt.getText();
            // Simple heuristic: look for (type*) patterns
            if (text.find("(") != std::string::npos && text.find("*)") != std::string::npos) {
                return true;
            }
        }

        return false;
    }

    if (name == "has_pointer_arithmetic") {
        // Check for pointer arithmetic
        // Usage: has_pointer_arithmetic()
        if (!context.current_function) return false;

        // Look for pointer arithmetic patterns
        for (const auto& stmt : context.current_function->getStatements()) {
            std::string text = stmt.getText();
            // Look for common pointer arithmetic patterns
            if ((text.find("++") != std::string::npos || text.find("--") != std::string::npos) &&
                (text.find("*") != std::string::npos || text.find("->") != std::string::npos)) {
                return true;
            }
            if (text.find("ptr") != std::string::npos &&
                (text.find(" + ") != std::string::npos || text.find(" - ") != std::string::npos)) {
                return true;
            }
        }

        return false;
    }

    if (name == "has_magic_number") {
        // Check for magic numbers (numeric literals > threshold in non-const context)
        // Usage: has_magic_number(100)
        if (!context.current_function) return false;

        int threshold = 10;
        if (!args.empty()) {
            threshold = anyToInt(args[0]);
        }

        // Look for numeric literals in statements
        for (const auto& stmt : context.current_function->getStatements()) {
            std::string text = stmt.getText();

            // Skip if it's a const declaration
            if (text.find("const") != std::string::npos) {
                continue;
            }

            // Simple heuristic: find numeric patterns
            std::regex num_pattern("\\b([0-9]+)\\b");
            std::smatch match;
            std::string::const_iterator search_start(text.cbegin());

            while (std::regex_search(search_start, text.cend(), match, num_pattern)) {
                try {
                    int value = std::stoi(match[1]);
                    if (value >= threshold && value != 0 && value != 1) {
                        return true;
                    }
                } catch (...) {
                    // Ignore parse errors
                }
                search_start = match.suffix().first;
            }
        }

        return false;
    }

    if (name == "has_goto_statement") {
        // Check for goto statements
        // Usage: has_goto_statement()
        if (!context.current_function) return false;

        for (const auto& stmt : context.current_function->getStatements()) {
            if (stmt.getText().find("goto ") != std::string::npos) {
                return true;
            }
        }

        return false;
    }

    if (name == "has_switch_fallthrough") {
        // Check for switch case fallthrough (missing break)
        // Usage: has_switch_fallthrough()
        if (!context.current_function) return false;

        // Look for case statements without break
        bool in_switch = false;
        bool has_case = false;
        bool has_break = false;

        for (const auto& stmt : context.current_function->getStatements()) {
            std::string text = stmt.getText();

            if (text.find("switch") != std::string::npos) {
                in_switch = true;
            }

            if (in_switch && text.find("case ") != std::string::npos) {
                if (has_case && !has_break) {
                    return true; // Previous case had no break
                }
                has_case = true;
                has_break = false;
            }

            if (text.find("break") != std::string::npos) {
                has_break = true;
            }

            if (text.find("}") != std::string::npos && in_switch) {
                in_switch = false;
                has_case = false;
                has_break = false;
            }
        }

        return false;
    }

    if (name == "uses_deprecated_function") {
        // Check for usage of deprecated functions
        // Usage: uses_deprecated_function()
        if (!context.current_function) return false;

        static const std::set<std::string> deprecated_funcs = {
            "gets", "strcpy", "strcat", "sprintf", "scanf", "tmpnam", "tempnam"
        };

        for (const auto& call : context.current_function->getCallSites()) {
            if (deprecated_funcs.find(call.getName()) != deprecated_funcs.end()) {
                return true;
            }
        }

        return false;
    }

    if (name == "has_deep_nesting") {
        // Check for deep nesting based on brace depth
        // Usage: has_deep_nesting(5)
        if (!context.current_function) return false;

        int max_depth = 4;
        if (!args.empty()) {
            max_depth = anyToInt(args[0]);
        }

        int current_depth = 0;
        int max_found = 0;

        for (const auto& stmt : context.current_function->getStatements()) {
            std::string text = stmt.getText();

            for (char ch : text) {
                if (ch == '{') {
                    current_depth++;
                    max_found = std::max(max_found, current_depth);
                } else if (ch == '}') {
                    current_depth--;
                }
            }
        }

        return max_found >= max_depth;
    }

    if (name == "has_string_literal") {
        // Check if function contains a specific string literal
        // Usage: has_string_literal("password")
        if (args.empty() || !context.current_function) return false;

        std::string search_str = anyToString(args[0]);

        for (const auto& stmt : context.current_function->getStatements()) {
            std::string text = stmt.getText();

            // Look for string literals containing the search string
            size_t pos = 0;
            while ((pos = text.find('"', pos)) != std::string::npos) {
                size_t end_pos = text.find('"', pos + 1);
                if (end_pos != std::string::npos) {
                    std::string literal = text.substr(pos + 1, end_pos - pos - 1);
                    if (literal.find(search_str) != std::string::npos) {
                        return true;
                    }
                    pos = end_pos + 1;
                } else {
                    break;
                }
            }
        }

        return false;
    }

    // ==================== CODING STANDARDS FUNCTIONS ====================

    // A. NAMING CONVENTION FUNCTIONS

    if (name == "check_function_naming") {
        // Check if function name follows naming convention
        // Usage: check_function_naming("snake_case") or check_function_naming("snake_case", "prefix_")
        if (args.empty() || !context.current_function) return true;

        std::string case_style_str = anyToString(args[0]);
        nomic::standards::CaseStyle case_style;

        if (case_style_str == "snake_case") case_style = nomic::standards::CaseStyle::SNAKE_CASE;
        else if (case_style_str == "UPPER_CASE") case_style = nomic::standards::CaseStyle::UPPER_CASE;
        else if (case_style_str == "camelCase") case_style = nomic::standards::CaseStyle::CAMEL_CASE;
        else if (case_style_str == "PascalCase") case_style = nomic::standards::CaseStyle::PASCAL_CASE;
        else return true; // Unknown style, skip check

        nomic::standards::NamingConvention conv(case_style);

        // Add prefix if provided
        if (args.size() > 1) {
            conv.setPrefix(anyToString(args[1]));
        }

        // Add suffix if provided
        if (args.size() > 2) {
            conv.setSuffix(anyToString(args[2]));
        }

        std::string error;
        return conv.validate(context.current_function->getName(), error);
    }

    if (name == "check_variable_naming") {
        // Check if variable name follows naming convention
        // Usage: check_variable_naming("snake_case") or check_variable_naming("UPPER_CASE", "", "_t")
        if (args.empty()) return true;

        std::string case_style_str = anyToString(args[0]);
        nomic::standards::CaseStyle case_style;

        if (case_style_str == "snake_case") case_style = nomic::standards::CaseStyle::SNAKE_CASE;
        else if (case_style_str == "UPPER_CASE") case_style = nomic::standards::CaseStyle::UPPER_CASE;
        else if (case_style_str == "camelCase") case_style = nomic::standards::CaseStyle::CAMEL_CASE;
        else if (case_style_str == "PascalCase") case_style = nomic::standards::CaseStyle::PASCAL_CASE;
        else return true;

        nomic::standards::NamingConvention conv(case_style);

        if (args.size() > 1) {
            conv.setPrefix(anyToString(args[1]));
        }

        if (args.size() > 2) {
            conv.setSuffix(anyToString(args[2]));
        }

        // Check all variables in current function
        if (context.current_function) {
            for (const auto& var : context.current_function->getLocalVariables()) {
                std::string error;
                if (!conv.validate(var.getName(), error)) {
                    return false;
                }
            }
        }

        return true;
    }

    if (name == "matches_case_style") {
        // Check if identifier matches case style
        // Usage: matches_case_style(fn.name, "snake_case")
        if (args.size() < 2) return false;

        std::string identifier = anyToString(args[0]);
        std::string case_style_str = anyToString(args[1]);

        nomic::standards::CaseStyle case_style;
        if (case_style_str == "snake_case") case_style = nomic::standards::CaseStyle::SNAKE_CASE;
        else if (case_style_str == "UPPER_CASE") case_style = nomic::standards::CaseStyle::UPPER_CASE;
        else if (case_style_str == "camelCase") case_style = nomic::standards::CaseStyle::CAMEL_CASE;
        else if (case_style_str == "PascalCase") case_style = nomic::standards::CaseStyle::PASCAL_CASE;
        else return false;

        return nomic::standards::util::matchesCaseStyle(identifier, case_style);
    }

    if (name == "has_prefix") {
        // Check if identifier has specific prefix
        // Usage: has_prefix(fn.name, "os_")
        if (args.size() < 2) return false;

        std::string identifier = anyToString(args[0]);
        std::string prefix = anyToString(args[1]);

        return identifier.find(prefix) == 0;
    }

    if (name == "has_suffix") {
        // Check if identifier has specific suffix
        // Usage: has_suffix(type.name, "_t")
        if (args.size() < 2) return false;

        std::string identifier = anyToString(args[0]);
        std::string suffix = anyToString(args[1]);

        if (identifier.length() < suffix.length()) return false;
        return identifier.substr(identifier.length() - suffix.length()) == suffix;
    }

    // B. DOCUMENTATION FUNCTIONS

    if (name == "has_function_doc") {
        // Check if function has documentation comment
        // Usage: has_function_doc()
        // NOTE: Requires comment tracking in semantic model (not yet implemented)
        // For now, always returns true to avoid false positives
        return true;
    }

    if (name == "has_brief_doc") {
        // Check if function has @brief documentation
        // Usage: has_brief_doc()
        // NOTE: Requires comment tracking in semantic model (not yet implemented)
        // For now, always returns true to avoid false positives
        return true;
    }

    if (name == "has_param_docs") {
        // Check if function has @param docs for all parameters
        // Usage: has_param_docs()
        // NOTE: Requires comment tracking in semantic model (not yet implemented)
        // For now, always returns true to avoid false positives
        return true;
    }

    if (name == "has_return_doc") {
        // Check if function has @return documentation
        // Usage: has_return_doc()
        // NOTE: Requires comment tracking in semantic model (not yet implemented)
        // For now, always returns true to avoid false positives
        return true;
    }

    if (name == "missing_doc") {
        // Returns true if function is missing documentation
        // Usage: missing_doc()
        // NOTE: Requires comment tracking in semantic model (not yet implemented)
        // For now, always returns false to avoid false positives
        return false;
    }

    // C. CODE ORGANIZATION FUNCTIONS

    if (name == "check_include_order") {
        // Check if includes are in correct order (system before project)
        // Usage: check_include_order()
        // This is a simplified check - would need to parse #include directives
        // For now, return true (would be implemented with proper include tracking)
        return true;
    }

    if (name == "has_header_guard") {
        // Check if header file has proper header guard
        // Usage: has_header_guard()
        // This would need file-level analysis
        // For now, return true (simplified)
        return true;
    }

    if (name == "check_static_first") {
        // Check if static functions are declared before non-static
        // Usage: check_static_first()
        // This would need proper function declaration tracking
        // For now, return true (simplified)
        return true;
    }

    if (name == "is_public_function") {
        // Check if function is public (not static)
        // Usage: is_public_function()
        if (!context.current_function) return false;

        return !context.current_function->isStatic();
    }

    if (name == "is_static_function") {
        // Check if function is static (internal)
        // Usage: is_static_function()
        if (!context.current_function) return false;

        return context.current_function->isStatic();
    }

    // ==================== TRANSFORMATION FUNCTIONS ====================

    if (name == "to_upper") {
        // Convert string to uppercase
        // Usage: to_upper(fn.name)
        if (args.empty()) return std::string();

        std::string str = anyToString(args[0]);
        std::transform(str.begin(), str.end(), str.begin(),
                      [](unsigned char c) { return std::toupper(c); });
        return str;
    }

    if (name == "to_lower") {
        // Convert string to lowercase
        // Usage: to_lower(fn.name)
        if (args.empty()) return std::string();

        std::string str = anyToString(args[0]);
        std::transform(str.begin(), str.end(), str.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        return str;
    }

    if (name == "substring") {
        // Extract substring
        // Usage: substring(str, start, length)
        if (args.size() < 2) return std::string();

        std::string str = anyToString(args[0]);
        int start = anyToInt(args[1]);

        if (start < 0 || start >= static_cast<int>(str.length())) {
            return std::string();
        }

        if (args.size() >= 3) {
            int length = anyToInt(args[2]);
            return str.substr(start, length);
        }

        return str.substr(start);
    }

    if (name == "length") {
        // Get string length
        // Usage: length(fn.name)
        if (args.empty()) return 0;

        std::string str = anyToString(args[0]);
        return static_cast<int>(str.length());
    }

    if (name == "starts_with") {
        // Check if string starts with prefix (case-sensitive)
        // Usage: starts_with(fn.name, "prefix")
        if (args.size() < 2) return false;

        std::string str = anyToString(args[0]);
        std::string prefix = anyToString(args[1]);

        return str.find(prefix) == 0;
    }

    if (name == "ends_with") {
        // Check if string ends with suffix (case-sensitive)
        // Usage: ends_with(fn.name, "suffix")
        if (args.size() < 2) return false;

        std::string str = anyToString(args[0]);
        std::string suffix = anyToString(args[1]);

        if (str.length() < suffix.length()) return false;
        return str.substr(str.length() - suffix.length()) == suffix;
    }

    if (name == "contains") {
        // Check if string contains substring
        // Usage: contains(fn.name, "substring")
        if (args.size() < 2) return false;

        std::string str = anyToString(args[0]);
        std::string substring = anyToString(args[1]);

        return str.find(substring) != std::string::npos;
    }

    if (name == "replace") {
        // Replace all occurrences of substring
        // Usage: replace(str, old, new)
        if (args.size() < 3) return std::string();

        std::string str = anyToString(args[0]);
        std::string old_str = anyToString(args[1]);
        std::string new_str = anyToString(args[2]);

        size_t pos = 0;
        while ((pos = str.find(old_str, pos)) != std::string::npos) {
            str.replace(pos, old_str.length(), new_str);
            pos += new_str.length();
        }

        return str;
    }

    if (name == "trim") {
        // Remove leading/trailing whitespace
        // Usage: trim(str)
        if (args.empty()) return std::string();

        std::string str = anyToString(args[0]);

        // Trim left
        size_t start = str.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return std::string();

        // Trim right
        size_t end = str.find_last_not_of(" \t\n\r");

        return str.substr(start, end - start + 1);
    }

    if (name == "split") {
        // Split string by delimiter
        // Usage: split(str, delimiter)
        if (args.size() < 2) return std::vector<std::any>();

        std::string str = anyToString(args[0]);
        std::string delim = anyToString(args[1]);

        std::vector<std::any> result;
        size_t start = 0;
        size_t end = str.find(delim);

        while (end != std::string::npos) {
            result.push_back(str.substr(start, end - start));
            start = end + delim.length();
            end = str.find(delim, start);
        }

        result.push_back(str.substr(start));
        return result;
    }

    if (name == "join") {
        // Join array of strings with delimiter
        // Usage: join(array, delimiter)
        if (args.size() < 2) return std::string();

        if (args[0].type() != typeid(std::vector<std::any>)) {
            return std::string();
        }

        auto vec = std::any_cast<std::vector<std::any>>(args[0]);
        std::string delim = anyToString(args[1]);

        std::string result;
        for (size_t i = 0; i < vec.size(); i++) {
            if (i > 0) result += delim;
            result += anyToString(vec[i]);
        }

        return result;
    }

    if (name == "regex_match") {
        // Check if string matches regex pattern
        // Usage: regex_match(str, pattern)
        if (args.size() < 2) return false;

        std::string str = anyToString(args[0]);
        std::string pattern = anyToString(args[1]);

        try {
            std::regex re(pattern);
            return std::regex_match(str, re);
        } catch (const std::regex_error&) {
            spdlog::warn("Invalid regex pattern: {}", pattern);
            return false;
        }
    }

    if (name == "regex_search") {
        // Check if string contains regex pattern
        // Usage: regex_search(str, pattern)
        if (args.size() < 2) return false;

        std::string str = anyToString(args[0]);
        std::string pattern = anyToString(args[1]);

        try {
            std::regex re(pattern);
            return std::regex_search(str, re);
        } catch (const std::regex_error&) {
            spdlog::warn("Invalid regex pattern: {}", pattern);
            return false;
        }
    }

    // Numeric transformation functions

    if (name == "abs") {
        // Absolute value
        // Usage: abs(value)
        if (args.empty()) return 0;

        double val = anyToDouble(args[0]);
        return std::abs(val);
    }

    if (name == "min") {
        // Minimum of values
        // Usage: min(a, b) or min(a, b, c, ...)
        if (args.empty()) return 0;

        double min_val = anyToDouble(args[0]);
        for (size_t i = 1; i < args.size(); i++) {
            min_val = std::min(min_val, anyToDouble(args[i]));
        }
        return min_val;
    }

    if (name == "max") {
        // Maximum of values
        // Usage: max(a, b) or max(a, b, c, ...)
        if (args.empty()) return 0;

        double max_val = anyToDouble(args[0]);
        for (size_t i = 1; i < args.size(); i++) {
            max_val = std::max(max_val, anyToDouble(args[i]));
        }
        return max_val;
    }

    if (name == "clamp") {
        // Clamp value between min and max
        // Usage: clamp(value, min, max)
        if (args.size() < 3) return 0;

        double val = anyToDouble(args[0]);
        double min_val = anyToDouble(args[1]);
        double max_val = anyToDouble(args[2]);

        return std::clamp(val, min_val, max_val);
    }

    // ==================== CONDITIONAL FUNCTIONS ====================

    if (name == "if_else") {
        // Conditional expression: if_else(condition, true_value, false_value)
        // Note: Both branches are eagerly evaluated (limitation of current design)
        // Usage: if_else(fn.parameter_count > 5, "many", "few")
        if (args.size() < 3) return std::any();

        bool condition = anyToBool(args[0]);
        return condition ? args[1] : args[2];
    }

    if (name == "coalesce") {
        // Return first non-empty value
        // Usage: coalesce(value1, value2, default)
        for (const auto& arg : args) {
            if (arg.type() == typeid(std::string)) {
                std::string str = anyToString(arg);
                if (!str.empty()) return str;
            } else if (arg.type() == typeid(bool)) {
                if (anyToBool(arg)) return arg;
            } else {
                // Non-empty value
                return arg;
            }
        }
        return std::any();
    }

    // ==================== HIGHER-ORDER / COLLECTION FUNCTIONS ====================

    if (name == "count") {
        // Count elements in collection
        // Usage: count(fn.parameters)
        if (args.empty()) return 0;

        if (args[0].type() == typeid(std::vector<std::any>)) {
            auto vec = std::any_cast<std::vector<std::any>>(args[0]);
            return static_cast<int>(vec.size());
        }

        return 0;
    }

    if (name == "sum") {
        // Sum numeric values in collection
        // Usage: sum(array)
        if (args.empty()) return 0.0;

        double total = 0.0;

        if (args[0].type() == typeid(std::vector<std::any>)) {
            auto vec = std::any_cast<std::vector<std::any>>(args[0]);
            for (const auto& item : vec) {
                total += anyToDouble(item);
            }
        } else {
            // Sum all arguments
            for (const auto& arg : args) {
                total += anyToDouble(arg);
            }
        }

        return total;
    }

    if (name == "avg") {
        // Average of numeric values
        // Usage: avg(array) or avg(a, b, c)
        if (args.empty()) return 0.0;

        double total = 0.0;
        int count = 0;

        if (args[0].type() == typeid(std::vector<std::any>)) {
            auto vec = std::any_cast<std::vector<std::any>>(args[0]);
            count = vec.size();
            for (const auto& item : vec) {
                total += anyToDouble(item);
            }
        } else {
            count = args.size();
            for (const auto& arg : args) {
                total += anyToDouble(arg);
            }
        }

        return count > 0 ? total / count : 0.0;
    }

    if (name == "filter") {
        // Filter collection by property value
        // Limited version: filter collection where property equals value
        // Usage: filter(fn.parameters, "type", "int")
        // Note: This is a simplified version - true higher-order would need lambdas
        if (args.size() < 3) return std::vector<std::any>();

        if (args[0].type() != typeid(std::vector<std::any>)) {
            return std::vector<std::any>();
        }

        auto collection = std::any_cast<std::vector<std::any>>(args[0]);
        std::string property = anyToString(args[1]);
        std::string target_value = anyToString(args[2]);

        std::vector<std::any> filtered;
        // This would need proper member access on collection items
        // For now, return the collection (placeholder)
        return collection;
    }

    if (name == "map_property") {
        // Map collection to property values
        // Usage: map_property(fn.parameters, "name")
        // Returns array of property values
        if (args.size() < 2) return std::vector<std::any>();

        if (args[0].type() != typeid(std::vector<std::any>)) {
            return std::vector<std::any>();
        }

        // This would need proper member access implementation
        // For now, return empty array (placeholder)
        return std::vector<std::any>();
    }

    if (name == "first") {
        // Get first element of collection
        // Usage: first(fn.parameters)
        if (args.empty()) return std::any();

        if (args[0].type() == typeid(std::vector<std::any>)) {
            auto vec = std::any_cast<std::vector<std::any>>(args[0]);
            if (!vec.empty()) return vec[0];
        }

        return std::any();
    }

    if (name == "last") {
        // Get last element of collection
        // Usage: last(fn.parameters)
        if (args.empty()) return std::any();

        if (args[0].type() == typeid(std::vector<std::any>)) {
            auto vec = std::any_cast<std::vector<std::any>>(args[0]);
            if (!vec.empty()) return vec.back();
        }

        return std::any();
    }

    if (name == "at") {
        // Get element at index
        // Usage: at(fn.parameters, 0)
        if (args.size() < 2) return std::any();

        if (args[0].type() == typeid(std::vector<std::any>)) {
            auto vec = std::any_cast<std::vector<std::any>>(args[0]);
            int index = anyToInt(args[1]);

            if (index >= 0 && index < static_cast<int>(vec.size())) {
                return vec[index];
            }
        }

        return std::any();
    }

    if (name == "empty") {
        // Check if collection is empty
        // Usage: empty(fn.parameters)
        if (args.empty()) return true;

        if (args[0].type() == typeid(std::vector<std::any>)) {
            auto vec = std::any_cast<std::vector<std::any>>(args[0]);
            return vec.empty();
        }

        if (args[0].type() == typeid(std::string)) {
            return anyToString(args[0]).empty();
        }

        return true;
    }

    // ==================== HELIOS CODE STANDARD FUNCTIONS ====================

    // regex_matches(text, pattern) - regex matching for code standard rules
    if (name == "regex_matches") {
        if (args.size() < 2) return false;
        std::string text = anyToString(args[0]);
        std::string pattern = anyToString(args[1]);
        try {
            std::regex re(pattern);
            return std::regex_match(text, re);  // Use regex_match for full string match
        } catch (const std::regex_error& e) {
            spdlog::warn("Invalid regex pattern '{}': {}", pattern, e.what());
            return false;
        }
    }

    // getLineCount() - get line count of current function
    if (name == "getLineCount") {
        if (!context.current_function) {
            spdlog::warn("getLineCount() called without current function context");
            return 0;
        }
        const auto& loc = context.current_function->getLocation();
        return static_cast<int>(loc.getEnd().getLine() - loc.getStart().getLine() + 1);
    }

    // getCyclomaticComplexity() - get cyclomatic complexity of current function
    if (name == "getCyclomaticComplexity") {
        if (!context.current_function) {
            spdlog::warn("getCyclomaticComplexity() called without current function context");
            return 0;
        }
        return static_cast<int>(context.current_function->getCyclomaticComplexity());
    }

    // getMaxNestingDepth() - get max nesting depth of current function
    if (name == "getMaxNestingDepth") {
        if (!context.current_function) {
            spdlog::warn("getMaxNestingDepth() called without current function context");
            return 0;
        }
        // Calculate max nesting depth from statements
        int max_depth = 0;
        int current_depth = 0;
        for (const auto& stmt : context.current_function->getStatements()) {
            if (stmt.getType() == Statement::COMPOUND_STMT ||
                stmt.getType() == Statement::IF_STMT ||
                stmt.getType() == Statement::FOR_STMT ||
                stmt.getType() == Statement::WHILE_STMT) {
                current_depth++;
                if (current_depth > max_depth) {
                    max_depth = current_depth;
                }
            }
            // This is simplified - a proper implementation would track braces/scopes
        }
        return max_depth > 0 ? max_depth : 1;
    }

    // param_count() - get parameter count of current function
    if (name == "param_count") {
        if (!context.current_function) {
            spdlog::warn("param_count() called without current function context");
            return 0;
        }
        return static_cast<int>(context.current_function->getParameters().size());
    }

    spdlog::warn("Unknown built-in function: {}", name);
    return std::any();
}

// DSLEvaluator::Impl implementation
class DSLEvaluator::Impl {
public:
    Impl() : evaluator_impl_() {}

    bool evaluateAssertion(const std::string& expression, EvaluationContext& context) {
        spdlog::debug("Evaluating assertion: {}", expression);

        try {
            // Parse expression
            Tokenizer tokenizer(expression);
            auto tokens = tokenizer.tokenize();
            Parser parser(tokens);
            auto ast = parser.parse();

            if (!ast) {
                spdlog::error("Failed to parse assertion");
                return false;
            }

            // Evaluate AST
            auto result = evaluator_impl_.evaluate(ast.get(), context);
            return anyToBool(result);
        } catch (const std::exception& e) {
            spdlog::error("Error evaluating assertion '{}': {}", expression, e.what());
            return false;
        }
    }

    std::any evaluateExpression(const std::string& expression, EvaluationContext& context) {
        try {
            // Parse expression
            Tokenizer tokenizer(expression);
            auto tokens = tokenizer.tokenize();
            Parser parser(tokens);
            auto ast = parser.parse();

            if (!ast) {
                return std::any();
            }

            // Evaluate AST
            return evaluator_impl_.evaluate(ast.get(), context);
        } catch (const std::exception& e) {
            spdlog::error("Error evaluating expression '{}': {}", expression, e.what());
            return std::any();
        }
    }

private:
    DSLEvaluatorImpl evaluator_impl_;
};

// DSLEvaluator public interface implementation
DSLEvaluator::DSLEvaluator() : pImpl(std::make_unique<Impl>()) {}

DSLEvaluator::~DSLEvaluator() = default;

bool DSLEvaluator::evaluateAssertion(const std::string& expression, EvaluationContext& context) {
    return pImpl->evaluateAssertion(expression, context);
}

std::any DSLEvaluator::evaluateExpression(const std::string& expression, EvaluationContext& context) {
    return pImpl->evaluateExpression(expression, context);
}

} // namespace nomic::dsl
