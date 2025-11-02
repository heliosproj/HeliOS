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

#include "nomic/dsl/tokenizer.h"
#include <cctype>
#include <stdexcept>

namespace nomic::dsl {

Tokenizer::Tokenizer(const std::string& input)
    : input_(input), pos_(0) {}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;

    while (pos_ < input_.length()) {
        skipWhitespace();

        if (pos_ >= input_.length()) {
            break;
        }

        char c = peek();

        // Numbers
        if (isDigit(c)) {
            tokens.push_back(readNumber());
        }
        // Strings
        else if (c == '"' || c == '\'') {
            tokens.push_back(readString());
        }
        // Identifiers and keywords
        else if (isAlpha(c) || c == '_') {
            tokens.push_back(readIdentifierOrKeyword());
        }
        // Operators and punctuation
        else {
            tokens.push_back(readOperator());
        }
    }

    tokens.push_back(Token(TokenType::END_OF_FILE, "", pos_));
    return tokens;
}

char Tokenizer::peek() const {
    if (pos_ < input_.length()) {
        return input_[pos_];
    }
    return '\0';
}

char Tokenizer::advance() {
    if (pos_ < input_.length()) {
        return input_[pos_++];
    }
    return '\0';
}

void Tokenizer::skipWhitespace() {
    while (pos_ < input_.length() && isWhitespace(peek())) {
        advance();
    }
}

Token Tokenizer::readNumber() {
    size_t start = pos_;
    std::string value;

    while (isDigit(peek())) {
        value += advance();
    }

    // Handle decimal point
    if (peek() == '.') {
        value += advance();
        while (isDigit(peek())) {
            value += advance();
        }
    }

    return Token(TokenType::NUMBER, value, start);
}

Token Tokenizer::readString() {
    size_t start = pos_;
    char quote = advance(); // consume opening quote
    std::string value;

    while (pos_ < input_.length() && peek() != quote) {
        if (peek() == '\\') {
            advance(); // consume backslash
            if (pos_ < input_.length()) {
                char escaped = advance();
                switch (escaped) {
                    case 'n': value += '\n'; break;
                    case 't': value += '\t'; break;
                    case 'r': value += '\r'; break;
                    case '\\': value += '\\'; break;
                    case '"': value += '"'; break;
                    case '\'': value += '\''; break;
                    default: value += escaped; break;
                }
            }
        } else {
            value += advance();
        }
    }

    if (peek() == quote) {
        advance(); // consume closing quote
    }

    return Token(TokenType::STRING, value, start);
}

Token Tokenizer::readIdentifierOrKeyword() {
    size_t start = pos_;
    std::string value;

    while (isAlphaNumeric(peek()) || peek() == '_') {
        value += advance();
    }

    TokenType type = getKeywordType(value);
    return Token(type, value, start);
}

Token Tokenizer::readOperator() {
    size_t start = pos_;
    char c = advance();

    switch (c) {
        case '.': return Token(TokenType::DOT, ".", start);
        case ',': return Token(TokenType::COMMA, ",", start);
        case '(': return Token(TokenType::LPAREN, "(", start);
        case ')': return Token(TokenType::RPAREN, ")", start);
        case '[': return Token(TokenType::LBRACKET, "[", start);
        case ']': return Token(TokenType::RBRACKET, "]", start);
        case '{': return Token(TokenType::LBRACE, "{", start);
        case '}': return Token(TokenType::RBRACE, "}", start);
        case '+': return Token(TokenType::PLUS, "+", start);
        case '-': return Token(TokenType::MINUS, "-", start);
        case '*': return Token(TokenType::MULTIPLY, "*", start);
        case '/': return Token(TokenType::DIVIDE, "/", start);

        case '=':
            if (peek() == '=') {
                advance();
                return Token(TokenType::EQ, "==", start);
            }
            return Token(TokenType::INVALID, "=", start);

        case '!':
            if (peek() == '=') {
                advance();
                return Token(TokenType::NE, "!=", start);
            }
            return Token(TokenType::INVALID, "!", start);

        case '<':
            if (peek() == '=') {
                advance();
                return Token(TokenType::LE, "<=", start);
            }
            return Token(TokenType::LT, "<", start);

        case '>':
            if (peek() == '=') {
                advance();
                return Token(TokenType::GE, ">=", start);
            }
            return Token(TokenType::GT, ">", start);

        default:
            return Token(TokenType::INVALID, std::string(1, c), start);
    }
}

bool Tokenizer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Tokenizer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Tokenizer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

bool Tokenizer::isWhitespace(char c) const {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

TokenType Tokenizer::getKeywordType(const std::string& word) const {
    if (word == "all" || word == "forall") return TokenType::ALL;
    if (word == "any") return TokenType::ANY;
    if (word == "exists") return TokenType::EXISTS;
    if (word == "none") return TokenType::NONE;
    if (word == "in") return TokenType::IN;
    if (word == "where") return TokenType::WHERE;
    if (word == "satisfies") return TokenType::SATISFIES;
    if (word == "and") return TokenType::AND;
    if (word == "or") return TokenType::OR;
    if (word == "not") return TokenType::NOT;
    if (word == "true") return TokenType::TRUE;
    if (word == "false") return TokenType::FALSE;

    return TokenType::IDENTIFIER;
}

} // namespace nomic::dsl
