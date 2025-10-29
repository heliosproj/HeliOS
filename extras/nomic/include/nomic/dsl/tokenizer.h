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

#ifndef NOMIC_DSL_TOKENIZER_H
#define NOMIC_DSL_TOKENIZER_H

#include <string>
#include <vector>

namespace nomic::dsl {

enum class TokenType {
    // Literals
    IDENTIFIER,      // foo, bar, fn
    NUMBER,          // 10, 3.14
    STRING,          // "hello"

    // Keywords
    ALL,             // all
    ANY,             // any
    EXISTS,          // exists
    NONE,            // none
    IN,              // in
    WHERE,           // where
    SATISFIES,       // satisfies
    AND,             // and
    OR,              // or
    NOT,             // not
    TRUE,            // true
    FALSE,           // false

    // Operators
    DOT,             // .
    COMMA,           // ,
    LPAREN,          // (
    RPAREN,          // )
    LBRACKET,        // [
    RBRACKET,        // ]
    LBRACE,          // {
    RBRACE,          // }

    EQ,              // ==
    NE,              // !=
    LT,              // <
    LE,              // <=
    GT,              // >
    GE,              // >=

    PLUS,            // +
    MINUS,           // -
    MULTIPLY,        // *
    DIVIDE,          // /

    // Special
    END_OF_FILE,
    INVALID
};

struct Token {
    TokenType type;
    std::string value;
    size_t position;

    Token(TokenType t, const std::string& v, size_t pos)
        : type(t), value(v), position(pos) {}
};

class Tokenizer {
public:
    explicit Tokenizer(const std::string& input);

    std::vector<Token> tokenize();

private:
    std::string input_;
    size_t pos_;

    char peek() const;
    char advance();
    void skipWhitespace();

    Token readNumber();
    Token readString();
    Token readIdentifierOrKeyword();
    Token readOperator();

    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;
    bool isWhitespace(char c) const;

    TokenType getKeywordType(const std::string& word) const;
};

} // namespace nomic::dsl

#endif // NOMIC_DSL_TOKENIZER_H
