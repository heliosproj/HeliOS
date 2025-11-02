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

#ifndef NOMIC_DSL_AST_NODES_H
#define NOMIC_DSL_AST_NODES_H

#include <string>
#include <memory>
#include <vector>

namespace nomic::dsl {

// AST Node base class
class ASTNode {
public:
    enum Type {
        IDENTIFIER,
        LITERAL,
        BINARY_OP,
        UNARY_OP,
        FUNCTION_CALL,
        QUANTIFIER,
        LIST_COMPREHENSION,
        MEMBER_ACCESS,
        LIST_LITERAL
    };

    virtual ~ASTNode() = default;
    virtual Type getType() const = 0;
};

// Identifier node
class IdentifierNode : public ASTNode {
public:
    explicit IdentifierNode(const std::string& name) : name_(name) {}
    Type getType() const override { return IDENTIFIER; }
    const std::string& getName() const { return name_; }
private:
    std::string name_;
};

// Literal node
class LiteralNode : public ASTNode {
public:
    explicit LiteralNode(const std::string& value) : value_(value) {}
    Type getType() const override { return LITERAL; }
    const std::string& getValue() const { return value_; }
private:
    std::string value_;
};

// Binary operator node
class BinaryOpNode : public ASTNode {
public:
    BinaryOpNode(const std::string& op,
                 std::unique_ptr<ASTNode> left,
                 std::unique_ptr<ASTNode> right)
        : op_(op), left_(std::move(left)), right_(std::move(right)) {}
    Type getType() const override { return BINARY_OP; }
    const std::string& getOp() const { return op_; }
    const ASTNode* getLeft() const { return left_.get(); }
    const ASTNode* getRight() const { return right_.get(); }
private:
    std::string op_;
    std::unique_ptr<ASTNode> left_;
    std::unique_ptr<ASTNode> right_;
};

// Unary operator node
class UnaryOpNode : public ASTNode {
public:
    UnaryOpNode(const std::string& op, std::unique_ptr<ASTNode> operand)
        : op_(op), operand_(std::move(operand)) {}
    Type getType() const override { return UNARY_OP; }
    const std::string& getOp() const { return op_; }
    const ASTNode* getOperand() const { return operand_.get(); }
private:
    std::string op_;
    std::unique_ptr<ASTNode> operand_;
};

// Function call node
class FunctionCallNode : public ASTNode {
public:
    FunctionCallNode(const std::string& name, std::vector<std::unique_ptr<ASTNode>> args)
        : name_(name), args_(std::move(args)) {}
    Type getType() const override { return FUNCTION_CALL; }
    const std::string& getName() const { return name_; }
    const std::vector<std::unique_ptr<ASTNode>>& getArgs() const { return args_; }
private:
    std::string name_;
    std::vector<std::unique_ptr<ASTNode>> args_;
};

// Quantifier node (all, any, exists, none)
class QuantifierNode : public ASTNode {
public:
    enum QuantifierType { ALL, ANY, EXISTS, NONE };
    QuantifierNode(QuantifierType qtype, const std::string& var,
                   std::unique_ptr<ASTNode> collection,
                   std::unique_ptr<ASTNode> where_clause,
                   std::unique_ptr<ASTNode> satisfies_clause)
        : qtype_(qtype), var_(var), collection_(std::move(collection)),
          where_clause_(std::move(where_clause)), satisfies_clause_(std::move(satisfies_clause)) {}
    Type getType() const override { return QUANTIFIER; }
    QuantifierType getQuantifierType() const { return qtype_; }
    const std::string& getVar() const { return var_; }
    const ASTNode* getCollection() const { return collection_.get(); }
    const ASTNode* getWhereClause() const { return where_clause_.get(); }
    const ASTNode* getSatisfiesClause() const { return satisfies_clause_.get(); }
private:
    QuantifierType qtype_;
    std::string var_;
    std::unique_ptr<ASTNode> collection_;
    std::unique_ptr<ASTNode> where_clause_;
    std::unique_ptr<ASTNode> satisfies_clause_;
};

// Member access node (e.g., fn.name, param.type)
class MemberAccessNode : public ASTNode {
public:
    MemberAccessNode(std::unique_ptr<ASTNode> object, const std::string& member)
        : object_(std::move(object)), member_(member) {}
    Type getType() const override { return MEMBER_ACCESS; }
    const ASTNode* getObject() const { return object_.get(); }
    const std::string& getMember() const { return member_; }
private:
    std::unique_ptr<ASTNode> object_;
    std::string member_;
};

// List literal node (e.g., ["malloc", "calloc"])
class ListLiteralNode : public ASTNode {
public:
    explicit ListLiteralNode(std::vector<std::unique_ptr<ASTNode>> elements)
        : elements_(std::move(elements)) {}
    Type getType() const override { return LIST_LITERAL; }
    const std::vector<std::unique_ptr<ASTNode>>& getElements() const { return elements_; }
private:
    std::vector<std::unique_ptr<ASTNode>> elements_;
};

} // namespace nomic::dsl

#endif // NOMIC_DSL_AST_NODES_H