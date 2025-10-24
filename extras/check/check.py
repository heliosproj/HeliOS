#!/usr/bin/env python3
"""
@file check.py
@author Manny Peterson <manny@heliosproj.org>
@brief HeliOS Source Code Compliance Checker
@version 0.6.0
@date 2025-10-24

@copyright
HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>

SPDX-License-Identifier: GPL-2.0-or-later

@description
HeliOS Source Code Compliance Checker
"""

import os
import re
import sys
import json
import yaml
import time
import pickle
import hashlib
import argparse
import multiprocessing
from pathlib import Path
from datetime import datetime
from collections import defaultdict, deque
from concurrent.futures import ProcessPoolExecutor, as_completed
from typing import List, Dict, Tuple, Optional, Set, Any, Union
from dataclasses import dataclass, field, asdict
from enum import Enum, auto
import logging

# Optional dependencies
try:
    from tqdm import tqdm
    TQDM_AVAILABLE = True
except ImportError:
    TQDM_AVAILABLE = False

# ============================================================================
# Token Types for C Language Parser
# ============================================================================

class TokenType(Enum):
    """Token types for C language lexer"""
    # Literals
    IDENTIFIER = auto()
    NUMBER = auto()
    STRING = auto()
    CHAR = auto()

    # Keywords
    KEYWORD = auto()
    TYPE_KEYWORD = auto()
    STORAGE_CLASS = auto()
    TYPE_QUALIFIER = auto()

    # Operators
    OPERATOR = auto()
    ASSIGNMENT = auto()
    COMPARISON = auto()
    LOGICAL = auto()
    ARITHMETIC = auto()
    BITWISE = auto()

    # Delimiters
    LPAREN = auto()
    RPAREN = auto()
    LBRACE = auto()
    RBRACE = auto()
    LBRACKET = auto()
    RBRACKET = auto()
    SEMICOLON = auto()
    COMMA = auto()
    DOT = auto()
    ARROW = auto()

    # Preprocessor
    PREPROCESSOR = auto()
    MACRO = auto()
    INCLUDE = auto()
    DEFINE = auto()
    IFDEF = auto()
    IFNDEF = auto()
    ENDIF = auto()

    # Comments
    COMMENT = auto()
    MULTILINE_COMMENT = auto()

    # Special
    NEWLINE = auto()
    WHITESPACE = auto()
    EOF = auto()
    UNKNOWN = auto()

@dataclass
class Token:
    """Represents a lexical token"""
    type: TokenType
    value: str
    line: int
    column: int
    file: str = ""
    macro_expansion: Optional[str] = None  # Track if token came from macro

# ============================================================================
# C Language Lexer
# ============================================================================

class CLexer:
    """Advanced C language lexer with macro awareness"""

    # C Keywords
    C_KEYWORDS = {
        'auto', 'break', 'case', 'char', 'const', 'continue', 'default',
        'do', 'double', 'else', 'enum', 'extern', 'float', 'for', 'goto',
        'if', 'inline', 'int', 'long', 'register', 'restrict', 'return',
        'short', 'signed', 'sizeof', 'static', 'struct', 'switch', 'typedef',
        'union', 'unsigned', 'void', 'volatile', 'while', '_Bool', '_Complex'
    }

    # Type keywords
    TYPE_KEYWORDS = {
        'void', 'char', 'short', 'int', 'long', 'float', 'double',
        'signed', 'unsigned', '_Bool', '_Complex'
    }

    # Storage class specifiers
    STORAGE_CLASSES = {'auto', 'register', 'static', 'extern', 'typedef'}

    # Type qualifiers
    TYPE_QUALIFIERS = {'const', 'restrict', 'volatile', '_Atomic'}

    # HeliOS-specific types
    HELIOS_TYPES = {
        'Return_t', 'Base_t', 'Addr_t', 'Task_t', 'Queue_t', 'Timer_t',
        'Device_t', 'Stream_t', 'TaskState_t', 'SchedulerState_t',
        'QueueMessage_t', 'Message_t', 'TaskList_t', 'MemoryRegion_t'
    }

    def __init__(self, macro_definitions: Optional[Dict[str, str]] = None):
        self.macro_definitions = macro_definitions or {}
        self.tokens = []
        self.current = 0
        self.line = 1
        self.column = 1
        self.source = ""
        self.file = ""

    def tokenize(self, source: str, file: str = "") -> List[Token]:
        """Tokenize C source code"""
        self.source = source
        self.file = file
        self.tokens = []
        self.current = 0
        self.line = 1
        self.column = 1

        while not self.is_at_end():
            self.scan_token()

        self.tokens.append(Token(TokenType.EOF, "", self.line, self.column, self.file))
        return self.tokens

    def scan_token(self):
        """Scan a single token"""
        c = self.advance()

        # Skip whitespace
        if c in ' \t\r':
            self.column += 1
            return

        # Newline
        if c == '\n':
            self.line += 1
            self.column = 1
            return

        # Comments
        if c == '/':
            if self.peek() == '/':
                self.scan_single_line_comment()
                return
            elif self.peek() == '*':
                self.scan_multi_line_comment()
                return
            else:
                self.add_token(TokenType.OPERATOR, c)
                return

        # Preprocessor directives
        if c == '#':
            self.scan_preprocessor()
            return

        # String literals
        if c == '"':
            self.scan_string()
            return

        # Character literals
        if c == "'":
            self.scan_char()
            return

        # Numbers
        if c.isdigit():
            self.scan_number()
            return

        # Identifiers and keywords
        if c.isalpha() or c == '_':
            self.scan_identifier()
            return

        # Multi-character operators
        if c == '=' and self.peek() == '=':
            self.advance()
            self.add_token(TokenType.COMPARISON, "==")
            return
        elif c == '!' and self.peek() == '=':
            self.advance()
            self.add_token(TokenType.COMPARISON, "!=")
            return
        elif c == '<' and self.peek() == '=':
            self.advance()
            self.add_token(TokenType.COMPARISON, "<=")
            return
        elif c == '>' and self.peek() == '=':
            self.advance()
            self.add_token(TokenType.COMPARISON, ">=")
            return
        elif c == '&' and self.peek() == '&':
            self.advance()
            self.add_token(TokenType.LOGICAL, "&&")
            return
        elif c == '|' and self.peek() == '|':
            self.advance()
            self.add_token(TokenType.LOGICAL, "||")
            return
        elif c == '-' and self.peek() == '>':
            self.advance()
            self.add_token(TokenType.ARROW, "->")
            return
        elif c == '+' and self.peek() == '+':
            self.advance()
            self.add_token(TokenType.OPERATOR, "++")
            return
        elif c == '-' and self.peek() == '-':
            self.advance()
            self.add_token(TokenType.OPERATOR, "--")
            return
        elif c == '<' and self.peek() == '<':
            self.advance()
            self.add_token(TokenType.BITWISE, "<<")
            return
        elif c == '>' and self.peek() == '>':
            self.advance()
            self.add_token(TokenType.BITWISE, ">>")
            return

        # Single character tokens
        single_char_tokens = {
            '(': TokenType.LPAREN,
            ')': TokenType.RPAREN,
            '{': TokenType.LBRACE,
            '}': TokenType.RBRACE,
            '[': TokenType.LBRACKET,
            ']': TokenType.RBRACKET,
            ';': TokenType.SEMICOLON,
            ',': TokenType.COMMA,
            '.': TokenType.DOT,
            '=': TokenType.ASSIGNMENT,
            '<': TokenType.COMPARISON,
            '>': TokenType.COMPARISON,
            '+': TokenType.ARITHMETIC,
            '-': TokenType.ARITHMETIC,
            '*': TokenType.ARITHMETIC,
            '/': TokenType.ARITHMETIC,
            '%': TokenType.ARITHMETIC,
            '&': TokenType.BITWISE,
            '|': TokenType.BITWISE,
            '^': TokenType.BITWISE,
            '~': TokenType.BITWISE,
            '!': TokenType.LOGICAL,
            '?': TokenType.OPERATOR,
            ':': TokenType.OPERATOR,
        }

        if c in single_char_tokens:
            self.add_token(single_char_tokens[c], c)
        else:
            self.add_token(TokenType.UNKNOWN, c)

    def scan_single_line_comment(self):
        """Scan single-line comment"""
        start_col = self.column
        comment = "//"
        self.advance()  # Skip second /

        while not self.is_at_end() and self.peek() != '\n':
            comment += self.advance()

        self.add_token(TokenType.COMMENT, comment)

    def scan_multi_line_comment(self):
        """Scan multi-line comment"""
        comment = "/*"
        self.advance()  # Skip *

        while not self.is_at_end():
            if self.peek() == '*' and self.peek_next() == '/':
                comment += self.advance()  # *
                comment += self.advance()  # /
                break
            else:
                c = self.advance()
                comment += c
                if c == '\n':
                    self.line += 1
                    self.column = 1

        self.add_token(TokenType.MULTILINE_COMMENT, comment)

    def scan_preprocessor(self):
        """Scan preprocessor directive"""
        directive = "#"

        # Skip whitespace after #
        while self.peek() in ' \t':
            directive += self.advance()

        # Read directive name
        while not self.is_at_end() and (self.peek().isalnum() or self.peek() == '_'):
            directive += self.advance()

        # Read rest of line (handling line continuations)
        while not self.is_at_end():
            if self.peek() == '\\' and self.peek_next() == '\n':
                directive += self.advance()  # \
                directive += self.advance()  # \n
                self.line += 1
                self.column = 1
            elif self.peek() == '\n':
                break
            else:
                directive += self.advance()

        # Determine specific preprocessor type
        if directive.strip().startswith("#define"):
            self.add_token(TokenType.DEFINE, directive)
            self.parse_macro_definition(directive)
        elif directive.strip().startswith("#include"):
            self.add_token(TokenType.INCLUDE, directive)
        elif directive.strip().startswith("#ifdef"):
            self.add_token(TokenType.IFDEF, directive)
        elif directive.strip().startswith("#ifndef"):
            self.add_token(TokenType.IFNDEF, directive)
        elif directive.strip().startswith("#endif"):
            self.add_token(TokenType.ENDIF, directive)
        else:
            self.add_token(TokenType.PREPROCESSOR, directive)

    def parse_macro_definition(self, directive: str):
        """Parse and store macro definition"""
        # Simple macro extraction (can be enhanced)
        match = re.match(r'#define\s+(\w+)(?:\((.*?)\))?\s+(.*)', directive.strip())
        if match:
            name = match.group(1)
            params = match.group(2)
            body = match.group(3)

            if params:
                self.macro_definitions[name] = {'params': params.split(','), 'body': body}
            else:
                self.macro_definitions[name] = {'params': None, 'body': body}

    def scan_string(self):
        """Scan string literal"""
        string = '"'

        while not self.is_at_end() and self.peek() != '"':
            if self.peek() == '\\':
                string += self.advance()  # \
                if not self.is_at_end():
                    string += self.advance()  # escaped char
            else:
                c = self.advance()
                string += c
                if c == '\n':
                    self.line += 1
                    self.column = 1

        if not self.is_at_end():
            string += self.advance()  # closing "

        self.add_token(TokenType.STRING, string)

    def scan_char(self):
        """Scan character literal"""
        char = "'"

        while not self.is_at_end() and self.peek() != "'":
            if self.peek() == '\\':
                char += self.advance()  # \
                if not self.is_at_end():
                    char += self.advance()  # escaped char
            else:
                char += self.advance()

        if not self.is_at_end():
            char += self.advance()  # closing '

        self.add_token(TokenType.CHAR, char)

    def scan_number(self):
        """Scan numeric literal"""
        start = self.current - 1

        # Hexadecimal
        if self.source[start] == '0' and self.peek() in 'xX':
            self.advance()  # x or X
            while not self.is_at_end() and (self.peek().isdigit() or self.peek() in 'abcdefABCDEF'):
                self.advance()
        # Octal or decimal
        else:
            while not self.is_at_end() and self.peek().isdigit():
                self.advance()

            # Float
            if self.peek() == '.' and self.peek_next().isdigit():
                self.advance()  # .
                while not self.is_at_end() and self.peek().isdigit():
                    self.advance()

            # Scientific notation
            if self.peek() in 'eE':
                self.advance()
                if self.peek() in '+-':
                    self.advance()
                while not self.is_at_end() and self.peek().isdigit():
                    self.advance()

        # Suffix (L, U, F, etc.)
        while not self.is_at_end() and self.peek() in 'lLuUfF':
            self.advance()

        self.add_token(TokenType.NUMBER, self.source[start:self.current])

    def scan_identifier(self):
        """Scan identifier or keyword"""
        start = self.current - 1

        while not self.is_at_end() and (self.peek().isalnum() or self.peek() == '_'):
            self.advance()

        text = self.source[start:self.current]

        # Check if it's a keyword
        if text in self.C_KEYWORDS:
            if text in self.TYPE_KEYWORDS:
                self.add_token(TokenType.TYPE_KEYWORD, text)
            elif text in self.STORAGE_CLASSES:
                self.add_token(TokenType.STORAGE_CLASS, text)
            elif text in self.TYPE_QUALIFIERS:
                self.add_token(TokenType.TYPE_QUALIFIER, text)
            else:
                self.add_token(TokenType.KEYWORD, text)
        # Check if it's a HeliOS type
        elif text in self.HELIOS_TYPES:
            self.add_token(TokenType.TYPE_KEYWORD, text)
        # Check if it's a known macro
        elif text in self.macro_definitions:
            self.add_token(TokenType.MACRO, text)
        else:
            self.add_token(TokenType.IDENTIFIER, text)

    def advance(self) -> str:
        """Consume and return next character"""
        if self.is_at_end():
            return '\0'
        c = self.source[self.current]
        self.current += 1
        self.column += 1
        return c

    def peek(self) -> str:
        """Look at next character without consuming"""
        if self.is_at_end():
            return '\0'
        return self.source[self.current]

    def peek_next(self) -> str:
        """Look at character after next"""
        if self.current + 1 >= len(self.source):
            return '\0'
        return self.source[self.current + 1]

    def is_at_end(self) -> bool:
        """Check if at end of source"""
        return self.current >= len(self.source)

    def add_token(self, type: TokenType, value: str):
        """Add token to list"""
        self.tokens.append(Token(type, value, self.line, self.column - len(value), self.file))

# ============================================================================
# Abstract Syntax Tree (AST) Nodes
# ============================================================================

class ASTNode:
    """Base class for AST nodes"""
    def __init__(self, line: int, column: int, file: str = ""):
        self.line = line
        self.column = column
        self.file = file

@dataclass
class FunctionNode:
    """AST node for function definition"""
    line: int
    column: int
    name: str
    return_type: str
    parameters: List[Dict[str, str]]
    body: 'BlockNode'
    file: str = ""
    storage_class: Optional[str] = None
    attributes: Dict[str, Any] = field(default_factory=dict)
    complexity: int = 0
    documentation: Optional[str] = None

@dataclass
class BlockNode:
    """AST node for code block"""
    line: int
    column: int
    statements: List[ASTNode]
    file: str = ""

@dataclass
class IfNode:
    """AST node for if statement"""
    line: int
    column: int
    condition: ASTNode
    then_branch: ASTNode
    file: str = ""
    else_branch: Optional[ASTNode] = None

@dataclass
class WhileNode:
    """AST node for while loop"""
    line: int
    column: int
    condition: ASTNode
    body: ASTNode
    file: str = ""

@dataclass
class ForNode:
    """AST node for for loop"""
    line: int
    column: int
    body: ASTNode
    file: str = ""
    init: Optional[ASTNode] = None
    condition: Optional[ASTNode] = None
    update: Optional[ASTNode] = None

@dataclass
class ReturnNode:
    """AST node for return statement"""
    line: int
    column: int
    file: str = ""
    value: Optional[ASTNode] = None
    macro_return: bool = False  # True if using __ReturnOk__ etc.

@dataclass
class CallNode:
    """AST node for function/macro call"""
    line: int
    column: int
    name: str
    file: str = ""
    arguments: List[ASTNode] = field(default_factory=list)
    is_macro: bool = False

@dataclass
class VariableNode:
    """AST node for variable declaration"""
    line: int
    column: int
    type: str
    name: str
    file: str = ""
    initializer: Optional[ASTNode] = None
    is_pointer: bool = False
    is_static: bool = False
    is_volatile: bool = False

# ============================================================================
# C Parser with AST Building
# ============================================================================

class CParser:
    """C parser that builds an AST"""

    def __init__(self, tokens: List[Token], macro_definitions: Optional[Dict] = None):
        self.tokens = tokens
        self.current = 0
        self.macro_definitions = macro_definitions or {}
        self.symbol_table = {}
        self.current_scope = None
        self.ast = None

    def parse(self) -> List[ASTNode]:
        """Parse tokens into AST"""
        nodes = []

        while not self.is_at_end():
            # Skip comments and preprocessor for now
            if self.check(TokenType.COMMENT, TokenType.MULTILINE_COMMENT,
                         TokenType.PREPROCESSOR, TokenType.DEFINE,
                         TokenType.INCLUDE, TokenType.IFDEF, TokenType.IFNDEF,
                         TokenType.ENDIF):
                self.advance()
                continue

            try:
                node = self.parse_declaration()
                if node:
                    nodes.append(node)
            except Exception as e:
                logging.debug(f"Parse error: {e}")
                self.synchronize()

        return nodes

    def parse_declaration(self) -> Optional[ASTNode]:
        """Parse top-level declaration"""
        # Check for function definition
        if self.is_function_definition():
            return self.parse_function()

        # Variable declaration or typedef
        if self.check(TokenType.TYPE_KEYWORD, TokenType.STORAGE_CLASS,
                     TokenType.TYPE_QUALIFIER, TokenType.IDENTIFIER):
            return self.parse_variable_or_typedef()

        # Skip unknown tokens
        self.advance()
        return None

    def is_function_definition(self) -> bool:
        """Check if current position is start of function definition"""
        saved = self.current

        # Skip storage class and type qualifiers
        while self.check(TokenType.STORAGE_CLASS, TokenType.TYPE_QUALIFIER):
            self.advance()

        # Must have return type
        if not self.check(TokenType.TYPE_KEYWORD, TokenType.IDENTIFIER):
            self.current = saved
            return False

        self.advance()  # Return type

        # Handle pointer return
        while self.match(TokenType.ARITHMETIC) and self.previous().value == '*':
            pass

        # Must have function name
        if not self.check(TokenType.IDENTIFIER):
            self.current = saved
            return False

        self.advance()  # Function name

        # Must have parameter list
        if not self.check(TokenType.LPAREN):
            self.current = saved
            return False

        # Skip to closing paren
        paren_count = 0
        while not self.is_at_end():
            if self.check(TokenType.LPAREN):
                paren_count += 1
            elif self.check(TokenType.RPAREN):
                paren_count -= 1
                if paren_count == 0:
                    self.advance()
                    break
            self.advance()

        # Check for opening brace (function body)
        result = self.check(TokenType.LBRACE)

        self.current = saved
        return result

    def parse_function(self) -> FunctionNode:
        """Parse function definition"""
        line = self.peek().line
        column = self.peek().column
        file = self.peek().file

        storage_class = None

        # Parse storage class
        if self.check(TokenType.STORAGE_CLASS):
            storage_class = self.advance().value

        # Skip type qualifiers
        while self.check(TokenType.TYPE_QUALIFIER):
            self.advance()

        # Parse return type
        return_type = ""
        if self.check(TokenType.TYPE_KEYWORD, TokenType.IDENTIFIER):
            return_type = self.advance().value

        # Handle pointer
        while self.match(TokenType.ARITHMETIC) and self.previous().value == '*':
            return_type += '*'

        # Parse function name
        name = self.consume(TokenType.IDENTIFIER, "Expected function name").value

        # Parse parameters
        self.consume(TokenType.LPAREN, "Expected '(' after function name")
        parameters = self.parse_parameters()
        self.consume(TokenType.RPAREN, "Expected ')' after parameters")

        # Parse body
        body = self.parse_block()

        # Calculate complexity
        complexity = self.calculate_complexity(body)

        # Extract attributes
        attributes = self.extract_function_attributes(name, body)

        return FunctionNode(
            line=line,
            column=column,
            name=name,
            return_type=return_type,
            parameters=parameters,
            body=body,
            file=file,
            storage_class=storage_class,
            attributes=attributes,
            complexity=complexity
        )

    def parse_parameters(self) -> List[Dict[str, str]]:
        """Parse function parameters"""
        parameters = []

        if self.check(TokenType.RPAREN):
            return parameters

        # Handle void parameter
        if self.check(TokenType.TYPE_KEYWORD) and self.peek().value == "void":
            next_token = self.peek_next()
            if next_token and next_token.type == TokenType.RPAREN:
                self.advance()  # Skip void
                return parameters

        while True:
            # Skip const/volatile
            while self.check(TokenType.TYPE_QUALIFIER):
                self.advance()

            # Get parameter type
            param_type = ""
            if self.check(TokenType.TYPE_KEYWORD, TokenType.IDENTIFIER):
                param_type = self.advance().value

            # Handle pointer
            while self.match(TokenType.ARITHMETIC) and self.previous().value == '*':
                param_type += '*'

            # Get parameter name (optional in declarations)
            param_name = ""
            if self.check(TokenType.IDENTIFIER):
                param_name = self.advance().value

            if param_type:
                parameters.append({'type': param_type, 'name': param_name})

            if not self.match(TokenType.COMMA):
                break

        return parameters

    def parse_block(self) -> BlockNode:
        """Parse code block"""
        self.consume(TokenType.LBRACE, "Expected '{'")

        line = self.previous().line
        column = self.previous().column
        file = self.previous().file

        statements = []

        while not self.check(TokenType.RBRACE) and not self.is_at_end():
            stmt = self.parse_statement()
            if stmt:
                statements.append(stmt)

        self.consume(TokenType.RBRACE, "Expected '}'")

        return BlockNode(line=line, column=column, statements=statements, file=file)

    def parse_statement(self) -> Optional[ASTNode]:
        """Parse a statement"""
        # Skip comments
        if self.check(TokenType.COMMENT, TokenType.MULTILINE_COMMENT):
            self.advance()
            return None

        # If statement
        if self.match(TokenType.KEYWORD) and self.previous().value == "if":
            return self.parse_if_statement()

        # While loop
        if self.match(TokenType.KEYWORD) and self.previous().value == "while":
            return self.parse_while_statement()

        # For loop
        if self.match(TokenType.KEYWORD) and self.previous().value == "for":
            return self.parse_for_statement()

        # Return statement
        if self.match(TokenType.KEYWORD) and self.previous().value == "return":
            return self.parse_return_statement()

        # Block
        if self.check(TokenType.LBRACE):
            return self.parse_block()

        # Function/macro call
        if self.check(TokenType.IDENTIFIER, TokenType.MACRO):
            return self.parse_expression_statement()

        # Variable declaration
        if self.check(TokenType.TYPE_KEYWORD, TokenType.STORAGE_CLASS, TokenType.TYPE_QUALIFIER):
            return self.parse_variable_declaration()

        # Skip other statements for now
        self.skip_to_semicolon()
        return None

    def parse_if_statement(self) -> IfNode:
        """Parse if statement"""
        line = self.previous().line
        column = self.previous().column
        file = self.previous().file

        self.consume(TokenType.LPAREN, "Expected '(' after 'if'")
        condition = self.parse_expression()
        self.consume(TokenType.RPAREN, "Expected ')' after condition")

        then_branch = self.parse_statement()
        else_branch = None

        if self.match(TokenType.KEYWORD) and self.previous().value == "else":
            else_branch = self.parse_statement()

        return IfNode(
            line=line,
            column=column,
            condition=condition,
            then_branch=then_branch,
            file=file,
            else_branch=else_branch
        )

    def parse_while_statement(self) -> WhileNode:
        """Parse while loop"""
        line = self.previous().line
        column = self.previous().column
        file = self.previous().file

        self.consume(TokenType.LPAREN, "Expected '(' after 'while'")
        condition = self.parse_expression()
        self.consume(TokenType.RPAREN, "Expected ')' after condition")

        body = self.parse_statement()

        return WhileNode(
            line=line,
            column=column,
            condition=condition,
            body=body,
            file=file
        )

    def parse_for_statement(self) -> ForNode:
        """Parse for loop"""
        line = self.previous().line
        column = self.previous().column
        file = self.previous().file

        self.consume(TokenType.LPAREN, "Expected '(' after 'for'")

        # Parse init
        init = None
        if not self.check(TokenType.SEMICOLON):
            init = self.parse_expression()
        self.consume(TokenType.SEMICOLON, "Expected ';' after for init")

        # Parse condition
        condition = None
        if not self.check(TokenType.SEMICOLON):
            condition = self.parse_expression()
        self.consume(TokenType.SEMICOLON, "Expected ';' after for condition")

        # Parse update
        update = None
        if not self.check(TokenType.RPAREN):
            update = self.parse_expression()
        self.consume(TokenType.RPAREN, "Expected ')' after for clauses")

        body = self.parse_statement()

        return ForNode(
            line=line,
            column=column,
            body=body,
            file=file,
            init=init,
            condition=condition,
            update=update
        )

    def parse_return_statement(self) -> ReturnNode:
        """Parse return statement"""
        line = self.previous().line
        column = self.previous().column
        file = self.previous().file

        value = None
        macro_return = False

        # Check for HeliOS return macros
        if self.check(TokenType.IDENTIFIER, TokenType.MACRO):
            next_token = self.peek()
            if next_token.value in ['__ReturnOk__', '__ReturnWithError__', '__ReturnWithValue__']:
                macro_return = True

        if not self.check(TokenType.SEMICOLON):
            value = self.parse_expression()

        self.consume(TokenType.SEMICOLON, "Expected ';' after return value")

        return ReturnNode(
            line=line,
            column=column,
            file=file,
            value=value,
            macro_return=macro_return
        )

    def parse_expression_statement(self) -> Optional[ASTNode]:
        """Parse expression statement (function call, etc.)"""
        expr = self.parse_expression()
        self.consume(TokenType.SEMICOLON, "Expected ';' after expression")
        return expr

    def parse_variable_declaration(self) -> VariableNode:
        """Parse variable declaration"""
        line = self.peek().line
        column = self.peek().column
        file = self.peek().file

        is_static = False
        is_volatile = False

        # Handle storage class
        if self.check(TokenType.STORAGE_CLASS):
            storage = self.advance().value
            if storage == "static":
                is_static = True

        # Handle type qualifiers
        while self.check(TokenType.TYPE_QUALIFIER):
            qualifier = self.advance().value
            if qualifier == "volatile":
                is_volatile = True

        # Get type
        var_type = ""
        if self.check(TokenType.TYPE_KEYWORD, TokenType.IDENTIFIER):
            var_type = self.advance().value

        # Handle pointer
        is_pointer = False
        while self.match(TokenType.ARITHMETIC) and self.previous().value == '*':
            var_type += '*'
            is_pointer = True

        # Get name
        name = ""
        if self.check(TokenType.IDENTIFIER):
            name = self.advance().value

        # Handle initializer
        initializer = None
        if self.match(TokenType.ASSIGNMENT):
            initializer = self.parse_expression()

        self.consume(TokenType.SEMICOLON, "Expected ';' after variable declaration")

        return VariableNode(
            line=line,
            column=column,
            type=var_type,
            name=name,
            file=file,
            initializer=initializer,
            is_pointer=is_pointer,
            is_static=is_static,
            is_volatile=is_volatile
        )

    def parse_expression(self) -> Optional[ASTNode]:
        """Parse expression (simplified)"""
        # For now, just collect tokens until statement end
        expr_tokens = []
        paren_depth = 0

        while not self.is_at_end():
            if self.check(TokenType.LPAREN):
                paren_depth += 1
            elif self.check(TokenType.RPAREN):
                if paren_depth == 0:
                    break
                paren_depth -= 1
            elif self.check(TokenType.SEMICOLON) and paren_depth == 0:
                break
            elif self.check(TokenType.COMMA) and paren_depth == 0:
                break

            expr_tokens.append(self.advance())

        # Check for function/macro calls
        if len(expr_tokens) > 0 and expr_tokens[0].type in (TokenType.IDENTIFIER, TokenType.MACRO):
            if len(expr_tokens) > 1 and expr_tokens[1].type == TokenType.LPAREN:
                return self.parse_call_from_tokens(expr_tokens)

        return None

    def parse_call_from_tokens(self, tokens: List[Token]) -> CallNode:
        """Parse function/macro call from tokens"""
        name = tokens[0].value
        is_macro = tokens[0].type == TokenType.MACRO or name.startswith('__')

        return CallNode(
            line=tokens[0].line,
            column=tokens[0].column,
            name=name,
            file=tokens[0].file,
            arguments=[],  # Simplified - not parsing args for now
            is_macro=is_macro
        )

    def parse_variable_or_typedef(self) -> Optional[ASTNode]:
        """Parse variable declaration or typedef"""
        # For now, skip to semicolon
        self.skip_to_semicolon()
        return None

    def calculate_complexity(self, node: ASTNode) -> int:
        """Calculate cyclomatic complexity of a node"""
        if not node:
            return 0

        complexity = 1

        # Recursively calculate complexity
        if isinstance(node, BlockNode):
            for stmt in node.statements:
                complexity += self.calculate_complexity(stmt) - 1
        elif isinstance(node, IfNode):
            complexity += 1
            complexity += self.calculate_complexity(node.then_branch) - 1
            if node.else_branch:
                complexity += self.calculate_complexity(node.else_branch) - 1
        elif isinstance(node, (WhileNode, ForNode)):
            complexity += 1
            complexity += self.calculate_complexity(node.body) - 1
        elif isinstance(node, CallNode):
            # Logical operators in conditions
            if node.name in ['&&', '||']:
                complexity += 1

        return complexity

    def extract_function_attributes(self, name: str, body: BlockNode) -> Dict[str, Any]:
        """Extract function attributes for rule checking"""
        attributes = {
            'has_function_enter': False,
            'has_function_exit': False,
            'is_public_api': name.startswith('x') if name else False,
            'uses_return_macros': False,
            'has_error_handling': False,
            'has_memory_checks': False,
            'has_pointer_checks': False
        }

        # Check for specific patterns in function body
        for stmt in body.statements if body else []:
            self.check_attributes_in_node(stmt, attributes)

        return attributes

    def check_attributes_in_node(self, node: ASTNode, attributes: Dict[str, Any]):
        """Recursively check for attributes in AST node"""
        if isinstance(node, CallNode):
            if node.name == 'FUNCTION_ENTER':
                attributes['has_function_enter'] = True
            elif node.name == 'FUNCTION_EXIT':
                attributes['has_function_exit'] = True
            elif node.name in ['__ReturnOk__', '__ReturnWithError__', '__ReturnWithValue__']:
                attributes['uses_return_macros'] = True
            elif node.name == '__AssertOnElse__':
                attributes['has_error_handling'] = True
            elif node.name in ['__PointerIsNull__', '__PointerIsNotNull__']:
                attributes['has_pointer_checks'] = True
            elif node.name in ['__KernelAllocateMemory__', '__HeapAllocateMemory__']:
                attributes['has_memory_checks'] = True
        elif isinstance(node, BlockNode):
            for stmt in node.statements:
                self.check_attributes_in_node(stmt, attributes)
        elif isinstance(node, IfNode):
            self.check_attributes_in_node(node.then_branch, attributes)
            if node.else_branch:
                self.check_attributes_in_node(node.else_branch, attributes)
        elif isinstance(node, (WhileNode, ForNode)):
            self.check_attributes_in_node(node.body, attributes)

    def skip_to_semicolon(self):
        """Skip tokens until semicolon"""
        while not self.is_at_end() and not self.check(TokenType.SEMICOLON):
            self.advance()
        if self.check(TokenType.SEMICOLON):
            self.advance()

    def synchronize(self):
        """Synchronize after parse error"""
        self.advance()

        while not self.is_at_end():
            if self.previous().type == TokenType.SEMICOLON:
                return

            if self.peek().type in (TokenType.KEYWORD, TokenType.TYPE_KEYWORD):
                return

            self.advance()

    def match(self, *types: TokenType) -> bool:
        """Check and consume if token matches any type"""
        for token_type in types:
            if self.check(token_type):
                self.advance()
                return True
        return False

    def check(self, *types: TokenType) -> bool:
        """Check if current token matches any type"""
        if self.is_at_end():
            return False
        return self.peek().type in types

    def advance(self) -> Token:
        """Consume current token"""
        if not self.is_at_end():
            self.current += 1
        return self.previous()

    def is_at_end(self) -> bool:
        """Check if at end of tokens"""
        return self.peek().type == TokenType.EOF

    def peek(self) -> Token:
        """Get current token"""
        if self.current < len(self.tokens):
            return self.tokens[self.current]
        return self.tokens[-1]  # EOF

    def peek_next(self) -> Optional[Token]:
        """Get next token"""
        if self.current + 1 < len(self.tokens):
            return self.tokens[self.current + 1]
        return None

    def previous(self) -> Token:
        """Get previous token"""
        return self.tokens[self.current - 1]

    def consume(self, token_type: TokenType, message: str) -> Token:
        """Consume token of specific type or raise error"""
        if self.check(token_type):
            return self.advance()

        current = self.peek()
        raise Exception(f"{message} at line {current.line}, column {current.column}")

# ============================================================================
# Context-Aware Analysis Engine
# ============================================================================

@dataclass
class AnalysisContext:
    """Context for code analysis"""
    file_path: str
    ast: List[ASTNode]
    tokens: List[Token]
    macro_definitions: Dict[str, Any]
    symbol_table: Dict[str, Any]
    includes: List[str]
    function_declarations: Dict[str, FunctionNode]
    global_variables: Dict[str, VariableNode]
    type_definitions: Dict[str, Any]
    cross_references: Dict[str, Set[str]]
    in_function: bool = False
    in_macro: bool = False
    current_function: Optional[FunctionNode] = None
    scope_stack: List[Dict] = field(default_factory=list)

    def has_declaration(self, name: str) -> bool:
        """Check if function has declaration"""
        return name in self.function_declarations

    def get_symbol(self, name: str) -> Optional[Any]:
        """Get symbol from current scope"""
        # Check local scopes (most recent first)
        for scope in reversed(self.scope_stack):
            if name in scope:
                return scope[name]

        # Check global scope
        if name in self.symbol_table:
            return self.symbol_table[name]

        return None

    def add_symbol(self, name: str, symbol: Any):
        """Add symbol to current scope"""
        if self.scope_stack:
            self.scope_stack[-1][name] = symbol
        else:
            self.symbol_table[name] = symbol

    def enter_scope(self):
        """Enter new scope"""
        self.scope_stack.append({})

    def exit_scope(self):
        """Exit current scope"""
        if self.scope_stack:
            self.scope_stack.pop()

# ============================================================================
# Data Models
# ============================================================================

class Severity(Enum):
    """Violation severity levels"""
    CRITICAL = 1
    IMPORTANT = 2
    RECOMMENDED = 3
    INFO = 4

class Confidence(Enum):
    """Confidence levels for violations"""
    VERY_HIGH = 0.95
    HIGH = 0.85
    MEDIUM = 0.70
    LOW = 0.50

@dataclass
class SourceLocation:
    """Location in source code"""
    file: str
    line: int
    column: int = 0
    end_line: int = 0

@dataclass
class Function:
    """Represents a C function"""
    name: str
    location: SourceLocation
    return_type: str = ""
    parameters: List[Dict[str, str]] = field(default_factory=list)
    body: str = ""
    complexity: int = 0
    attributes: Dict[str, Any] = field(default_factory=dict)
    ast_node: Optional[FunctionNode] = None
    documentation: Optional[str] = None
    is_static: bool = False

    @property
    def has_doxygen_comment(self) -> bool:
        """Check if function has doxygen documentation"""
        return self.documentation is not None and '/**' in self.documentation

@dataclass
class Violation:
    """Coding standard violation"""
    rule_id: str
    severity: Severity
    location: SourceLocation
    message: str
    category: str = ""
    fix_suggestion: str = ""
    confidence: float = 0.85
    context: Optional[str] = None
    suppressed: bool = False

@dataclass
class FileAnalysis:
    """Analysis results for a single file"""
    filepath: str
    violations: List[Violation]
    functions: List[Function]
    metrics: Dict[str, Any]
    parse_errors: List[str]
    analysis_time: float
    ast: Optional[List[ASTNode]] = None
    context: Optional[AnalysisContext] = None

# ============================================================================
# Smart Pattern Matching System
# ============================================================================

class SmartPattern:
    """Intelligent pattern matcher with context awareness"""

    def __init__(self, pattern: str, context_requirements: Optional[Dict] = None,
                 confidence_modifier: float = 1.0):
        self.pattern = pattern
        self.context_requirements = context_requirements or {}
        self.confidence_modifier = confidence_modifier
        self.compiled_pattern = None

        # Compile regex pattern
        try:
            self.compiled_pattern = re.compile(pattern)
        except re.error as e:
            logging.warning(f"Invalid pattern '{pattern}': {e}")

    def matches(self, text: str, context: AnalysisContext) -> Tuple[bool, float]:
        """Check if pattern matches with confidence score"""
        # Check context requirements first
        confidence = self.confidence_modifier

        if self.context_requirements:
            if self.context_requirements.get('in_function') and not context.in_function:
                return False, 0.0

            if self.context_requirements.get('not_in_macro') and context.in_macro:
                confidence *= 0.7  # Lower confidence in macro context

            if self.context_requirements.get('is_public_api'):
                if context.current_function and not context.current_function.name.startswith('x'):
                    return False, 0.0

        # Check pattern match
        if self.compiled_pattern and self.compiled_pattern.search(text):
            return True, confidence

        return False, 0.0

class TokenPattern:
    """Token-based pattern matcher"""

    def __init__(self, token_sequence: List[Tuple[TokenType, Optional[str]]]):
        self.token_sequence = token_sequence

    def matches(self, tokens: List[Token], start: int = 0) -> Tuple[bool, int]:
        """Check if token sequence matches starting at position"""
        pos = start

        for expected_type, expected_value in self.token_sequence:
            if pos >= len(tokens):
                return False, start

            token = tokens[pos]

            if token.type != expected_type:
                return False, start

            if expected_value and token.value != expected_value:
                return False, start

            pos += 1

        return True, pos

# ============================================================================
# Rule Engine
# ============================================================================

class RuleEngine:
    """Advanced rule engine with semantic understanding"""

    def __init__(self, rules_file: str):
        self.rules = self._load_rules(rules_file)
        self.suppression_patterns = []
        self.confidence_threshold = 0.70

    def _load_rules(self, rules_file: str) -> Dict:
        """Load rules from YAML file"""
        try:
            with open(rules_file, 'r') as f:
                data = yaml.safe_load(f)
                return data.get('rules', {})
        except Exception as e:
            logging.error(f"Error loading rules from {rules_file}: {e}")
            return {}

    def check_function(self, func: Function, rule_id: str, rule_config: Dict,
                      context: AnalysisContext) -> List[Violation]:
        """Check a function against a rule with context"""
        violations = []
        check_type = rule_config.get('check_type', 'pattern')

        # Map check types to handler methods
        handlers = {
            'pattern': self._check_pattern_rule,
            'builtin': self._check_builtin_rule,
            'semantic': self._check_semantic_rule,
            'composite': self._check_composite_rule,
            'ast': self._check_ast_rule,
            'dataflow': self._check_dataflow_rule,
            'helios': self._check_helios_specific_rule
        }

        handler = handlers.get(check_type)
        if handler:
            violations.extend(handler(func, rule_id, rule_config, context))

        # Apply confidence filtering
        violations = [v for v in violations if v.confidence >= self.confidence_threshold]

        # Check for suppression
        violations = self._apply_suppressions(violations, context)

        return violations

    def _check_pattern_rule(self, func: Function, rule_id: str, config: Dict,
                           context: AnalysisContext) -> List[Violation]:
        """Enhanced pattern-based rule checking"""
        violations = []
        patterns = config.get('patterns', {})
        exclude_patterns = config.get('exclude_patterns', [])

        for pattern_name, pattern_config in patterns.items():
            # Handle both simple string patterns and complex pattern configs
            if isinstance(pattern_config, str):
                pattern_str = pattern_config
                pattern_context = {}
            else:
                pattern_str = pattern_config.get('pattern', '')
                pattern_context = pattern_config.get('context', {})

            smart_pattern = SmartPattern(pattern_str, pattern_context)
            matched, confidence = smart_pattern.matches(func.body, context)

            if matched:
                # Check exclusions
                should_exclude = False
                for exclude in exclude_patterns:
                    if re.search(exclude, func.body):
                        should_exclude = True
                        break

                if not should_exclude:
                    violations.append(Violation(
                        rule_id=rule_id,
                        severity=Severity[config.get('severity', 'RECOMMENDED')],
                        location=func.location,
                        message=config.get('message', f"Pattern '{pattern_name}' matched in {func.name}"),
                        category=config.get('category', 'General'),
                        fix_suggestion=config.get('fix_suggestion', ''),
                        confidence=confidence
                    ))

        return violations

    def _check_builtin_rule(self, func: Function, rule_id: str, config: Dict,
                           context: AnalysisContext) -> List[Violation]:
        """Enhanced built-in rule checking"""
        violations = []
        builtin_type = config.get('builtin_type', '')

        if builtin_type == 'function_structure':
            violations.extend(self._check_function_structure(func, rule_id, config, context))
        elif builtin_type == 'parameter_naming':
            violations.extend(self._check_parameter_naming(func, rule_id, config, context))
        elif builtin_type == 'early_returns':
            violations.extend(self._check_early_returns(func, rule_id, config, context))
        elif builtin_type == 'memory_safety':
            violations.extend(self._check_memory_safety(func, rule_id, config, context))

        return violations

    def _check_function_structure(self, func: Function, rule_id: str, config: Dict,
                                 context: AnalysisContext) -> List[Violation]:
        """Check function structure requirements"""
        violations = []
        confidence = 0.95

        # Adjust confidence based on context
        if func.ast_node and func.ast_node.storage_class == 'inline':
            confidence *= 0.8  # Lower confidence for inline functions

        if config.get('require_enter') and not func.attributes.get('has_function_enter'):
            # Check if it's a macro-generated function
            if not self._is_macro_generated_function(func, context):
                violations.append(Violation(
                    rule_id=rule_id,
                    severity=Severity[config.get('severity', 'CRITICAL')],
                    location=func.location,
                    message=f"Function '{func.name}' missing FUNCTION_ENTER",
                    category=config.get('category', 'Function Structure'),
                    fix_suggestion=config.get('fix_suggestion', 'Add FUNCTION_ENTER after opening brace'),
                    confidence=confidence
                ))

        if config.get('require_exit') and not func.attributes.get('has_function_exit'):
            if not self._is_macro_generated_function(func, context):
                violations.append(Violation(
                    rule_id=rule_id,
                    severity=Severity[config.get('severity', 'CRITICAL')],
                    location=func.location,
                    message=f"Function '{func.name}' missing FUNCTION_EXIT",
                    category=config.get('category', 'Function Structure'),
                    fix_suggestion=config.get('fix_suggestion', 'Add FUNCTION_EXIT before closing brace'),
                    confidence=confidence
                ))

        return violations

    def _check_parameter_naming(self, func: Function, rule_id: str, config: Dict,
                               context: AnalysisContext) -> List[Violation]:
        """Check parameter naming conventions"""
        violations = []
        suffix = config.get('require_suffix', '_')
        exclude_types = config.get('exclude_types', [])

        for param in func.parameters:
            param_name = param.get('name', '')
            param_type = param.get('type', '')

            # Skip if no name (abstract declarations)
            if not param_name:
                continue

            # Check exclusions
            should_exclude = any(excl in param_type for excl in exclude_types)

            if not should_exclude and not param_name.endswith(suffix):
                confidence = 0.90

                # Lower confidence for common false positives
                if param_name in ['argc', 'argv', 'main']:
                    confidence *= 0.5

                violations.append(Violation(
                    rule_id=rule_id,
                    severity=Severity[config.get('severity', 'IMPORTANT')],
                    location=func.location,
                    message=f"Parameter '{param_name}' missing '{suffix}' suffix",
                    category=config.get('category', 'Naming Convention'),
                    fix_suggestion=f"Rename to '{param_name}{suffix}'",
                    confidence=confidence
                ))

        return violations

    def _check_early_returns(self, func: Function, rule_id: str, config: Dict,
                            context: AnalysisContext) -> List[Violation]:
        """Check for early returns using AST"""
        violations = []

        if not func.ast_node:
            return violations

        allowed_patterns = config.get('allowed_patterns', ['__ReturnOk__'])

        # Walk AST to find return statements
        return_nodes = self._find_return_nodes(func.ast_node.body)

        for return_node in return_nodes:
            if not return_node.macro_return:
                # Check if it's in an allowed context
                if not self._is_return_allowed(return_node, allowed_patterns, context):
                    violations.append(Violation(
                        rule_id=rule_id,
                        severity=Severity[config.get('severity', 'CRITICAL')],
                        location=SourceLocation(
                            file=func.location.file,
                            line=return_node.line
                        ),
                        message=config.get('message', 'Early return detected'),
                        category=config.get('category', 'Function Structure'),
                        fix_suggestion=config.get('fix_suggestion', 'Use __ReturnOk__() instead'),
                        confidence=0.90
                    ))

        return violations

    def _check_memory_safety(self, func: Function, rule_id: str, config: Dict,
                            context: AnalysisContext) -> List[Violation]:
        """Check memory safety requirements"""
        violations = []

        if not func.ast_node:
            return violations

        # Track allocations and frees
        allocations = []
        frees = []

        self._find_memory_operations(func.ast_node.body, allocations, frees)

        # Check for unmatched allocations
        for alloc in allocations:
            if not self._has_corresponding_free(alloc, frees, func.ast_node):
                violations.append(Violation(
                    rule_id=rule_id,
                    severity=Severity.CRITICAL,
                    location=SourceLocation(
                        file=func.location.file,
                        line=alloc.line
                    ),
                    message=f"Memory allocation at line {alloc.line} may not be freed",
                    category="Memory Management",
                    fix_suggestion="Ensure allocated memory is freed in all code paths",
                    confidence=0.75  # Lower confidence as this needs path analysis
                ))

        return violations

    def _check_semantic_rule(self, func: Function, rule_id: str, config: Dict,
                            context: AnalysisContext) -> List[Violation]:
        """Enhanced semantic rule checking"""
        violations = []

        # Complexity check
        if 'max_complexity' in config:
            if func.complexity > config['max_complexity']:
                violations.append(Violation(
                    rule_id=rule_id,
                    severity=Severity[config.get('severity', 'RECOMMENDED')],
                    location=func.location,
                    message=f"Function '{func.name}' complexity {func.complexity} exceeds maximum {config['max_complexity']}",
                    category=config.get('category', 'Complexity'),
                    fix_suggestion=config.get('fix_suggestion', 'Refactor into smaller functions'),
                    confidence=0.95
                ))

        # Function length check
        if 'max_lines' in config:
            lines = func.body.count('\n')
            if lines > config['max_lines']:
                violations.append(Violation(
                    rule_id=rule_id,
                    severity=Severity[config.get('severity', 'RECOMMENDED')],
                    location=func.location,
                    message=f"Function '{func.name}' has {lines} lines, exceeds maximum {config['max_lines']}",
                    category=config.get('category', 'Size'),
                    fix_suggestion=config.get('fix_suggestion', 'Split into smaller functions'),
                    confidence=0.95
                ))

        return violations

    def _check_composite_rule(self, func: Function, rule_id: str, config: Dict,
                             context: AnalysisContext) -> List[Violation]:
        """Check composite rules"""
        violations = []

        required_rules = config.get('requires_all', [])
        any_of_rules = config.get('requires_any', [])

        # Check all required rules
        all_matched = True
        for req_rule_id in required_rules:
            if req_rule_id in self.rules:
                req_violations = self.check_function(func, req_rule_id, self.rules[req_rule_id], context)
                if not req_violations:
                    all_matched = False
                    break

        if not all_matched:
            violations.append(Violation(
                rule_id=rule_id,
                severity=Severity[config.get('severity', 'IMPORTANT')],
                location=func.location,
                message=config.get('message', f"Composite rule {rule_id} not satisfied for {func.name}"),
                category=config.get('category', 'Composite'),
                fix_suggestion=config.get('fix_suggestion', 'Check component rules'),
                confidence=0.85
            ))

        return violations

    def _check_ast_rule(self, func: Function, rule_id: str, config: Dict,
                       context: AnalysisContext) -> List[Violation]:
        """Check rules using AST analysis"""
        violations = []

        if not func.ast_node:
            return violations

        # Example: Check for uninitialized variables
        if config.get('check_uninitialized'):
            uninitialized = self._find_uninitialized_variables(func.ast_node)
            for var in uninitialized:
                violations.append(Violation(
                    rule_id=rule_id,
                    severity=Severity.IMPORTANT,
                    location=SourceLocation(
                        file=func.location.file,
                        line=var.line
                    ),
                    message=f"Variable '{var.name}' may be used uninitialized",
                    category="Initialization",
                    fix_suggestion=f"Initialize '{var.name}' at declaration",
                    confidence=0.80
                ))

        return violations

    def _check_dataflow_rule(self, func: Function, rule_id: str, config: Dict,
                            context: AnalysisContext) -> List[Violation]:
        """Check dataflow-based rules"""
        violations = []

        if not func.ast_node:
            return violations

        # Example: Check for null pointer dereferences
        if config.get('check_null_deref'):
            null_derefs = self._find_potential_null_derefs(func.ast_node, context)
            for deref in null_derefs:
                violations.append(Violation(
                    rule_id=rule_id,
                    severity=Severity.CRITICAL,
                    location=SourceLocation(
                        file=func.location.file,
                        line=deref['line']
                    ),
                    message=f"Potential null pointer dereference of '{deref['variable']}'",
                    category="Memory Safety",
                    fix_suggestion=f"Check '{deref['variable']}' for NULL before dereferencing",
                    confidence=deref['confidence']
                ))

        return violations

    def _check_helios_specific_rule(self, func: Function, rule_id: str, config: Dict,
                                   context: AnalysisContext) -> List[Violation]:
        """Check HeliOS-specific rules"""
        violations = []

        rule_type = config.get('helios_rule_type', '')

        if rule_type == 'state_machine':
            violations.extend(self._check_state_machine_rules(func, rule_id, config, context))
        elif rule_type == 'scheduler':
            violations.extend(self._check_scheduler_rules(func, rule_id, config, context))
        elif rule_type == 'interrupt_safety':
            violations.extend(self._check_interrupt_safety(func, rule_id, config, context))
        elif rule_type == 'api_consistency':
            violations.extend(self._check_api_consistency(func, rule_id, config, context))

        return violations

    def _check_state_machine_rules(self, func: Function, rule_id: str, config: Dict,
                                  context: AnalysisContext) -> List[Violation]:
        """Check HeliOS state machine rules"""
        violations = []

        # Check for invalid task state transitions
        valid_transitions = {
            'TaskStateReady': ['TaskStateRunning', 'TaskStateSuspended'],
            'TaskStateRunning': ['TaskStateReady', 'TaskStateBlocked', 'TaskStateSuspended'],
            'TaskStateBlocked': ['TaskStateReady'],
            'TaskStateSuspended': ['TaskStateReady']
        }

        # Find state assignments in function
        state_pattern = r'(\w+)->state\s*=\s*(TaskState\w+)'
        matches = re.finditer(state_pattern, func.body)

        for match in matches:
            var_name = match.group(1)
            new_state = match.group(2)

            # Try to determine current state (simplified)
            # In real implementation, would use dataflow analysis
            confidence = 0.70

            # Check if transition might be invalid
            if new_state not in ['TaskStateReady', 'TaskStateRunning', 'TaskStateBlocked', 'TaskStateSuspended']:
                violations.append(Violation(
                    rule_id=rule_id,
                    severity=Severity.CRITICAL,
                    location=func.location,
                    message=f"Unknown task state '{new_state}'",
                    category="State Machine",
                    fix_suggestion="Use valid TaskState values",
                    confidence=0.95
                ))

        return violations

    def _check_scheduler_rules(self, func: Function, rule_id: str, config: Dict,
                              context: AnalysisContext) -> List[Violation]:
        """Check HeliOS scheduler rules"""
        violations = []

        # Check for scheduler state consistency
        if 'xTaskStart' in func.name or 'xTaskStop' in func.name:
            # These functions should check scheduler state
            if 'scheduler' not in func.body:
                violations.append(Violation(
                    rule_id=rule_id,
                    severity=Severity.IMPORTANT,
                    location=func.location,
                    message=f"Function '{func.name}' should check scheduler state",
                    category="Scheduler",
                    fix_suggestion="Add scheduler state check before operation",
                    confidence=0.75
                ))

        return violations

    def _check_interrupt_safety(self, func: Function, rule_id: str, config: Dict,
                               context: AnalysisContext) -> List[Violation]:
        """Check interrupt safety rules"""
        violations = []

        # Check for proper interrupt disable/enable pairs
        disable_pattern = r'__DisableInterrupts__\s*\(\s*\)'
        enable_pattern = r'__EnableInterrupts__\s*\(\s*\)'

        disable_matches = list(re.finditer(disable_pattern, func.body))
        enable_matches = list(re.finditer(enable_pattern, func.body))

        if len(disable_matches) != len(enable_matches):
            violations.append(Violation(
                rule_id=rule_id,
                severity=Severity.CRITICAL,
                location=func.location,
                message=f"Mismatched interrupt disable/enable pairs in '{func.name}'",
                category="Concurrency",
                fix_suggestion="Ensure every __DisableInterrupts__() has matching __EnableInterrupts__()",
                confidence=0.85
            ))

        # Check for shared data access without protection
        if func.attributes.get('accesses_shared_data') and not disable_matches:
            violations.append(Violation(
                rule_id=rule_id,
                severity=Severity.IMPORTANT,
                location=func.location,
                message=f"Function '{func.name}' accesses shared data without interrupt protection",
                category="Concurrency",
                fix_suggestion="Use interrupt disable/enable around shared data access",
                confidence=0.70
            ))

        return violations

    def _check_api_consistency(self, func: Function, rule_id: str, config: Dict,
                              context: AnalysisContext) -> List[Violation]:
        """Check HeliOS API consistency"""
        violations = []

        # Check public API functions
        if func.name.startswith('x'):
            # Should return Return_t
            if func.return_type != 'Return_t':
                violations.append(Violation(
                    rule_id=rule_id,
                    severity=Severity.IMPORTANT,
                    location=func.location,
                    message=f"Public API function '{func.name}' should return Return_t",
                    category="API Design",
                    fix_suggestion="Change return type to Return_t",
                    confidence=0.90
                ))

            # Should have documentation
            if not func.has_doxygen_comment:
                violations.append(Violation(
                    rule_id=rule_id,
                    severity=Severity.RECOMMENDED,
                    location=func.location,
                    message=f"Public API function '{func.name}' lacks documentation",
                    category="Documentation",
                    fix_suggestion="Add Doxygen documentation comment",
                    confidence=0.95
                ))

            # Should be declared in header
            if not context.has_declaration(func.name):
                violations.append(Violation(
                    rule_id=rule_id,
                    severity=Severity.IMPORTANT,
                    location=func.location,
                    message=f"Public API function '{func.name}' not declared in header",
                    category="API Design",
                    fix_suggestion="Add function declaration to appropriate header file",
                    confidence=0.80
                ))

        return violations

    # Helper methods

    def _is_macro_generated_function(self, func: Function, context: AnalysisContext) -> bool:
        """Check if function is generated by macro"""
        # Look for common macro patterns in function name or surrounding code
        macro_patterns = [
            r'__\w+__',  # Double underscore pattern
            r'DEFINE_\w+_FUNCTION',  # Macro definition pattern
            r'IMPLEMENT_\w+',  # Implementation macro
        ]

        for pattern in macro_patterns:
            if re.search(pattern, func.name):
                return True

        # Check if function is within macro expansion context
        if context.in_macro:
            return True

        return False

    def _find_return_nodes(self, node: ASTNode) -> List[ReturnNode]:
        """Find all return nodes in AST"""
        returns = []

        if isinstance(node, ReturnNode):
            returns.append(node)
        elif isinstance(node, BlockNode):
            for stmt in node.statements:
                returns.extend(self._find_return_nodes(stmt))
        elif isinstance(node, IfNode):
            returns.extend(self._find_return_nodes(node.then_branch))
            if node.else_branch:
                returns.extend(self._find_return_nodes(node.else_branch))
        elif isinstance(node, (WhileNode, ForNode)):
            returns.extend(self._find_return_nodes(node.body))

        return returns

    def _is_return_allowed(self, return_node: ReturnNode, allowed_patterns: List[str],
                          context: AnalysisContext) -> bool:
        """Check if return is in allowed context"""
        # Check if using allowed macro
        if return_node.macro_return:
            return True

        # Check if in error handling context
        # (Would need more sophisticated analysis in real implementation)

        return False

    def _find_memory_operations(self, node: ASTNode, allocations: List, frees: List):
        """Find memory allocation and free operations"""
        if isinstance(node, CallNode):
            if node.name in ['__KernelAllocateMemory__', '__HeapAllocateMemory__',
                           'malloc', 'calloc', 'realloc']:
                allocations.append(node)
            elif node.name in ['__KernelFreeMemory__', '__HeapFreeMemory__', 'free']:
                frees.append(node)
        elif isinstance(node, BlockNode):
            for stmt in node.statements:
                self._find_memory_operations(stmt, allocations, frees)
        elif isinstance(node, IfNode):
            self._find_memory_operations(node.then_branch, allocations, frees)
            if node.else_branch:
                self._find_memory_operations(node.else_branch, allocations, frees)
        elif isinstance(node, (WhileNode, ForNode)):
            self._find_memory_operations(node.body, allocations, frees)

    def _has_corresponding_free(self, alloc: CallNode, frees: List, func_node: FunctionNode) -> bool:
        """Check if allocation has corresponding free"""
        # Simplified check - in real implementation would do path analysis
        return len(frees) > 0

    def _find_uninitialized_variables(self, func_node: FunctionNode) -> List[VariableNode]:
        """Find potentially uninitialized variables"""
        uninitialized = []

        def check_node(node: ASTNode):
            if isinstance(node, VariableNode):
                if not node.initializer and not node.is_static:
                    uninitialized.append(node)
            elif isinstance(node, BlockNode):
                for stmt in node.statements:
                    check_node(stmt)
            # Add other node types as needed

        check_node(func_node.body)
        return uninitialized

    def _find_potential_null_derefs(self, func_node: FunctionNode,
                                   context: AnalysisContext) -> List[Dict]:
        """Find potential null pointer dereferences"""
        derefs = []

        # Simplified - would need proper dataflow analysis
        # Look for pointer usage without prior null check

        return derefs

    def _apply_suppressions(self, violations: List[Violation],
                           context: AnalysisContext) -> List[Violation]:
        """Apply suppression rules"""
        filtered = []

        for violation in violations:
            # Check for inline suppression comments
            suppression_pattern = rf'//\s*helios-disable\s+{violation.rule_id}'
            if re.search(suppression_pattern, context.file_path):
                violation.suppressed = True
                continue

            # Check global suppressions
            if self._is_globally_suppressed(violation, context):
                violation.suppressed = True
                continue

            filtered.append(violation)

        return filtered

    def _is_globally_suppressed(self, violation: Violation,
                               context: AnalysisContext) -> bool:
        """Check if violation is globally suppressed"""
        # Check file-level suppressions
        for pattern in self.suppression_patterns:
            if pattern.matches(context.file_path):
                return True

        return False

# ============================================================================
# Cross-Reference Analyzer
# ============================================================================

class CrossReferenceAnalyzer:
    """Analyze cross-file references and dependencies"""

    def __init__(self):
        self.symbol_database = {}
        self.call_graph = defaultdict(set)
        self.include_graph = defaultdict(set)
        self.file_symbols = defaultdict(set)

    def analyze_file(self, file_path: str, context: AnalysisContext):
        """Analyze a file and update cross-references"""
        # Record functions defined in this file
        for func in context.function_declarations.values():
            self.symbol_database[func.name] = {
                'file': file_path,
                'type': 'function',
                'node': func
            }
            self.file_symbols[file_path].add(func.name)

        # Record global variables
        for var_name, var_node in context.global_variables.items():
            self.symbol_database[var_name] = {
                'file': file_path,
                'type': 'variable',
                'node': var_node
            }
            self.file_symbols[file_path].add(var_name)

        # Record includes
        for include in context.includes:
            self.include_graph[file_path].add(include)

    def build_call_graph(self, contexts: Dict[str, AnalysisContext]):
        """Build function call graph across all files"""
        for file_path, context in contexts.items():
            for func_name, func_node in context.function_declarations.items():
                # Find function calls within this function
                calls = self._extract_function_calls(func_node)
                for called_func in calls:
                    self.call_graph[func_name].add(called_func)

    def find_orphaned_functions(self) -> List[str]:
        """Find functions that are never called"""
        all_functions = set(self.symbol_database.keys())
        called_functions = set()

        for calls in self.call_graph.values():
            called_functions.update(calls)

        orphaned = all_functions - called_functions - {'main'}  # Exclude main

        # Filter out public API functions (they might be called externally)
        orphaned = [f for f in orphaned if not f.startswith('x')]

        return list(orphaned)

    def check_header_implementation_consistency(self, header_file: str,
                                               source_file: str) -> List[str]:
        """Check consistency between header and implementation"""
        issues = []

        header_symbols = self.file_symbols.get(header_file, set())
        source_symbols = self.file_symbols.get(source_file, set())

        # Functions declared but not implemented
        declared_not_implemented = header_symbols - source_symbols
        for symbol in declared_not_implemented:
            issues.append(f"Function '{symbol}' declared but not implemented")

        # Functions implemented but not declared (if not static)
        implemented_not_declared = source_symbols - header_symbols
        for symbol in implemented_not_declared:
            if symbol in self.symbol_database:
                sym_info = self.symbol_database[symbol]
                if sym_info['type'] == 'function':
                    func_node = sym_info['node']
                    if not func_node.storage_class == 'static':
                        issues.append(f"Function '{symbol}' implemented but not declared in header")

        return issues

    def _extract_function_calls(self, func_node: FunctionNode) -> Set[str]:
        """Extract function calls from a function"""
        calls = set()

        def visit_node(node: ASTNode):
            if isinstance(node, CallNode):
                calls.add(node.name)
            elif isinstance(node, BlockNode):
                for stmt in node.statements:
                    visit_node(stmt)
            elif isinstance(node, IfNode):
                visit_node(node.then_branch)
                if node.else_branch:
                    visit_node(node.else_branch)
            elif isinstance(node, (WhileNode, ForNode)):
                visit_node(node.body)

        if func_node.body:
            visit_node(func_node.body)

        return calls

# ============================================================================
# Code Parser
# ============================================================================

class CodeParser:
    """Parser with full capabilities"""

    def __init__(self):
        self.lexer = CLexer()
        self.cross_ref = CrossReferenceAnalyzer()
        self.macro_definitions = {}

    def parse_file(self, filepath: str, content: str) -> Dict:
        """Parse file with full analysis"""
        # Extract documentation comments
        doc_comments = self._extract_documentation(content)

        # Tokenize
        tokens = self.lexer.tokenize(content, filepath)

        # Parse to AST
        parser = CParser(tokens, self.macro_definitions)
        ast = parser.parse()

        # Build context
        context = self._build_context(filepath, ast, tokens)

        # Extract functions with complete information
        functions = self._extract_functions(ast, content, doc_comments, context)

        return {
            'content': content,
            'tokens': tokens,
            'ast': ast,
            'functions': functions,
            'context': context,
            'filepath': filepath
        }

    def _extract_documentation(self, content: str) -> Dict[int, str]:
        """Extract documentation comments by line number"""
        docs = {}
        doc_pattern = r'/\*\*.*?\*/'

        for match in re.finditer(doc_pattern, content, re.DOTALL):
            doc_text = match.group()
            # Find line number
            line_num = content[:match.start()].count('\n') + 1
            docs[line_num] = doc_text

        return docs

    def _build_context(self, filepath: str, ast: List[ASTNode],
                      tokens: List[Token]) -> AnalysisContext:
        """Build analysis context"""
        context = AnalysisContext(
            file_path=filepath,
            ast=ast,
            tokens=tokens,
            macro_definitions=self.lexer.macro_definitions,
            symbol_table={},
            includes=[],
            function_declarations={},
            global_variables={},
            type_definitions={},
            cross_references={}
        )

        # Extract includes
        for token in tokens:
            if token.type == TokenType.INCLUDE:
                match = re.search(r'["<]([^">]+)[">]', token.value)
                if match:
                    context.includes.append(match.group(1))

        # Build symbol table from AST
        for node in ast:
            if isinstance(node, FunctionNode):
                context.function_declarations[node.name] = node
                context.symbol_table[node.name] = node
            elif isinstance(node, VariableNode):
                context.global_variables[node.name] = node
                context.symbol_table[node.name] = node

        return context

    def _extract_functions(self, ast: List[ASTNode], content: str,
                                   doc_comments: Dict[int, str],
                                   context: AnalysisContext) -> List[Function]:
        """Extract functions with complete information"""
        functions = []

        for node in ast:
            if isinstance(node, FunctionNode):
                # Find function in original content
                func_pattern = rf'\b{re.escape(node.name)}\s*\([^)]*\)\s*\{{'
                match = re.search(func_pattern, content)

                if match:
                    # Extract function body
                    start = match.start()
                    brace_start = content.find('{', match.end() - 1)

                    # Match braces to find end
                    brace_count = 1
                    pos = brace_start + 1
                    while pos < len(content) and brace_count > 0:
                        if content[pos] == '{':
                            brace_count += 1
                        elif content[pos] == '}':
                            brace_count -= 1
                        pos += 1

                    body = content[brace_start:pos]

                    # Look for documentation
                    doc = None
                    for doc_line in range(node.line - 5, node.line):
                        if doc_line in doc_comments:
                            doc = doc_comments[doc_line]
                            break

                    func = Function(
                        name=node.name,
                        location=SourceLocation(
                            file=context.file_path,
                            line=node.line
                        ),
                        return_type=node.return_type,
                        parameters=node.parameters,
                        body=body,
                        complexity=node.complexity,
                        attributes=node.attributes,
                        ast_node=node,
                        documentation=doc,
                        is_static=(node.storage_class == 'static')
                    )

                    functions.append(func)

        return functions

# ============================================================================
# Main Compliance Checker
# ============================================================================

class ComplianceChecker:
    """HeliOS Source Code Compliance Checker"""

    def __init__(self, rules_file: str, cache_enabled: bool = True,
                 parallel: bool = True, confidence_threshold: float = 0.70):
        self.rules_file = rules_file
        self.rule_engine = RuleEngine(rules_file)
        self.rule_engine.confidence_threshold = confidence_threshold
        self.parser = CodeParser()
        self.cross_ref = CrossReferenceAnalyzer()
        self.cache = CacheManager() if cache_enabled else None
        self.parallel = parallel
        self.rules_hash = self._get_rules_hash()
        self.file_contexts = {}

    def _get_rules_hash(self) -> str:
        """Get hash of rules file"""
        try:
            with open(self.rules_file, 'rb') as f:
                return hashlib.md5(f.read()).hexdigest()
        except:
            return "default"

    def check_file(self, filepath: str) -> FileAnalysis:
        """Check a single file"""
        start_time = time.time()

        # Check cache
        if self.cache:
            cached = self.cache.get(filepath, self.rules_hash)
            if cached:
                return cached

        # Read file
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
        except Exception as e:
            return FileAnalysis(
                filepath=filepath,
                violations=[],
                functions=[],
                metrics={},
                parse_errors=[str(e)],
                analysis_time=0
            )

        # Parse file
        try:
            parsed = self.parser.parse_file(filepath, content)
        except Exception as e:
            logging.debug(f"Parse error in {filepath}: {e}")
            parsed = {
                'functions': [],
                'context': AnalysisContext(
                    file_path=filepath,
                    ast=[],
                    tokens=[],
                    macro_definitions={},
                    symbol_table={},
                    includes=[],
                    function_declarations={},
                    global_variables={},
                    type_definitions={},
                    cross_references={}
                )
            }

        functions = parsed.get('functions', [])
        context = parsed.get('context')

        # Store context for cross-reference analysis
        self.file_contexts[filepath] = context

        # Update cross-reference analyzer
        self.cross_ref.analyze_file(filepath, context)

        # Check all rules with context
        violations = []
        for func in functions:
            func.location.file = filepath
            context.in_function = True
            context.current_function = func.ast_node

            for rule_id, rule_config in self.rule_engine.rules.items():
                if rule_config.get('enabled', True):
                    violations.extend(
                        self.rule_engine.check_function(func, rule_id, rule_config, context)
                    )

            context.in_function = False
            context.current_function = None

        # Add file-level checks
        violations.extend(self._check_file_level_rules(filepath, context))

        # Calculate metrics
        metrics = {
            'total_functions': len(functions),
            'total_lines': len(content.split('\n')),
            'avg_complexity': sum(f.complexity for f in functions) / len(functions) if functions else 0,
            'total_tokens': len(context.tokens),
            'macro_count': len(context.macro_definitions),
            'include_count': len(context.includes),
            'global_variables': len(context.global_variables),
            'static_functions': sum(1 for f in functions if f.is_static),
            'public_api_functions': sum(1 for f in functions if f.name.startswith('x'))
        }

        analysis = FileAnalysis(
            filepath=filepath,
            violations=violations,
            functions=functions,
            metrics=metrics,
            parse_errors=[],
            analysis_time=time.time() - start_time,
            ast=parsed.get('ast'),
            context=context
        )

        # Cache result
        if self.cache:
            self.cache.set(filepath, self.rules_hash, analysis)

        return analysis

    def _check_file_level_rules(self, filepath: str, context: AnalysisContext) -> List[Violation]:
        """Check file-level rules"""
        violations = []

        # Check for missing header guards in .h files
        if filepath.endswith('.h'):
            if not self._has_header_guard(context):
                violations.append(Violation(
                    rule_id="FILE-001",
                    severity=Severity.IMPORTANT,
                    location=SourceLocation(file=filepath, line=1),
                    message="Header file missing include guards",
                    category="File Structure",
                    fix_suggestion="Add #ifndef/#define/#endif guards",
                    confidence=0.95
                ))

        # Check for excessive file size
        if context.tokens:
            line_count = context.tokens[-1].line if context.tokens else 0
            if line_count > 2000:
                violations.append(Violation(
                    rule_id="FILE-002",
                    severity=Severity.RECOMMENDED,
                    location=SourceLocation(file=filepath, line=1),
                    message=f"File has {line_count} lines, consider splitting",
                    category="File Structure",
                    fix_suggestion="Split into smaller, more focused files",
                    confidence=0.90
                ))

        return violations

    def _has_header_guard(self, context: AnalysisContext) -> bool:
        """Check if header file has include guards"""
        has_ifndef = False
        has_define = False
        has_endif = False

        for token in context.tokens[:50]:  # Check first 50 tokens
            if token.type == TokenType.IFNDEF:
                has_ifndef = True
            elif token.type == TokenType.DEFINE and has_ifndef:
                has_define = True

        for token in context.tokens[-20:]:  # Check last 20 tokens
            if token.type == TokenType.ENDIF:
                has_endif = True

        return has_ifndef and has_define and has_endif

    def check_files(self, filepaths: List[str]) -> List[FileAnalysis]:
        """Check multiple files with cross-reference analysis"""
        results = []

        # First pass: analyze all files
        if self.parallel and len(filepaths) > 1:
            with ProcessPoolExecutor(max_workers=multiprocessing.cpu_count()) as executor:
                futures = {executor.submit(self.check_file, fp): fp for fp in filepaths}

                iterator = tqdm(as_completed(futures), total=len(futures),
                              desc="Analyzing files") if TQDM_AVAILABLE else as_completed(futures)

                for future in iterator:
                    try:
                        result = future.result(timeout=30)
                        results.append(result)
                    except Exception as e:
                        filepath = futures[future]
                        logging.error(f"Error checking {filepath}: {e}")
        else:
            iterator = tqdm(filepaths, desc="Analyzing files") if TQDM_AVAILABLE else filepaths
            for filepath in iterator:
                try:
                    result = self.check_file(filepath)
                    results.append(result)
                except Exception as e:
                    logging.error(f"Error checking {filepath}: {e}")

        # Second pass: cross-reference analysis
        if len(results) > 1:
            self._perform_cross_reference_analysis(results)

        return results

    def _perform_cross_reference_analysis(self, analyses: List[FileAnalysis]):
        """Perform cross-reference analysis and add violations"""
        # Build complete call graph
        self.cross_ref.build_call_graph(self.file_contexts)

        # Find orphaned functions
        orphaned = self.cross_ref.find_orphaned_functions()
        for func_name in orphaned:
            if func_name in self.cross_ref.symbol_database:
                sym_info = self.cross_ref.symbol_database[func_name]
                file_path = sym_info['file']

                # Find the analysis for this file
                for analysis in analyses:
                    if analysis.filepath == file_path:
                        analysis.violations.append(Violation(
                            rule_id="XREF-001",
                            severity=Severity.INFO,
                            location=SourceLocation(file=file_path, line=1),
                            message=f"Function '{func_name}' is never called",
                            category="Dead Code",
                            fix_suggestion="Remove if unused or add appropriate calls",
                            confidence=0.75
                        ))
                        break

        # Check header/implementation consistency
        # (Would need to pair .h and .c files - simplified here)

    def generate_report(self, analyses: List[FileAnalysis], format: str = 'text') -> str:
        """Generate report"""
        if format == 'json':
            return self._generate_json_report(analyses)
        elif format == 'html':
            return self._generate_html_report(analyses)
        else:
            return self._generate_text_report(analyses)

    def _generate_text_report(self, analyses: List[FileAnalysis]) -> str:
        """Generate text report"""
        lines = []
        lines.append("=" * 80)
        lines.append("HeliOS Source Code Compliance Checker Report")
        lines.append("=" * 80)

        # Summary
        total_files = len(analyses)
        total_violations = sum(len(a.violations) for a in analyses)
        total_suppressed = sum(1 for a in analyses for v in a.violations if v.suppressed)

        severity_counts = defaultdict(int)
        category_counts = defaultdict(int)
        confidence_distribution = defaultdict(int)

        for analysis in analyses:
            for v in analysis.violations:
                if not v.suppressed:
                    severity_counts[v.severity.name] += 1
                    category_counts[v.category] += 1

                    # Bucket confidence scores
                    if v.confidence >= 0.90:
                        confidence_distribution['Very High (≥90%)'] += 1
                    elif v.confidence >= 0.80:
                        confidence_distribution['High (80-89%)'] += 1
                    elif v.confidence >= 0.70:
                        confidence_distribution['Medium (70-79%)'] += 1
                    else:
                        confidence_distribution['Low (<70%)'] += 1

        lines.append(f"\nFiles analyzed: {total_files}")
        lines.append(f"Total violations: {total_violations}")
        if total_suppressed > 0:
            lines.append(f"Suppressed violations: {total_suppressed}")

        if severity_counts:
            lines.append("\nBy Severity:")
            for severity in ['CRITICAL', 'IMPORTANT', 'RECOMMENDED', 'INFO']:
                if severity in severity_counts:
                    lines.append(f"  {severity}: {severity_counts[severity]}")

        if category_counts:
            lines.append("\nTop Categories:")
            for category, count in sorted(category_counts.items(),
                                         key=lambda x: x[1], reverse=True)[:5]:
                lines.append(f"  {category}: {count}")

        if confidence_distribution:
            lines.append("\nConfidence Distribution:")
            for level, count in sorted(confidence_distribution.items()):
                lines.append(f"  {level}: {count}")

        # Metrics summary
        total_functions = sum(a.metrics.get('total_functions', 0) for a in analyses)
        total_complexity = sum(
            a.metrics.get('avg_complexity', 0) * a.metrics.get('total_functions', 0)
            for a in analyses
        )
        avg_complexity = total_complexity / total_functions if total_functions > 0 else 0

        lines.append("\nCode Metrics:")
        lines.append(f"  Total functions: {total_functions}")
        lines.append(f"  Average complexity: {avg_complexity:.1f}")

        public_api_count = sum(a.metrics.get('public_api_functions', 0) for a in analyses)
        static_count = sum(a.metrics.get('static_functions', 0) for a in analyses)

        lines.append(f"  Public API functions: {public_api_count}")
        lines.append(f"  Static functions: {static_count}")

        # Cache statistics
        if self.cache:
            stats = self.cache.get_stats()
            lines.append(f"\nCache Performance:")
            lines.append(f"  Hit rate: {stats['hit_rate']:.1f}%")
            lines.append(f"  Memory hits: {stats['memory_hits']}")
            lines.append(f"  Disk hits: {stats['disk_hits']}")
            lines.append(f"  Misses: {stats['misses']}")

        # Detailed violations
        if analyses:
            lines.append("\n" + "-" * 80)
            lines.append("Violations by file:")
            lines.append("-" * 80)

            for analysis in sorted(analyses, key=lambda a: len(a.violations), reverse=True):
                if analysis.violations:
                    unsuppressed = [v for v in analysis.violations if not v.suppressed]
                    if unsuppressed:
                        lines.append(f"\n{analysis.filepath}: {len(unsuppressed)} violations")

                        # Group by severity
                        by_severity = defaultdict(list)
                        for v in unsuppressed:
                            by_severity[v.severity].append(v)

                        for severity in [Severity.CRITICAL, Severity.IMPORTANT,
                                       Severity.RECOMMENDED, Severity.INFO]:
                            if severity in by_severity:
                                lines.append(f"  {severity.name}:")
                                for v in by_severity[severity][:5]:
                                    conf_str = f"[{v.confidence:.0%}]" if v.confidence < 0.90 else ""
                                    lines.append(
                                        f"    - Line {v.location.line}: {v.message} {conf_str}"
                                    )
                                if len(by_severity[severity]) > 5:
                                    lines.append(
                                        f"    ... and {len(by_severity[severity]) - 5} more"
                                    )

        # Performance summary
        total_time = sum(a.analysis_time for a in analyses)
        lines.append("\n" + "-" * 80)
        lines.append("Performance:")
        lines.append("-" * 80)
        lines.append(f"Total analysis time: {total_time:.2f}s")
        if total_files > 0:
            lines.append(f"Average time per file: {total_time/total_files:.3f}s")

        # Recommendations
        lines.append("\n" + "-" * 80)
        lines.append("Recommendations:")
        lines.append("-" * 80)

        if severity_counts.get('CRITICAL', 0) > 0:
            lines.append("• Address CRITICAL violations immediately")

        high_complexity_files = [
            a.filepath for a in analyses
            if a.metrics.get('avg_complexity', 0) > 15
        ]
        if high_complexity_files:
            lines.append(f"• Refactor high-complexity files: {', '.join(high_complexity_files[:3])}")

        low_confidence_count = confidence_distribution.get('Low (<70%)', 0)
        if low_confidence_count > 10:
            lines.append(f"• Review {low_confidence_count} low-confidence violations for false positives")

        return '\n'.join(lines)

    def _generate_json_report(self, analyses: List[FileAnalysis]) -> str:
        """Generate JSON report"""
        report = {
            'timestamp': datetime.now().isoformat(),
            'summary': {
                'total_files': len(analyses),
                'total_violations': sum(len(a.violations) for a in analyses),
                'confidence_threshold': self.rule_engine.confidence_threshold
            },
            'files': []
        }

        for analysis in analyses:
            file_report = {
                'filepath': analysis.filepath,
                'violations': [
                    {
                        'rule_id': v.rule_id,
                        'severity': v.severity.name,
                        'line': v.location.line,
                        'message': v.message,
                        'category': v.category,
                        'fix_suggestion': v.fix_suggestion,
                        'confidence': v.confidence,
                        'suppressed': v.suppressed
                    }
                    for v in analysis.violations
                ],
                'metrics': analysis.metrics,
                'analysis_time': analysis.analysis_time
            }
            report['files'].append(file_report)

        return json.dumps(report, indent=2)

    def _generate_html_report(self, analyses: List[FileAnalysis]) -> str:
        """Generate HTML report"""
        html = ['<!DOCTYPE html><html><head><title>HeliOS Source Code Compliance Checker Report</title>']
        html.append('<meta charset="UTF-8">')
        html.append('<style>')
        html.append('body { font-family: "Segoe UI", Arial, sans-serif; margin: 20px; background: #f5f5f5; }')
        html.append('.container { max-width: 1200px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }')
        html.append('h1 { color: #2c3e50; border-bottom: 3px solid #3498db; padding-bottom: 10px; }')
        html.append('h2 { color: #34495e; margin-top: 30px; }')
        html.append('table { border-collapse: collapse; width: 100%; margin-top: 20px; }')
        html.append('th, td { border: 1px solid #ddd; padding: 12px; text-align: left; }')
        html.append('th { background-color: #3498db; color: white; font-weight: 600; }')
        html.append('tr:nth-child(even) { background-color: #f9f9f9; }')
        html.append('tr:hover { background-color: #f5f5f5; }')
        html.append('.critical { color: #e74c3c; font-weight: bold; }')
        html.append('.important { color: #e67e22; font-weight: 600; }')
        html.append('.recommended { color: #27ae60; }')
        html.append('.info { color: #3498db; }')
        html.append('.confidence { color: #7f8c8d; font-size: 0.9em; }')
        html.append('.stats { display: flex; justify-content: space-around; margin: 20px 0; }')
        html.append('.stat-box { background: #ecf0f1; padding: 15px; border-radius: 5px; text-align: center; flex: 1; margin: 0 10px; }')
        html.append('.stat-value { font-size: 2em; font-weight: bold; color: #2c3e50; }')
        html.append('.stat-label { color: #7f8c8d; margin-top: 5px; }')
        html.append('</style></head><body>')
        html.append('<div class="container">')

        html.append('<h1>🔍 HeliOS Source Code Compliance Checker Report</h1>')
        html.append(f'<p>Generated: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}</p>')

        # Statistics boxes
        total_violations = sum(len(a.violations) for a in analyses)
        critical_count = sum(1 for a in analyses for v in a.violations
                           if v.severity == Severity.CRITICAL and not v.suppressed)

        html.append('<div class="stats">')
        html.append(f'<div class="stat-box"><div class="stat-value">{len(analyses)}</div><div class="stat-label">Files Analyzed</div></div>')
        html.append(f'<div class="stat-box"><div class="stat-value">{total_violations}</div><div class="stat-label">Total Violations</div></div>')
        html.append(f'<div class="stat-box"><div class="stat-value">{critical_count}</div><div class="stat-label">Critical Issues</div></div>')

        avg_confidence = 0
        if total_violations > 0:
            avg_confidence = sum(v.confidence for a in analyses for v in a.violations) / total_violations
            html.append(f'<div class="stat-box"><div class="stat-value">{avg_confidence:.0%}</div><div class="stat-label">Avg Confidence</div></div>')

        html.append('</div>')

        # Violations table
        html.append('<h2>📋 Violations</h2>')
        html.append('<table>')
        html.append('<tr><th>File</th><th>Line</th><th>Severity</th><th>Rule</th><th>Message</th><th>Confidence</th></tr>')

        for analysis in analyses:
            for v in analysis.violations:
                if not v.suppressed:
                    severity_class = v.severity.name.lower()
                    html.append(f'<tr>')
                    html.append(f'<td>{os.path.basename(analysis.filepath)}</td>')
                    html.append(f'<td>{v.location.line}</td>')
                    html.append(f'<td class="{severity_class}">{v.severity.name}</td>')
                    html.append(f'<td>{v.rule_id}</td>')
                    html.append(f'<td>{v.message}</td>')
                    html.append(f'<td class="confidence">{v.confidence:.0%}</td>')
                    html.append(f'</tr>')

        html.append('</table>')
        html.append('</div>')
        html.append('</body></html>')

        return '\n'.join(html)

# ============================================================================
# Utility Functions
# ============================================================================

def find_source_files(paths: List[str], extensions: List[str]) -> List[str]:
    """Find all source files in given paths"""
    source_files = []

    for path in paths:
        path = Path(path)
        if path.is_file():
            if any(str(path).endswith(ext) for ext in extensions):
                source_files.append(str(path))
        elif path.is_dir():
            for ext in extensions:
                source_files.extend(str(f) for f in path.rglob(f'*{ext}'))

    return sorted(set(source_files))

# ============================================================================
# Cache Manager (from original)
# ============================================================================

class CacheManager:
    """Multi-level caching system"""

    def __init__(self, cache_dir: str = ".helios_cache"):
        self.cache_dir = Path(cache_dir)
        self.cache_dir.mkdir(exist_ok=True)
        self.memory_cache = {}
        self.stats = defaultdict(int)

    def get_file_hash(self, filepath: str) -> str:
        """Get hash of file content"""
        try:
            with open(filepath, 'rb') as f:
                return hashlib.md5(f.read()).hexdigest()
        except:
            return ""

    def get(self, filepath: str, rules_hash: str) -> Optional[FileAnalysis]:
        """Get cached analysis"""
        cache_key = f"{filepath}:{rules_hash}"

        # Check memory cache
        if cache_key in self.memory_cache:
            self.stats['memory_hits'] += 1
            return self.memory_cache[cache_key]

        # Check disk cache
        file_hash = self.get_file_hash(filepath)
        cache_file = self.cache_dir / f"{hashlib.md5(cache_key.encode()).hexdigest()}.pkl"

        if cache_file.exists():
            try:
                with open(cache_file, 'rb') as f:
                    cached_data = pickle.load(f)
                    if cached_data.get('file_hash') == file_hash:
                        self.stats['disk_hits'] += 1
                        analysis = cached_data['analysis']
                        self.memory_cache[cache_key] = analysis
                        return analysis
            except:
                pass

        self.stats['misses'] += 1
        return None

    def set(self, filepath: str, rules_hash: str, analysis: FileAnalysis):
        """Store analysis in cache"""
        cache_key = f"{filepath}:{rules_hash}"
        file_hash = self.get_file_hash(filepath)

        # Store in memory cache
        self.memory_cache[cache_key] = analysis

        # Store in disk cache
        cache_file = self.cache_dir / f"{hashlib.md5(cache_key.encode()).hexdigest()}.pkl"
        try:
            with open(cache_file, 'wb') as f:
                pickle.dump({
                    'file_hash': file_hash,
                    'analysis': analysis,
                    'timestamp': time.time()
                }, f)
        except:
            pass

    def get_stats(self) -> Dict:
        """Get cache statistics"""
        total = sum(self.stats.values())
        hit_rate = 0
        if total > 0:
            hits = self.stats['memory_hits'] + self.stats['disk_hits']
            hit_rate = (hits / total) * 100

        return {
            'memory_hits': self.stats['memory_hits'],
            'disk_hits': self.stats['disk_hits'],
            'misses': self.stats['misses'],
            'hit_rate': hit_rate
        }

# ============================================================================
# Main Entry Point
# ============================================================================

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description='HeliOS Source Code Compliance Checker',
        formatter_class=argparse.RawDescriptionHelpFormatter
    )

    parser.add_argument(
        'paths', nargs='*', default=['../../src/'],
        help='Files or directories to check'
    )

    parser.add_argument(
        '-r', '--rules', default='helios.yaml',
        help='Path to rules file'
    )

    parser.add_argument(
        '-o', '--output', choices=['text', 'json', 'html'],
        default='text', help='Output format'
    )

    parser.add_argument(
        '-f', '--output-file',
        help='Output file (default: stdout)'
    )

    parser.add_argument(
        '--confidence-threshold', type=float, default=0.70,
        help='Minimum confidence threshold for violations (0.0-1.0)'
    )

    parser.add_argument(
        '--no-cache', action='store_true',
        help='Disable caching'
    )

    parser.add_argument(
        '--no-parallel', action='store_true',
        help='Disable parallel processing'
    )

    parser.add_argument(
        '-v', '--verbose', action='store_true',
        help='Verbose output'
    )

    parser.add_argument(
        '--extensions', nargs='+', default=['.c', '.h'],
        help='File extensions to check'
    )

    parser.add_argument(
        '--show-suppressed', action='store_true',
        help='Show suppressed violations in report'
    )

    args = parser.parse_args()

    # Configure logging
    if args.verbose:
        logging.basicConfig(level=logging.INFO)
    else:
        logging.basicConfig(level=logging.WARNING)

    # Initialize checker
    checker = ComplianceChecker(
        rules_file=args.rules,
        cache_enabled=not args.no_cache,
        parallel=not args.no_parallel,
        confidence_threshold=args.confidence_threshold
    )

    # Find source files
    source_files = find_source_files(args.paths, args.extensions)

    if not source_files:
        print("No source files found")
        return 1

    print(f"Checking {len(source_files)} files...")

    # Check files
    analyses = checker.check_files(source_files)

    # Generate report
    report = checker.generate_report(analyses, args.output)

    # Output report
    if args.output_file:
        with open(args.output_file, 'w') as f:
            f.write(report)
        print(f"Report saved to {args.output_file}")
    else:
        print(report)

    # Return exit code based on critical violations
    total_critical = sum(
        1 for a in analyses
        for v in a.violations
        if v.severity == Severity.CRITICAL and not v.suppressed
    )

    return 1 if total_critical > 0 else 0

if __name__ == '__main__':
    sys.exit(main())