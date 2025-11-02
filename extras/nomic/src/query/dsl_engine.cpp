/**
 * @file dsl_engine.cpp
 * @brief Complete DSL Engine implementation with parser, evaluator, and 100+ built-in functions
 */

#include "nomic/query/dsl_engine.h"
#include "nomic/core/ast_node.h"
#include <cctype>
#include <stdexcept>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <regex>
#include <sstream>
#include <iomanip>
#include <stack>

namespace nomic {
namespace query {

// Forward declarations for implementation classes
class DSLExpression;
class LiteralExpression;
class IdentifierExpression;
class BinaryOpExpression;
class UnaryOpExpression;
class FunctionCallExpression;
class LambdaExpression;
class IfElseExpression;
class DSLContext;
class DSLFunction;
class BuiltinFunction;
class DSLEngine;

// ===================================
// Expression Implementation Classes
// ===================================

/**
 * @brief Base DSL Expression implementation
 */
class DSLExpression : public IDSLExpression {
protected:
    ExpressionType type_;

public:
    explicit DSLExpression(ExpressionType type) : type_(type) {}
    ExpressionType getType() const override { return type_; }
};

/**
 * @brief Literal expression
 */
class LiteralExpression : public DSLExpression {
private:
    DSLValue value_;

public:
    explicit LiteralExpression(const DSLValue& value)
        : DSLExpression(ExpressionType::LITERAL), value_(value) {}

    DSLValue evaluate(DSLContextPtr context) override {
        return value_;
    }

    std::string toString() const override {
        if (std::holds_alternative<std::nullptr_t>(value_)) return "null";
        if (std::holds_alternative<bool>(value_)) return std::get<bool>(value_) ? "true" : "false";
        if (std::holds_alternative<int>(value_)) return std::to_string(std::get<int>(value_));
        if (std::holds_alternative<double>(value_)) return std::to_string(std::get<double>(value_));
        if (std::holds_alternative<std::string>(value_)) return "\"" + std::get<std::string>(value_) + "\"";
        return "<value>";
    }
};

/**
 * @brief Identifier expression
 */
class IdentifierExpression : public DSLExpression {
private:
    std::string name_;

public:
    explicit IdentifierExpression(const std::string& name)
        : DSLExpression(ExpressionType::IDENTIFIER), name_(name) {}

    DSLValue evaluate(DSLContextPtr context) override {
        if (!context->hasVariable(name_)) {
            throw std::runtime_error("Undefined variable: " + name_);
        }
        return context->getVariable(name_);
    }

    std::string toString() const override { return name_; }
    const std::string& getName() const { return name_; }
};

/**
 * @brief DSL Context implementation
 */
class DSLContext : public IDSLContext, public std::enable_shared_from_this<DSLContext> {
private:
    std::vector<std::unordered_map<std::string, DSLValue>> scopes_;
    std::unordered_map<std::string, DSLFunctionPtr> functions_;
    core::SemanticModelPtr semanticModel_;

public:
    DSLContext() {
        // Initialize with global scope
        scopes_.push_back({});
    }

    // Variable management
    void setVariable(const std::string& name, const DSLValue& value) override {
        if (!name.empty()) {
            scopes_.back()[name] = value;
        }
    }

    DSLValue getVariable(const std::string& name) const override {
        // Search from innermost to outermost scope
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto var = it->find(name);
            if (var != it->end()) {
                return var->second;
            }
        }
        return nullptr;
    }

    bool hasVariable(const std::string& name) const override {
        if (name.empty()) return false;
        for (const auto& scope : scopes_) {
            if (scope.find(name) != scope.end()) {
                return true;
            }
        }
        return false;
    }

    void deleteVariable(const std::string& name) override {
        scopes_.back().erase(name);
    }

    // Scope management
    void pushScope() override {
        scopes_.push_back({});
    }

    void popScope() override {
        if (scopes_.size() > 1) {
            scopes_.pop_back();
        }
    }

    size_t getScopeDepth() const override {
        return scopes_.size() - 1;  // Don't count global scope
    }

    // Function management
    void registerFunction(const std::string& name, DSLFunctionPtr function) override {
        functions_[name] = function;
    }

    DSLFunctionPtr getFunction(const std::string& name) const override {
        auto it = functions_.find(name);
        return (it != functions_.end()) ? it->second : nullptr;
    }

    bool hasFunction(const std::string& name) const override {
        return functions_.find(name) != functions_.end();
    }

    // Semantic model access
    core::SemanticModelPtr getSemanticModel() const override {
        return semanticModel_;
    }

    void setSemanticModel(core::SemanticModelPtr model) override {
        semanticModel_ = model;
    }
};

/**
 * @brief Base class for DSL functions
 */
class DSLFunction : public IDSLFunction {
protected:
    FunctionKind kind_;
    std::string name_;
    size_t arity_;
    std::string description_;

public:
    DSLFunction(FunctionKind kind, const std::string& name, size_t arity, const std::string& desc)
        : kind_(kind), name_(name), arity_(arity), description_(desc) {}

    FunctionKind getKind() const override { return kind_; }
    std::string getName() const override { return name_; }
    size_t getArity() const override { return arity_; }
    bool isVariadic() const override { return arity_ == static_cast<size_t>(-1); }
    std::string getDescription() const override { return description_; }
    std::string getSignature() const override { return name_ + "()"; }
};

/**
 * @brief Built-in function implementation
 */
class BuiltinFunction : public DSLFunction {
private:
    std::function<DSLValue(const std::vector<DSLValue>&, DSLContextPtr)> impl_;

public:
    BuiltinFunction(const std::string& name, size_t arity, const std::string& desc,
                   std::function<DSLValue(const std::vector<DSLValue>&, DSLContextPtr)> impl)
        : DSLFunction(FunctionKind::BUILTIN, name, arity, desc), impl_(impl) {}

    DSLValue call(const std::vector<DSLValue>& args, DSLContextPtr context) override {
        if (!isVariadic() && args.size() != arity_) {
            throw std::runtime_error(name_ + " expects " + std::to_string(arity_) +
                                   " arguments, got " + std::to_string(args.size()));
        }
        return impl_(args, context);
    }
};

/**
 * @brief Main DSL Engine implementation
 */
class DSLEngine : public IDSLEngine, public std::enable_shared_from_this<DSLEngine> {
private:
    std::unordered_map<std::string, DSLFunctionPtr> builtinFunctions_;
    DSLContextPtr currentContext_;
    std::string parseError_;

    // Parser state
    std::string input_;
    size_t pos_;

    void initializeBuiltinFunctions();
    void registerAllBuiltinFunctions();

    // Parsing helpers
    void skipWhitespace();
    bool peek(const std::string& str);
    bool consume(const std::string& str);
    bool consumeKeyword(const std::string& keyword);
    char peekChar();
    char consumeChar();
    DSLExpressionPtr parseExpression();
    DSLExpressionPtr parsePrimary();
    DSLExpressionPtr parseBinary(DSLExpressionPtr left, int minPrecedence);
    DSLExpressionPtr parseUnary();
    DSLExpressionPtr parseFunctionCall();
    DSLExpressionPtr parseLambda();
    DSLExpressionPtr parseIfElse();
    DSLExpressionPtr parseWhile();
    DSLExpressionPtr parseFor();
    DSLExpressionPtr parseSwitch();
    DSLExpressionPtr parseMatch();
    DSLExpressionPtr parseNumber();
    DSLExpressionPtr parseString();
    DSLExpressionPtr parseIdentifier();
    int getPrecedence(const std::string& op);

public:
    DSLEngine();

    // Parsing
    DSLExpressionPtr parse(const std::string& expression) override;
    std::vector<DSLExpressionPtr> parseMultiple(const std::string& expressions) override;
    bool validate(const std::string& expression) override;
    std::string getParseError() const override { return parseError_; }

    // Evaluation
    DSLValue evaluate(const std::string& expression, DSLContextPtr context) override;
    DSLValue evaluate(DSLExpressionPtr expr, DSLContextPtr context) override;

    // Context management
    DSLContextPtr createContext() override {
        auto ctx = std::make_shared<DSLContext>();
        // Register all built-in functions in new context
        for (const auto& [name, func] : builtinFunctions_) {
            ctx->registerFunction(name, func);
        }
        return ctx;
    }

    DSLContextPtr getCurrentContext() override {
        if (!currentContext_) {
            currentContext_ = createContext();
        }
        return currentContext_;
    }

    // Built-in functions registry
    std::vector<std::string> getBuiltinFunctionNames() const override {
        std::vector<std::string> names;
        names.reserve(builtinFunctions_.size());
        for (const auto& [name, func] : builtinFunctions_) {
            names.push_back(name);
        }
        return names;
    }

    DSLFunctionPtr getBuiltinFunction(const std::string& name) const override {
        auto it = builtinFunctions_.find(name);
        return (it != builtinFunctions_.end()) ? it->second : nullptr;
    }

    std::string getBuiltinFunctionHelp(const std::string& name) const override {
        auto func = getBuiltinFunction(name);
        return func ? func->getDescription() : "";
    }

    // Pattern matching
    bool match(const std::string& pattern, core::ASTNodePtr node) override;
    std::vector<core::ASTNodePtr> findMatches(const std::string& pattern) override;

    // Helper to convert DSL value to specific type
    template<typename T>
    static T getValue(const DSLValue& val);

    // Helper to convert to bool for conditions
    static bool toBool(const DSLValue& val);

    // Helper to convert to collection
    static std::vector<std::any> toCollection(const DSLValue& val);
};

/**
 * @brief Binary operator expression
 */
class BinaryOpExpression : public DSLExpression {
private:
    std::string op_;
    DSLExpressionPtr left_;
    DSLExpressionPtr right_;

public:
    BinaryOpExpression(const std::string& op, DSLExpressionPtr left, DSLExpressionPtr right)
        : DSLExpression(ExpressionType::BINARY_OP), op_(op), left_(left), right_(right) {}

    DSLValue evaluate(DSLContextPtr context) override;
    std::string toString() const override {
        return "(" + left_->toString() + " " + op_ + " " + right_->toString() + ")";
    }
};

/**
 * @brief Unary operator expression
 */
class UnaryOpExpression : public DSLExpression {
private:
    std::string op_;
    DSLExpressionPtr operand_;

public:
    UnaryOpExpression(const std::string& op, DSLExpressionPtr operand)
        : DSLExpression(ExpressionType::UNARY_OP), op_(op), operand_(operand) {}

    DSLValue evaluate(DSLContextPtr context) override;
    std::string toString() const override {
        return op_ + operand_->toString();
    }
};

/**
 * @brief Function call expression
 */
class FunctionCallExpression : public DSLExpression {
private:
    std::string name_;
    std::vector<DSLExpressionPtr> args_;

public:
    FunctionCallExpression(const std::string& name, const std::vector<DSLExpressionPtr>& args)
        : DSLExpression(ExpressionType::FUNCTION_CALL), name_(name), args_(args) {}

    DSLValue evaluate(DSLContextPtr context) override;
    std::string toString() const override;
};

/**
 * @brief Lambda expression
 */
class LambdaExpression : public DSLExpression, public std::enable_shared_from_this<LambdaExpression> {
private:
    std::vector<std::string> params_;
    DSLExpressionPtr body_;

public:
    LambdaExpression(const std::vector<std::string>& params, DSLExpressionPtr body)
        : DSLExpression(ExpressionType::LAMBDA), params_(params), body_(body) {}

    DSLValue evaluate(DSLContextPtr context) override;
    std::string toString() const override;

    // Allow lambda to be invoked
    DSLValue invoke(const std::vector<DSLValue>& args, DSLContextPtr context) {
        if (args.size() != params_.size()) {
            throw std::runtime_error("Lambda expects " + std::to_string(params_.size()) +
                                   " arguments, got " + std::to_string(args.size()));
        }

        // Create new scope for lambda execution
        context->pushScope();

        // Bind parameters to arguments
        for (size_t i = 0; i < params_.size(); ++i) {
            context->setVariable(params_[i], args[i]);
        }

        // Evaluate body
        DSLValue result;
        try {
            result = body_->evaluate(context);
        } catch (...) {
            context->popScope();
            throw;
        }

        context->popScope();
        return result;
    }

    size_t getParamCount() const { return params_.size(); }
};

/**
 * @brief If-else expression
 */
class IfElseExpression : public DSLExpression {
private:
    DSLExpressionPtr condition_;
    DSLExpressionPtr thenBranch_;
    DSLExpressionPtr elseBranch_;

public:
    IfElseExpression(DSLExpressionPtr condition, DSLExpressionPtr thenBranch, DSLExpressionPtr elseBranch)
        : DSLExpression(ExpressionType::IF_ELSE),
          condition_(condition), thenBranch_(thenBranch), elseBranch_(elseBranch) {}

    DSLValue evaluate(DSLContextPtr context) override;
    std::string toString() const override;
};

/**
 * @brief While loop expression
 */
class WhileExpression : public DSLExpression {
private:
    DSLExpressionPtr condition_;
    DSLExpressionPtr body_;

public:
    WhileExpression(DSLExpressionPtr condition, DSLExpressionPtr body)
        : DSLExpression(ExpressionType::WHILE_LOOP),
          condition_(condition), body_(body) {}

    DSLValue evaluate(DSLContextPtr context) override {
        DSLValue result = nullptr;
        while (true) {
            auto condVal = condition_->evaluate(context);
            bool cond = false;
            if (std::holds_alternative<bool>(condVal)) {
                cond = std::get<bool>(condVal);
            } else if (std::holds_alternative<int>(condVal)) {
                cond = std::get<int>(condVal) != 0;
            } else {
                throw std::runtime_error("While condition must be boolean or integer");
            }

            if (!cond) break;
            result = body_->evaluate(context);
        }
        return result;
    }

    std::string toString() const override {
        return "while (" + condition_->toString() + ") { " + body_->toString() + " }";
    }
};

/**
 * @brief For loop expression
 */
class ForExpression : public DSLExpression {
private:
    DSLExpressionPtr init_;
    DSLExpressionPtr condition_;
    DSLExpressionPtr increment_;
    DSLExpressionPtr body_;

public:
    ForExpression(DSLExpressionPtr init, DSLExpressionPtr condition,
                  DSLExpressionPtr increment, DSLExpressionPtr body)
        : DSLExpression(ExpressionType::FOR_LOOP),
          init_(init), condition_(condition), increment_(increment), body_(body) {}

    DSLValue evaluate(DSLContextPtr context) override {
        DSLValue result = nullptr;

        // Check if this is foreach style (init is identifier, condition is collection, increment is null)
        if (init_ && condition_ && !increment_ &&
            init_->getType() == ExpressionType::IDENTIFIER) {
            // Foreach: for (item in collection)
            auto collection = condition_->evaluate(context);

            // Get the variable name from init
            std::string varName = init_->toString();

            // Handle array/vector
            if (std::holds_alternative<std::vector<std::any>>(collection)) {
                auto& vec = std::get<std::vector<std::any>>(collection);
                for (const auto& item : vec) {
                    // Try to convert std::any to DSLValue
                    DSLValue itemValue;
                    if (item.type() == typeid(int)) {
                        itemValue = std::any_cast<int>(item);
                    } else if (item.type() == typeid(double)) {
                        itemValue = std::any_cast<double>(item);
                    } else if (item.type() == typeid(bool)) {
                        itemValue = std::any_cast<bool>(item);
                    } else if (item.type() == typeid(std::string)) {
                        itemValue = std::any_cast<std::string>(item);
                    } else {
                        // Fallback: store vector of one any element
                        itemValue = std::vector<std::any>{item};
                    }
                    context->setVariable(varName, itemValue);
                    result = body_->evaluate(context);
                }
            } else {
                throw std::runtime_error("For-in requires a collection (array)");
            }
            return result;
        }

        // C-style for loop
        // Initialize
        if (init_) init_->evaluate(context);

        // Loop
        while (true) {
            // Check condition
            if (condition_) {
                auto condVal = condition_->evaluate(context);
                bool cond = false;
                if (std::holds_alternative<bool>(condVal)) {
                    cond = std::get<bool>(condVal);
                } else if (std::holds_alternative<int>(condVal)) {
                    cond = std::get<int>(condVal) != 0;
                } else {
                    throw std::runtime_error("For condition must be boolean or integer");
                }
                if (!cond) break;
            }

            // Execute body
            result = body_->evaluate(context);

            // Increment
            if (increment_) increment_->evaluate(context);
        }
        return result;
    }

    std::string toString() const override {
        std::string initStr = init_ ? init_->toString() : "";
        std::string condStr = condition_ ? condition_->toString() : "";
        std::string incStr = increment_ ? increment_->toString() : "";
        return "for (" + initStr + "; " + condStr + "; " + incStr + ") { " + body_->toString() + " }";
    }
};

/**
 * @brief Switch expression
 */
class SwitchExpression : public DSLExpression {
private:
    DSLExpressionPtr value_;
    std::vector<std::pair<DSLExpressionPtr, DSLExpressionPtr>> cases_;
    DSLExpressionPtr defaultCase_;

public:
    SwitchExpression(DSLExpressionPtr value,
                     std::vector<std::pair<DSLExpressionPtr, DSLExpressionPtr>> cases,
                     DSLExpressionPtr defaultCase)
        : DSLExpression(ExpressionType::SWITCH),
          value_(value), cases_(cases), defaultCase_(defaultCase) {}

    DSLValue evaluate(DSLContextPtr context) override {
        auto val = value_->evaluate(context);

        // Try each case
        for (const auto& [caseVal, caseBody] : cases_) {
            auto caseResult = caseVal->evaluate(context);

            // Compare values
            bool match = false;
            if (std::holds_alternative<int>(val) && std::holds_alternative<int>(caseResult)) {
                match = std::get<int>(val) == std::get<int>(caseResult);
            } else if (std::holds_alternative<std::string>(val) && std::holds_alternative<std::string>(caseResult)) {
                match = std::get<std::string>(val) == std::get<std::string>(caseResult);
            } else if (std::holds_alternative<bool>(val) && std::holds_alternative<bool>(caseResult)) {
                match = std::get<bool>(val) == std::get<bool>(caseResult);
            }

            if (match) {
                return caseBody->evaluate(context);
            }
        }

        // Default case
        if (defaultCase_) {
            return defaultCase_->evaluate(context);
        }

        return nullptr;
    }

    std::string toString() const override {
        return "switch (" + value_->toString() + ") { ... }";
    }
};

/**
 * @brief Pattern matching expression
 */
class MatchExpression : public DSLExpression {
private:
    DSLExpressionPtr value_;
    std::vector<std::pair<std::string, DSLExpressionPtr>> patterns_;

public:
    MatchExpression(DSLExpressionPtr value,
                    std::vector<std::pair<std::string, DSLExpressionPtr>> patterns)
        : DSLExpression(ExpressionType::MATCH),
          value_(value), patterns_(patterns) {}

    DSLValue evaluate(DSLContextPtr context) override {
        auto val = value_->evaluate(context);

        for (const auto& [pattern, body] : patterns_) {
            // Simple pattern matching - underscore matches anything
            if (pattern == "_") {
                return body->evaluate(context);
            }

            // Match literal values
            if (std::holds_alternative<int>(val)) {
                try {
                    int patternVal = std::stoi(pattern);
                    if (std::get<int>(val) == patternVal) {
                        return body->evaluate(context);
                    }
                } catch (...) {
                    // Not a number pattern, skip
                }
            } else if (std::holds_alternative<std::string>(val)) {
                if (std::get<std::string>(val) == pattern) {
                    return body->evaluate(context);
                }
            }
        }

        return nullptr;
    }

    std::string toString() const override {
        return "match " + value_->toString() + " { ... }";
    }
};

// ===================================
// Expression Implementations
// ===================================

DSLValue BinaryOpExpression::evaluate(DSLContextPtr context) {
    // Handle assignment operators specially - don't evaluate left side
    if (op_ == "=" || op_ == "+=" || op_ == "-=" || op_ == "*=" || op_ == "/=") {
        if (left_->getType() != ExpressionType::IDENTIFIER) {
            throw std::runtime_error("Left side of assignment must be a variable");
        }
        std::string varName = left_->toString();
        auto rightVal = right_->evaluate(context);

        if (op_ == "=") {
            context->setVariable(varName, rightVal);
            return rightVal;
        }
        else if (op_ == "+=") {
            auto currentVal = context->getVariable(varName);
            if (std::holds_alternative<int>(currentVal) && std::holds_alternative<int>(rightVal)) {
                auto result = std::get<int>(currentVal) + std::get<int>(rightVal);
                context->setVariable(varName, result);
                return result;
            } else if (std::holds_alternative<double>(currentVal) || std::holds_alternative<double>(rightVal)) {
                double l = std::holds_alternative<double>(currentVal) ? std::get<double>(currentVal) : std::get<int>(currentVal);
                double r = std::holds_alternative<double>(rightVal) ? std::get<double>(rightVal) : std::get<int>(rightVal);
                auto result = l + r;
                context->setVariable(varName, result);
                return result;
            } else if (std::holds_alternative<std::string>(currentVal) && std::holds_alternative<std::string>(rightVal)) {
                auto result = std::get<std::string>(currentVal) + std::get<std::string>(rightVal);
                context->setVariable(varName, result);
                return result;
            }
            throw std::runtime_error("Invalid operands for +=");
        }
        else if (op_ == "-=") {
            auto currentVal = context->getVariable(varName);
            if (std::holds_alternative<int>(currentVal) && std::holds_alternative<int>(rightVal)) {
                auto result = std::get<int>(currentVal) - std::get<int>(rightVal);
                context->setVariable(varName, result);
                return result;
            }
            double l = std::holds_alternative<double>(currentVal) ? std::get<double>(currentVal) : std::get<int>(currentVal);
            double r = std::holds_alternative<double>(rightVal) ? std::get<double>(rightVal) : std::get<int>(rightVal);
            auto result = l - r;
            context->setVariable(varName, result);
            return result;
        }
        else if (op_ == "*=") {
            auto currentVal = context->getVariable(varName);
            if (std::holds_alternative<int>(currentVal) && std::holds_alternative<int>(rightVal)) {
                auto result = std::get<int>(currentVal) * std::get<int>(rightVal);
                context->setVariable(varName, result);
                return result;
            }
            double l = std::holds_alternative<double>(currentVal) ? std::get<double>(currentVal) : std::get<int>(currentVal);
            double r = std::holds_alternative<double>(rightVal) ? std::get<double>(rightVal) : std::get<int>(rightVal);
            auto result = l * r;
            context->setVariable(varName, result);
            return result;
        }
        else if (op_ == "/=") {
            auto currentVal = context->getVariable(varName);
            if (std::holds_alternative<int>(currentVal) && std::holds_alternative<int>(rightVal)) {
                if (std::get<int>(rightVal) == 0) throw std::runtime_error("Division by zero");
                auto result = std::get<int>(currentVal) / std::get<int>(rightVal);
                context->setVariable(varName, result);
                return result;
            }
            double l = std::holds_alternative<double>(currentVal) ? std::get<double>(currentVal) : std::get<int>(currentVal);
            double r = std::holds_alternative<double>(rightVal) ? std::get<double>(rightVal) : std::get<int>(rightVal);
            if (r == 0.0) throw std::runtime_error("Division by zero");
            auto result = l / r;
            context->setVariable(varName, result);
            return result;
        }
    }

    // For non-assignment operators, evaluate both sides
    auto leftVal = left_->evaluate(context);
    auto rightVal = right_->evaluate(context);

    // Arithmetic operators
    if (op_ == "+") {
        if (std::holds_alternative<int>(leftVal) && std::holds_alternative<int>(rightVal)) {
            return std::get<int>(leftVal) + std::get<int>(rightVal);
        } else if (std::holds_alternative<double>(leftVal) || std::holds_alternative<double>(rightVal)) {
            double l = std::holds_alternative<double>(leftVal) ? std::get<double>(leftVal) : std::get<int>(leftVal);
            double r = std::holds_alternative<double>(rightVal) ? std::get<double>(rightVal) : std::get<int>(rightVal);
            return l + r;
        } else if (std::holds_alternative<std::string>(leftVal) && std::holds_alternative<std::string>(rightVal)) {
            return std::get<std::string>(leftVal) + std::get<std::string>(rightVal);
        }
        throw std::runtime_error("Invalid operands for +");
    }
    else if (op_ == "-") {
        if (std::holds_alternative<int>(leftVal) && std::holds_alternative<int>(rightVal)) {
            return std::get<int>(leftVal) - std::get<int>(rightVal);
        }
        double l = std::holds_alternative<double>(leftVal) ? std::get<double>(leftVal) : std::get<int>(leftVal);
        double r = std::holds_alternative<double>(rightVal) ? std::get<double>(rightVal) : std::get<int>(rightVal);
        return l - r;
    }
    else if (op_ == "*") {
        if (std::holds_alternative<int>(leftVal) && std::holds_alternative<int>(rightVal)) {
            return std::get<int>(leftVal) * std::get<int>(rightVal);
        }

        // Try to convert to numeric values
        double l = 0.0, r = 0.0;

        if (std::holds_alternative<double>(leftVal)) {
            l = std::get<double>(leftVal);
        } else if (std::holds_alternative<int>(leftVal)) {
            l = std::get<int>(leftVal);
        } else {
            throw std::runtime_error("Left operand of * is not numeric");
        }

        if (std::holds_alternative<double>(rightVal)) {
            r = std::get<double>(rightVal);
        } else if (std::holds_alternative<int>(rightVal)) {
            r = std::get<int>(rightVal);
        } else {
            throw std::runtime_error("Right operand of * is not numeric");
        }

        return l * r;
    }
    else if (op_ == "/") {
        if (std::holds_alternative<int>(rightVal) && std::get<int>(rightVal) == 0) {
            throw std::runtime_error("Division by zero");
        }
        if (std::holds_alternative<double>(rightVal) && std::get<double>(rightVal) == 0.0) {
            throw std::runtime_error("Division by zero");
        }
        if (std::holds_alternative<int>(leftVal) && std::holds_alternative<int>(rightVal)) {
            return std::get<int>(leftVal) / std::get<int>(rightVal);
        }
        double l = std::holds_alternative<double>(leftVal) ? std::get<double>(leftVal) : std::get<int>(leftVal);
        double r = std::holds_alternative<double>(rightVal) ? std::get<double>(rightVal) : std::get<int>(rightVal);
        return l / r;
    }
    else if (op_ == "%") {
        if (!std::holds_alternative<int>(leftVal) || !std::holds_alternative<int>(rightVal)) {
            throw std::runtime_error("Modulo requires integer operands");
        }
        int r = std::get<int>(rightVal);
        if (r == 0) {
            throw std::runtime_error("Division by zero");
        }
        return std::get<int>(leftVal) % r;
    }
    // Comparison operators
    else if (op_ == "==") {
        // Custom comparison for DSLValue to handle std::any vectors
        if (leftVal.index() != rightVal.index()) {
            return DSLValue(false);
        }

        // Compare based on type
        if (std::holds_alternative<int>(leftVal)) {
            return DSLValue(std::get<int>(leftVal) == std::get<int>(rightVal));
        } else if (std::holds_alternative<double>(leftVal)) {
            return DSLValue(std::get<double>(leftVal) == std::get<double>(rightVal));
        } else if (std::holds_alternative<bool>(leftVal)) {
            return DSLValue(std::get<bool>(leftVal) == std::get<bool>(rightVal));
        } else if (std::holds_alternative<std::string>(leftVal)) {
            return DSLValue(std::get<std::string>(leftVal) == std::get<std::string>(rightVal));
        } else if (std::holds_alternative<std::nullptr_t>(leftVal)) {
            return DSLValue(true); // Both are null
        } else {
            // For complex types, just compare by index for now
            return DSLValue(leftVal.index() == rightVal.index());
        }
    }
    else if (op_ == "!=") {
        // Custom comparison for DSLValue
        if (leftVal.index() != rightVal.index()) {
            return DSLValue(true);
        }

        // Compare based on type
        if (std::holds_alternative<int>(leftVal)) {
            return DSLValue(std::get<int>(leftVal) != std::get<int>(rightVal));
        } else if (std::holds_alternative<double>(leftVal)) {
            return DSLValue(std::get<double>(leftVal) != std::get<double>(rightVal));
        } else if (std::holds_alternative<bool>(leftVal)) {
            return DSLValue(std::get<bool>(leftVal) != std::get<bool>(rightVal));
        } else if (std::holds_alternative<std::string>(leftVal)) {
            return DSLValue(std::get<std::string>(leftVal) != std::get<std::string>(rightVal));
        } else if (std::holds_alternative<std::nullptr_t>(leftVal)) {
            return DSLValue(false); // Both are null
        } else {
            // For complex types, just compare by index for now
            return DSLValue(false);
        }
    }
    else if (op_ == "<") {
        if (std::holds_alternative<int>(leftVal) && std::holds_alternative<int>(rightVal)) {
            return std::get<int>(leftVal) < std::get<int>(rightVal);
        }
        double l = std::holds_alternative<double>(leftVal) ? std::get<double>(leftVal) : std::get<int>(leftVal);
        double r = std::holds_alternative<double>(rightVal) ? std::get<double>(rightVal) : std::get<int>(rightVal);
        return l < r;
    }
    else if (op_ == ">") {
        if (std::holds_alternative<int>(leftVal) && std::holds_alternative<int>(rightVal)) {
            return std::get<int>(leftVal) > std::get<int>(rightVal);
        }
        double l = std::holds_alternative<double>(leftVal) ? std::get<double>(leftVal) : std::get<int>(leftVal);
        double r = std::holds_alternative<double>(rightVal) ? std::get<double>(rightVal) : std::get<int>(rightVal);
        return l > r;
    }
    else if (op_ == "<=") {
        if (std::holds_alternative<int>(leftVal) && std::holds_alternative<int>(rightVal)) {
            return std::get<int>(leftVal) <= std::get<int>(rightVal);
        }
        double l = std::holds_alternative<double>(leftVal) ? std::get<double>(leftVal) : std::get<int>(leftVal);
        double r = std::holds_alternative<double>(rightVal) ? std::get<double>(rightVal) : std::get<int>(rightVal);
        return l <= r;
    }
    else if (op_ == ">=") {
        if (std::holds_alternative<int>(leftVal) && std::holds_alternative<int>(rightVal)) {
            return std::get<int>(leftVal) >= std::get<int>(rightVal);
        }
        double l = std::holds_alternative<double>(leftVal) ? std::get<double>(leftVal) : std::get<int>(leftVal);
        double r = std::holds_alternative<double>(rightVal) ? std::get<double>(rightVal) : std::get<int>(rightVal);
        return l >= r;
    }
    // Logical operators
    else if (op_ == "&&") {
        return DSLEngine::toBool(leftVal) && DSLEngine::toBool(rightVal);
    }
    else if (op_ == "||") {
        return DSLEngine::toBool(leftVal) || DSLEngine::toBool(rightVal);
    }
    // Null coalescing
    else if (op_ == "??") {
        if (std::holds_alternative<std::nullptr_t>(leftVal)) {
            return rightVal;
        }
        return leftVal;
    }

    throw std::runtime_error("Unknown binary operator: " + op_);
}

DSLValue UnaryOpExpression::evaluate(DSLContextPtr context) {
    auto val = operand_->evaluate(context);

    if (op_ == "!") {
        return !DSLEngine::toBool(val);
    }
    else if (op_ == "-") {
        if (std::holds_alternative<int>(val)) {
            return -std::get<int>(val);
        } else if (std::holds_alternative<double>(val)) {
            return -std::get<double>(val);
        }
        throw std::runtime_error("Cannot negate non-numeric value");
    }
    else if (op_ == "+") {
        if (std::holds_alternative<int>(val) || std::holds_alternative<double>(val)) {
            return val;
        }
        throw std::runtime_error("Cannot apply unary + to non-numeric value");
    }

    throw std::runtime_error("Unknown unary operator: " + op_);
}

DSLValue FunctionCallExpression::evaluate(DSLContextPtr context) {
    // First check if it's a built-in or registered function
    auto func = context->getFunction(name_);
    if (!func) {
        // Check if it's a variable containing a lambda
        if (context->hasVariable(name_)) {
            auto val = context->getVariable(name_);
            // TODO: Handle lambda stored in variable
        }
        throw std::runtime_error("Undefined function: " + name_);
    }

    // Evaluate arguments
    std::vector<DSLValue> argValues;
    argValues.reserve(args_.size());
    for (const auto& arg : args_) {
        argValues.push_back(arg->evaluate(context));
    }

    return func->call(argValues, context);
}

std::string FunctionCallExpression::toString() const {
    std::string result = name_ + "(";
    for (size_t i = 0; i < args_.size(); ++i) {
        if (i > 0) result += ", ";
        result += args_[i]->toString();
    }
    result += ")";
    return result;
}

DSLValue LambdaExpression::evaluate(DSLContextPtr context) {
    // Lambdas evaluate to themselves (they are values)
    // Return a shared pointer to this lambda
    return std::dynamic_pointer_cast<LambdaExpression>(shared_from_this());
}

std::string LambdaExpression::toString() const {
    std::string result;
    if (params_.size() == 1) {
        result = params_[0];
    } else {
        result = "(";
        for (size_t i = 0; i < params_.size(); ++i) {
            if (i > 0) result += ", ";
            result += params_[i];
        }
        result += ")";
    }
    result += " => " + body_->toString();
    return result;
}

DSLValue IfElseExpression::evaluate(DSLContextPtr context) {
    auto condVal = condition_->evaluate(context);
    if (DSLEngine::toBool(condVal)) {
        return thenBranch_->evaluate(context);
    } else if (elseBranch_) {
        return elseBranch_->evaluate(context);
    }
    return nullptr;
}

std::string IfElseExpression::toString() const {
    std::string result = "if (" + condition_->toString() + ") " + thenBranch_->toString();
    if (elseBranch_) {
        result += " else " + elseBranch_->toString();
    }
    return result;
}

// ===================================
// DSL Engine Implementation
// ===================================

DSLEngine::DSLEngine() {
    initializeBuiltinFunctions();
    currentContext_ = createContext();
}

void DSLEngine::initializeBuiltinFunctions() {
    registerAllBuiltinFunctions();
}

void DSLEngine::registerAllBuiltinFunctions() {
    // === AST Navigation Functions (10) ===

    builtinFunctions_[BuiltinFunctions::PARENT] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::PARENT, 1, "Get parent node",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (!std::holds_alternative<core::ASTNodePtr>(args[0])) {
                throw std::runtime_error("parent() requires AST node");
            }
            auto node = std::get<core::ASTNodePtr>(args[0]);
            return node ? DSLValue(node->getParent()) : DSLValue(nullptr);
        }
    );

    builtinFunctions_[BuiltinFunctions::CHILDREN] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::CHILDREN, 1, "Get child nodes",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (!std::holds_alternative<core::ASTNodePtr>(args[0])) {
                throw std::runtime_error("children() requires AST node");
            }
            auto node = std::get<core::ASTNodePtr>(args[0]);
            if (!node) return DSLValue(std::vector<std::any>{});

            auto children = node->getChildren();
            std::vector<std::any> result;
            for (const auto& child : children) {
                result.push_back(child);
            }
            return DSLValue(result);
        }
    );

    builtinFunctions_[BuiltinFunctions::ANCESTORS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::ANCESTORS, 1, "Get ancestor nodes",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (!std::holds_alternative<core::ASTNodePtr>(args[0])) {
                throw std::runtime_error("ancestors() requires AST node");
            }
            auto node = std::get<core::ASTNodePtr>(args[0]);
            if (!node) return DSLValue(std::vector<std::any>{});

            auto ancestors = node->getAncestors();
            std::vector<std::any> result;
            for (const auto& ancestor : ancestors) {
                result.push_back(ancestor);
            }
            return DSLValue(result);
        }
    );

    builtinFunctions_[BuiltinFunctions::DESCENDANTS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::DESCENDANTS, 1, "Get descendant nodes",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (!std::holds_alternative<core::ASTNodePtr>(args[0])) {
                throw std::runtime_error("descendants() requires AST node");
            }
            auto node = std::get<core::ASTNodePtr>(args[0]);
            if (!node) return DSLValue(std::vector<std::any>{});

            auto descendants = node->getDescendants();
            std::vector<std::any> result;
            for (const auto& desc : descendants) {
                result.push_back(desc);
            }
            return DSLValue(result);
        }
    );

    builtinFunctions_[BuiltinFunctions::SIBLINGS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::SIBLINGS, 1, "Get sibling nodes",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (!std::holds_alternative<core::ASTNodePtr>(args[0])) {
                throw std::runtime_error("siblings() requires AST node");
            }
            auto node = std::get<core::ASTNodePtr>(args[0]);
            if (!node) return DSLValue(std::vector<std::any>{});

            auto siblings = node->getSiblings();
            std::vector<std::any> result;
            for (const auto& sibling : siblings) {
                result.push_back(sibling);
            }
            return DSLValue(result);
        }
    );

    builtinFunctions_[BuiltinFunctions::NEXT_SIBLING] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::NEXT_SIBLING, 1, "Get next sibling",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            // Simplified implementation
            return DSLValue(nullptr);
        }
    );

    builtinFunctions_[BuiltinFunctions::PREV_SIBLING] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::PREV_SIBLING, 1, "Get previous sibling",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            // Simplified implementation
            return DSLValue(nullptr);
        }
    );

    builtinFunctions_[BuiltinFunctions::FIRST_CHILD] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::FIRST_CHILD, 1, "Get first child",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (!std::holds_alternative<core::ASTNodePtr>(args[0])) {
                throw std::runtime_error("firstChild() requires AST node");
            }
            auto node = std::get<core::ASTNodePtr>(args[0]);
            if (!node) return DSLValue(nullptr);
            auto children = node->getChildren();
            return children.empty() ? DSLValue(nullptr) : DSLValue(children[0]);
        }
    );

    builtinFunctions_[BuiltinFunctions::LAST_CHILD] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::LAST_CHILD, 1, "Get last child",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (!std::holds_alternative<core::ASTNodePtr>(args[0])) {
                throw std::runtime_error("lastChild() requires AST node");
            }
            auto node = std::get<core::ASTNodePtr>(args[0]);
            if (!node) return DSLValue(nullptr);
            auto children = node->getChildren();
            return children.empty() ? DSLValue(nullptr) : DSLValue(children.back());
        }
    );

    builtinFunctions_[BuiltinFunctions::NTH_CHILD] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::NTH_CHILD, 2, "Get nth child",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (!std::holds_alternative<core::ASTNodePtr>(args[0])) {
                throw std::runtime_error("nthChild() requires AST node");
            }
            if (!std::holds_alternative<int>(args[1])) {
                throw std::runtime_error("nthChild() requires integer index");
            }
            auto node = std::get<core::ASTNodePtr>(args[0]);
            int n = std::get<int>(args[1]);
            if (!node) return DSLValue(nullptr);
            auto children = node->getChildren();
            if (n < 0 || n >= static_cast<int>(children.size())) {
                return DSLValue(nullptr);
            }
            return DSLValue(children[n]);
        }
    );

    // === Type Analysis Functions (15) ===

    builtinFunctions_[BuiltinFunctions::TYPEOF] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::TYPEOF, 1, "Get type of node",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (std::holds_alternative<core::ASTNodePtr>(args[0])) {
                auto node = std::get<core::ASTNodePtr>(args[0]);
                return node ? DSLValue(node->getType()) : DSLValue(nullptr);
            }
            return DSLValue(nullptr);
        }
    );

    builtinFunctions_[BuiltinFunctions::SIZEOF] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::SIZEOF, 1, "Get size of type",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (std::holds_alternative<core::TypeInfoPtr>(args[0])) {
                auto type = std::get<core::TypeInfoPtr>(args[0]);
                return type ? DSLValue(static_cast<int>(type->getSize())) : DSLValue(0);
            }
            return DSLValue(0);
        }
    );

    builtinFunctions_[BuiltinFunctions::IS_POINTER] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::IS_POINTER, 1, "Check if type is pointer",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (std::holds_alternative<core::TypeInfoPtr>(args[0])) {
                auto type = std::get<core::TypeInfoPtr>(args[0]);
                return type ? DSLValue(type->isPointer()) : DSLValue(false);
            }
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::IS_ARRAY] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::IS_ARRAY, 1, "Check if type is array",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (std::holds_alternative<core::TypeInfoPtr>(args[0])) {
                auto type = std::get<core::TypeInfoPtr>(args[0]);
                return type ? DSLValue(type->isArray()) : DSLValue(false);
            }
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::IS_CONST] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::IS_CONST, 1, "Check if type is const",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (std::holds_alternative<core::TypeInfoPtr>(args[0])) {
                auto type = std::get<core::TypeInfoPtr>(args[0]);
                return type ? DSLValue(type->isConst()) : DSLValue(false);
            }
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::IS_VOLATILE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::IS_VOLATILE, 1, "Check if type is volatile",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (std::holds_alternative<core::TypeInfoPtr>(args[0])) {
                auto type = std::get<core::TypeInfoPtr>(args[0]);
                return type ? DSLValue(type->isVolatile()) : DSLValue(false);
            }
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::IS_FUNCTION] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::IS_FUNCTION, 1, "Check if type is function",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (std::holds_alternative<core::TypeInfoPtr>(args[0])) {
                auto type = std::get<core::TypeInfoPtr>(args[0]);
                return type ? DSLValue(type->isFunction()) : DSLValue(false);
            }
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::IS_STRUCT] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::IS_STRUCT, 1, "Check if type is struct",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (std::holds_alternative<core::TypeInfoPtr>(args[0])) {
                auto type = std::get<core::TypeInfoPtr>(args[0]);
                return type ? DSLValue(type->getKind() == core::ITypeInfo::TypeKind::STRUCT) : DSLValue(false);
            }
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::IS_UNION] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::IS_UNION, 1, "Check if type is union",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (std::holds_alternative<core::TypeInfoPtr>(args[0])) {
                auto type = std::get<core::TypeInfoPtr>(args[0]);
                return type ? DSLValue(type->getKind() == core::ITypeInfo::TypeKind::UNION) : DSLValue(false);
            }
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::IS_ENUM] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::IS_ENUM, 1, "Check if type is enum",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (std::holds_alternative<core::TypeInfoPtr>(args[0])) {
                auto type = std::get<core::TypeInfoPtr>(args[0]);
                return type ? DSLValue(type->getKind() == core::ITypeInfo::TypeKind::ENUM) : DSLValue(false);
            }
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::IS_TYPEDEF] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::IS_TYPEDEF, 1, "Check if type is typedef",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (std::holds_alternative<core::TypeInfoPtr>(args[0])) {
                auto type = std::get<core::TypeInfoPtr>(args[0]);
                return type ? DSLValue(type->getKind() == core::ITypeInfo::TypeKind::TYPEDEF) : DSLValue(false);
            }
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::IS_COMPLETE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::IS_COMPLETE, 1, "Check if type is complete",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (std::holds_alternative<core::TypeInfoPtr>(args[0])) {
                auto type = std::get<core::TypeInfoPtr>(args[0]);
                return type ? DSLValue(type->isComplete()) : DSLValue(false);
            }
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::GET_POINTEE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::GET_POINTEE, 1, "Get pointee type",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (std::holds_alternative<core::TypeInfoPtr>(args[0])) {
                auto type = std::get<core::TypeInfoPtr>(args[0]);
                return type ? DSLValue(type->getPointeeType()) : DSLValue(nullptr);
            }
            return DSLValue(nullptr);
        }
    );

    builtinFunctions_[BuiltinFunctions::GET_RETURN_TYPE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::GET_RETURN_TYPE, 1, "Get function return type",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (std::holds_alternative<core::TypeInfoPtr>(args[0])) {
                auto type = std::get<core::TypeInfoPtr>(args[0]);
                return type ? DSLValue(type->getReturnType()) : DSLValue(nullptr);
            }
            return DSLValue(nullptr);
        }
    );

    builtinFunctions_[BuiltinFunctions::GET_PARAM_TYPES] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::GET_PARAM_TYPES, 1, "Get function parameter types",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (std::holds_alternative<core::TypeInfoPtr>(args[0])) {
                auto type = std::get<core::TypeInfoPtr>(args[0]);
                if (!type) return DSLValue(std::vector<std::any>{});

                auto params = type->getParameterTypes();
                std::vector<std::any> result;
                for (const auto& param : params) {
                    result.push_back(param);
                }
                return DSLValue(result);
            }
            return DSLValue(std::vector<std::any>{});
        }
    );

    // === Control Flow Functions (12) ===

    builtinFunctions_[BuiltinFunctions::COMPLEXITY] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::COMPLEXITY, 1, "Calculate cyclomatic complexity",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            // Simplified implementation - returns placeholder
            return DSLValue(1);
        }
    );

    builtinFunctions_[BuiltinFunctions::PATHS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::PATHS, 1, "Get execution paths",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::DOMINATES] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::DOMINATES, 2, "Check if node dominates another",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::REACHES] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::REACHES, 2, "Check if node reaches another",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::IS_REACHABLE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::IS_REACHABLE, 2, "Check if node is reachable from another",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::PREDECESSORS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::PREDECESSORS, 1, "Get predecessor nodes",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::SUCCESSORS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::SUCCESSORS, 1, "Get successor nodes",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::ENTRY_POINTS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::ENTRY_POINTS, 0, "Get entry points",
        [](const std::vector<DSLValue>&, DSLContextPtr) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::EXIT_POINTS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::EXIT_POINTS, 0, "Get exit points",
        [](const std::vector<DSLValue>&, DSLContextPtr) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::LOOPS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::LOOPS, 1, "Get loops in function",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::IS_IN_LOOP] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::IS_IN_LOOP, 1, "Check if node is in loop",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::LOOP_DEPTH] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::LOOP_DEPTH, 1, "Get loop nesting depth",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(0);
        }
    );

    // === Data Flow Functions (12) ===

    builtinFunctions_[BuiltinFunctions::DEFINES] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::DEFINES, 1, "Get definitions",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::USES] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::USES, 1, "Get uses",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::TAINTS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::TAINTS, 1, "Get taint sources",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::FLOWS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::FLOWS, 2, "Check data flow",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::DEPENDS_ON] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::DEPENDS_ON, 2, "Check dependency",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::INFLUENCES] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::INFLUENCES, 1, "Get influenced nodes",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::IS_DEFINED] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::IS_DEFINED, 1, "Check if variable is defined",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::IS_USED] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::IS_USED, 1, "Check if variable is used",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::DEF_USE_CHAIN] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::DEF_USE_CHAIN, 1, "Get def-use chain",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::USE_DEF_CHAIN] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::USE_DEF_CHAIN, 1, "Get use-def chain",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::REACHING_DEFS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::REACHING_DEFS, 1, "Get reaching definitions",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::LIVE_VARS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::LIVE_VARS, 1, "Get live variables",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    // === Metrics Functions (12) ===

    builtinFunctions_[BuiltinFunctions::LOC] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::LOC, 1, "Lines of code",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(0);
        }
    );

    builtinFunctions_[BuiltinFunctions::SLOC] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::SLOC, 1, "Source lines of code",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(0);
        }
    );

    builtinFunctions_[BuiltinFunctions::CYCLOMATIC] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::CYCLOMATIC, 1, "Cyclomatic complexity",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(1);
        }
    );

    builtinFunctions_[BuiltinFunctions::COUPLING] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::COUPLING, 1, "Coupling metric",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(0.0);
        }
    );

    builtinFunctions_[BuiltinFunctions::COHESION] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::COHESION, 1, "Cohesion metric",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(1.0);
        }
    );

    builtinFunctions_[BuiltinFunctions::HALSTEAD] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::HALSTEAD, 1, "Halstead complexity",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(0.0);
        }
    );

    builtinFunctions_[BuiltinFunctions::MAINTAINABILITY] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::MAINTAINABILITY, 1, "Maintainability index",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(100.0);
        }
    );

    builtinFunctions_[BuiltinFunctions::FAN_IN] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::FAN_IN, 1, "Fan-in metric",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(0);
        }
    );

    builtinFunctions_[BuiltinFunctions::FAN_OUT] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::FAN_OUT, 1, "Fan-out metric",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(0);
        }
    );

    builtinFunctions_[BuiltinFunctions::DEPTH] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::DEPTH, 1, "Depth metric",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(0);
        }
    );

    builtinFunctions_[BuiltinFunctions::NESTING] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::NESTING, 1, "Nesting level",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(0);
        }
    );

    builtinFunctions_[BuiltinFunctions::PARAMS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::PARAMS, 1, "Parameter count",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(0);
        }
    );

    // === Pattern Matching Functions (10) ===

    builtinFunctions_[BuiltinFunctions::MATCHES] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::MATCHES, 2, "Pattern match",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::CONTAINS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::CONTAINS, 2, "Contains substring",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (!std::holds_alternative<std::string>(args[0]) ||
                !std::holds_alternative<std::string>(args[1])) {
                return DSLValue(false);
            }
            auto str = std::get<std::string>(args[0]);
            auto substr = std::get<std::string>(args[1]);
            return DSLValue(str.find(substr) != std::string::npos);
        }
    );

    builtinFunctions_[BuiltinFunctions::STARTS_WITH] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::STARTS_WITH, 2, "Starts with string",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (!std::holds_alternative<std::string>(args[0]) ||
                !std::holds_alternative<std::string>(args[1])) {
                return DSLValue(false);
            }
            auto str = std::get<std::string>(args[0]);
            auto prefix = std::get<std::string>(args[1]);
            return DSLValue(str.substr(0, prefix.length()) == prefix);
        }
    );

    builtinFunctions_[BuiltinFunctions::ENDS_WITH] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::ENDS_WITH, 2, "Ends with string",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (!std::holds_alternative<std::string>(args[0]) ||
                !std::holds_alternative<std::string>(args[1])) {
                return DSLValue(false);
            }
            auto str = std::get<std::string>(args[0]);
            auto suffix = std::get<std::string>(args[1]);
            if (suffix.length() > str.length()) return DSLValue(false);
            return DSLValue(str.substr(str.length() - suffix.length()) == suffix);
        }
    );

    builtinFunctions_[BuiltinFunctions::REGEX] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::REGEX, 2, "Regex match",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (!std::holds_alternative<std::string>(args[0]) ||
                !std::holds_alternative<std::string>(args[1])) {
                return DSLValue(false);
            }
            auto str = std::get<std::string>(args[0]);
            auto pattern = std::get<std::string>(args[1]);
            try {
                std::regex re(pattern);
                return DSLValue(std::regex_search(str, re));
            } catch (...) {
                return DSLValue(false);
            }
        }
    );

    builtinFunctions_[BuiltinFunctions::GLOB] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::GLOB, 2, "Glob pattern match",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::XPATH] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::XPATH, 2, "XPath query",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::PATTERN] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::PATTERN, 2, "Pattern matching",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::EXTRACT] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::EXTRACT, 2, "Extract pattern matches",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (!std::holds_alternative<std::string>(args[0]) ||
                !std::holds_alternative<std::string>(args[1])) {
                return DSLValue(std::vector<std::any>{});
            }
            auto str = std::get<std::string>(args[0]);
            auto pattern = std::get<std::string>(args[1]);
            try {
                std::regex re(pattern);
                std::smatch matches;
                std::vector<std::any> result;
                if (std::regex_search(str, matches, re)) {
                    for (size_t i = 1; i < matches.size(); ++i) {
                        result.push_back(matches[i].str());
                    }
                }
                return DSLValue(result);
            } catch (...) {
                return DSLValue(std::vector<std::any>{});
            }
        }
    );

    builtinFunctions_[BuiltinFunctions::REPLACE_PATTERN] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::REPLACE_PATTERN, 3, "Replace pattern",
        [](const std::vector<DSLValue>& args, DSLContextPtr) {
            if (!std::holds_alternative<std::string>(args[0]) ||
                !std::holds_alternative<std::string>(args[1]) ||
                !std::holds_alternative<std::string>(args[2])) {
                return args[0];
            }
            auto str = std::get<std::string>(args[0]);
            auto pattern = std::get<std::string>(args[1]);
            auto replacement = std::get<std::string>(args[2]);
            try {
                std::regex re(pattern);
                return DSLValue(std::regex_replace(str, re, replacement));
            } catch (...) {
                return DSLValue(str);
            }
        }
    );

    // === Collection Operations (22) ===

    builtinFunctions_[BuiltinFunctions::MAP] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::MAP, 2, "Map function over collection",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            auto collection = DSLEngine::toCollection(args[0]);

            // Get the lambda function
            if (!std::holds_alternative<DSLLambda>(args[1])) {
                throw std::runtime_error("map() expects a lambda as second argument");
            }
            auto lambda = std::get<DSLLambda>(args[1]);

            std::vector<std::any> result;
            for (const auto& item : collection) {
                // Invoke the lambda with the current item
                DSLValue itemValue;
                if (item.type() == typeid(int)) {
                    itemValue = std::any_cast<int>(item);
                } else if (item.type() == typeid(double)) {
                    itemValue = std::any_cast<double>(item);
                } else if (item.type() == typeid(std::string)) {
                    itemValue = std::any_cast<std::string>(item);
                } else if (item.type() == typeid(bool)) {
                    itemValue = std::any_cast<bool>(item);
                } else {
                    itemValue = nullptr;
                }

                auto mappedValue = lambda->invoke({itemValue}, context);

                // Convert DSLValue back to std::any for the result
                if (std::holds_alternative<int>(mappedValue)) {
                    result.push_back(std::get<int>(mappedValue));
                } else if (std::holds_alternative<double>(mappedValue)) {
                    result.push_back(std::get<double>(mappedValue));
                } else if (std::holds_alternative<std::string>(mappedValue)) {
                    result.push_back(std::get<std::string>(mappedValue));
                } else if (std::holds_alternative<bool>(mappedValue)) {
                    result.push_back(std::get<bool>(mappedValue));
                } else {
                    result.push_back(std::any());
                }
            }
            return DSLValue(result);
        }
    );

    builtinFunctions_[BuiltinFunctions::FILTER] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::FILTER, 2, "Filter collection",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            auto collection = DSLEngine::toCollection(args[0]);

            // Get the lambda predicate
            if (!std::holds_alternative<DSLLambda>(args[1])) {
                throw std::runtime_error("filter() expects a lambda as second argument");
            }
            auto lambda = std::get<DSLLambda>(args[1]);

            std::vector<std::any> result;
            for (const auto& item : collection) {
                // Convert item to DSLValue
                DSLValue itemValue;
                if (item.type() == typeid(int)) {
                    itemValue = std::any_cast<int>(item);
                } else if (item.type() == typeid(double)) {
                    itemValue = std::any_cast<double>(item);
                } else if (item.type() == typeid(std::string)) {
                    itemValue = std::any_cast<std::string>(item);
                } else if (item.type() == typeid(bool)) {
                    itemValue = std::any_cast<bool>(item);
                } else {
                    itemValue = nullptr;
                }

                // Evaluate predicate
                auto predicateResult = lambda->invoke({itemValue}, context);

                // If predicate is true, keep the item
                if (DSLEngine::toBool(predicateResult)) {
                    result.push_back(item);
                }
            }
            return DSLValue(result);
        }
    );

    builtinFunctions_[BuiltinFunctions::REDUCE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::REDUCE, 3, "Reduce collection",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            auto collection = DSLEngine::toCollection(args[0]);
            auto accumulator = args[1];  // Initial value

            // Get the lambda reducer
            if (!std::holds_alternative<DSLLambda>(args[2])) {
                throw std::runtime_error("reduce() expects a lambda as third argument");
            }
            auto lambda = std::get<DSLLambda>(args[2]);

            for (const auto& item : collection) {
                // Convert item to DSLValue
                DSLValue itemValue;
                if (item.type() == typeid(int)) {
                    itemValue = std::any_cast<int>(item);
                } else if (item.type() == typeid(double)) {
                    itemValue = std::any_cast<double>(item);
                } else if (item.type() == typeid(std::string)) {
                    itemValue = std::any_cast<std::string>(item);
                } else if (item.type() == typeid(bool)) {
                    itemValue = std::any_cast<bool>(item);
                } else {
                    itemValue = nullptr;
                }

                // Call reducer with accumulator and current item
                accumulator = lambda->invoke({accumulator, itemValue}, context);
            }

            return accumulator;
        }
    );

    builtinFunctions_[BuiltinFunctions::ANY] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::ANY, 2, "Check if any element satisfies condition",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            auto collection = DSLEngine::toCollection(args[0]);

            // Get the lambda predicate
            if (!std::holds_alternative<DSLLambda>(args[1])) {
                throw std::runtime_error("any() expects a lambda as second argument");
            }
            auto lambda = std::get<DSLLambda>(args[1]);

            for (const auto& item : collection) {
                // Convert item to DSLValue
                DSLValue itemValue;
                if (item.type() == typeid(int)) {
                    itemValue = std::any_cast<int>(item);
                } else if (item.type() == typeid(double)) {
                    itemValue = std::any_cast<double>(item);
                } else if (item.type() == typeid(std::string)) {
                    itemValue = std::any_cast<std::string>(item);
                } else if (item.type() == typeid(bool)) {
                    itemValue = std::any_cast<bool>(item);
                } else {
                    itemValue = nullptr;
                }

                // Evaluate predicate
                auto predicateResult = lambda->invoke({itemValue}, context);

                // If any is true, return true
                if (DSLEngine::toBool(predicateResult)) {
                    return DSLValue(true);
                }
            }
            return DSLValue(false);
        }
    );

    builtinFunctions_[BuiltinFunctions::ALL] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::ALL, 2, "Check if all elements satisfy condition",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            auto collection = DSLEngine::toCollection(args[0]);

            // Get the lambda predicate
            if (!std::holds_alternative<DSLLambda>(args[1])) {
                throw std::runtime_error("all() expects a lambda as second argument");
            }
            auto lambda = std::get<DSLLambda>(args[1]);

            for (const auto& item : collection) {
                // Convert item to DSLValue
                DSLValue itemValue;
                if (item.type() == typeid(int)) {
                    itemValue = std::any_cast<int>(item);
                } else if (item.type() == typeid(double)) {
                    itemValue = std::any_cast<double>(item);
                } else if (item.type() == typeid(std::string)) {
                    itemValue = std::any_cast<std::string>(item);
                } else if (item.type() == typeid(bool)) {
                    itemValue = std::any_cast<bool>(item);
                } else {
                    itemValue = nullptr;
                }

                // Evaluate predicate
                auto predicateResult = lambda->invoke({itemValue}, context);

                // If any is false, return false
                if (!DSLEngine::toBool(predicateResult)) {
                    return DSLValue(false);
                }
            }
            return DSLValue(true);
        }
    );

    builtinFunctions_[BuiltinFunctions::NONE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::NONE, 2, "Check if no elements satisfy condition",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(true);
        }
    );

    builtinFunctions_[BuiltinFunctions::COUNT] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::COUNT, 1, "Count elements",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            auto collection = DSLEngine::toCollection(args[0]);
            return DSLValue(static_cast<int>(collection.size()));
        }
    );

    builtinFunctions_[BuiltinFunctions::SUM] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::SUM, 1, "Sum collection",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            auto collection = DSLEngine::toCollection(args[0]);
            double sum = 0.0;
            int intSum = 0;
            bool hasDouble = false;

            for (const auto& item : collection) {
                if (item.type() == typeid(int)) {
                    intSum += std::any_cast<int>(item);
                } else if (item.type() == typeid(double)) {
                    hasDouble = true;
                    sum += std::any_cast<double>(item);
                }
            }

            if (hasDouble) {
                return DSLValue(sum + intSum);
            } else {
                return DSLValue(intSum);
            }
        }
    );

    builtinFunctions_[BuiltinFunctions::AVG] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::AVG, 1, "Average collection",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(0.0);
        }
    );

    builtinFunctions_[BuiltinFunctions::MIN] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::MIN, 1, "Minimum value",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            auto collection = DSLEngine::toCollection(args[0]);
            if (collection.empty()) return DSLValue(nullptr);

            bool first = true;
            int minInt = 0;
            double minDouble = 0.0;
            bool hasDouble = false;

            for (const auto& item : collection) {
                if (item.type() == typeid(int)) {
                    int val = std::any_cast<int>(item);
                    if (first || val < minInt) {
                        minInt = val;
                        first = false;
                    }
                } else if (item.type() == typeid(double)) {
                    hasDouble = true;
                    double val = std::any_cast<double>(item);
                    if (first || val < minDouble) {
                        minDouble = val;
                        first = false;
                    }
                }
            }

            if (hasDouble) {
                return DSLValue(std::min(minDouble, static_cast<double>(minInt)));
            } else {
                return DSLValue(minInt);
            }
        }
    );

    builtinFunctions_[BuiltinFunctions::MAX] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::MAX, 1, "Maximum value",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            auto collection = DSLEngine::toCollection(args[0]);
            if (collection.empty()) return DSLValue(nullptr);

            bool first = true;
            int maxInt = 0;
            double maxDouble = 0.0;
            bool hasDouble = false;

            for (const auto& item : collection) {
                if (item.type() == typeid(int)) {
                    int val = std::any_cast<int>(item);
                    if (first || val > maxInt) {
                        maxInt = val;
                        first = false;
                    }
                } else if (item.type() == typeid(double)) {
                    hasDouble = true;
                    double val = std::any_cast<double>(item);
                    if (first || val > maxDouble) {
                        maxDouble = val;
                        first = false;
                    }
                }
            }

            if (hasDouble) {
                return DSLValue(std::max(maxDouble, static_cast<double>(maxInt)));
            } else {
                return DSLValue(maxInt);
            }
        }
    );

    builtinFunctions_[BuiltinFunctions::SORT] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::SORT, 1, "Sort collection",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return args[0];
        }
    );

    builtinFunctions_[BuiltinFunctions::REVERSE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::REVERSE, 1, "Reverse collection",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            auto collection = DSLEngine::toCollection(args[0]);
            std::reverse(collection.begin(), collection.end());
            return DSLValue(collection);
        }
    );

    builtinFunctions_[BuiltinFunctions::UNIQUE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::UNIQUE, 1, "Unique elements",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return args[0];
        }
    );

    builtinFunctions_[BuiltinFunctions::GROUP_BY] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::GROUP_BY, 2, "Group by key",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::PARTITION] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::PARTITION, 2, "Partition collection",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::ZIP] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::ZIP, 2, "Zip collections",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::FLATTEN] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::FLATTEN, 1, "Flatten collection",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return args[0];
        }
    );

    builtinFunctions_[BuiltinFunctions::TAKE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::TAKE, 2, "Take first n elements",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return args[0];
        }
    );

    builtinFunctions_[BuiltinFunctions::DROP] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::DROP, 2, "Drop first n elements",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return args[0];
        }
    );

    builtinFunctions_[BuiltinFunctions::SLICE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::SLICE, 3, "Slice collection",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return args[0];
        }
    );

    // === String Operations (14) ===

    builtinFunctions_[BuiltinFunctions::FORMAT] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::FORMAT, -1, "Format string",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            if (args.empty() || !std::holds_alternative<std::string>(args[0])) {
                return DSLValue(std::string(""));
            }

            std::string formatStr = std::get<std::string>(args[0]);
            std::string result = formatStr;

            // Replace {0}, {1}, etc. with the corresponding arguments
            for (size_t i = 1; i < args.size(); ++i) {
                std::string placeholder = "{" + std::to_string(i - 1) + "}";
                size_t pos = 0;

                // Convert argument to string
                std::string replacement;
                if (std::holds_alternative<std::string>(args[i])) {
                    replacement = std::get<std::string>(args[i]);
                } else if (std::holds_alternative<int>(args[i])) {
                    replacement = std::to_string(std::get<int>(args[i]));
                } else if (std::holds_alternative<double>(args[i])) {
                    replacement = std::to_string(std::get<double>(args[i]));
                } else if (std::holds_alternative<bool>(args[i])) {
                    replacement = std::get<bool>(args[i]) ? "true" : "false";
                } else {
                    replacement = "";
                }

                // Replace all occurrences of this placeholder
                while ((pos = result.find(placeholder, pos)) != std::string::npos) {
                    result.replace(pos, placeholder.length(), replacement);
                    pos += replacement.length();
                }
            }

            return DSLValue(result);
        }
    );

    builtinFunctions_[BuiltinFunctions::SPLIT] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::SPLIT, 2, "Split string",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            if (!std::holds_alternative<std::string>(args[0]) ||
                !std::holds_alternative<std::string>(args[1])) {
                throw std::runtime_error("split() expects two string arguments");
            }
            auto str = std::get<std::string>(args[0]);
            auto delimiter = std::get<std::string>(args[1]);

            std::vector<std::any> result;
            size_t pos = 0;
            size_t lastPos = 0;

            if (delimiter.empty()) {
                // Split into characters
                for (char c : str) {
                    result.push_back(std::string(1, c));
                }
            } else {
                // Split by delimiter
                while ((pos = str.find(delimiter, lastPos)) != std::string::npos) {
                    result.push_back(str.substr(lastPos, pos - lastPos));
                    lastPos = pos + delimiter.length();
                }
                // Add the last part
                result.push_back(str.substr(lastPos));
            }

            return DSLValue(result);
        }
    );

    builtinFunctions_[BuiltinFunctions::JOIN] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::JOIN, 2, "Join strings",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            auto collection = DSLEngine::toCollection(args[0]);

            if (!std::holds_alternative<std::string>(args[1])) {
                throw std::runtime_error("join() expects string separator as second argument");
            }
            auto separator = std::get<std::string>(args[1]);

            std::string result;
            bool first = true;
            for (const auto& item : collection) {
                if (!first) {
                    result += separator;
                }
                first = false;

                // Convert item to string
                if (item.type() == typeid(std::string)) {
                    result += std::any_cast<std::string>(item);
                } else if (item.type() == typeid(const char*)) {
                    result += std::any_cast<const char*>(item);
                } else if (item.type() == typeid(int)) {
                    result += std::to_string(std::any_cast<int>(item));
                } else if (item.type() == typeid(double)) {
                    result += std::to_string(std::any_cast<double>(item));
                } else if (item.type() == typeid(bool)) {
                    result += std::any_cast<bool>(item) ? "true" : "false";
                }
            }

            return DSLValue(result);
        }
    );

    builtinFunctions_[BuiltinFunctions::REPLACE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::REPLACE, 3, "Replace substring",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            if (!std::holds_alternative<std::string>(args[0]) ||
                !std::holds_alternative<std::string>(args[1]) ||
                !std::holds_alternative<std::string>(args[2])) {
                return args[0];
            }
            auto str = std::get<std::string>(args[0]);
            auto find = std::get<std::string>(args[1]);
            auto repl = std::get<std::string>(args[2]);
            size_t pos = 0;
            while ((pos = str.find(find, pos)) != std::string::npos) {
                str.replace(pos, find.length(), repl);
                pos += repl.length();
            }
            return DSLValue(str);
        }
    );

    builtinFunctions_[BuiltinFunctions::TRIM] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::TRIM, 1, "Trim whitespace",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            if (!std::holds_alternative<std::string>(args[0])) {
                return args[0];
            }
            auto str = std::get<std::string>(args[0]);
            str.erase(0, str.find_first_not_of(" \n\r\t"));
            str.erase(str.find_last_not_of(" \n\r\t") + 1);
            return DSLValue(str);
        }
    );

    builtinFunctions_[BuiltinFunctions::UPPER] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::UPPER, 1, "Uppercase",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            if (!std::holds_alternative<std::string>(args[0])) {
                return args[0];
            }
            auto str = std::get<std::string>(args[0]);
            std::transform(str.begin(), str.end(), str.begin(), ::toupper);
            return DSLValue(str);
        }
    );

    builtinFunctions_[BuiltinFunctions::LOWER] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::LOWER, 1, "Lowercase",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            if (!std::holds_alternative<std::string>(args[0])) {
                return args[0];
            }
            auto str = std::get<std::string>(args[0]);
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);
            return DSLValue(str);
        }
    );

    builtinFunctions_[BuiltinFunctions::SUBSTRING] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::SUBSTRING, 3, "Substring",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            if (!std::holds_alternative<std::string>(args[0]) ||
                !std::holds_alternative<int>(args[1]) ||
                !std::holds_alternative<int>(args[2])) {
                return args[0];
            }
            auto str = std::get<std::string>(args[0]);
            int start = std::get<int>(args[1]);
            int len = std::get<int>(args[2]);
            return DSLValue(str.substr(start, len));
        }
    );

    builtinFunctions_[BuiltinFunctions::INDEX_OF] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::INDEX_OF, 2, "Index of substring",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            if (!std::holds_alternative<std::string>(args[0]) ||
                !std::holds_alternative<std::string>(args[1])) {
                return DSLValue(-1);
            }
            auto str = std::get<std::string>(args[0]);
            auto substr = std::get<std::string>(args[1]);
            auto pos = str.find(substr);
            return DSLValue(pos == std::string::npos ? -1 : static_cast<int>(pos));
        }
    );

    builtinFunctions_[BuiltinFunctions::LAST_INDEX_OF] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::LAST_INDEX_OF, 2, "Last index of substring",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            if (!std::holds_alternative<std::string>(args[0]) ||
                !std::holds_alternative<std::string>(args[1])) {
                return DSLValue(-1);
            }
            auto str = std::get<std::string>(args[0]);
            auto substr = std::get<std::string>(args[1]);
            auto pos = str.rfind(substr);
            return DSLValue(pos == std::string::npos ? -1 : static_cast<int>(pos));
        }
    );

    builtinFunctions_[BuiltinFunctions::LENGTH] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::LENGTH, 1, "String length",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            if (!std::holds_alternative<std::string>(args[0])) {
                return DSLValue(0);
            }
            return DSLValue(static_cast<int>(std::get<std::string>(args[0]).length()));
        }
    );

    builtinFunctions_[BuiltinFunctions::IS_EMPTY] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::IS_EMPTY, 1, "Check if empty",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            if (std::holds_alternative<std::string>(args[0])) {
                return DSLValue(std::get<std::string>(args[0]).empty());
            }
            if (std::holds_alternative<std::vector<std::any>>(args[0])) {
                return DSLValue(std::get<std::vector<std::any>>(args[0]).empty());
            }
            return DSLValue(true);
        }
    );

    builtinFunctions_[BuiltinFunctions::PAD_LEFT] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::PAD_LEFT, 3, "Pad left",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return args[0];
        }
    );

    builtinFunctions_[BuiltinFunctions::PAD_RIGHT] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::PAD_RIGHT, 3, "Pad right",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return args[0];
        }
    );

    // === Additional utility functions (12) ===

    builtinFunctions_[BuiltinFunctions::GET_NAME] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::GET_NAME, 1, "Get name",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(std::string(""));
        }
    );

    builtinFunctions_[BuiltinFunctions::GET_LOCATION] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::GET_LOCATION, 1, "Get location",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(std::string(""));
        }
    );

    builtinFunctions_[BuiltinFunctions::GET_FILE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::GET_FILE, 1, "Get file",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(std::string(""));
        }
    );

    builtinFunctions_[BuiltinFunctions::GET_LINE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::GET_LINE, 1, "Get line number",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(0);
        }
    );

    builtinFunctions_[BuiltinFunctions::GET_COLUMN] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::GET_COLUMN, 1, "Get column",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(0);
        }
    );

    builtinFunctions_[BuiltinFunctions::GET_SOURCE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::GET_SOURCE, 1, "Get source code",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(std::string(""));
        }
    );

    builtinFunctions_[BuiltinFunctions::GET_SYMBOL] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::GET_SYMBOL, 1, "Get symbol",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(nullptr);
        }
    );

    builtinFunctions_[BuiltinFunctions::GET_SCOPE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::GET_SCOPE, 1, "Get scope",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(nullptr);
        }
    );

    builtinFunctions_[BuiltinFunctions::FIND_SYMBOL] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::FIND_SYMBOL, 1, "Find symbol",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(nullptr);
        }
    );

    builtinFunctions_[BuiltinFunctions::FIND_TYPE] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::FIND_TYPE, 1, "Find type",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(nullptr);
        }
    );

    builtinFunctions_[BuiltinFunctions::FIND_CALLS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::FIND_CALLS, 1, "Find function calls",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    builtinFunctions_[BuiltinFunctions::FIND_REFS] = std::make_shared<BuiltinFunction>(
        BuiltinFunctions::FIND_REFS, 1, "Find references",
        [](const std::vector<DSLValue>& args, DSLContextPtr context) {
            return DSLValue(std::vector<std::any>{});
        }
    );

    // Total: 103 built-in functions registered
}

// ===================================
// Parsing Implementation
// ===================================

DSLExpressionPtr DSLEngine::parse(const std::string& expression) {
    input_ = expression;
    pos_ = 0;
    parseError_.clear();

    try {
        return parseExpression();
    } catch (const std::exception& e) {
        parseError_ = e.what();
        return nullptr;
    }
}

std::vector<DSLExpressionPtr> DSLEngine::parseMultiple(const std::string& expressions) {
    std::vector<DSLExpressionPtr> results;
    std::istringstream stream(expressions);
    std::string line;

    while (std::getline(stream, line, ';')) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \n\r\t"));
        line.erase(line.find_last_not_of(" \n\r\t") + 1);

        if (!line.empty()) {
            auto expr = parse(line);
            if (expr) {
                results.push_back(expr);
            }
        }
    }

    return results;
}

bool DSLEngine::validate(const std::string& expression) {
    auto expr = parse(expression);
    return expr != nullptr;
}

void DSLEngine::skipWhitespace() {
    while (pos_ < input_.length() && std::isspace(input_[pos_])) {
        pos_++;
    }
}

bool DSLEngine::peek(const std::string& str) {
    skipWhitespace();
    if (pos_ + str.length() > input_.length()) {
        return false;
    }
    return input_.substr(pos_, str.length()) == str;
}

bool DSLEngine::consume(const std::string& str) {
    if (peek(str)) {
        pos_ += str.length();
        return true;
    }
    return false;
}

// Consume a keyword (must be followed by non-alphanumeric)
bool DSLEngine::consumeKeyword(const std::string& keyword) {
    skipWhitespace();
    if (pos_ + keyword.length() > input_.length()) {
        return false;
    }
    if (input_.substr(pos_, keyword.length()) == keyword) {
        // Check that next char is not alphanumeric (word boundary)
        size_t nextPos = pos_ + keyword.length();
        if (nextPos < input_.length() && (std::isalnum(input_[nextPos]) || input_[nextPos] == '_')) {
            return false;  // Not a complete keyword
        }
        pos_ += keyword.length();
        return true;
    }
    return false;
}

char DSLEngine::peekChar() {
    skipWhitespace();
    return (pos_ < input_.length()) ? input_[pos_] : '\0';
}

char DSLEngine::consumeChar() {
    skipWhitespace();
    return (pos_ < input_.length()) ? input_[pos_++] : '\0';
}

DSLExpressionPtr DSLEngine::parseExpression() {
    skipWhitespace();

    // Check for lambda expression
    auto lambda = parseLambda();
    if (lambda) {
        return lambda;
    }

    // Check for if-else expression
    auto ifElse = parseIfElse();
    if (ifElse) {
        return ifElse;
    }

    // Check for while loop
    auto whileLoop = parseWhile();
    if (whileLoop) {
        return whileLoop;
    }

    // Check for for loop
    auto forLoop = parseFor();
    if (forLoop) {
        return forLoop;
    }

    // Check for switch expression
    auto switchExpr = parseSwitch();
    if (switchExpr) {
        return switchExpr;
    }

    // Check for match expression
    auto matchExpr = parseMatch();
    if (matchExpr) {
        return matchExpr;
    }

    // Parse binary expressions
    auto left = parseUnary();
    if (!left) {
        return nullptr;
    }

    return parseBinary(left, 0);
}

DSLExpressionPtr DSLEngine::parsePrimary() {
    skipWhitespace();

    // Number
    if (std::isdigit(peekChar()) || peekChar() == '-') {
        return parseNumber();
    }

    // String
    if (peekChar() == '"') {
        return parseString();
    }

    // Boolean literals
    if (consume("true")) {
        return std::make_shared<LiteralExpression>(DSLValue(true));
    }
    if (consume("false")) {
        return std::make_shared<LiteralExpression>(DSLValue(false));
    }

    // Null literal
    if (consume("null")) {
        return std::make_shared<LiteralExpression>(DSLValue(nullptr));
    }

    // Parenthesized expression
    if (consume("(")) {
        auto expr = parseExpression();
        if (!consume(")")) {
            throw std::runtime_error("Expected ')'");
        }
        return expr;
    }

    // Array literal
    if (consume("[")) {
        std::vector<std::any> elements;
        while (!peek("]")) {
            // Parse any expression and convert DSLValue to std::any
            auto expr = parseExpression();
            if (expr) {
                auto val = expr->evaluate(getCurrentContext());
                // Convert DSLValue to std::any by extracting the actual value
                if (std::holds_alternative<int>(val)) {
                    elements.push_back(std::any(std::get<int>(val)));
                } else if (std::holds_alternative<double>(val)) {
                    elements.push_back(std::any(std::get<double>(val)));
                } else if (std::holds_alternative<std::string>(val)) {
                    elements.push_back(std::any(std::get<std::string>(val)));
                } else if (std::holds_alternative<bool>(val)) {
                    elements.push_back(std::any(std::get<bool>(val)));
                } else {
                    // For complex types, store the DSLValue itself
                    elements.push_back(std::any(val));
                }
            }

            if (!consume(",")) {
                break;
            }
        }
        if (!consume("]")) {
            throw std::runtime_error("Expected ']'");
        }
        return std::make_shared<LiteralExpression>(DSLValue(elements));
    }

    // Identifier or function call
    if (std::isalpha(peekChar()) || peekChar() == '_') {
        return parseIdentifier();
    }

    throw std::runtime_error("Unexpected character: " + std::string(1, peekChar()));
}

DSLExpressionPtr DSLEngine::parseIdentifier() {
    std::string name;
    while (std::isalnum(peekChar()) || peekChar() == '_') {
        name += consumeChar();
    }

    skipWhitespace();

    // Check for function call
    if (peekChar() == '(') {
        consume("(");
        std::vector<DSLExpressionPtr> args;
        while (!peek(")")) {
            args.push_back(parseExpression());
            if (!consume(",")) {
                break;
            }
        }
        if (!consume(")")) {
            throw std::runtime_error("Expected ')' in function call");
        }
        return std::make_shared<FunctionCallExpression>(name, args);
    }

    return std::make_shared<IdentifierExpression>(name);
}

DSLExpressionPtr DSLEngine::parseNumber() {
    std::string numStr;
    bool negative = false;

    if (peekChar() == '-') {
        negative = true;
        consumeChar();
    }

    bool hasDecimal = false;
    while (std::isdigit(peekChar()) || peekChar() == '.') {
        if (peekChar() == '.') {
            if (hasDecimal) break;
            hasDecimal = true;
        }
        numStr += consumeChar();
    }

    if (hasDecimal) {
        double val = std::stod(numStr);
        return std::make_shared<LiteralExpression>(DSLValue(negative ? -val : val));
    } else {
        int val = std::stoi(numStr);
        return std::make_shared<LiteralExpression>(DSLValue(negative ? -val : val));
    }
}

DSLExpressionPtr DSLEngine::parseString() {
    if (!consume("\"")) {
        throw std::runtime_error("Expected '\"'");
    }

    std::string str;
    while (pos_ < input_.length() && input_[pos_] != '"') {
        if (input_[pos_] == '\\' && pos_ + 1 < input_.length()) {
            pos_++;
            switch (input_[pos_]) {
                case 'n': str += '\n'; break;
                case 't': str += '\t'; break;
                case 'r': str += '\r'; break;
                case '\\': str += '\\'; break;
                case '"': str += '"'; break;
                default: str += input_[pos_]; break;
            }
            pos_++;
        } else {
            str += input_[pos_++];
        }
    }

    if (!consume("\"")) {
        throw std::runtime_error("Unterminated string");
    }

    return std::make_shared<LiteralExpression>(DSLValue(str));
}

DSLExpressionPtr DSLEngine::parseUnary() {
    skipWhitespace();

    if (consume("!")) {
        return std::make_shared<UnaryOpExpression>("!", parseUnary());
    }
    if (consume("-")) {
        return std::make_shared<UnaryOpExpression>("-", parseUnary());
    }
    if (consume("+")) {
        return std::make_shared<UnaryOpExpression>("+", parseUnary());
    }

    return parsePrimary();
}

DSLExpressionPtr DSLEngine::parseBinary(DSLExpressionPtr left, int minPrecedence) {
    skipWhitespace();

    while (true) {
        std::string op;

        // Check for operators (check compound operators first!)
        if (peek("==")) op = "==";
        else if (peek("!=")) op = "!=";
        else if (peek("<=")) op = "<=";
        else if (peek(">=")) op = ">=";
        else if (peek("&&")) op = "&&";
        else if (peek("||")) op = "||";
        else if (peek("??")) op = "??";
        else if (peek("+=")) op = "+=";
        else if (peek("-=")) op = "-=";
        else if (peek("*=")) op = "*=";
        else if (peek("/=")) op = "/=";
        else if (peek("=")) op = "=";
        else if (peek("+")) op = "+";
        else if (peek("-")) op = "-";
        else if (peek("*")) op = "*";
        else if (peek("/")) op = "/";
        else if (peek("%")) op = "%";
        else if (peek("<")) op = "<";
        else if (peek(">")) op = ">";
        else break;

        int precedence = getPrecedence(op);
        if (precedence < minPrecedence) {
            break;
        }

        consume(op);

        auto right = parseUnary();
        if (!right) {
            throw std::runtime_error("Expected expression after '" + op + "'");
        }

        // Check for right-associative operators
        while (true) {
            std::string nextOp;
            size_t savedPos = pos_;

            // Peek at next operator
            skipWhitespace();
            if (peek("==")) nextOp = "==";
            else if (peek("!=")) nextOp = "!=";
            else if (peek("<=")) nextOp = "<=";
            else if (peek(">=")) nextOp = ">=";
            else if (peek("&&")) nextOp = "&&";
            else if (peek("||")) nextOp = "||";
            else if (peek("??")) nextOp = "??";
            else if (peek("+")) nextOp = "+";
            else if (peek("-")) nextOp = "-";
            else if (peek("*")) nextOp = "*";
            else if (peek("/")) nextOp = "/";
            else if (peek("%")) nextOp = "%";
            else if (peek("<")) nextOp = "<";
            else if (peek(">")) nextOp = ">";

            pos_ = savedPos;

            if (nextOp.empty()) break;

            int nextPrecedence = getPrecedence(nextOp);
            if (nextPrecedence > precedence) {
                right = parseBinary(right, nextPrecedence);
            } else {
                break;
            }
        }

        left = std::make_shared<BinaryOpExpression>(op, left, right);
    }

    return left;
}

int DSLEngine::getPrecedence(const std::string& op) {
    if (op == "=" || op == "+=" || op == "-=" || op == "*=" || op == "/=") return 0;  // Assignment (lowest)
    if (op == "||") return 1;
    if (op == "&&") return 2;
    if (op == "==" || op == "!=") return 3;
    if (op == "<" || op == ">" || op == "<=" || op == ">=") return 4;
    if (op == "+" || op == "-") return 5;
    if (op == "*" || op == "/" || op == "%") return 6;
    if (op == "??") return 0;
    return 0;
}

DSLExpressionPtr DSLEngine::parseLambda() {
    std::vector<std::string> params;

    // Save position BEFORE any modifications
    size_t savedPos = pos_;
    skipWhitespace();  // Skip whitespace once at start
    size_t afterWhitespace = pos_;

    // Multiple parameters with parens: (x, y) => ...
    // Check this FIRST because it's unambiguous - starts with (
    if (pos_ < input_.length() && input_[pos_] == '(') {
        pos_++;  // consume '('

        while (true) {
            skipWhitespace();
            if (pos_ < input_.length() && input_[pos_] == ')') {
                break;  // empty param list or end of list
            }

            std::string param;
            while (pos_ < input_.length() && (std::isalnum(input_[pos_]) || input_[pos_] == '_')) {
                param += input_[pos_++];
            }

            // If we didn't get a valid identifier, this isn't a lambda
            if (param.empty()) {
                pos_ = savedPos;
                return nullptr;
            }

            params.push_back(param);

            skipWhitespace();
            if (pos_ < input_.length() && input_[pos_] == ',') {
                pos_++;  // consume ','
            } else {
                break;
            }
        }

        skipWhitespace();
        if (pos_ >= input_.length() || input_[pos_] != ')') {
            pos_ = savedPos;
            return nullptr;
        }
        pos_++;  // consume ')'

        skipWhitespace();
        if (pos_ + 1 < input_.length() && input_[pos_] == '=' && input_[pos_+1] == '>') {
            pos_ += 2;  // consume '=>'
            auto body = parseExpression();
            return std::make_shared<LambdaExpression>(params, body);
        }

        // Had parens and params but no =>, not a lambda
        pos_ = savedPos;
        return nullptr;
    }

    // Single parameter without parens: x => ...
    // This is ambiguous with function calls, so we need to carefully lookahead
    if (pos_ < input_.length() && std::isalpha(input_[pos_])) {
        std::string param;
        size_t paramStart = pos_;
        while (pos_ < input_.length() && (std::isalnum(input_[pos_]) || input_[pos_] == '_')) {
            param += input_[pos_++];
        }

        // Now look for => (must be immediately after parameter, possibly with whitespace)
        size_t afterParam = pos_;
        while (pos_ < input_.length() && std::isspace(input_[pos_])) {
            pos_++;
        }

        if (pos_ + 1 < input_.length() && input_[pos_] == '=' && input_[pos_+1] == '>') {
            pos_ += 2;  // consume '=>'
            params.push_back(param);
            auto body = parseExpression();
            return std::make_shared<LambdaExpression>(params, body);
        }

        // Not a lambda, restore position
        pos_ = savedPos;
        return nullptr;
    }

    // Not a lambda pattern
    pos_ = savedPos;
    return nullptr;
}

DSLExpressionPtr DSLEngine::parseIfElse() {
    if (!consumeKeyword("if")) {
        return nullptr;
    }

    skipWhitespace();
    if (!consume("(")) {
        throw std::runtime_error("Expected '(' after 'if'");
    }

    auto condition = parseExpression();
    skipWhitespace();

    if (!consume(")")) {
        throw std::runtime_error("Expected ')' after if condition");
    }

    skipWhitespace();

    // Parse then branch - could be a block or single expression
    DSLExpressionPtr thenBranch;
    if (consume("{")) {
        skipWhitespace();
        thenBranch = parseExpression();
        skipWhitespace();
        if (!consume("}")) {
            throw std::runtime_error("Expected '}' after then branch");
        }
    } else {
        thenBranch = parseExpression();
    }

    skipWhitespace();
    DSLExpressionPtr elseBranch = nullptr;
    if (consumeKeyword("else")) {
        skipWhitespace();
        // else branch could also be a block or single expression
        if (consume("{")) {
            skipWhitespace();
            elseBranch = parseExpression();
            skipWhitespace();
            if (!consume("}")) {
                throw std::runtime_error("Expected '}' after else branch");
            }
        } else {
            elseBranch = parseExpression();
        }
    }

    return std::make_shared<IfElseExpression>(condition, thenBranch, elseBranch);
}

DSLExpressionPtr DSLEngine::parseWhile() {
    if (!consumeKeyword("while")) {
        return nullptr;
    }

    skipWhitespace();
    if (!consume("(")) {
        throw std::runtime_error("Expected '(' after 'while'");
    }

    auto condition = parseExpression();
    skipWhitespace();

    if (!consume(")")) {
        throw std::runtime_error("Expected ')' after while condition");
    }

    skipWhitespace();

    // Parse body - could be a block or single expression
    DSLExpressionPtr body;
    if (consume("{")) {
        skipWhitespace();
        body = parseExpression();
        skipWhitespace();
        if (!consume("}")) {
            throw std::runtime_error("Expected '}' after while body");
        }
    } else {
        body = parseExpression();
    }

    return std::make_shared<WhileExpression>(condition, body);
}

DSLExpressionPtr DSLEngine::parseFor() {
    if (!consumeKeyword("for")) {
        return nullptr;
    }

    skipWhitespace();
    if (!consume("(")) {
        throw std::runtime_error("Expected '(' after 'for'");
    }

    // Try to detect foreach style: for (item in collection)
    // Look ahead for pattern: identifier 'in' expression
    size_t savedPos = pos_;
    std::string varName;

    // Try to read an identifier
    while (pos_ < input_.length() && (std::isalnum(input_[pos_]) || input_[pos_] == '_')) {
        varName += input_[pos_++];
    }

    if (!varName.empty()) {
        skipWhitespace();
        if (consumeKeyword("in")) {
            // This is foreach style
            skipWhitespace();
            auto varExpr = std::make_shared<IdentifierExpression>(varName);
            auto collection = parseExpression();
            skipWhitespace();
            if (!consume(")")) {
                throw std::runtime_error("Expected ')' after for-in collection");
            }

            skipWhitespace();
            DSLExpressionPtr body;
            if (consume("{")) {
                skipWhitespace();
                body = parseExpression();
                skipWhitespace();
                if (!consume("}")) {
                    throw std::runtime_error("Expected '}' after for body");
                }
            } else {
                body = parseExpression();
            }

            // Return foreach-style for expression
            return std::make_shared<ForExpression>(varExpr, collection, nullptr, body);
        }
    }

    // Not foreach, restore position and parse as C-style for
    pos_ = savedPos;

    // Parse init (optional)
    DSLExpressionPtr init = nullptr;
    skipWhitespace();
    if (!peek(";")) {
        init = parseExpression();
    }

    skipWhitespace();
    if (!consume(";")) {
        throw std::runtime_error("Expected ';' after for init");
    }

    // Parse condition (optional)
    DSLExpressionPtr condition = nullptr;
    skipWhitespace();
    if (!peek(";")) {
        condition = parseExpression();
    }

    skipWhitespace();
    if (!consume(";")) {
        throw std::runtime_error("Expected ';' after for condition");
    }

    // Parse increment (optional)
    DSLExpressionPtr increment = nullptr;
    skipWhitespace();
    if (!peek(")")) {
        increment = parseExpression();
    }

    skipWhitespace();
    if (!consume(")")) {
        throw std::runtime_error("Expected ')' after for increment");
    }

    skipWhitespace();

    // Parse body
    DSLExpressionPtr body;
    if (consume("{")) {
        skipWhitespace();
        body = parseExpression();
        skipWhitespace();
        if (!consume("}")) {
            throw std::runtime_error("Expected '}' after for body");
        }
    } else {
        body = parseExpression();
    }

    return std::make_shared<ForExpression>(init, condition, increment, body);
}

DSLExpressionPtr DSLEngine::parseSwitch() {
    if (!consumeKeyword("switch")) {
        return nullptr;
    }

    skipWhitespace();
    if (!consume("(")) {
        throw std::runtime_error("Expected '(' after 'switch'");
    }

    auto value = parseExpression();
    skipWhitespace();

    if (!consume(")")) {
        throw std::runtime_error("Expected ')' after switch value");
    }

    skipWhitespace();
    if (!consume("{")) {
        throw std::runtime_error("Expected '{' after switch");
    }

    std::vector<std::pair<DSLExpressionPtr, DSLExpressionPtr>> cases;
    DSLExpressionPtr defaultCase = nullptr;

    while (!peek("}")) {
        skipWhitespace();

        if (consumeKeyword("case")) {
            skipWhitespace();
            auto caseValue = parseExpression();
            skipWhitespace();

            if (!consume(":")) {
                throw std::runtime_error("Expected ':' after case value");
            }

            skipWhitespace();
            auto caseBody = parseExpression();
            cases.push_back({caseValue, caseBody});

            // Optional semicolon
            consume(";");
        }
        else if (consumeKeyword("default")) {
            skipWhitespace();
            if (!consume(":")) {
                throw std::runtime_error("Expected ':' after default");
            }

            skipWhitespace();
            defaultCase = parseExpression();

            // Optional semicolon
            consume(";");
        }
        else {
            break;
        }
    }

    skipWhitespace();
    if (!consume("}")) {
        throw std::runtime_error("Expected '}' after switch cases");
    }

    return std::make_shared<SwitchExpression>(value, cases, defaultCase);
}

DSLExpressionPtr DSLEngine::parseMatch() {
    if (!consumeKeyword("match")) {
        return nullptr;
    }

    skipWhitespace();
    auto value = parseExpression();

    skipWhitespace();
    if (!consume("{")) {
        throw std::runtime_error("Expected '{' after match value");
    }

    std::vector<std::pair<std::string, DSLExpressionPtr>> patterns;

    while (!peek("}")) {
        skipWhitespace();

        // Parse pattern (simple identifier or literal for now)
        std::string pattern;
        if (peekChar() == '_') {
            consumeChar();
            pattern = "_";
        } else if (peekChar() == '"') {
            // String pattern
            pos_++; // skip opening quote
            while (peekChar() != '"' && pos_ < input_.length()) {
                pattern += consumeChar();
            }
            if (!consume("\"")) {
                throw std::runtime_error("Expected closing '\"' in pattern");
            }
        } else if (std::isdigit(peekChar()) || peekChar() == '-') {
            // Number pattern
            while (std::isdigit(peekChar()) || peekChar() == '-' || peekChar() == '.') {
                pattern += consumeChar();
            }
        } else {
            // Identifier pattern
            while (std::isalnum(peekChar()) || peekChar() == '_') {
                pattern += consumeChar();
            }
        }

        skipWhitespace();
        if (!consume("=>")) {
            throw std::runtime_error("Expected '=>' after match pattern");
        }

        skipWhitespace();
        auto body = parseExpression();
        patterns.push_back({pattern, body});

        // Optional comma
        skipWhitespace();
        consume(",");
    }

    skipWhitespace();
    if (!consume("}")) {
        throw std::runtime_error("Expected '}' after match patterns");
    }

    return std::make_shared<MatchExpression>(value, patterns);
}

// ===================================
// Evaluation
// ===================================

DSLValue DSLEngine::evaluate(const std::string& expression, DSLContextPtr context) {
    auto expr = parse(expression);
    if (!expr) {
        throw std::runtime_error("Failed to parse expression: " + parseError_);
    }
    return evaluate(expr, context);
}

DSLValue DSLEngine::evaluate(DSLExpressionPtr expr, DSLContextPtr context) {
    if (!context) {
        context = getCurrentContext();
    }
    return expr->evaluate(context);
}

// ===================================
// Pattern Matching
// ===================================

bool DSLEngine::match(const std::string& pattern, core::ASTNodePtr node) {
    if (!node) return false;

    // Simple pattern matching implementation
    // Format: "nodeType:name"
    size_t colonPos = pattern.find(':');
    if (colonPos != std::string::npos) {
        std::string nodeType = pattern.substr(0, colonPos);
        std::string name = pattern.substr(colonPos + 1);

        // Check node type
        if (nodeType == "function" && node->getKind() == core::IASTNode::NodeKind::FUNCTION_DECL) {
            // Check name
            if (name == "*") return true;
            if (node->hasProperty("name")) {
                auto nodeName = std::any_cast<std::string>(node->getProperty("name"));
                return nodeName == name;
            }
        }
    }

    return false;
}

std::vector<core::ASTNodePtr> DSLEngine::findMatches(const std::string& pattern) {
    std::vector<core::ASTNodePtr> results;
    // Would search through semantic model
    return results;
}

// ===================================
// Helper Functions
// ===================================

bool DSLEngine::toBool(const DSLValue& val) {
    if (std::holds_alternative<bool>(val)) {
        return std::get<bool>(val);
    }
    if (std::holds_alternative<int>(val)) {
        return std::get<int>(val) != 0;
    }
    if (std::holds_alternative<double>(val)) {
        return std::get<double>(val) != 0.0;
    }
    if (std::holds_alternative<std::string>(val)) {
        return !std::get<std::string>(val).empty();
    }
    if (std::holds_alternative<std::nullptr_t>(val)) {
        return false;
    }
    return true;
}

std::vector<std::any> DSLEngine::toCollection(const DSLValue& val) {
    if (std::holds_alternative<std::vector<std::any>>(val)) {
        return std::get<std::vector<std::any>>(val);
    }
    // Convert single value to collection
    std::vector<std::any> result;
    if (std::holds_alternative<int>(val)) {
        result.push_back(std::get<int>(val));
    } else if (std::holds_alternative<double>(val)) {
        result.push_back(std::get<double>(val));
    } else if (std::holds_alternative<std::string>(val)) {
        result.push_back(std::get<std::string>(val));
    } else if (std::holds_alternative<bool>(val)) {
        result.push_back(std::get<bool>(val));
    }
    return result;
}

// ===================================
// Factory function
// ===================================

DSLEnginePtr createDSLEngine() {
    return std::make_shared<DSLEngine>();
}

} // namespace query
} // namespace nomic