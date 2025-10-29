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

#include "nomic/dsl/parser.h"
#include <stdexcept>
#include <spdlog/spdlog.h>

namespace nomic::dsl {

Parser::Parser(const std::vector<Token>& tokens)
    : tokens_(tokens), current_(0) {}

std::unique_ptr<ASTNode> Parser::parse() {
    try {
        return expression();
    } catch (const std::exception& e) {
        spdlog::error("Parse error: {}", e.what());
        return nullptr;
    }
}

std::unique_ptr<ASTNode> Parser::expression() {
    // Check for quantifiers first
    if (match({TokenType::ALL, TokenType::ANY, TokenType::EXISTS, TokenType::NONE})) {
        return quantifier();
    }
    return logicalOr();
}

std::unique_ptr<ASTNode> Parser::quantifier() {
    // We already consumed the quantifier keyword in expression()
    Token quantifier_token = previous();

    QuantifierNode::QuantifierType qtype;
    if (quantifier_token.type == TokenType::ALL) {
        qtype = QuantifierNode::ALL;
    } else if (quantifier_token.type == TokenType::ANY) {
        qtype = QuantifierNode::ANY;
    } else if (quantifier_token.type == TokenType::EXISTS) {
        qtype = QuantifierNode::EXISTS;
    } else {
        qtype = QuantifierNode::NONE;
    }

    consume(TokenType::LPAREN, "Expected '(' after quantifier");

    // Variable name
    consume(TokenType::IDENTIFIER, "Expected variable name in quantifier");
    std::string var_name = previous().value;

    consume(TokenType::IN, "Expected 'in' in quantifier");

    // Collection expression
    auto collection = logicalOr();

    // Optional where clause
    std::unique_ptr<ASTNode> where_clause = nullptr;
    if (match(TokenType::WHERE)) {
        where_clause = logicalOr();
    }

    // Satisfies clause
    consume(TokenType::SATISFIES, "Expected 'satisfies' in quantifier");
    auto satisfies_clause = logicalOr();

    consume(TokenType::RPAREN, "Expected ')' after quantifier");

    return std::make_unique<QuantifierNode>(
        qtype, var_name, std::move(collection),
        std::move(where_clause), std::move(satisfies_clause)
    );
}

std::unique_ptr<ASTNode> Parser::logicalOr() {
    auto left = logicalAnd();

    while (match(TokenType::OR)) {
        auto right = logicalAnd();
        left = std::make_unique<BinaryOpNode>("or", std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::logicalAnd() {
    auto left = comparison();

    while (match(TokenType::AND)) {
        auto right = comparison();
        left = std::make_unique<BinaryOpNode>("and", std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::comparison() {
    auto left = additive();

    if (match({TokenType::EQ, TokenType::NE, TokenType::LT, TokenType::LE,
               TokenType::GT, TokenType::GE, TokenType::IN})) {
        Token op = previous();
        auto right = additive();
        left = std::make_unique<BinaryOpNode>(op.value, std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::additive() {
    auto left = multiplicative();

    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        auto right = multiplicative();
        left = std::make_unique<BinaryOpNode>(op.value, std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::multiplicative() {
    auto left = unary();

    while (match({TokenType::MULTIPLY, TokenType::DIVIDE})) {
        Token op = previous();
        auto right = unary();
        left = std::make_unique<BinaryOpNode>(op.value, std::move(left), std::move(right));
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::unary() {
    if (match({TokenType::NOT, TokenType::MINUS})) {
        Token op = previous();
        auto operand = unary();
        return std::make_unique<UnaryOpNode>(op.value, std::move(operand));
    }

    return postfix();
}

std::unique_ptr<ASTNode> Parser::postfix() {
    auto expr = primary();

    while (true) {
        if (match(TokenType::DOT)) {
            // Member access
            consume(TokenType::IDENTIFIER, "Expected member name after '.'");
            std::string member = previous().value;
            expr = std::make_unique<MemberAccessNode>(std::move(expr), member);
        } else if (match(TokenType::LPAREN)) {
            // Function call
            std::vector<std::unique_ptr<ASTNode>> args;

            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(expression());
                } while (match(TokenType::COMMA));
            }

            consume(TokenType::RPAREN, "Expected ')' after arguments");

            // Convert expr to function name
            if (expr->getType() == ASTNode::IDENTIFIER) {
                auto* ident = static_cast<IdentifierNode*>(expr.get());
                std::string func_name = ident->getName();
                expr = std::make_unique<FunctionCallNode>(func_name, std::move(args));
            } else {
                throw std::runtime_error("Cannot call non-identifier");
            }
        } else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<ASTNode> Parser::primary() {
    // Literals
    if (match(TokenType::TRUE)) {
        return std::make_unique<LiteralNode>("true");
    }
    if (match(TokenType::FALSE)) {
        return std::make_unique<LiteralNode>("false");
    }
    if (match(TokenType::NUMBER)) {
        return std::make_unique<LiteralNode>(previous().value);
    }
    if (match(TokenType::STRING)) {
        return std::make_unique<LiteralNode>(previous().value);
    }
    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<IdentifierNode>(previous().value);
    }

    // List literal
    if (match(TokenType::LBRACKET)) {
        return list();
    }

    // Parenthesized expression
    if (match(TokenType::LPAREN)) {
        auto expr = expression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }

    throw std::runtime_error("Expected expression at position " +
                           std::to_string(peek().position));
}

std::unique_ptr<ASTNode> Parser::list() {
    // We already consumed the '['
    std::vector<std::unique_ptr<ASTNode>> elements;

    if (!check(TokenType::RBRACKET)) {
        do {
            elements.push_back(expression());
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RBRACKET, "Expected ']' after list elements");

    return std::make_unique<ListLiteralNode>(std::move(elements));
}

// Helper methods

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        current_++;
    }
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

void Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        advance();
        return;
    }

    throw std::runtime_error(message + " at position " +
                           std::to_string(peek().position) +
                           ", got '" + peek().value + "'");
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

} // namespace nomic::dsl
