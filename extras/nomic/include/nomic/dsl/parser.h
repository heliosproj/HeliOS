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

#ifndef NOMIC_DSL_PARSER_H
#define NOMIC_DSL_PARSER_H

#include "nomic/dsl/tokenizer.h"
#include "nomic/dsl/ast_nodes.h"
#include <memory>
#include <vector>

namespace nomic::dsl {

/**
 * Recursive descent parser for Nomic DSL
 *
 * Grammar (simplified):
 * expression := quantifier | logical_or
 * quantifier := (ALL|ANY|EXISTS|NONE) '(' IDENT 'in' expression ['where' expression] 'satisfies' expression ')'
 * logical_or := logical_and ('or' logical_and)*
 * logical_and := comparison ('and' comparison)*
 * comparison := additive (('=='|'!='|'<'|'<='|'>'|'>=') additive)?
 * additive := multiplicative (('+'|'-') multiplicative)*
 * multiplicative := unary (('*'|'/') unary)*
 * unary := ('not' | '-') unary | postfix
 * postfix := primary ('.' IDENT | '(' arglist ')')*
 * primary := IDENT | NUMBER | STRING | TRUE | FALSE | list | '(' expression ')'
 * list := '[' [expression (',' expression)*] ']'
 * arglist := [expression (',' expression)*]
 */
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    std::unique_ptr<ASTNode> parse();

private:
    const std::vector<Token>& tokens_;
    size_t current_;

    // Parsing methods (from lowest to highest precedence)
    std::unique_ptr<ASTNode> expression();
    std::unique_ptr<ASTNode> quantifier();
    std::unique_ptr<ASTNode> logicalOr();
    std::unique_ptr<ASTNode> logicalAnd();
    std::unique_ptr<ASTNode> comparison();
    std::unique_ptr<ASTNode> additive();
    std::unique_ptr<ASTNode> multiplicative();
    std::unique_ptr<ASTNode> unary();
    std::unique_ptr<ASTNode> postfix();
    std::unique_ptr<ASTNode> primary();
    std::unique_ptr<ASTNode> list();

    // Helper methods
    const Token& peek() const;
    const Token& previous() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(const std::vector<TokenType>& types);
    void consume(TokenType type, const std::string& message);
    bool isAtEnd() const;
};

} // namespace nomic::dsl

#endif // NOMIC_DSL_PARSER_H
