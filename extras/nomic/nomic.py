#!/usr/bin/env python3
"""
@file nomic.py
@author Manny Peterson
@brief Nomic C code semantic analsysis tool.
@version 0.5.0
@date 2025-10-25

@copyright
Nomic C Code Analysis Copyright (C) 2025-2026 Manny Peterson

    SPDX-License-Identifier: GPL-2.0-or-later

"""

from __future__ import annotations
from dataclasses import dataclass, field, asdict
from typing import List, Dict, Optional, Tuple, Union, Literal, Any, Set, Iterable, Callable, Pattern
import argparse
import ast
from collections import deque
from contextlib import contextmanager
import functools
import json
import operator
import os
import re
import shlex
import sys
import textwrap
try:  # Optional dependency; tool still runs without PyYAML.
    import yaml  # type: ignore
except ImportError:  # pragma: no cover - environment without PyYAML
    yaml = None  # type: ignore
try:  # Optional libclang integration.
    from clang import cindex as clang_cindex  # type: ignore
except ImportError:  # pragma: no cover - environment without libclang
    clang_cindex = None  # type: ignore


NOMIC_PRODUCT_NAME = "Nomic"
NOMIC_VERSION = "0.5.0"
NOMIC_AUTHOR = "Manny Peterson"
NOMIC_COPYRIGHT = "Nomic C Code Analysis Copyright (C) 2025-2026 Manny Peterson"
NOMIC_LICENSE = "GPL-2.0-or-later"

_HELP_HEADER = textwrap.dedent(
    f"""\
{NOMIC_PRODUCT_NAME}: Semantic rule enforcement for C

Version : {NOMIC_VERSION}
Author  : {NOMIC_AUTHOR}
License : {NOMIC_LICENSE}
{NOMIC_COPYRIGHT}
"""
).rstrip()


if clang_cindex is not None:
    _STATEMENT_CURSOR_KINDS = {
        clang_cindex.CursorKind.RETURN_STMT,
        clang_cindex.CursorKind.BINARY_OPERATOR,
        clang_cindex.CursorKind.COMPOUND_ASSIGNMENT_OPERATOR,
        clang_cindex.CursorKind.DECL_STMT,
        clang_cindex.CursorKind.UNEXPOSED_EXPR,
        clang_cindex.CursorKind.CALL_EXPR,
    }
else:  # pragma: no cover - stub mode
    _STATEMENT_CURSOR_KINDS: Set[Any] = set()


# ============================================================
# =============== SOURCE LOCATION & CONTEXT ==================
# ============================================================

@dataclass
class SourceLocation:
    file: str
    line: int
    column: int

    @property
    def filename(self) -> str:
        return os.path.basename(self.file)


@dataclass
class SourceRange:
    file: str
    line_start: int
    col_start: int
    line_end: int
    col_end: int

    @property
    def start_line(self) -> int:
        return self.line_start

    @property
    def end_line(self) -> int:
        return self.line_end


@dataclass
class PreprocessorContext:
    """
    Records what #defines / conditional compilation context was active
    for a given node.
    """
    active_defines: Set[str] = field(default_factory=set)
    guard_condition: Optional[str] = None  # e.g. "HW_TIMER1 && DEBUG"


@dataclass
class Annotation:
    """
    Inline annotations / pragmas / special comments relevant to rules.
    Example: // @ALLOW_BLOCKING_IN_ISR
    """
    text: str
    location: SourceLocation


# ============================================================
# ==================== TYPE DECLARATIONS =====================
# ============================================================

@dataclass
class FieldDecl:
    """Represents a struct/union/enum field including bitfield metadata."""
    name: str
    ctype: str  # resolved type string
    is_bitfield: bool = False
    bit_width: Optional[int] = None
    source_range: Optional[SourceRange] = None


@dataclass
class TypeDecl:
    """
    Captures typedef / struct / union / enum declarations emitted by clang,
    including attributes, fields, and source metadata.
    """
    name: str
    kind: Literal["typedef", "struct", "union", "enum"]
    fields: List[FieldDecl] = field(default_factory=list)  # for struct/union/enum
    underlying_type: Optional[str] = None  # e.g. "unsigned long *" for typedef
    attributes: List[str] = field(default_factory=list)    # e.g. ['packed', 'aligned(4)']
    visibility: Literal["file_local", "project_visible"] = "project_visible"
    source_range: Optional[SourceRange] = None
    preprocessor: PreprocessorContext = field(default_factory=PreprocessorContext)
    annotations: List[Annotation] = field(default_factory=list)


# ============================================================
# ===================== SYMBOL USAGE =========================
# ============================================================

@dataclass
class WriteSite:
    """Records a write to a variable, capturing structural context (branch/loop/macro)."""
    location: SourceRange
    in_branch: Optional[str] = None     # "then", "else", "case", etc.
    in_loop: Optional[str] = None       # "for", "while", "do_while"
    in_macro_expansion: bool = False
    guarded_condition: Optional[str] = None  # "if (err != 0)"
    preprocessor: PreprocessorContext = field(default_factory=PreprocessorContext)


@dataclass
class ReadSite:
    """Records a read from a variable with the same structural metadata as WriteSite."""
    location: SourceRange
    in_branch: Optional[str] = None
    in_loop: Optional[str] = None
    in_macro_expansion: bool = False
    guarded_condition: Optional[str] = None
    preprocessor: PreprocessorContext = field(default_factory=PreprocessorContext)


# ============================================================
# ======================= VARIABLES ==========================
# ============================================================

@dataclass
class Variable:
    """
    Represents globals, statics, locals, params, etc., including type
    qualifiers, storage, annotations, and tracked read/write sites.
    """
    name: str
    ctype: str  # fully resolved C type
    storage: Literal["auto", "static", "extern", "register", "typedef_name", "unknown"] = "unknown"
    linkage: Literal["internal", "external", "none"] = "none"  # internal=static, external=linker-visible
    is_const: bool = False
    is_volatile: bool = False
    is_atomic: bool = False

    scope: Literal["file", "function", "block", "param"] = "file"
    decl_function: Optional[str] = None  # function name if local or param

    # Naming helpers
    prefix: Optional[str] = None
    suffix: Optional[str] = None

    source_range: Optional[SourceRange] = None
    preprocessor: PreprocessorContext = field(default_factory=PreprocessorContext)
    annotations: List[Annotation] = field(default_factory=list)

    writes: List[WriteSite] = field(default_factory=list)
    reads: List[ReadSite] = field(default_factory=list)
    written_by: Set[str] = field(default_factory=set)
    read_by: Set[str] = field(default_factory=set)
    is_initialized: bool = False
    original_type: Optional[str] = None
    has_mutex_protection: bool = False

    @property
    def type_name(self) -> str:
        return self.ctype

    @property
    def base_type(self) -> str:
        text = self.ctype or ""
        text = text.replace("const", "").replace("volatile", "")
        return text.replace("*", "").strip()

    @property
    def is_global(self) -> bool:
        return self.scope == "file" and self.storage != "static"

    @property
    def is_static(self) -> bool:
        return self.storage == "static"

    @property
    def is_extern(self) -> bool:
        return self.storage == "extern"

    @property
    def is_parameter(self) -> bool:
        return self.scope == "param"

    @property
    def is_local(self) -> bool:
        return self.scope in ("function", "block")

    @property
    def parent_function(self) -> Optional[str]:
        return self.decl_function

    @property
    def is_modified(self) -> bool:
        return bool(self.writes)

    @property
    def source_location(self) -> Optional[SourceLocation]:
        if self.source_range:
            return SourceLocation(
                file=self.source_range.file,
                line=self.source_range.line_start,
                column=self.source_range.col_start,
            )
        return None

    @property
    def type_equivalent(self) -> str:
        return get_type_equivalent(self.ctype)

    @property
    def is_custom_type(self) -> bool:
        return is_custom_type(self.type_name)


TYPE_MAP: Dict[str, str] = {
    "u8": "uint8_t",
    "u16": "uint16_t",
    "u32": "uint32_t",
    "u64": "uint64_t",
}

CUSTOM_TYPE_SUFFIXES: Tuple[str, ...] = ("_t",)
CUSTOM_TYPE_REGISTRY: Set[str] = set()


def register_custom_types(types: Iterable[str]) -> None:
    CUSTOM_TYPE_REGISTRY.update(t for t in types if t)


def is_custom_type(type_name: Optional[str]) -> bool:
    if not type_name:
        return False
    lowered = type_name.strip()
    if lowered in CUSTOM_TYPE_REGISTRY:
        return True
    if lowered in TYPE_MAP.values() or lowered in TYPE_MAP.keys():
        return True
    return any(lowered.endswith(suffix) for suffix in CUSTOM_TYPE_SUFFIXES)


def get_type_equivalent(c_type: Optional[str]) -> str:
    if not c_type:
        return ""
    key = c_type.strip()
    return TYPE_MAP.get(key, key)


def check_type_compliance(var: Variable) -> bool:
    return is_custom_type(var.type_name)


# ============================================================
# ===================== CALL SITES ===========================
# ============================================================

@dataclass
class CallSite:
    """Describes an individual function call expression discovered by clang."""
    callee_name: str                           # text in code
    callee_symbol: Optional[str] = None        # resolved canonical name, if known

    args: List[str] = field(default_factory=list)  # simplified arg reprs
    location: Optional[SourceRange] = None

    in_branch: Optional[str] = None            # "then", "else", "case"
    in_loop: Optional[str] = None              # "for", "while", "do_while"
    in_switch: bool = False
    in_macro_expansion: bool = False

    # Semantic classification for rule logic
    is_blocking_api: bool = False
    is_allocator: bool = False
    is_lock: bool = False
    is_unlock: bool = False

    preprocessor: PreprocessorContext = field(default_factory=PreprocessorContext)


# ============================================================
# ================= CONTROL FLOW GRAPH =======================
# ============================================================

_IDENTIFIER_RE = re.compile(r"\b[_A-Za-z][_A-Za-z0-9]*\b")


def _extract_identifiers(text: str) -> List[str]:
    return _IDENTIFIER_RE.findall(text or "")


@dataclass
class Statement:
    """Rich statement wrapper used inside CFG blocks and structured statements."""
    text: str
    contains_call: bool = False
    contains_return: bool = False
    contains_assignment: bool = False
    contains_macro: bool = False
    macro_names: List[str] = field(default_factory=list)
    variables_read: List[str] = field(default_factory=list)
    variables_written: List[str] = field(default_factory=list)

    def __post_init__(self) -> None:
        normalized = (self.text or "").strip()
        lowered = normalized.lower()
        assignment_tokens = ["=", "+=", "-=", "*=", "/=", "%=", "|=", "&=", "^=", ">>=", "<<="]
        self.contains_assignment = any(token in normalized for token in assignment_tokens)
        self.contains_return = "return" in lowered
        self.contains_call = "(" in normalized and ")" in normalized and not normalized.startswith("{")
        macro_candidates = [
            token for token in _extract_identifiers(normalized) if token.isupper()
        ]
        self.macro_names = macro_candidates
        self.contains_macro = bool(macro_candidates)

        if self.contains_assignment and "=" in normalized:
            lhs, _, rhs = normalized.partition("=")
            self.variables_written = _extract_identifiers(lhs)
            self.variables_read = _extract_identifiers(rhs)
        else:
            self.variables_read = _extract_identifiers(normalized)
            self.variables_written = []


def _statement_from_text(text: str) -> "Statement":
    return Statement(text=text or "")


def _coerce_pattern_text(value: Any) -> str:
    if value is None:
        return ""
    if isinstance(value, Statement):
        return value.text
    if isinstance(value, str):
        return value
    if isinstance(value, Function):
        return "\n".join(stmt.text for stmt in value.all_statements)
    if isinstance(value, BlockStmt):
        return "\n".join(stmt.text for stmt in value.statements)
    if isinstance(value, BasicBlock):
        return "\n".join(stmt.text for stmt in value.statements)
    if isinstance(value, (list, tuple, set)):
        fragments = [_coerce_pattern_text(item) for item in value]
        return "\n".join(fragment for fragment in fragments if fragment)
    text_attr = getattr(value, "text", None)
    if isinstance(text_attr, str):
        return text_attr
    return str(value)


@dataclass
class PatternMatch:
    """Represents a single regex match result with helpful context."""
    pattern: str
    match: str
    groups: Tuple[str, ...]
    groupdict: Dict[str, str]
    span: Tuple[int, int]
    line_span: Tuple[int, int]
    context: Any


class _PatternEngine:
    """Caches compiled regex patterns for reuse within the DSL."""

    def __init__(self) -> None:
        self._cache: Dict[Tuple[str, int], Pattern[str]] = {}

    def compile(self, pattern: str, flags: int) -> Pattern[str]:
        key = (pattern, flags)
        compiled = self._cache.get(key)
        if compiled is None:
            compiled = re.compile(pattern, flags)
            self._cache[key] = compiled
        return compiled


_PATTERN_ENGINE = _PatternEngine()


def _pattern_flags(
    *,
    ignore_case: bool,
    multiline: bool,
    dotall: bool,
    extra_flags: Optional[int] = None,
) -> int:
    flags = extra_flags or 0
    if ignore_case:
        flags |= re.IGNORECASE
    if multiline:
        flags |= re.MULTILINE
    if dotall:
        flags |= re.DOTALL
    return flags


def pattern_search(
    target: Any,
    pattern: str,
    *,
    ignore_case: bool = False,
    multiline: bool = True,
    dotall: bool = True,
    literal: bool = False,
    max_matches: int = 256,
    flags: Optional[int] = None,
) -> List[PatternMatch]:
    """Return PatternMatch objects for regex hits over aggregated text."""
    text = _coerce_pattern_text(target)
    if not text or not pattern:
        return []
    effective_pattern = re.escape(pattern) if literal else pattern
    compiled = _PATTERN_ENGINE.compile(
        effective_pattern,
        _pattern_flags(ignore_case=ignore_case, multiline=multiline, dotall=dotall, extra_flags=flags),
    )
    matches: List[PatternMatch] = []
    limit = max_matches if max_matches > 0 else 256
    for match in compiled.finditer(text):
        start, end = match.span()
        line_start = text.count("\n", 0, start) + 1
        line_end = text.count("\n", 0, end) + 1
        matches.append(
            PatternMatch(
                pattern=pattern,
                match=match.group(0),
                groups=match.groups(),
                groupdict=match.groupdict(),
                span=(start, end),
                line_span=(line_start, line_end),
                context=target,
            )
        )
        if len(matches) >= limit:
            break
    return matches


def pattern_matches(target: Any, pattern: str, **kwargs: Any) -> bool:
    """Return True if the pattern matches anywhere in the target text."""
    return bool(pattern_search(target, pattern, max_matches=1, **kwargs))


def pattern_findall(target: Any, pattern: str, **kwargs: Any) -> List[str]:
    """Return just the matched strings for convenience."""
    return [match.match for match in pattern_search(target, pattern, **kwargs)]


def pattern_extract(
    target: Any,
    pattern: str,
    group: Union[int, str, None] = None,
    **kwargs: Any,
) -> List[Optional[str]]:
    """Extract a specific capture group (by index or name) from each match."""
    results: List[Optional[str]] = []
    for match in pattern_search(target, pattern, **kwargs):
        if group is None or group == 0:
            results.append(match.match)
        elif isinstance(group, int):
            idx = group - 1
            results.append(match.groups[idx] if 0 <= idx < len(match.groups) else None)
        else:
            results.append(match.groupdict.get(group))
    return results


@dataclass
class SemanticMatch:
    """Describes a statement whose semantic predicate and optional pattern matched."""
    statement: Statement
    block: Optional["BasicBlock"]
    function: Optional["Function"]
    pattern_match: Optional[PatternMatch]


def _iter_statements_with_context(target: Any) -> Iterable[Tuple[Statement, Optional["BasicBlock"], Optional["Function"]]]:
    if target is None:
        return []
    if isinstance(target, Function):
        for block in target.cfg.blocks.values():
            for stmt in block.statements:
                yield stmt, block, target
        return
    if isinstance(target, BasicBlock):
        function = target.function
        for stmt in target.statements:
            yield stmt, target, function
        return
    if isinstance(target, BlockStmt):
        for stmt in target.statements:
            yield stmt, None, None
        return
    if isinstance(target, Statement):
        yield target, None, None
        return
    if isinstance(target, Iterable) and not isinstance(target, (str, bytes)):
        for item in target:
            yield from _iter_statements_with_context(item)
        return
    text = _coerce_pattern_text(target)
    if text:
        stmt = _statement_from_text(text)
        yield stmt, None, None


def _call_predicate(predicate: Optional[Callable[..., bool]], statement: Statement, block: Optional["BasicBlock"], function: Optional["Function"]) -> bool:
    if predicate is None:
        return True
    try:
        return bool(predicate(statement, block, function))
    except TypeError:
        return bool(predicate(statement))


def _pattern_scope_text(statement: Statement, block: Optional["BasicBlock"], function: Optional["Function"], scope: str) -> str:
    scope = (scope or "statement").lower()
    if scope == "block" and block is not None:
        return "\n".join(stmt.text for stmt in block.statements)
    if scope == "function" and function is not None:
        return "\n".join(stmt.text for stmt in function.all_statements)
    return statement.text


def semantic_pattern_matches(
    target: Any,
    *,
    predicate: Optional[Callable[..., bool]] = None,
    pattern: Optional[str] = None,
    pattern_scope: str = "statement",
    max_results: int = 256,
    pattern_kwargs: Optional[Dict[str, Any]] = None,
) -> List[SemanticMatch]:
    """
    Combine semantic predicates with (optional) regex pattern matching.
    Returns SemanticMatch objects with surrounding context.
    """
    results: List[SemanticMatch] = []
    pattern_kwargs = pattern_kwargs or {}
    limit = max_results if max_results > 0 else 256
    for statement, block, function in _iter_statements_with_context(target) or []:
        if not _call_predicate(predicate, statement, block, function):
            continue
        if pattern:
            text = _pattern_scope_text(statement, block, function, pattern_scope)
            matches = pattern_search(text, pattern, **pattern_kwargs)
            if not matches:
                continue
            for pm in matches:
                results.append(SemanticMatch(statement=statement, block=block, function=function, pattern_match=pm))
                if len(results) >= limit:
                    return results
        else:
            results.append(SemanticMatch(statement=statement, block=block, function=function, pattern_match=None))
            if len(results) >= limit:
                return results
    return results


def semantic_pattern_any(target: Any, **kwargs: Any) -> bool:
    """Boolean helper to quickly test if any semantic+pattern match exists."""
    return bool(semantic_pattern_matches(target, max_results=1, **kwargs))


def semantic_statements(target: Any, predicate: Optional[Callable[..., bool]] = None) -> List[Statement]:
    """Return statements satisfying the provided semantic predicate."""
    statements: List[Statement] = []
    for statement, block, function in _iter_statements_with_context(target) or []:
        if _call_predicate(predicate, statement, block, function):
            statements.append(statement)
    return statements


def _gather_paths_from_target(
    target: Any,
    *,
    max_paths: int = 256,
) -> List[List[BasicBlock]]:
    if isinstance(target, Function):
        return target.cfg.get_paths_to_exit(max_paths=max_paths)
    if isinstance(target, ControlFlowGraph):
        return target.get_paths_to_exit(max_paths=max_paths)
    if isinstance(target, BasicBlock):
        function = target.function
        if function is None:
            return [[target]]
        cfg = function.cfg
        paths: List[List[BasicBlock]] = []
        stack: List[Tuple[int, List[int]]] = [(target.block_id, [target.block_id])]
        while stack and len(paths) < max_paths:
            block_id, path = stack.pop()
            block = cfg.blocks.get(block_id)
            if block is None:
                continue
            if block.is_exit_block or not block.successors:
                paths.append([cfg.blocks[b] for b in path if b in cfg.blocks])
                continue
            for succ in block.successors:
                if succ in path:
                    continue
                stack.append((succ, path + [succ]))
        return paths
    return []


def path_matches(
    target: Any,
    pattern: str,
    *,
    ignore_case: bool = False,
    multiline: bool = True,
    dotall: bool = True,
    max_paths: int = 256,
) -> bool:
    """Returns True if any control-flow path's textual representation matches the regex."""
    paths = _gather_paths_from_target(target, max_paths=max_paths)
    if not paths:
        return False
    for path in paths:
        text = "\n".join(stmt.text for block in path for stmt in block.statements if stmt.text)
        if pattern_matches(text, pattern, ignore_case=ignore_case, multiline=multiline, dotall=dotall):
            return True
    return False


def path_requires(
    target: Any,
    *,
    must_match: Optional[str] = None,
    forbid_match: Optional[str] = None,
    max_paths: int = 256,
    pattern_kwargs: Optional[Dict[str, Any]] = None,
) -> bool:
    """
    Ensures that every path satisfies `must_match` (if provided) and that no path matches `forbid_match`.
    """
    pattern_kwargs = pattern_kwargs or {}
    paths = _gather_paths_from_target(target, max_paths=max_paths)
    if not paths:
        return False
    for path in paths:
        text = "\n".join(stmt.text for block in path for stmt in block.statements if stmt.text)
        if must_match and not pattern_matches(text, must_match, **pattern_kwargs):
            return False
        if forbid_match and pattern_matches(text, forbid_match, **pattern_kwargs):
            return False
    return True


@dataclass
class BasicBlock:
    """CFG node describing linearized statements plus intra-block calls/reads/writes."""
    block_id: int

    statements: List[Statement] = field(default_factory=list)
    calls: List[CallSite] = field(default_factory=list)
    writes: List[WriteSite] = field(default_factory=list)
    reads: List[ReadSite] = field(default_factory=list)

    enclosing_construct: Optional[str] = None  # "if_then", "if_else", "loop", "switch_case", etc.
    branch_condition: Optional[str] = None     # textual condition for this arm

    successors: List[int] = field(default_factory=list)
    predecessors: List[int] = field(default_factory=list)
    dominators: Set[int] = field(default_factory=set)
    postdominators: Set[int] = field(default_factory=set)

    is_exit_block: bool = False
    loop_depth: int = 0
    parent_loop: Optional["LoopStmt"] = None
    is_loop_header: bool = False
    is_loop_exit: bool = False

    source_range: Optional[SourceRange] = None
    preprocessor: PreprocessorContext = field(default_factory=PreprocessorContext)
    function: Optional["Function"] = field(default=None, repr=False)

    @property
    def first_statement(self) -> Optional[Statement]:
        return self.statements[0] if self.statements else None

    @property
    def last_statement(self) -> Optional[Statement]:
        return self.statements[-1] if self.statements else None

    @property
    def call_sites(self) -> List[CallSite]:
        return list(self.calls)


@dataclass
class ControlFlowGraph:
    """Owns the per-function CFG including blocks, entry, exits, and analyses."""
    blocks: Dict[int, BasicBlock] = field(default_factory=dict)
    _entry_block_id: Optional[int] = None
    _exit_block_ids: List[int] = field(default_factory=list)

    @property
    def entry_block_id(self) -> Optional[int]:
        return self._entry_block_id

    @property
    def entry_block(self) -> Optional[BasicBlock]:
        if self._entry_block_id is None:
            return None
        return self.blocks.get(self._entry_block_id)

    @entry_block.setter
    def entry_block(self, value: Optional[Union[int, BasicBlock]]) -> None:
        if isinstance(value, BasicBlock):
            self._entry_block_id = value.block_id
        else:
            self._entry_block_id = value

    @property
    def exit_block_ids(self) -> List[int]:
        return list(self._exit_block_ids)

    @property
    def exit_blocks(self) -> List[BasicBlock]:
        return [self.blocks[bid] for bid in self._exit_block_ids if bid in self.blocks]

    @exit_blocks.setter
    def exit_blocks(self, values: List[Union[int, BasicBlock]]) -> None:
        ids: List[int] = []
        for value in values:
            if isinstance(value, BasicBlock):
                ids.append(value.block_id)
            else:
                ids.append(value)
        self._exit_block_ids = ids

    @property
    def exit_block(self) -> Optional[BasicBlock]:
        blocks = self.exit_blocks
        return blocks[0] if blocks else None

    def all_exit_paths_postdominated_by(self, matcher: Any) -> bool:
        """
        Semantic contract:
        Returns True if for every exit path from entry_block to any exit block,
        there exists a node satisfying `matcher` that postdominates the point
        of interest (e.g. an unlock after lock).

        The matcher receives BasicBlock objects. This implementation uses
        the post-dominator sets computed during CFG finalization.
        """
        if not self.blocks or self.entry_block is None:
            return False
        entry_block = self.entry_block
        if entry_block is None:
            return False
        entry_postdoms = entry_block.postdominators
        if not entry_postdoms:
            return False
        for block_id, block in self.blocks.items():
            if block_id in entry_postdoms and matcher(block):
                return True
        return False

    def has_path_without(self, matcher: Any) -> bool:
        """
        Return True if there exists a path from entry to any exit that never satisfies matcher.
        """
        entry = self.entry_block
        if entry is None:
            return False
        stack: List[Tuple[int, bool]] = [(entry.block_id, bool(matcher(entry)))]
        visited: Dict[int, bool] = {}
        while stack:
            block_id, seen_match = stack.pop()
            block = self.blocks.get(block_id)
            if block is None:
                continue
            if block.is_exit_block and not seen_match:
                return True
            for succ in block.successors:
                next_block = self.blocks.get(succ)
                if next_block is None:
                    continue
                next_seen = seen_match or bool(matcher(next_block))
                if visited.get(succ) is True and next_seen:
                    continue
                visited[succ] = next_seen
                stack.append((succ, next_seen))
        return False

    def get_paths_to_exit(self, max_paths: int = 1024) -> List[List[BasicBlock]]:
        """
        Enumerate paths from entry to exits up to `max_paths` results.
        """
        entry = self.entry_block
        if entry is None:
            return []
        paths: List[List[BasicBlock]] = []
        stack: List[Tuple[int, List[int]]] = [(entry.block_id, [entry.block_id])]
        while stack:
            block_id, path = stack.pop()
            block = self.blocks.get(block_id)
            if block is None:
                continue
            if block.is_exit_block or not block.successors:
                paths.append([self.blocks[b] for b in path if b in self.blocks])
                if len(paths) >= max_paths:
                    break
                continue
            for succ in block.successors:
                if succ in path:
                    continue  # avoid infinite loops
                stack.append((succ, path + [succ]))
        return paths


def _cfg_for_block(block: BasicBlock) -> Optional[ControlFlowGraph]:
    if block.function is None:
        return None
    return block.function.cfg


def all_paths_reach(start_block: BasicBlock, condition: Callable[[BasicBlock], bool]) -> bool:
    cfg = _cfg_for_block(start_block)
    if cfg is None:
        return False
    stack: List[Tuple[int, bool]] = [(start_block.block_id, bool(condition(start_block)))]
    visited: Set[Tuple[int, bool]] = set()
    while stack:
        block_id, seen = stack.pop()
        block = cfg.blocks.get(block_id)
        if block is None:
            continue
        if block.is_exit_block and not seen:
            return False
        for succ in block.successors:
            succ_block = cfg.blocks.get(succ)
            if succ_block is None:
                continue
            next_seen = seen or bool(condition(succ_block))
            key = (succ, next_seen)
            if key in visited:
                continue
            visited.add(key)
            stack.append((succ, next_seen))
    return True


def any_path_reaches(start_block: BasicBlock, condition: Callable[[BasicBlock], bool]) -> bool:
    cfg = _cfg_for_block(start_block)
    if cfg is None:
        return False
    stack: List[int] = [start_block.block_id]
    visited: Set[int] = set()
    while stack:
        block_id = stack.pop()
        block = cfg.blocks.get(block_id)
        if block is None or block_id in visited:
            continue
        visited.add(block_id)
        if condition(block):
            return True
        for succ in block.successors:
            stack.append(succ)
    return False


def paths_between(
    start_block: BasicBlock,
    target_block: BasicBlock,
    *,
    max_paths: int = 256,
) -> List[List[BasicBlock]]:
    if start_block.function is None or target_block.function is None:
        return []
    if start_block.function is not target_block.function:
        return []
    cfg = start_block.function.cfg
    paths: List[List[BasicBlock]] = []
    stack: List[Tuple[int, List[int]]] = [(start_block.block_id, [start_block.block_id])]
    while stack and len(paths) < max_paths:
        block_id, path = stack.pop()
        if block_id == target_block.block_id:
            paths.append([cfg.blocks[b] for b in path if b in cfg.blocks])
            continue
        block = cfg.blocks.get(block_id)
        if block is None:
            continue
        for succ in block.successors:
            if succ in path:
                continue
            stack.append((succ, path + [succ]))
    return paths


# ============================================================
# ================= FLOW CONSTRUCT NODES =====================
# ============================================================

@dataclass
class BlockStmt:
    """
    Generic block { ... } of code:
    - then-block of an if
    - else-block of an if
    - body of a loop
    - body of a switch case
    """
    statements: List[Statement] = field(default_factory=list)
    writes: List[WriteSite] = field(default_factory=list)
    reads: List[ReadSite] = field(default_factory=list)
    calls: List[CallSite] = field(default_factory=list)
    locals_declared_here: List[Variable] = field(default_factory=list)

    source_range: Optional[SourceRange] = None
    preprocessor: PreprocessorContext = field(default_factory=PreprocessorContext)


@dataclass
class IfStmt:
    condition: str
    then_block: BlockStmt
    else_block: Optional[BlockStmt] = None

    parent_function: Optional[str] = None  # function name
    source_range: Optional[SourceRange] = None
    preprocessor: PreprocessorContext = field(default_factory=PreprocessorContext)
    annotations: List[Annotation] = field(default_factory=list)


@dataclass
class LoopStmt:
    kind: Literal["for", "while", "do_while"]
    condition: Optional[str]  # do/while has postcondition
    body: BlockStmt

    parent_function: Optional[str] = None
    source_range: Optional[SourceRange] = None
    preprocessor: PreprocessorContext = field(default_factory=PreprocessorContext)
    annotations: List[Annotation] = field(default_factory=list)


@dataclass
class SwitchCaseBlock:
    """Encapsulates a single `case` or `default` arm inside a switch statement."""
    labels: List[str]                     # e.g. ["STATE_ERR", "3"] or ["default"]
    body: BlockStmt
    source_range: Optional[SourceRange] = None


@dataclass
class SwitchStmt:
    control_expr: str
    cases: List[SwitchCaseBlock] = field(default_factory=list)
    has_default: bool = False

    parent_function: Optional[str] = None
    source_range: Optional[SourceRange] = None
    preprocessor: PreprocessorContext = field(default_factory=PreprocessorContext)
    annotations: List[Annotation] = field(default_factory=list)


# ============================================================
# ======================== FUNCTION ==========================
# ============================================================

@dataclass
class Function:
    name: str
    return_type: str
    parameters: List[Variable] = field(default_factory=list)
    local_vars: List[Variable] = field(default_factory=list)

    linkage: Literal["internal", "external"] = "external"  # static => internal
    storage_class: Optional[str] = None  # "static", "extern", etc.

    attributes: List[str] = field(default_factory=list)  # compiler attrs like interrupt, naked
    annotations: List[Annotation] = field(default_factory=list)

    inline_hint: bool = False
    isr_hint: bool = False

    calls: List[CallSite] = field(default_factory=list)
    cfg: ControlFlowGraph = field(default_factory=ControlFlowGraph)
    exit_points: List[int] = field(default_factory=list)  # block_ids of exit blocks

    if_stmts: List[IfStmt] = field(default_factory=list)
    loops: List[LoopStmt] = field(default_factory=list)
    switches: List[SwitchStmt] = field(default_factory=list)

    globals_written: List[str] = field(default_factory=list)
    globals_read: List[str] = field(default_factory=list)

    called_by: List[str] = field(default_factory=list)
    dominates: List[str] = field(default_factory=list)

    source_range: Optional[SourceRange] = None
    preprocessor: PreprocessorContext = field(default_factory=PreprocessorContext)

    # For naming rules
    prefix: Optional[str] = None
    suffix: Optional[str] = None

    cyclomatic_complexity: int = 1
    cognitive_complexity: int = 0
    nesting_depth: int = 0

    @property
    def storage(self) -> Optional[str]:
        """Backwards-compatible alias for storage_class."""
        return self.storage_class

    @storage.setter
    def storage(self, value: Optional[str]) -> None:
        self.storage_class = value

    @property
    def is_static(self) -> bool:
        return self.storage_class == "static"

    @property
    def is_inline(self) -> bool:
        return self.storage_class == "inline" or self.inline_hint

    @property
    def is_extern(self) -> bool:
        if self.storage_class == "extern":
            return True
        return self.storage_class is None and self.linkage == "external" and not self.is_static

    @property
    def is_isr(self) -> bool:
        if self.isr_hint:
            return True
        name = (self.name or "").lower()
        if "isr" in name or name.endswith("_handler") or name.endswith("handler"):
            return True
        for attr in self.attributes:
            lowered = attr.lower()
            if any(token in lowered for token in ("interrupt", "isr", "irq")):
                return True
        return False

    @is_isr.setter
    def is_isr(self, value: bool) -> None:
        self.isr_hint = value

    @property
    def is_api(self) -> bool:
        return is_api_function(self)

    @property
    def is_callback(self) -> bool:
        return is_callback_function(self)

    @property
    def is_syscall(self) -> bool:
        return is_syscall_function(self)

    @property
    def has_critical_section(self) -> bool:
        return any(call.is_lock or call.is_unlock for call in self.calls)

    @property
    def has_mutex_protection(self) -> bool:
        has_lock = any(call.is_lock for call in self.calls)
        has_unlock = any(call.is_unlock for call in self.calls)
        if has_lock and has_unlock:
            return True
        return any(var.has_mutex_protection for var in self.local_vars + self.parameters)

    @property
    def all_statements(self) -> List[Statement]:
        stmts: List[Statement] = []
        for block_id in sorted(self.cfg.blocks.keys()):
            stmts.extend(self.cfg.blocks[block_id].statements)
        return stmts

    @property
    def entry_statements(self) -> List[Statement]:
        entry = self.cfg.entry_block
        return list(entry.statements) if entry else []

    @property
    def exit_statements(self) -> List[Statement]:
        statements: List[Statement] = []
        for block in self.cfg.exit_blocks:
            statements.extend(block.statements)
        return statements

    def update_called_by(self, callers: Iterable[str]) -> None:
        self.called_by = sorted(set(callers))

    def update_dominates(self, dominated: Iterable[str]) -> None:
        self.dominates = sorted(set(dominated))


API_NAMING_PREFIXES: Tuple[str, ...] = ("api_", "svc_", "public_", "exposed_")
CALLBACK_SUFFIXES: Tuple[str, ...] = ("_cb", "_callback", "_handler")
SYSCALL_PREFIXES: Tuple[str, ...] = ("sys_", "syscall_", "k_", "os_")


def is_api_function(fn: Function, prefixes: Optional[Iterable[str]] = None) -> bool:
    if fn.is_static:
        return False
    prefix_tuple = tuple(p.lower() for p in (prefixes or API_NAMING_PREFIXES))
    name = (fn.name or "").lower()
    if any(name.startswith(pref) for pref in prefix_tuple):
        return True
    return any("api" in (ann.text or "").lower() for ann in fn.annotations)


def is_callback_function(fn: Function, suffixes: Optional[Iterable[str]] = None) -> bool:
    suffix_tuple = tuple(s.lower() for s in (suffixes or CALLBACK_SUFFIXES))
    name = (fn.name or "").lower()
    if any(name.endswith(suf) for suf in suffix_tuple):
        return True
    return any("callback" in (ann.text or "").lower() for ann in fn.annotations)


def is_syscall_function(fn: Function, prefixes: Optional[Iterable[str]] = None) -> bool:
    prefix_tuple = tuple(p.lower() for p in (prefixes or SYSCALL_PREFIXES))
    name = (fn.name or "").lower()
    if any(name.startswith(pref) for pref in prefix_tuple):
        return True
    return any("syscall" in attr.lower() for attr in fn.attributes)


def detect_isr(fn: Function) -> bool:
    return fn.is_isr


# ============================================================
# ========================= MACROS ===========================
# ============================================================

@dataclass
class MacroDefinition:
    """Represents a C macro emitted by the preprocessor walker."""
    name: str
    kind: Literal["object_like", "function_like"]
    params: List[str] = field(default_factory=list)

    body_tokens: List[str] = field(default_factory=list)
    body_statements: List[str] = field(default_factory=list)

    is_wrapped_single_stmt: bool = False  # e.g. do { ... } while(0)

    source_range: Optional[SourceRange] = None
    preprocessor: PreprocessorContext = field(default_factory=PreprocessorContext)
    annotations: List[Annotation] = field(default_factory=list)

    prefix: Optional[str] = None
    suffix: Optional[str] = None


# ============================================================
# =================== TRANSLATION UNIT =======================
# ============================================================

@dataclass
class TranslationUnit:
    """
    Represents one compiled unit (a .c file + headers + preprocessor result).
    """
    path: str  # path to the .c file that produced this TU

    includes: List[str] = field(default_factory=list)
    macros: List[MacroDefinition] = field(default_factory=list)
    types: List[TypeDecl] = field(default_factory=list)
    globals: List[Variable] = field(default_factory=list)
    functions: List[Function] = field(default_factory=list)

    # Symbol table for quick lookup in this TU: name -> Variable
    symbol_table: Dict[str, Variable] = field(default_factory=dict)

    # Active defines at TU scope
    active_defines: Set[str] = field(default_factory=set)

    # Optional module classification (like "drivers/uart")
    module: Optional[str] = None


# ============================================================
# ===================== PROJECT DATABASE =====================
# ============================================================

@dataclass
class ProjectDB:
    """
    Project-wide index built from all TranslationUnits.
    Enables cross-TU rules like:
    - "only one definition of g_schedulerState"
    - "only code in boot/ can call enter_bootloader()"
    - "no one else can include uart_internal.h"
    """
    translation_units: List[TranslationUnit] = field(default_factory=list)

    # Indexes
    functions_by_name: Dict[str, List[Function]] = field(default_factory=dict)
    globals_by_name: Dict[str, List[Variable]] = field(default_factory=dict)
    macros_by_name: Dict[str, List[MacroDefinition]] = field(default_factory=dict)

    include_usage: Dict[str, List[str]] = field(default_factory=dict)  # header -> list of TU paths

    # caller -> set(callee)
    call_graph: Dict[str, Set[str]] = field(default_factory=dict)

    # Policy / semantic metadata:
    # e.g. { "sleep_ms": {"is_blocking_api": True} }
    function_metadata: Dict[str, Dict[str, Any]] = field(default_factory=dict)

    # Optional: directory/module access rules
    module_rules: Dict[str, Any] = field(default_factory=dict)


# ============================================================
# ======================= RULE MODELS ========================
# ============================================================

@dataclass
class Rule:
    """
    Representation of a single rule loaded from YAML.
    This mirrors the YAML contract:

    - id: unique rule id
    - description: human text
    - severity: "error" | "warning" | ...
    - scope: which IR node type the rule iterates over,
             e.g. "Function", "IfStmt", "TranslationUnit", "ProjectDB"
    - select: textual DSL block that binds targets
    - assert_code: textual DSL block describing what must be true
    - message: template for violation message
    - exceptions: list of textual conditions that suppress violations
    - tags: optional list of strings
    - fixit: optional template for suggested fix
    """
    id: str
    description: str
    severity: str
    scope: str  # "Function", "IfStmt", "MacroDefinition", "ProjectDB", etc.

    select: str
    assert_code: str
    message: str

    exceptions: List[str] = field(default_factory=list)
    tags: List[str] = field(default_factory=list)
    fixit: Optional[str] = None


@dataclass
class ViolationContextFunction:
    """Structured context describing the offending function when relevant."""
    name: str
    is_isr: bool
    linkage: str
    attributes: List[str]


@dataclass
class ViolationContextCall:
    """Metadata for a call-site context attached to a violation."""
    callee: Optional[str]
    in_macro: bool
    in_branch: Optional[str]
    in_loop: Optional[str]


@dataclass
class ViolationContextCF:
    """Records whether control-flow reasoning was path sensitive and exhaustive."""
    path_sensitive: bool
    all_paths_proven: bool


@dataclass
class ViolationContextPreproc:
    """Summarizes preprocessor state active at the violation location."""
    active_defines: List[str]
    guard_condition: Optional[str]


@dataclass
class ViolationExtrasSymbol:
    """Additional symbol metadata in violation extras (e.g., known policy flags)."""
    name: str
    decl_file: Optional[str]
    is_blocking_api: Optional[bool]


@dataclass
class Violation:
    """
    Rich violation object that we will eventually serialize to JSON.
    """
    rule_id: str
    severity: str
    message: str

    location: Dict[str, Any]  # {file, line_start, col_start, line_end, col_end}

    context: Dict[str, Any] = field(default_factory=dict)
    suggested_fix: Optional[str] = None
    extras: Dict[str, Any] = field(default_factory=dict)


# ============================================================
# ==================== RULE EVALUATION =======================
# ============================================================


def _mark_safe_callable(func: Any) -> Any:
    setattr(func, "_nomic_safe_callable", True)
    return func


def _wrap_safe_callable(func: Any) -> Any:
    @functools.wraps(func)
    def _safe_wrapper(*args: Any, **kwargs: Any) -> Any:
        return func(*args, **kwargs)

    return _mark_safe_callable(_safe_wrapper)


def _wrap_dynamic_callable(func: Any) -> Any:
    def _safe_wrapper(*args: Any, **kwargs: Any) -> Any:
        return func(*args, **kwargs)

    return _mark_safe_callable(_safe_wrapper)


def _exists_helper(iterable: Any) -> bool:
    for item in iterable:
        if item:
            return True
    return False


def _forall_helper(iterable: Any) -> bool:
    for item in iterable:
        if not item:
            return False
    return True


def _count_helper(iterable: Any) -> int:
    return sum(1 for item in iterable if item)


def _implies_helper(left: Any, right: Any) -> bool:
    return (not bool(left)) or bool(right)


_SAFE_BASE_CALLABLES: Dict[str, Any] = {
    "len": _wrap_safe_callable(len),
    "any": _wrap_safe_callable(any),
    "all": _wrap_safe_callable(all),
    "sum": _wrap_safe_callable(sum),
    "min": _wrap_safe_callable(min),
    "max": _wrap_safe_callable(max),
    "sorted": _wrap_safe_callable(sorted),
    "abs": _wrap_safe_callable(abs),
    "exists": _wrap_safe_callable(_exists_helper),
    "forall": _wrap_safe_callable(_forall_helper),
    "count": _wrap_safe_callable(_count_helper),
    "_nomic_implies": _wrap_safe_callable(_implies_helper),
    "implies": _wrap_safe_callable(_implies_helper),
    "hasattr": _wrap_safe_callable(hasattr),
    "safe_getattr": _wrap_safe_callable(lambda obj, name, default=None: getattr(obj, name, default)),
}

TEMPLATE_PATTERN = re.compile(r"\{\{\s*([^{}]+?)\s*\}\}")


class ExpressionEvalError(Exception):
    """Raised when the DSL expression evaluator encounters an unsafe or invalid construct."""


_IMPLIES_KEYWORD = "implies"
_BRACKET_PAIRS = {"(": ")", "[": "]", "{": "}"}
_QUANTIFIER_KEYWORDS = ("exists", "forall", "count")


def _rewrite_quantifiers(expr: str) -> str:
    """
    Support explicit quantifier syntax: e.g.
        exists(item in collection if predicate)
    Which is rewritten into a generator expression handed to the helper.
    """
    if not expr or not any(keyword in expr for keyword in _QUANTIFIER_KEYWORDS):
        return expr

    def _is_boundary(text: str, index: int, keyword: str) -> bool:
        before = text[index - 1] if index > 0 else ""
        after_index = index + len(keyword)
        after = text[after_index] if after_index < len(text) else ""
        return (not before or not (before.isalnum() or before == "_")) and (
            not after or after.isspace() or after == "("
        )

    def _convert_body(body: str) -> str:
        parts = body.split(" in ", 1)
        if len(parts) != 2:
            return body
        target = parts[0].strip()
        remainder = parts[1].strip()
        if not target or not remainder:
            return body
        if " if " in remainder:
            iterable_part, condition = remainder.split(" if ", 1)
            iterable_part = iterable_part.strip()
            condition = condition.strip()
        else:
            iterable_part = remainder
            condition = ""
        generator = f"{target} for {target} in ({iterable_part})"
        if condition:
            generator += f" if ({condition})"
        return generator

    result: List[str] = []
    index = 0
    length = len(expr)
    while index < length:
        matched = False
        for keyword in _QUANTIFIER_KEYWORDS:
            if expr.startswith(keyword, index) and _is_boundary(expr, index, keyword):
                cursor = index + len(keyword)
                while cursor < length and expr[cursor].isspace():
                    cursor += 1
                if cursor >= length or expr[cursor] != "(":
                    continue
                depth = 1
                body_start = cursor + 1
                cursor += 1
                while cursor < length and depth > 0:
                    ch = expr[cursor]
                    if ch == "(":
                        depth += 1
                    elif ch == ")":
                        depth -= 1
                    cursor += 1
                if depth != 0:
                    # Unbalanced; leave expression untouched
                    continue
                body = expr[body_start : cursor - 1]
                converted = _convert_body(body)
                result.append(f"{keyword}({converted})")
                index = cursor
                matched = True
                break
        if not matched:
            result.append(expr[index])
            index += 1
    return "".join(result)


def _rewrite_implies_expression(expr: str) -> str:
    """
    Transform infix 'implies' operators into calls to the safe helper.
    The DSL allows expressions like `a implies b`, which we desugar into
    `_nomic_implies(a, b)` while respecting parentheses, brackets, and strings.
    """
    if _IMPLIES_KEYWORD not in expr:
        return expr

    def _is_identifier_char(ch: str) -> bool:
        return ch.isalnum() or ch in {"_", "."}

    def _skip_string_literal(text: str, start: int) -> int:
        quote = text[start]
        triple = text.startswith(quote * 3, start)
        delimiter = quote * (3 if triple else 1)
        idx = start + len(delimiter)
        while idx < len(text):
            if text.startswith(delimiter, idx):
                return idx + len(delimiter)
            if text[idx] == "\\":
                idx += 2
            else:
                idx += 1
        raise ExpressionEvalError("unterminated string literal in DSL expression")

    def _match_keyword(text: str, index: int) -> bool:
        if not text.startswith(_IMPLIES_KEYWORD, index):
            return False
        before = text[index - 1] if index > 0 else ""
        after_index = index + len(_IMPLIES_KEYWORD)
        after = text[after_index] if after_index < len(text) else ""
        if before and _is_identifier_char(before):
            return False
        if after and _is_identifier_char(after):
            return False
        return True

    def _find_top_level_keyword(text: str) -> int:
        stack: List[str] = []
        idx = 0
        length = len(text)
        while idx < length:
            ch = text[idx]
            if ch in {"'", '"'}:
                idx = _skip_string_literal(text, idx)
                continue
            if ch in _BRACKET_PAIRS:
                stack.append(_BRACKET_PAIRS[ch])
                idx += 1
                continue
            if stack and ch == stack[-1]:
                stack.pop()
                idx += 1
                continue
            if stack:
                idx += 1
                continue
            if _match_keyword(text, idx):
                return idx
            idx += 1
        return -1

    def _rewrite_segment(text: str) -> str:
        stripped = text.strip()
        if not stripped:
            return stripped
        if _IMPLIES_KEYWORD not in stripped:
            return stripped
        keyword_index = _find_top_level_keyword(stripped)
        if keyword_index != -1:
            left = stripped[:keyword_index]
            right = stripped[keyword_index + len(_IMPLIES_KEYWORD):]
            if not left.strip() or not right.strip():
                raise ExpressionEvalError("invalid 'implies' operand")
            left_expr = _rewrite_segment(left)
            right_expr = _rewrite_segment(right)
            return f"_nomic_implies({left_expr}, {right_expr})"
        return _rewrite_nested(stripped)

    def _rewrite_nested(text: str) -> str:
        if _IMPLIES_KEYWORD not in text:
            return text
        result: List[str] = []
        idx = 0
        length = len(text)
        while idx < length:
            ch = text[idx]
            if ch in {"'", '"'}:
                start = idx
                idx = _skip_string_literal(text, idx)
                result.append(text[start:idx])
                continue
            closing = _BRACKET_PAIRS.get(ch)
            if closing:
                start = idx
                idx += 1
                depth = 1
                while idx < length and depth:
                    current = text[idx]
                    if current in {"'", '"'}:
                        idx = _skip_string_literal(text, idx)
                        continue
                    if current == ch:
                        depth += 1
                    elif current == closing:
                        depth -= 1
                    idx += 1
                if depth != 0:
                    raise ExpressionEvalError("unbalanced brackets in DSL expression")
                inner = text[start + 1 : idx - 1]
                rewritten_inner = _rewrite_segment(inner)
                result.append(ch + rewritten_inner + closing)
                continue
            result.append(ch)
            idx += 1
        return "".join(result)

    return _rewrite_segment(expr)


class _SafeExpressionInterpreter:
    """
    Evaluates a restricted subset of Python expressions by walking the AST.
    Supports boolean logic, arithmetic, comparisons, attribute access, indexing,
    safe function calls, and literals/containers.
    """

    _BIN_OPS = {
        ast.Add: operator.add,
        ast.Sub: operator.sub,
        ast.Mult: operator.mul,
        ast.Div: operator.truediv,
        ast.FloorDiv: operator.floordiv,
        ast.Mod: operator.mod,
        ast.Pow: operator.pow,
        ast.BitAnd: operator.and_,
        ast.BitOr: operator.or_,
        ast.BitXor: operator.xor,
        ast.LShift: operator.lshift,
        ast.RShift: operator.rshift,
    }
    _UNARY_OPS = {
        ast.Not: operator.not_,
        ast.USub: operator.neg,
        ast.UAdd: operator.pos,
        ast.Invert: operator.invert,
    }

    def __init__(self) -> None:
        self._cache: Dict[str, ast.AST] = {}

    def evaluate(self, expr: str, env: Dict[str, Any]) -> Any:
        expr = expr.strip()
        if not expr:
            return True
        expr = _rewrite_quantifiers(expr)
        expr = _rewrite_implies_expression(expr)

        try:
            tree = self._cache.get(expr)
            if tree is None:
                tree = ast.parse(expr, mode="eval")
                self._cache[expr] = tree
        except SyntaxError as exc:  # pragma: no cover
            raise ExpressionEvalError(f"invalid expression '{expr}': {exc}") from exc

        return self._eval_node(tree.body, env)

    def _eval_node(self, node: ast.AST, env: Dict[str, Any]) -> Any:
        if isinstance(node, ast.BoolOp):
            if isinstance(node.op, ast.And):
                result = True
                for value in node.values:
                    result = result and bool(self._eval_node(value, env))
                    if not result:
                        break
                return result
            if isinstance(node.op, ast.Or):
                result = False
                for value in node.values:
                    result = result or bool(self._eval_node(value, env))
                    if result:
                        break
                return result
            raise ExpressionEvalError("unsupported boolean operator")

        if isinstance(node, ast.UnaryOp):
            op = self._UNARY_OPS.get(type(node.op))
            if not op:
                raise ExpressionEvalError("unsupported unary operator")
            operand = self._eval_node(node.operand, env)
            return op(operand)

        if isinstance(node, ast.BinOp):
            op = self._BIN_OPS.get(type(node.op))
            if not op:
                raise ExpressionEvalError("unsupported binary operator")
            left = self._eval_node(node.left, env)
            right = self._eval_node(node.right, env)
            return op(left, right)

        if isinstance(node, ast.Compare):
            left = self._eval_node(node.left, env)
            for operator_node, comparator in zip(node.ops, node.comparators):
                right = self._eval_node(comparator, env)
                if not self._evaluate_comparison(operator_node, left, right):
                    return False
                left = right
            return True

        if isinstance(node, ast.IfExp):
            condition = self._eval_node(node.test, env)
            branch = node.body if condition else node.orelse
            return self._eval_node(branch, env)

        if isinstance(node, ast.Lambda):
            return self._build_lambda(node, env)

        if isinstance(node, ast.Attribute):
            value = self._eval_node(node.value, env)
            if node.attr.startswith("_"):
                raise ExpressionEvalError("access to private attributes is not allowed")
            attr_value = getattr(value, node.attr)
            if callable(attr_value) and not getattr(attr_value, "_nomic_safe_callable", False):
                attr_value = _wrap_dynamic_callable(attr_value)
            return attr_value

        if isinstance(node, ast.Name):
            if node.id in env:
                return env[node.id]
            raise ExpressionEvalError(f"unknown identifier '{node.id}'")

        if isinstance(node, ast.Constant):
            return node.value

        if isinstance(node, ast.GeneratorExp):
            return self._build_generator(node, env)

        if isinstance(node, ast.ListComp):
            return list(self._comprehension_values(node.generators, env, node.elt))

        if isinstance(node, ast.SetComp):
            return set(self._comprehension_values(node.generators, env, node.elt))

        if isinstance(node, ast.DictComp):
            return {
                key: value
                for key, value in self._comprehension_items(node.generators, env, node.key, node.value)
            }

        if isinstance(node, ast.Call):
            func_obj = self._eval_node(node.func, env)
            if not getattr(func_obj, "_nomic_safe_callable", False):
                raise ExpressionEvalError("call to unsafe function is not allowed")
            args = [self._eval_node(arg, env) for arg in node.args]
            kwargs = {kw.arg: self._eval_node(kw.value, env) for kw in node.keywords if kw.arg}
            return func_obj(*args, **kwargs)

        if isinstance(node, ast.Subscript):
            value = self._eval_node(node.value, env)
            key = self._eval_slice(node.slice, env)
            return value[key]

        if isinstance(node, ast.List):
            return [self._eval_node(elt, env) for elt in node.elts]

        if isinstance(node, ast.Tuple):
            return tuple(self._eval_node(elt, env) for elt in node.elts)

        if isinstance(node, ast.Set):
            return {self._eval_node(elt, env) for elt in node.elts}

        if isinstance(node, ast.Dict):
            keys = [self._eval_node(k, env) if k is not None else None for k in node.keys]
            values = [self._eval_node(v, env) for v in node.values]
            return {k: v for k, v in zip(keys, values)}

        raise ExpressionEvalError(f"unsupported expression node: {type(node).__name__}")

    def _eval_slice(self, slice_node: ast.AST, env: Dict[str, Any]) -> Any:
        if isinstance(slice_node, ast.Slice):
            lower = self._eval_node(slice_node.lower, env) if slice_node.lower else None
            upper = self._eval_node(slice_node.upper, env) if slice_node.upper else None
            step = self._eval_node(slice_node.step, env) if slice_node.step else None
            return slice(lower, upper, step)
        return self._eval_node(slice_node, env)

    def _evaluate_comparison(self, operator_node: ast.AST, left: Any, right: Any) -> bool:
        if isinstance(operator_node, ast.Eq):
            return left == right
        if isinstance(operator_node, ast.NotEq):
            return left != right
        if isinstance(operator_node, ast.Gt):
            return left > right
        if isinstance(operator_node, ast.GtE):
            return left >= right
        if isinstance(operator_node, ast.Lt):
            return left < right
        if isinstance(operator_node, ast.LtE):
            return left <= right
        if isinstance(operator_node, ast.In):
            return left in right
        if isinstance(operator_node, ast.NotIn):
            return left not in right
        if isinstance(operator_node, ast.Is):
            return left is right
        if isinstance(operator_node, ast.IsNot):
            return left is not right
        raise ExpressionEvalError("unsupported comparison operator")

    def _build_lambda(self, node: ast.Lambda, env: Dict[str, Any]) -> Any:
        arg_names = [arg.arg for arg in node.args.args]
        default_values = [self._eval_node(default, env) for default in node.args.defaults]

        def _lambda_impl(*lambda_args: Any) -> Any:
            local_env = dict(env)
            bound_args = list(lambda_args)
            if len(bound_args) < len(arg_names) and default_values:
                missing = len(arg_names) - len(bound_args)
                bound_args.extend(default_values[-missing:])
            for name, value in zip(arg_names, bound_args):
                local_env[name] = value
            return self._eval_node(node.body, local_env)

        return _wrap_dynamic_callable(lambda *args, **kwargs: _lambda_impl(*args, **kwargs))

    def _build_generator(self, node: ast.GeneratorExp, env: Dict[str, Any]) -> Any:
        def generator() -> Any:
            yield from self._comprehension_values(node.generators, env, node.elt)

        return generator()

    def _comprehension_values(
        self,
        generators: List[ast.comprehension],
        env: Dict[str, Any],
        value_node: ast.AST,
    ) -> Any:
        yield from self._iterate_comprehension(generators, env, value_node, None)

    def _comprehension_items(
        self,
        generators: List[ast.comprehension],
        env: Dict[str, Any],
        key_node: ast.AST,
        value_node: ast.AST,
    ) -> Any:
        yield from self._iterate_comprehension(generators, env, value_node, key_node)

    def _iterate_comprehension(
        self,
        generators: List[ast.comprehension],
        env: Dict[str, Any],
        value_node: ast.AST,
        key_node: Optional[ast.AST],
    ):
        def recurse(index: int, current_env: Dict[str, Any]):
            if index == len(generators):
                if key_node is None:
                    yield self._eval_node(value_node, current_env)
                else:
                    yield (
                        self._eval_node(key_node, current_env),
                        self._eval_node(value_node, current_env),
                    )
                return

            comp = generators[index]
            iterable = self._eval_node(comp.iter, current_env)
            for item in iterable:
                new_env = dict(current_env)
                self._assign_comprehension_target(new_env, comp.target, item)
                if all(bool(self._eval_node(condition, new_env)) for condition in comp.ifs):
                    yield from recurse(index + 1, new_env)

        yield from recurse(0, dict(env))

    def _assign_comprehension_target(self, env: Dict[str, Any], target: ast.AST, value: Any) -> None:
        if isinstance(target, ast.Name):
            env[target.id] = value
            return
        if isinstance(target, (ast.Tuple, ast.List)):
            values = list(value) if not isinstance(value, (list, tuple)) else value
            if len(target.elts) != len(values):
                raise ExpressionEvalError("comprehension target length mismatch")
            for subtarget, subvalue in zip(target.elts, values):
                self._assign_comprehension_target(env, subtarget, subvalue)
            return
        raise ExpressionEvalError("unsupported comprehension target")


class RuleEngine:
    """
    Drives evaluation of YAML rules against the ProjectDB.

    Responsibilities:
    * Collect IR objects for each rule scope.
    * Interpret the declarative DSL (select/assert/exceptions).
    * Emit Violation objects with fully-populated context.
    """

    def __init__(self, project_db: ProjectDB, rules: List[Rule]) -> None:
        self.project_db = project_db
        self.rules = rules
        self._scope_cache: Dict[str, List[Any]] = {}
        self._unknown_scopes: Set[str] = set()
        self._unknown_scope_reported: Set[str] = set()
        self._dsl_notice_emitted: bool = False
        self._expr_error_reported: Set[Tuple[str, str, str]] = set()
        self._expr_interpreter = _SafeExpressionInterpreter()

    def evaluate(self) -> List[Violation]:
        """
        Iterate each rule, gather candidates for the declared scope, bind aliases,
        run `select` predicates, evaluate assertions/exceptions, and materialize
        `Violation` objects when assertions fail.
        """
        violations: List[Violation] = []

        for rule in self.rules:
            nodes, recognized = self._collect_scope_objects(rule.scope)
            if not recognized:
                self._maybe_report_unknown_scope(rule, rule.scope)
                continue

            if not nodes:
                continue

            self._notify_dsl_unimplemented(rule)
            bindings, predicate = self._parse_select(rule.select, rule.scope)
            primary_alias, primary_scope = bindings[0]

            if primary_scope != rule.scope:
                nodes, recognized = self._collect_scope_objects(primary_scope)
                if not recognized:
                    self._maybe_report_unknown_scope(rule, primary_scope)
                    continue

            if not nodes:
                continue

            assert_expr = (rule.assert_code or "").strip()
            exception_exprs = [expr.strip() for expr in rule.exceptions if expr.strip()]

            binding_objects: List[List[Any]] = []
            binding_objects.append(nodes)

            scopes_valid = True
            for _, scope_name in bindings[1:]:
                objs, recognized = self._collect_scope_objects(scope_name)
                if not recognized:
                    self._maybe_report_unknown_scope(rule, scope_name)
                    scopes_valid = False
                    break
                binding_objects.append(objs)

            if not scopes_valid:
                continue

            if any(len(obj_list) == 0 for obj_list in binding_objects):
                continue

            for primary_obj in binding_objects[0]:
                env = self._create_base_env()
                self._assign_alias(env, primary_alias, primary_obj, primary=True)
                self._evaluate_binding_combinations(
                    rule=rule,
                    bindings=bindings,
                    binding_objects=binding_objects,
                    predicate=predicate,
                    assert_expr=assert_expr,
                    exception_exprs=exception_exprs,
                    env=env,
                    violations=violations,
                    start_index=1,
                    primary_node=primary_obj,
                )

        return violations

    def _parse_select(self, select_expr: Optional[str], scope: str) -> Tuple[List[Tuple[str, str]], str]:
        """
        Extract (bindings, predicate_expression) from the rule's select block.
        Supported forms:
          - "alias: Scope where expr"
          - "alias: Scope"
          - "alias1: Scope1, alias2: Scope2 where expr"
          - "where expr"
          - plain expression (defaults alias -> obj)
          - empty/None => predicate True
        """
        if not select_expr:
            return [("obj", scope)], "True"

        text = select_expr.strip()
        if not text:
            return [("obj", scope)], "True"

        predicate = "True"
        select_part = text
        lower = text.lower()
        if lower.startswith("where "):
            predicate = text[6:].strip() or "True"
            select_part = ""
        else:
            where_idx = lower.find(" where ")
            if where_idx != -1:
                predicate = text[where_idx + len(" where "):].strip() or "True"
                select_part = text[:where_idx].strip()

        binding_specs = [spec.strip() for spec in select_part.split(",") if spec.strip()]
        bindings: List[Tuple[str, str]] = []
        if not binding_specs:
            bindings.append(("obj", scope))
        else:
            for idx, spec in enumerate(binding_specs):
                if ":" in spec:
                    alias_part, scope_part = spec.split(":", 1)
                    alias_name = alias_part.strip() or f"obj{idx}"
                    scope_name = scope_part.strip() or scope
                else:
                    alias_name = spec.strip() or f"obj{idx}"
                    scope_name = scope
                bindings.append((alias_name, scope_name))

        return bindings, predicate

    def _create_base_env(self) -> Dict[str, Any]:
        """Construct the safe evaluation environment shared by all expressions."""
        env: Dict[str, Any] = dict(_SAFE_BASE_CALLABLES)
        env.update(
            {
                "project": self.project_db,
                "project_db": self.project_db,
                "functions_by_name": self.project_db.functions_by_name,
                "globals_by_name": self.project_db.globals_by_name,
                "call_edge": self._make_env_callable(self._call_edge_helper),
                "calls_function": self._make_env_callable(self._calls_function_helper),
                "call_path_exists": self._make_env_callable(self._call_path_helper),
                "reachable_functions": self._make_env_callable(self._reachable_functions_helper),
                "all_paths_reach": self._make_env_callable(all_paths_reach),
                "any_path_reaches": self._make_env_callable(any_path_reaches),
                "paths_between": self._make_env_callable(paths_between),
                "has_blocking_call_path": self._make_env_callable(self._blocking_call_path_helper),
                "detect_isr": self._make_env_callable(detect_isr),
                "check_type_compliance": self._make_env_callable(check_type_compliance),
                "is_api_function": self._make_env_callable(is_api_function),
                "is_callback_function": self._make_env_callable(is_callback_function),
                "is_syscall_function": self._make_env_callable(is_syscall_function),
                "pattern_search": self._make_env_callable(pattern_search),
                "pattern_matches": self._make_env_callable(pattern_matches),
                "pattern_findall": self._make_env_callable(pattern_findall),
                "pattern_extract": self._make_env_callable(pattern_extract),
                "semantic_pattern_matches": self._make_env_callable(semantic_pattern_matches),
                "semantic_pattern_any": self._make_env_callable(semantic_pattern_any),
                "semantic_statements": self._make_env_callable(semantic_statements),
                "path_matches": self._make_env_callable(path_matches),
                "path_requires": self._make_env_callable(path_requires),
                "policy_lookup": self._make_env_callable(self._policy_lookup_helper),
                "function_policy": self._make_env_callable(self._function_policy_helper),
            }
        )
        return env

    def _make_env_callable(self, func: Any) -> Any:
        """Wrap helper functions so the AST interpreter treats them as safe."""
        @functools.wraps(func)
        def _wrapper(*args: Any, **kwargs: Any) -> Any:
            return func(*args, **kwargs)

        return _mark_safe_callable(_wrapper)

    def _assign_alias(self, env: Dict[str, Any], alias: str, node: Any, *, primary: bool = False) -> None:
        """
        Bind the alias name to the underlying IR node for downstream expressions.
        The first binding also becomes `obj` for backwards compatibility.
        """
        env[alias] = node
        if primary:
            env["obj"] = node
            env["_scope_name"] = getattr(node, "name", None)

    def _evaluate_binding_combinations(
        self,
        *,
        rule: Rule,
        bindings: List[Tuple[str, str]],
        binding_objects: List[List[Any]],
        predicate: str,
        assert_expr: str,
        exception_exprs: List[str],
        env: Dict[str, Any],
        violations: List[Violation],
        start_index: int,
        primary_node: Any,
    ) -> None:
        def recurse(index: int, current_env: Dict[str, Any]) -> None:
            if index == len(bindings):
                self._evaluate_rule_conditions(
                    rule=rule,
                    env=current_env,
                    predicate=predicate,
                    assert_expr=assert_expr,
                    exception_exprs=exception_exprs,
                    violations=violations,
                    node=primary_node,
                )
                return

            alias, _ = bindings[index]
            for obj in binding_objects[index]:
                next_env = dict(current_env)
                self._assign_alias(next_env, alias, obj, primary=False)
                recurse(index + 1, next_env)

        recurse(start_index, env)

    def _evaluate_rule_conditions(
        self,
        *,
        rule: Rule,
        env: Dict[str, Any],
        predicate: str,
        assert_expr: str,
        exception_exprs: List[str],
        violations: List[Violation],
        node: Any,
    ) -> None:
        if not self._evaluate_expression(predicate or "True", env, rule, stage="select"):
            return

        result = self._evaluate_expression(assert_expr or "True", env, rule, stage="assert")
        if result:
            return

        for exception_expr in exception_exprs:
            if self._evaluate_expression(exception_expr, env, rule, stage="exception"):
                return

        violations.append(self._build_violation(rule, env, node))

    def _call_edge_helper(self, caller: Any, callee: Any) -> bool:
        """Return True if the ProjectDB call graph shows a direct call edge."""
        caller_name = self._extract_symbol_name(caller)
        callee_name = self._extract_symbol_name(callee)
        if not caller_name or not callee_name:
            return False
        return callee_name in self.project_db.call_graph.get(caller_name, set())

    def _calls_function_helper(self, function_obj: Any, callee: Any) -> bool:
        """Return True if `function_obj.calls` contains `callee` (name or object)."""
        if not isinstance(function_obj, Function):
            return False
        target = self._extract_symbol_name(callee)
        if not target:
            return False
        for call in function_obj.calls:
            if target and target in {call.callee_symbol, call.callee_name}:
                return True
        return False

    def _call_path_helper(self, caller: Any, callee: Any, max_depth: int = 64) -> bool:
        """Breadth-first search bounded by `max_depth` to find a path from caller to callee."""
        start = self._extract_symbol_name(caller)
        target = self._extract_symbol_name(callee)
        if not start or not target:
            return False
        if start == target:
            return True
        visited = set([start])
        queue: deque[Tuple[str, int]] = deque([(start, 0)])
        while queue:
            symbol, depth = queue.popleft()
            if depth >= max_depth:
                continue
            for neighbor in self.project_db.call_graph.get(symbol, set()):
                if neighbor == target:
                    return True
                if neighbor not in visited:
                    visited.add(neighbor)
                    queue.append((neighbor, depth + 1))
        return False

    def _reachable_functions_helper(self, caller: Any, max_depth: int = 64) -> List[str]:
        """Return a sorted list of names reachable from `caller` within `max_depth` hops."""
        start = self._extract_symbol_name(caller)
        if not start:
            return []
        visited = set([start])
        queue: deque[Tuple[str, int]] = deque([(start, 0)])
        reachable: Set[str] = set()
        while queue:
            symbol, depth = queue.popleft()
            if depth >= max_depth:
                continue
            for neighbor in self.project_db.call_graph.get(symbol, set()):
                if neighbor not in visited:
                    visited.add(neighbor)
                    reachable.add(neighbor)
                    queue.append((neighbor, depth + 1))
        return sorted(reachable)

    def _blocking_call_path_helper(self, caller: Any, max_depth: int = 32) -> bool:
        start = self._extract_symbol_name(caller)
        if not start:
            return False
        visited = set([start])
        queue: deque[Tuple[str, int]] = deque([(start, 0)])
        while queue:
            symbol, depth = queue.popleft()
            if depth > max_depth:
                continue
            for fn in self.project_db.functions_by_name.get(symbol, []):
                if any(call.is_blocking_api for call in fn.calls):
                    return True
            for neighbor in self.project_db.call_graph.get(symbol, set()):
                if neighbor not in visited:
                    visited.add(neighbor)
                    queue.append((neighbor, depth + 1))
        return False

    def _policy_lookup_helper(self, module_name: str, key: Optional[str] = None, default: Any = None) -> Any:
        module = self.project_db.module_rules.get(module_name)
        if module is None:
            return default
        if key is None:
            return module
        if isinstance(module, dict):
            return module.get(key, default)
        return default

    def _function_policy_helper(self, function_obj: Any, key: str, default: Any = None) -> Any:
        symbol = self._extract_symbol_name(function_obj)
        if not symbol:
            return default
        metadata = self.project_db.function_metadata.get(symbol)
        if metadata is None:
            return default
        return metadata.get(key, default)

    def _extract_symbol_name(self, obj: Any) -> Optional[str]:
        if obj is None:
            return None
        if isinstance(obj, str):
            return obj
        return getattr(obj, "name", None)

    def _eval_raw_expression(
        self,
        expr: str,
        env: Dict[str, Any],
        rule: Rule,
        *,
        stage: str,
    ) -> Any:
        expr = (expr or "").strip()
        if not expr:
            return True
        try:
            return self._expr_interpreter.evaluate(expr, env)
        except ExpressionEvalError as exc:
            self._report_eval_error(rule, stage, expr, exc)
            return False if stage in {"select", "assert", "exception"} else ""
        except Exception as exc:  # pragma: no cover - safeguard
            self._report_eval_error(rule, stage, expr, exc)
            return False if stage in {"select", "assert", "exception"} else ""

    def _evaluate_expression(
        self,
        expr: str,
        env: Dict[str, Any],
        rule: Rule,
        *,
        stage: str,
    ) -> bool:
        result = self._eval_raw_expression(expr, env, rule, stage=stage)
        return bool(result)

    def _report_eval_error(
        self,
        rule: Rule,
        stage: str,
        expr: str,
        exc: Exception,
    ) -> None:
        key = (rule.id, stage, expr)
        if key in self._expr_error_reported:
            return
        sys.stderr.write(
            f"[nomic] Failed to evaluate {stage} expression for rule '{rule.id}': "
            f"{expr!r} ({exc}).\n"
        )
        self._expr_error_reported.add(key)

    def _build_violation(self, rule: Rule, env: Dict[str, Any], node: Any) -> Violation:
        location = self._object_location(node)
        message = self._render_template(rule.message, env, rule, field="message")
        suggested_fix = None
        if rule.fixit:
            rendered_fix = self._render_template(rule.fixit, env, rule, field="fixit")
            if rendered_fix:
                suggested_fix = rendered_fix
        context = {
            "scope": rule.scope,
            "name": getattr(node, "name", None),
        }
        return Violation(
            rule_id=rule.id,
            severity=rule.severity,
            message=message,
            location=location,
            context=context,
            suggested_fix=suggested_fix,
            extras={"rule_tags": rule.tags},
        )

    def _object_location(self, node: Any) -> Dict[str, Any]:
        source_range = getattr(node, "source_range", None)
        if isinstance(source_range, SourceRange):
            return {
                "file": source_range.file,
                "line_start": source_range.line_start,
                "col_start": source_range.col_start,
                "line_end": source_range.line_end,
                "col_end": source_range.col_end,
            }

        location = getattr(node, "location", None)
        if isinstance(location, SourceLocation):
            return {
                "file": location.file,
                "line_start": location.line,
                "col_start": location.column,
                "line_end": location.line,
                "col_end": location.column,
            }

        file_hint = getattr(node, "path", None) or getattr(node, "file", None) or "<unknown>"
        return {
            "file": file_hint,
            "line_start": 0,
            "col_start": 0,
            "line_end": 0,
            "col_end": 0,
        }

    def _render_template(
        self,
        template: Optional[str],
        env: Dict[str, Any],
        rule: Rule,
        *,
        field: str,
    ) -> str:
        if not template:
            return ""

        def replace(match: re.Match[str]) -> str:
            expr = match.group(1)
            value = self._eval_raw_expression(expr, env, rule, stage=f"template:{field}")
            return "" if value is None else str(value)

        return TEMPLATE_PATTERN.sub(replace, template)

    def _collect_scope_objects(self, scope: str) -> Tuple[List[Any], bool]:
        """
        Map a rule scope string to a concrete list of IR objects.
        Returns (objects, recognized_scope_flag).
        """
        if scope in self._scope_cache:
            return self._scope_cache[scope], scope not in self._unknown_scopes

        known = True
        objects: List[Any] = []

        if scope == "ProjectDB":
            objects = [self.project_db]
        elif scope == "TranslationUnit":
            objects = list(self.project_db.translation_units)
        elif scope == "Function":
            funcs: List[Function] = []
            for tu in self.project_db.translation_units:
                funcs.extend(tu.functions)
            objects = funcs
        elif scope == "MacroDefinition":
            macros: List[MacroDefinition] = []
            for tu in self.project_db.translation_units:
                macros.extend(tu.macros)
            objects = macros
        elif scope == "TypeDecl":
            types: List[TypeDecl] = []
            for tu in self.project_db.translation_units:
                types.extend(tu.types)
            objects = types
        elif scope == "Variable":
            variables: List[Variable] = []
            for tu in self.project_db.translation_units:
                variables.extend(tu.globals)
                for fn in tu.functions:
                    variables.extend(fn.parameters)
                    variables.extend(fn.local_vars)
            objects = variables
        elif scope == "CallSite":
            callsites: List[CallSite] = []
            functions, _ = self._collect_scope_objects("Function")
            for fn in functions:
                callsites.extend(fn.calls)
                for block in fn.cfg.blocks.values():
                    callsites.extend(block.calls)
                for if_stmt in fn.if_stmts:
                    callsites.extend(if_stmt.then_block.calls)
                    if if_stmt.else_block:
                        callsites.extend(if_stmt.else_block.calls)
                for loop in fn.loops:
                    callsites.extend(loop.body.calls)
                for switch in fn.switches:
                    for case in switch.cases:
                        callsites.extend(case.body.calls)
            objects = callsites
        elif scope == "IfStmt":
            ifs: List[IfStmt] = []
            functions, _ = self._collect_scope_objects("Function")
            for fn in functions:
                ifs.extend(fn.if_stmts)
            objects = ifs
        elif scope == "LoopStmt":
            loops: List[LoopStmt] = []
            functions, _ = self._collect_scope_objects("Function")
            for fn in functions:
                loops.extend(fn.loops)
            objects = loops
        elif scope == "SwitchStmt":
            switches: List[SwitchStmt] = []
            functions, _ = self._collect_scope_objects("Function")
            for fn in functions:
                switches.extend(fn.switches)
            objects = switches
        elif scope == "SwitchCaseBlock":
            cases: List[SwitchCaseBlock] = []
            switches, _ = self._collect_scope_objects("SwitchStmt")
            for switch in switches:
                cases.extend(switch.cases)
            objects = cases
        elif scope == "BlockStmt":
            blocks: List[BlockStmt] = []
            ifs, _ = self._collect_scope_objects("IfStmt")
            for if_stmt in ifs:
                blocks.append(if_stmt.then_block)
                if if_stmt.else_block:
                    blocks.append(if_stmt.else_block)
            loops, _ = self._collect_scope_objects("LoopStmt")
            for loop in loops:
                blocks.append(loop.body)
            switches, _ = self._collect_scope_objects("SwitchStmt")
            for switch in switches:
                for case in switch.cases:
                    blocks.append(case.body)
            objects = blocks
        elif scope == "ControlFlowGraph":
            cfgs: List[ControlFlowGraph] = []
            functions, _ = self._collect_scope_objects("Function")
            for fn in functions:
                cfgs.append(fn.cfg)
            objects = cfgs
        elif scope == "BasicBlock":
            blocks: List[BasicBlock] = []
            cfgs, _ = self._collect_scope_objects("ControlFlowGraph")
            for cfg in cfgs:
                blocks.extend(cfg.blocks.values())
            objects = blocks
        else:
            known = False

        if not known:
            self._unknown_scopes.add(scope)

        self._scope_cache[scope] = objects
        return objects, known

    def _maybe_report_unknown_scope(self, rule: Rule, scope_name: Optional[str] = None) -> None:
        """
        Emit a single warning per unknown scope to help authors debug rules.
        """
        scope = scope_name or rule.scope
        if scope in self._unknown_scope_reported:
            return

        sys.stderr.write(
            f"[nomic] Unknown rule scope '{scope}' referenced by rule '{rule.id}'.\n"
        )
        self._unknown_scope_reported.add(scope)

    def _notify_dsl_unimplemented(self, rule: Rule, emitted: bool = True) -> None:
        """
        Inform the user once that the DSL interpreter currently supports only
        simple Pythonic expressions.
        """
        if self._dsl_notice_emitted:
            return
        sys.stderr.write(
            "[nomic] Rule evaluation currently supports simple Python-style "
            "expressions for select/assert/exceptions; complex DSL features "
            f"are not yet implemented (rule '{rule.id}').\n"
        )
        self._dsl_notice_emitted = True


# ============================================================
# ==================== YAML RULE LOADING =====================
# ============================================================

def load_rules_from_yaml(yaml_paths: List[str]) -> List[Rule]:
    """
    Load Rule objects from YAML rule files.

    Behavior:
    * Supports both list-based YAML streams and objects containing a `rules` list.
    * Keeps PyYAML optional; missing dependency results in a warning plus empty rule set.
    * Validates required fields and logs the offending file/doc when skipping malformed entries.
    """
    if not yaml_paths:
        return []

    if yaml is None:
        sys.stderr.write(
            "[nomic] PyYAML is not installed; skipping rule files.\n"
        )
        return []

    def _to_str_list(value: Any) -> List[str]:
        if value is None:
            return []
        if isinstance(value, list):
            return [str(v) for v in value if v is not None]
        return [str(value)]

    def _normalize_rule_docs(doc: Any) -> List[Dict[str, Any]]:
        if doc is None:
            return []
        if isinstance(doc, list):
            return [item for item in doc if isinstance(item, dict)]
        if isinstance(doc, dict):
            if isinstance(doc.get("rules"), list):
                return [item for item in doc["rules"] if isinstance(item, dict)]
            return [doc]
        return []

    def _build_rule(raw_rule: Dict[str, Any], origin: str) -> Optional[Rule]:
        select_expr = raw_rule.get("select")
        assert_expr = raw_rule.get("assert_code", raw_rule.get("assert"))
        required_fields = {
            "id": raw_rule.get("id"),
            "severity": raw_rule.get("severity"),
            "scope": raw_rule.get("scope"),
            "select": select_expr,
            "assert_code": assert_expr,
            "message": raw_rule.get("message"),
        }
        missing = [name for name, value in required_fields.items() if value in (None, "")]
        if missing:
            sys.stderr.write(
                f"[nomic] Skipping rule from {origin}: missing required field(s) {missing}.\n"
            )
            return None

        return Rule(
            id=str(required_fields["id"]),
            description=str(raw_rule.get("description", "")),
            severity=str(required_fields["severity"]),
            scope=str(required_fields["scope"]),
            select=str(select_expr),
            assert_code=str(assert_expr),
            message=str(required_fields["message"]),
            exceptions=_to_str_list(raw_rule.get("exceptions")),
            tags=_to_str_list(raw_rule.get("tags")),
            fixit=str(raw_rule["fixit"]) if raw_rule.get("fixit") is not None else None,
        )

    rules: List[Rule] = []
    for path in yaml_paths:
        try:
            with open(path, "r", encoding="utf-8") as handle:
                documents = list(yaml.safe_load_all(handle))
        except FileNotFoundError:
            sys.stderr.write(f"[nomic] Rule file not found: {path}\n")
            continue
        except OSError as exc:
            sys.stderr.write(f"[nomic] Could not read rule file {path}: {exc}\n")
            continue

        for doc_index, doc in enumerate(documents):
            for raw_rule in _normalize_rule_docs(doc):
                origin = f"{path}#doc{doc_index + 1}"
                rule = _build_rule(raw_rule, origin)
                if rule:
                    rules.append(rule)

    return rules


# ============================================================
# ================= PROJECT BUILD PIPELINE ===================
# ============================================================

_CLANG_MISSING_WARNED = False


def build_translation_unit_from_clang(path: str) -> TranslationUnit:
    """
    Parse a C file with libclang (when available) and hydrate the core IR.

    Falls back to a stub (empty) translation unit if libclang is unavailable or the
    file cannot be parsed so the CLI remains functional in constrained environments.
    """
    canonical_path = os.path.abspath(path)

    if clang_cindex is None:
        _warn_once_clang_missing()
        return _build_stub_translation_unit(path)

    if not os.path.exists(canonical_path):
        sys.stderr.write(f"[nomic] Input file not found: {path}\n")
        return _build_stub_translation_unit(path)

    try:
        index = clang_cindex.Index.create()
    except Exception as exc:  # pragma: no cover - libclang internal failure
        sys.stderr.write(f"[nomic] Failed to initialize libclang: {exc}\n")
        return _build_stub_translation_unit(path)

    args = _default_clang_args()
    options = _clang_parse_options()

    try:
        clang_tu = index.parse(canonical_path, args=args, options=options)
    except Exception as exc:
        sys.stderr.write(
            f"[nomic] libclang could not parse '{path}': {exc}\n"
        )
        return _build_stub_translation_unit(path)

    return _translate_clang_tu(clang_tu, canonical_path, path)


def stitch_project_db(tus: List[TranslationUnit]) -> ProjectDB:
    """
    Build ProjectDB indices from TranslationUnits.
    """
    db = ProjectDB(translation_units=tus)

    # Index functions
    for tu in tus:
        for fn in tu.functions:
            db.functions_by_name.setdefault(fn.name, []).append(fn)
            # call graph approximation:
            callerset = db.call_graph.setdefault(fn.name, set())
            for call in fn.calls:
                target = call.callee_symbol or call.callee_name
                if target:
                    callerset.add(target)

    # Index globals
    for tu in tus:
        for g in tu.globals:
            db.globals_by_name.setdefault(g.name, []).append(g)

    for tu in tus:
        variable_iterables = [tu.globals]
        for fn in tu.functions:
            variable_iterables.append(fn.parameters)
            variable_iterables.append(fn.local_vars)
        for collection in variable_iterables:
            for var in collection:
                if var.writes and not var.is_initialized:
                    var.is_initialized = True

    # Index macros
    for tu in tus:
        for m in tu.macros:
            db.macros_by_name.setdefault(m.name, []).append(m)

    # Includes
    for tu in tus:
        for header in tu.includes:
            db.include_usage.setdefault(header, []).append(tu.path)

    # You can pre-seed policy metadata here if desired.
    # Example: known blocking APIs, etc.
    # db.function_metadata["sleep_ms"] = {"is_blocking_api": True}

    reverse_graph: Dict[str, Set[str]] = {}
    all_nodes: Set[str] = set(db.call_graph.keys())
    for callees in db.call_graph.values():
        all_nodes.update(callees)
    for node in all_nodes:
        reverse_graph.setdefault(node, set())
    for caller, callees in db.call_graph.items():
        for callee in callees:
            reverse_graph.setdefault(callee, set()).add(caller)
    dominators = _compute_call_graph_dominators(db.call_graph)
    for name, functions in db.functions_by_name.items():
        callers = reverse_graph.get(name, set())
        dominated = {node for node, domset in dominators.items() if name in domset and node != name}
        for fn in functions:
            fn.update_called_by(callers)
            fn.update_dominates(dominated)

    return db


def _compute_call_graph_dominators(graph: Dict[str, Set[str]]) -> Dict[str, Set[str]]:
    nodes: Set[str] = set(graph.keys())
    for targets in graph.values():
        nodes.update(targets)
    if not nodes:
        return {}
    predecessors: Dict[str, Set[str]] = {node: set() for node in nodes}
    for caller, callees in graph.items():
        for callee in callees:
            predecessors.setdefault(callee, set()).add(caller)
        predecessors.setdefault(caller, predecessors.get(caller, set()))
    entry_nodes = {node for node in nodes if not predecessors.get(node)}
    if not entry_nodes:
        entry_nodes = set(nodes)
    dominators: Dict[str, Set[str]] = {node: set(nodes) for node in nodes}
    for entry in entry_nodes:
        dominators[entry] = {entry}
    changed = True
    while changed:
        changed = False
        for node in nodes:
            if node in entry_nodes:
                continue
            preds = predecessors.get(node, set())
            if preds:
                intersect = set(nodes)
                for pred in preds:
                    intersect &= dominators.get(pred, set(nodes))
            else:
                intersect = set(nodes)
            intersect.add(node)
            if intersect != dominators[node]:
                dominators[node] = intersect
                changed = True
    return dominators


def _warn_once_clang_missing() -> None:
    global _CLANG_MISSING_WARNED
    if _CLANG_MISSING_WARNED:
        return
    sys.stderr.write(
        "[nomic] clang.cindex is not available; using stub parser.\n"
    )
    _CLANG_MISSING_WARNED = True


def _default_clang_args() -> List[str]:
    """
    Determine the clang arguments to use. Users can append additional flags
    via the NOMIC_CLANG_ARGS environment variable.
    """
    base = ["-x", "c", "-std=c11"]
    extra = os.environ.get("NOMIC_CLANG_ARGS")
    if extra:
        base.extend(shlex.split(extra))
    return base


def _clang_parse_options() -> int:
    """
    Choose libclang parse options that surface macros and preprocessing info.
    """
    options = 0
    if clang_cindex is None:
        return options
    options |= clang_cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD
    options |= clang_cindex.TranslationUnit.PARSE_INCLUDE_BRIEF_COMMENTS_IN_CODE_COMPLETION
    return options


def _build_stub_translation_unit(
    path: str,
    *,
    includes: Optional[List[str]] = None,
    macros: Optional[List[MacroDefinition]] = None,
    types: Optional[List[TypeDecl]] = None,
    globals_list: Optional[List[Variable]] = None,
    functions: Optional[List[Function]] = None,
    active_defines: Optional[Set[str]] = None,
) -> TranslationUnit:
    globals_list = list(globals_list or [])
    symbol_table = {var.name: var for var in globals_list if var.name}
    return TranslationUnit(
        path=path,
        includes=list(includes or []),
        macros=list(macros or []),
        types=list(types or []),
        globals=globals_list,
        functions=list(functions or []),
        symbol_table=symbol_table,
        active_defines=set(active_defines or set()),
        module=None,
    )


def _translate_clang_tu(
    clang_tu: "clang_cindex.TranslationUnit",
    canonical_main: str,
    display_path: str,
) -> TranslationUnit:
    includes = _collect_includes(clang_tu)
    functions, globals_list, macros, types = _collect_ir_nodes(clang_tu, canonical_main)
    symbol_table = {var.name: var for var in globals_list if var.name}
    active_defines = {macro.name for macro in macros if macro.name}
    return TranslationUnit(
        path=display_path,
        includes=includes,
        macros=macros,
        types=types,
        globals=globals_list,
        functions=functions,
        symbol_table=symbol_table,
        active_defines=active_defines,
        module=None,
    )


def _collect_includes(clang_tu: "clang_cindex.TranslationUnit") -> List[str]:
    includes: List[str] = []
    seen: Set[str] = set()
    for inc in clang_tu.get_includes():
        if inc.include is None:
            continue
        inc_path = os.path.normpath(inc.include.name)
        if inc_path not in seen:
            includes.append(inc_path)
            seen.add(inc_path)
    return includes


def _enter_control_construct(function: Optional[Function], control_depth: int) -> int:
    """
    Track nesting/cognitive complexity when stepping into a control construct.
    Returns the incremented depth to use for nested children.
    """
    if function is None:
        return control_depth
    new_depth = control_depth + 1
    if new_depth > function.nesting_depth:
        function.nesting_depth = new_depth
    function.cognitive_complexity += 1 + max(control_depth, 0)
    return new_depth


def _collect_ir_nodes(
    clang_tu: "clang_cindex.TranslationUnit",
    canonical_main: str,
) -> Tuple[List[Function], List[Variable], List[MacroDefinition], List[TypeDecl]]:
    """Walk the clang AST recursively, populating IR lists and wiring CFG metadata."""
    functions: List[Function] = []
    globals_list: List[Variable] = []
    macros: List[MacroDefinition] = []
    types: List[TypeDecl] = []

    def visit(
        cursor: "clang_cindex.Cursor",
        current_function: Optional[Function],
        control_depth: int = 0,
    ) -> None:
        # Always traverse the TU root even if it has no explicit location.
        if cursor.kind != clang_cindex.CursorKind.TRANSLATION_UNIT:
            if not _cursor_in_main_file(cursor, canonical_main):
                return

        next_function = current_function

        if cursor.kind == clang_cindex.CursorKind.FUNCTION_DECL and cursor.is_definition():
            fn = _function_from_cursor(cursor, canonical_main)
            functions.append(fn)
            next_function = fn
        elif (
            cursor.kind == clang_cindex.CursorKind.VAR_DECL
            and cursor.semantic_parent
            and cursor.semantic_parent.kind == clang_cindex.CursorKind.TRANSLATION_UNIT
        ):
            globals_list.append(_global_from_cursor(cursor, canonical_main))
        elif cursor.kind == clang_cindex.CursorKind.MACRO_DEFINITION:
            macro = _macro_from_cursor(cursor, canonical_main)
            if macro:
                macros.append(macro)
        elif cursor.kind in (
            clang_cindex.CursorKind.TYPEDEF_DECL,
            clang_cindex.CursorKind.STRUCT_DECL,
            clang_cindex.CursorKind.UNION_DECL,
            clang_cindex.CursorKind.ENUM_DECL,
        ):
            type_decl = _type_decl_from_cursor(cursor, canonical_main)
            if type_decl:
                types.append(type_decl)
        elif cursor.kind == clang_cindex.CursorKind.CALL_EXPR and current_function is not None:
            callsite = _callsite_from_cursor(cursor, canonical_main)
            if callsite:
                current_function.calls.append(callsite)
                _append_call_to_cfg(current_function, callsite)
        elif cursor.kind == clang_cindex.CursorKind.IF_STMT and current_function is not None:
            branch_depth = _enter_control_construct(current_function, control_depth)
            if_stmt = _if_stmt_from_cursor(cursor, canonical_main, current_function.name)
            current_function.if_stmts.append(if_stmt)
            condition_block = _record_basic_block(
                current_function,
                f"If: {if_stmt.condition}",
                cursor,
                canonical_main,
                enclosing_construct="if_stmt",
                branch_condition=if_stmt.condition,
            )
            then_child = _find_child_of_kind(cursor, clang_cindex.CursorKind.COMPOUND_STMT, occurrence=0)
            else_child = _find_child_of_kind(cursor, clang_cindex.CursorKind.COMPOUND_STMT, occurrence=1)
            else_if_child = None
            if else_child is None:
                else_if_child = _find_child_of_kind(cursor, clang_cindex.CursorKind.IF_STMT, occurrence=0)
            branch_children: List["clang_cindex.Cursor"] = []
            if then_child is not None:
                branch_children.append(then_child)
            if else_child is not None:
                branch_children.append(else_child)
            elif else_if_child is not None:
                branch_children.append(else_if_child)

            branch_exit_blocks: List[int] = []
            if branch_children:
                for child in branch_children:
                    with _push_pending_predecessors(current_function, [condition_block.block_id]):
                        visit(child, next_function, branch_depth)
                        exit_id = _current_block_id(current_function)
                        if exit_id is not None:
                            branch_exit_blocks.append(exit_id)
            else:
                branch_exit_blocks.append(condition_block.block_id)

            if else_child is None and else_if_child is None and condition_block.block_id not in branch_exit_blocks:
                branch_exit_blocks.append(condition_block.block_id)

            _set_pending_predecessors(current_function, branch_exit_blocks or [condition_block.block_id])
        elif cursor.kind in (
            clang_cindex.CursorKind.FOR_STMT,
            clang_cindex.CursorKind.WHILE_STMT,
            clang_cindex.CursorKind.DO_STMT,
        ) and current_function is not None:
            loop_depth = _enter_control_construct(current_function, control_depth)
            loop_stmt = _loop_stmt_from_cursor(cursor, canonical_main, current_function.name)
            current_function.loops.append(loop_stmt)
            with _push_loop_context(current_function, loop_stmt):
                loop_block = _record_basic_block(
                    current_function,
                    f"Loop: {loop_stmt.kind}",
                    cursor,
                    canonical_main,
                    enclosing_construct="loop_stmt",
                    branch_condition=loop_stmt.condition,
                )
                loop_block.is_loop_header = True
                loop_block.parent_loop = loop_stmt
                body_child = _find_child_of_kind(cursor, clang_cindex.CursorKind.COMPOUND_STMT, occurrence=0) or cursor
                with _push_pending_predecessors(current_function, [loop_block.block_id]):
                    visit(body_child, next_function, loop_depth)
                    exit_id = _current_block_id(current_function)
                    if exit_id is not None:
                        exit_block = current_function.cfg.blocks.get(exit_id)
                        if exit_block is not None:
                            exit_block.is_loop_exit = True
                            exit_block.parent_loop = loop_stmt
                        _connect_blocks(current_function, exit_id, loop_block.block_id)
                _set_pending_predecessors(current_function, [loop_block.block_id])
        elif cursor.kind == clang_cindex.CursorKind.SWITCH_STMT and current_function is not None:
            switch_depth = _enter_control_construct(current_function, control_depth)
            switch_stmt = _switch_stmt_from_cursor(cursor, canonical_main, current_function.name)
            current_function.switches.append(switch_stmt)
            switch_block = _record_basic_block(
                current_function,
                "switch",
                cursor,
                canonical_main,
                enclosing_construct="switch_stmt",
                branch_condition=switch_stmt.control_expr,
            )
            case_children = [
                child
                for child in cursor.get_children()
                if child.kind in (clang_cindex.CursorKind.CASE_STMT, clang_cindex.CursorKind.DEFAULT_STMT)
            ]
            case_exits: List[int] = []
            if case_children:
                for case_child in case_children:
                    with _push_pending_predecessors(current_function, [switch_block.block_id]):
                        visit(case_child, next_function, switch_depth)
                        exit_id = _current_block_id(current_function)
                        if exit_id is not None:
                            case_exits.append(exit_id)
            else:
                case_exits.append(switch_block.block_id)
            _set_pending_predecessors(current_function, case_exits or [switch_block.block_id])

        if current_function is not None and clang_cindex is not None:
            if cursor.kind in _STATEMENT_CURSOR_KINDS:
                _record_statement_in_block(current_function, cursor, canonical_main)
            if cursor.kind == clang_cindex.CursorKind.BINARY_OPERATOR and _looks_like_assignment(cursor):
                block = _current_block(current_function)
                if block is not None:
                    _record_write_site(block, cursor, canonical_main)
            if cursor.kind == clang_cindex.CursorKind.DECL_REF_EXPR:
                block = _current_block(current_function)
                if block is not None:
                    block.reads.append(_record_read_site(cursor, canonical_main))

        try:
            children = list(cursor.get_children())
        except Exception:
            children = []
        for child in children:
            visit(child, next_function, control_depth)

    visit(clang_tu.cursor, None, 0)
    for fn in functions:
        _finalize_function_cfg(fn)
    return functions, globals_list, macros, types


def _cursor_in_main_file(cursor: "clang_cindex.Cursor", canonical_main: str) -> bool:
    loc = cursor.location
    if loc is None or loc.file is None:
        return False
    return os.path.abspath(loc.file.name) == canonical_main


def _make_source_location(
    location: "clang_cindex.SourceLocation",
    fallback_path: str,
) -> SourceLocation:
    file_name = location.file.name if location.file is not None else fallback_path
    return SourceLocation(file=file_name, line=location.line, column=location.column)


def _make_source_range(
    extent: "clang_cindex.SourceRange",
    fallback_path: str,
) -> SourceRange:
    start = extent.start
    end = extent.end
    return SourceRange(
        file=start.file.name if start.file is not None else fallback_path,
        line_start=start.line,
        col_start=start.column,
        line_end=end.line,
        col_end=end.column,
    )


def _cursor_attributes(cursor: "clang_cindex.Cursor") -> List[str]:
    getter = getattr(cursor, "get_attributes", None)
    if getter is None:
        return []
    try:
        return [attr.spelling for attr in getter()]
    except Exception:
        return []


def _storage_class_to_variable_storage(
    storage_class: "clang_cindex.StorageClass",
) -> str:
    if clang_cindex is None:
        return "unknown"
    mapping = {
        clang_cindex.StorageClass.INVALID: "unknown",
        clang_cindex.StorageClass.AUTO: "auto",
        clang_cindex.StorageClass.STATIC: "static",
        clang_cindex.StorageClass.EXTERN: "extern",
        clang_cindex.StorageClass.REGISTER: "register",
        clang_cindex.StorageClass.NONE: "unknown",
    }
    return mapping.get(storage_class, "unknown")


def _linkage_to_string(linkage: "clang_cindex.LinkageKind") -> str:
    if clang_cindex is None:
        return "external"
    kind = linkage
    lk = clang_cindex.LinkageKind
    none_kind = getattr(lk, "NONE", getattr(lk, "NO_LINKAGE", None))
    unique_external = getattr(lk, "UNIQUE_EXTERNAL", None)
    if kind == lk.INTERNAL:
        return "internal"
    if unique_external is not None and kind == unique_external:
        return "internal"
    if none_kind is not None and kind == none_kind:
        return "none"
    return "external"


def _function_from_cursor(
    cursor: "clang_cindex.Cursor",
    canonical_main: str,
) -> Function:
    source_range = _make_source_range(cursor.extent, canonical_main)
    storage = _storage_class_to_variable_storage(cursor.storage_class)
    linkage = "internal" if storage == "static" else _linkage_to_string(cursor.linkage)
    storage_class = storage if storage != "unknown" else None

    parameters: List[Variable] = []
    try:
        args = list(cursor.get_arguments())
    except Exception:
        args = []
    for index, arg in enumerate(args):
        parameters.append(_param_variable_from_cursor(arg, canonical_main, cursor.spelling or f"fn_{id(cursor)}", index))

    attributes = _cursor_attributes(cursor)
    inline_hint = bool(getattr(cursor, "is_inline_function", lambda: False)())
    interrupt_attr = any(
        token in attr.lower()
        for attr in attributes
        for token in ("interrupt", "isr", "irq")
    )

    function = Function(
        name=cursor.spelling or cursor.displayname or "<anonymous>",
        return_type=cursor.result_type.spelling if cursor.result_type else "void",
        parameters=parameters,
        local_vars=[],
        linkage=linkage,
        storage_class=storage_class,
        attributes=attributes,
        annotations=[],
        inline_hint=inline_hint,
        isr_hint=interrupt_attr,
        calls=[],
        cfg=ControlFlowGraph(),
        exit_points=[],
        if_stmts=[],
        loops=[],
        switches=[],
        globals_written=[],
        globals_read=[],
        source_range=source_range,
        preprocessor=PreprocessorContext(),
        prefix=None,
        suffix=None,
    )
    _initialize_function_cfg(function)
    return function


def _param_variable_from_cursor(
    cursor: "clang_cindex.Cursor",
    canonical_main: str,
    function_name: str,
    ordinal: int,
) -> Variable:
    source_range = _make_source_range(cursor.extent, canonical_main)
    name = cursor.spelling or cursor.displayname or f"param_{ordinal}"
    return Variable(
        name=name,
        ctype=cursor.type.spelling if cursor.type else "",
        storage="auto",
        linkage="none",
        is_const=_type_is_const(cursor.type),
        is_volatile=_type_is_volatile(cursor.type),
        is_atomic=_type_is_atomic(cursor.type),
        scope="param",
        decl_function=function_name,
        prefix=None,
        suffix=None,
        source_range=source_range,
        preprocessor=PreprocessorContext(),
        annotations=[],
        writes=[],
        reads=[],
    )


def _global_from_cursor(
    cursor: "clang_cindex.Cursor",
    canonical_main: str,
) -> Variable:
    source_range = _make_source_range(cursor.extent, canonical_main)
    storage = _storage_class_to_variable_storage(cursor.storage_class)
    linkage = _linkage_to_string(cursor.linkage)
    return Variable(
        name=cursor.spelling or cursor.displayname or "<anonymous>",
        ctype=cursor.type.spelling if cursor.type else "",
        storage=storage,
        linkage=linkage,
        is_const=_type_is_const(cursor.type),
        is_volatile=_type_is_volatile(cursor.type),
        is_atomic=_type_is_atomic(cursor.type),
        scope="file",
        decl_function=None,
        prefix=None,
        suffix=None,
        source_range=source_range,
        preprocessor=PreprocessorContext(),
        annotations=[],
        writes=[],
        reads=[],
    )


def _macro_from_cursor(
    cursor: "clang_cindex.Cursor",
    canonical_main: str,
) -> Optional[MacroDefinition]:
    name = cursor.spelling
    if not name:
        return None

    try:
        raw_tokens = [token.spelling for token in cursor.get_tokens()]
    except Exception:
        raw_tokens = []

    tokens = list(raw_tokens)
    # Trim the leading '# define' tokens if present.
    if len(tokens) >= 2 and tokens[0] == "#" and tokens[1] == "define":
        tokens = tokens[2:]

    body_tokens: List[str] = []
    params: List[str] = []
    kind: Literal["object_like", "function_like"] = "object_like"

    if tokens and tokens[0] == name:
        tokens = tokens[1:]

    if tokens and tokens[0] == "(":
        kind = "function_like"
        idx = 1
        current: List[str] = []
        while idx < len(tokens):
            tok = tokens[idx]
            if tok == ")":
                if current:
                    params.append("".join(current))
                    current = []
                idx += 1
                break
            elif tok == ",":
                if current:
                    params.append("".join(current))
                    current = []
            else:
                current.append(tok)
            idx += 1
        body_tokens = tokens[idx:]
    else:
        body_tokens = tokens

    body_statement = " ".join(body_tokens).strip()
    source_range = _make_source_range(cursor.extent, canonical_main)

    return MacroDefinition(
        name=name,
        kind=kind,
        params=params,
        body_tokens=body_tokens,
        body_statements=[body_statement] if body_statement else [],
        is_wrapped_single_stmt=False,
        source_range=source_range,
        preprocessor=PreprocessorContext(),
        annotations=[],
        prefix=None,
        suffix=None,
    )


def _callsite_from_cursor(
    cursor: "clang_cindex.Cursor",
    canonical_main: str,
) -> Optional[CallSite]:
    callee_name = cursor.displayname or cursor.spelling
    callee_symbol = None
    referenced = getattr(cursor, "referenced", None)
    if referenced is not None:
        callee_symbol = referenced.spelling or referenced.displayname
        if not callee_name:
            callee_name = callee_symbol

    args: List[str] = []
    arg_getter = getattr(cursor, "get_arguments", None)
    if arg_getter is not None:
        try:
            for arg in arg_getter():
                args.append(arg.displayname or arg.spelling or "")
        except Exception:
            pass

    if not callee_name:
        callee_name = "<unknown>"

    return CallSite(
        callee_name=callee_name,
        callee_symbol=callee_symbol,
        args=args,
        location=_make_source_range(cursor.extent, canonical_main),
        in_branch=None,
        in_loop=None,
        in_switch=False,
        in_macro_expansion=False,
        is_blocking_api=False,
        is_allocator=False,
        is_lock=False,
        is_unlock=False,
        preprocessor=PreprocessorContext(),
    )


def _type_decl_from_cursor(
    cursor: "clang_cindex.Cursor",
    canonical_main: str,
) -> Optional[TypeDecl]:
    kind_map = {
        clang_cindex.CursorKind.TYPEDEF_DECL: "typedef",
        clang_cindex.CursorKind.STRUCT_DECL: "struct",
        clang_cindex.CursorKind.UNION_DECL: "union",
        clang_cindex.CursorKind.ENUM_DECL: "enum",
    }
    kind = kind_map.get(cursor.kind)
    if kind is None:
        return None

    name = cursor.spelling or cursor.displayname
    if not name:
        # Skip anonymous declarations for now.
        return None

    if cursor.kind != clang_cindex.CursorKind.TYPEDEF_DECL and not cursor.is_definition():
        return None

    fields: List[FieldDecl] = []
    if cursor.kind in (
        clang_cindex.CursorKind.STRUCT_DECL,
        clang_cindex.CursorKind.UNION_DECL,
    ):
        for child in cursor.get_children():
            if child.kind != clang_cindex.CursorKind.FIELD_DECL:
                continue
            field_range = _make_source_range(child.extent, canonical_main)
            is_bitfield = bool(getattr(child, "is_bitfield", lambda: False)())
            bit_width = None
            if is_bitfield:
                try:
                    bit_width = child.get_bitfield_width()
                except Exception:
                    bit_width = None
            fields.append(
                FieldDecl(
                    name=child.spelling or child.displayname or "",
                    ctype=child.type.spelling if child.type else "",
                    is_bitfield=is_bitfield,
                    bit_width=bit_width,
                    source_range=field_range,
                )
            )
    elif cursor.kind == clang_cindex.CursorKind.ENUM_DECL:
        for child in cursor.get_children():
            if child.kind != clang_cindex.CursorKind.ENUM_CONSTANT_DECL:
                continue
            field_range = _make_source_range(child.extent, canonical_main)
            fields.append(
                FieldDecl(
                    name=child.spelling or "",
                    ctype="enum_constant",
                    is_bitfield=False,
                    bit_width=None,
                    source_range=field_range,
                )
            )

    underlying = None
    if cursor.kind == clang_cindex.CursorKind.TYPEDEF_DECL:
        try:
            underlying = cursor.underlying_typedef_type.spelling  # type: ignore[attr-defined]
        except Exception:
            underlying = None

    type_decl = TypeDecl(
        name=name,
        kind=kind,  # type: ignore[arg-type]
        fields=fields,
        underlying_type=underlying,
        attributes=_cursor_attributes(cursor),
        visibility="project_visible",
        source_range=_make_source_range(cursor.extent, canonical_main),
        preprocessor=PreprocessorContext(),
        annotations=[],
    )
    return type_decl


def _initialize_function_cfg(function: Function) -> None:
    """Ensure every function starts with a canonical entry block and CFG state."""
    if function.cfg.blocks:
        return
    entry_block = BasicBlock(
        block_id=0,
        statements=[_statement_from_text("entry")],
        successors=[],
        predecessors=[],
        source_range=function.source_range,
        preprocessor=function.preprocessor,
        function=function,
    )
    function.cfg.blocks[0] = entry_block
    function.cfg.entry_block = entry_block
    function.cfg.exit_blocks = [entry_block]
    function.exit_points = [0]
    setattr(function, "_cfg_state", {"next_block_id": 1, "current_block_id": 0, "loop_stack": []})


def _get_cfg_state(function: Function) -> Dict[str, Any]:
    state = getattr(function, "_cfg_state", None)
    if state is None:
        state = {
            "next_block_id": len(function.cfg.blocks) or 1,
            "current_block_id": function.cfg.entry_block_id or 0,
            "loop_stack": [],
        }
        setattr(function, "_cfg_state", state)
    return state


def _current_block_id(function: Function) -> Optional[int]:
    state = _get_cfg_state(function)
    return state.get("current_block_id")


def _set_pending_predecessors(function: Function, preds: List[int]) -> None:
    state = _get_cfg_state(function)
    preds = list(preds)
    state["pending_predecessors"] = preds
    if len(preds) == 1:
        state["current_block_id"] = preds[0]
    else:
        state["current_block_id"] = None


@contextmanager
def _push_pending_predecessors(function: Function, preds: List[int]):
    state = _get_cfg_state(function)
    previous = list(state.get("pending_predecessors", []))
    state["pending_predecessors"] = list(preds)
    try:
        yield
    finally:
        state["pending_predecessors"] = previous


@contextmanager
def _push_loop_context(function: Function, loop_stmt: LoopStmt):
    state = _get_cfg_state(function)
    stack: List[LoopStmt] = state.setdefault("loop_stack", [])  # type: ignore[assignment]
    stack.append(loop_stmt)
    try:
        yield
    finally:
        stack.pop()


def _connect_blocks(function: Function, from_block_id: Optional[int], to_block_id: Optional[int]) -> None:
    if from_block_id is None or to_block_id is None:
        return
    from_block = function.cfg.blocks.get(from_block_id)
    to_block = function.cfg.blocks.get(to_block_id)
    if from_block is None or to_block is None:
        return
    if to_block_id not in from_block.successors:
        from_block.successors.append(to_block_id)
    if from_block_id not in to_block.predecessors:
        to_block.predecessors.append(from_block_id)


def _record_basic_block(
    function: Function,
    description: str,
    cursor: "clang_cindex.Cursor",
    canonical_main: str,
    *,
    enclosing_construct: Optional[str] = None,
    branch_condition: Optional[str] = None,
) -> BasicBlock:
    """
    Create a new `BasicBlock` anchored to the provided cursor and connect it to
    all pending predecessors. Pending predecessors are established by control
    constructs (if/loop/switch) before visiting descendant cursors.
    """
    state = _get_cfg_state(function)
    block_id = state["next_block_id"]
    state["next_block_id"] += 1

    loop_stack: List[Any] = state.get("loop_stack", [])

    block = BasicBlock(
        block_id=block_id,
        statements=[_statement_from_text(description)],
        enclosing_construct=enclosing_construct,
        branch_condition=branch_condition,
        source_range=_make_source_range(cursor.extent, canonical_main),
        preprocessor=PreprocessorContext(),
        loop_depth=len(loop_stack),
        parent_loop=loop_stack[-1] if loop_stack else None,
        function=function,
    )

    pending = state.get("pending_predecessors", [])
    if pending:
        predecessor_ids = list(pending)
    else:
        current_block_id = state.get("current_block_id")
        predecessor_ids = [current_block_id] if current_block_id is not None else []

    for pred_id in predecessor_ids:
        prev_block = function.cfg.blocks.get(pred_id)
        if prev_block is not None:
            if block_id not in prev_block.successors:
                prev_block.successors.append(block_id)
            if prev_block.block_id not in block.predecessors:
                block.predecessors.append(prev_block.block_id)

    state["pending_predecessors"] = []

    function.cfg.blocks[block_id] = block
    function.cfg.exit_blocks = [block_id]
    function.exit_points = [block_id]
    state["current_block_id"] = block_id
    return block


def _append_call_to_cfg(function: Function, callsite: CallSite) -> None:
    block = _current_block(function)
    if block is None:
        return
    block.calls.append(callsite)
    if block.source_range is None:
        block.source_range = callsite.location


def _current_block(function: Function) -> Optional[BasicBlock]:
    state = _get_cfg_state(function)
    block_id = state.get("current_block_id")
    if block_id is None:
        return None
    return function.cfg.blocks.get(block_id)


def _finalize_function_cfg(function: Function) -> None:
    cfg = function.cfg
    if not cfg.blocks:
        function.cyclomatic_complexity = 1
        return
    if cfg.entry_block_id is None:
        cfg.entry_block = min(cfg.blocks.keys())
    exit_ids = [bid for bid, block in cfg.blocks.items() if not block.successors]
    if not exit_ids:
        exit_ids = [max(cfg.blocks.keys())]
    cfg.exit_blocks = exit_ids
    for bid, block in cfg.blocks.items():
        block.is_exit_block = bid in exit_ids
    function.exit_points = exit_ids

    block_ids = list(cfg.blocks.keys())
    entry = cfg.entry_block_id
    dominators: Dict[int, Set[int]] = {bid: set(block_ids) for bid in block_ids}
    if entry is not None:
        dominators[entry] = {entry}

    changed = True
    while changed:
        changed = False
        for bid in block_ids:
            if bid == entry:
                continue
            block = cfg.blocks[bid]
            preds = block.predecessors
            if preds:
                new_set = set(block_ids)
                for pred in preds:
                    new_set &= dominators.get(pred, set(block_ids))
            else:
                new_set = set(block_ids)
            new_set.add(bid)
            if new_set != dominators[bid]:
                dominators[bid] = new_set
                changed = True

    for bid, doms in dominators.items():
        cfg.blocks[bid].dominators = set(doms)

    postdominators: Dict[int, Set[int]] = {bid: set(block_ids) for bid in block_ids}
    for exit_id in exit_ids:
        postdominators[exit_id] = {exit_id}

    changed = True
    while changed:
        changed = False
        for bid in block_ids:
            if bid in exit_ids:
                continue
            block = cfg.blocks[bid]
            succs = block.successors
            if succs:
                new_set = set(block_ids)
                for succ in succs:
                    new_set &= postdominators.get(succ, set(block_ids))
            else:
                new_set = set(block_ids)
            new_set.add(bid)
            if new_set != postdominators[bid]:
                postdominators[bid] = new_set
                changed = True

    for bid, pdoms in postdominators.items():
        cfg.blocks[bid].postdominators = set(pdoms)

    function.cyclomatic_complexity = _calculate_cyclomatic_complexity(function)


def _calculate_cyclomatic_complexity(function: Function) -> int:
    cfg = function.cfg
    if not cfg.blocks:
        return 1
    edges = sum(len(block.successors) for block in cfg.blocks.values())
    nodes = len(cfg.blocks)
    components = 1 if nodes > 0 else 0
    complexity = edges - nodes + 2 * components
    return complexity if complexity > 0 else 1


def _record_statement_in_block(
    function: Optional[Function],
    cursor: "clang_cindex.Cursor",
    canonical_main: str,
) -> None:
    """
    Attach a textual representation of the cursor to the current block,
    synthesizing an implicit block when statements appear outside of an
    explicit `_record_basic_block` context (rare with unstructured cursors).
    """
    if function is None:
        return
    block = _current_block(function)
    if block is None:
        state = _get_cfg_state(function)
        pending = state.get("pending_predecessors", [])
        if pending:
            block = _record_basic_block(
                function,
                "block",
                cursor,
                canonical_main,
            )
        else:
            return
    snippet = _cursor_snippet(cursor)
    if snippet:
        block.statements.append(_statement_from_text(snippet))


def _record_write_site(
    container: Union[BasicBlock, BlockStmt],
    cursor: "clang_cindex.Cursor",
    canonical_main: str,
) -> None:
    """Append a WriteSite describing the cursor into the provided container."""
    location = _make_source_range(cursor.extent, canonical_main)
    site = WriteSite(
        location=location,
        preprocessor=PreprocessorContext(),
    )
    container.writes.append(site)


def _record_read_site(
    cursor: "clang_cindex.Cursor",
    canonical_main: str,
) -> ReadSite:
    """Return a ReadSite (caller decides whether to attach to block or statement)."""
    location = _make_source_range(cursor.extent, canonical_main)
    return ReadSite(
        location=location,
        preprocessor=PreprocessorContext(),
    )


def _if_stmt_from_cursor(
    cursor: "clang_cindex.Cursor",
    canonical_main: str,
    function_name: str,
) -> IfStmt:
    condition = _cursor_condition_text(cursor)
    then_child = _find_child_of_kind(cursor, clang_cindex.CursorKind.COMPOUND_STMT, occurrence=0)
    else_child = _find_child_of_kind(cursor, clang_cindex.CursorKind.COMPOUND_STMT, occurrence=1)
    else_if_child = None
    if else_child is None:
        else_if_child = _find_child_of_kind(cursor, clang_cindex.CursorKind.IF_STMT, occurrence=0)

    then_block = _block_stmt_from_cursor(then_child or cursor, canonical_main)
    else_block_stmt = None
    if else_child is not None or else_if_child is not None:
        else_block_stmt = _block_stmt_from_cursor((else_child or else_if_child) or cursor, canonical_main)

    return IfStmt(
        condition=condition,
        then_block=then_block,
        else_block=else_block_stmt,
        parent_function=function_name,
        source_range=_make_source_range(cursor.extent, canonical_main),
        preprocessor=PreprocessorContext(),
        annotations=[],
    )


def _loop_stmt_from_cursor(
    cursor: "clang_cindex.Cursor",
    canonical_main: str,
    function_name: str,
) -> LoopStmt:
    if cursor.kind == clang_cindex.CursorKind.FOR_STMT:
        loop_kind: Literal["for", "while", "do_while"] = "for"
    elif cursor.kind == clang_cindex.CursorKind.WHILE_STMT:
        loop_kind = "while"
    else:
        loop_kind = "do_while"

    body_child = _find_child_of_kind(cursor, clang_cindex.CursorKind.COMPOUND_STMT, occurrence=0)
    loop_body = _block_stmt_from_cursor(body_child or cursor, canonical_main)

    return LoopStmt(
        kind=loop_kind,
        condition=_cursor_condition_text(cursor),
        body=loop_body,
        parent_function=function_name,
        source_range=_make_source_range(cursor.extent, canonical_main),
        preprocessor=PreprocessorContext(),
        annotations=[],
    )


def _switch_stmt_from_cursor(
    cursor: "clang_cindex.Cursor",
    canonical_main: str,
    function_name: str,
) -> SwitchStmt:
    cases: List[SwitchCaseBlock] = []
    has_default = False

    for child in cursor.get_children():
        if child.kind == clang_cindex.CursorKind.CASE_STMT:
            labels = [_cursor_condition_text(child)]
            body_child = _find_child_of_kind(child, clang_cindex.CursorKind.COMPOUND_STMT, occurrence=0)
            cases.append(
                SwitchCaseBlock(
                    labels=labels,
                    body=_block_stmt_from_cursor(body_child or child, canonical_main),
                    source_range=_make_source_range(child.extent, canonical_main),
                )
            )
        elif child.kind == clang_cindex.CursorKind.DEFAULT_STMT:
            has_default = True
            body_child = _find_child_of_kind(child, clang_cindex.CursorKind.COMPOUND_STMT, occurrence=0)
            cases.append(
                SwitchCaseBlock(
                    labels=["default"],
                    body=_block_stmt_from_cursor(body_child or child, canonical_main),
                    source_range=_make_source_range(child.extent, canonical_main),
                )
            )

    return SwitchStmt(
        control_expr=_cursor_condition_text(cursor),
        cases=cases,
        has_default=has_default,
        parent_function=function_name,
        source_range=_make_source_range(cursor.extent, canonical_main),
        preprocessor=PreprocessorContext(),
        annotations=[],
    )


def _block_stmt_from_cursor(
    cursor: Optional["clang_cindex.Cursor"],
    canonical_main: str,
) -> BlockStmt:
    if cursor is None:
        return BlockStmt(statements=[], source_range=None, preprocessor=PreprocessorContext())
    statements: List[Statement] = []
    writes: List[WriteSite] = []
    calls: List[CallSite] = []
    reads: List[ReadSite] = []
    for child in cursor.get_children():
        snippet = _cursor_snippet(child)
        if snippet:
            statements.append(_statement_from_text(snippet))
        if child.kind == clang_cindex.CursorKind.CALL_EXPR:
            call = _callsite_from_cursor(child, canonical_main)
            if call:
                calls.append(call)
        elif child.kind == clang_cindex.CursorKind.DECL_REF_EXPR:
            reads.append(_record_read_site(child, canonical_main))
        elif child.kind == clang_cindex.CursorKind.BINARY_OPERATOR and _looks_like_assignment(child):
            writes.append(
                WriteSite(
                    location=_make_source_range(child.extent, canonical_main),
                    preprocessor=PreprocessorContext(),
                )
            )
    return BlockStmt(
        statements=statements,
        writes=writes,
        reads=reads,
        calls=calls,
        source_range=_make_source_range(cursor.extent, canonical_main),
        preprocessor=PreprocessorContext(),
    )


def _find_child_of_kind(
    cursor: "clang_cindex.Cursor",
    kind: "clang_cindex.CursorKind",
    *,
    occurrence: int = 0,
) -> Optional["clang_cindex.Cursor"]:
    matches = [child for child in cursor.get_children() if child.kind == kind]
    if len(matches) > occurrence:
        return matches[occurrence]
    return None


def _cursor_condition_text(cursor: "clang_cindex.Cursor") -> str:
    text = cursor.displayname or cursor.spelling
    if text:
        return text
    try:
        tokens = [t.spelling for t in cursor.get_tokens()]
        if tokens:
            return " ".join(tokens[:32])
    except Exception:
        pass
    return "<expr>"


def _cursor_snippet(cursor: "clang_cindex.Cursor", token_limit: int = 32) -> str:
    text = cursor.displayname or cursor.spelling
    if text:
        return text.strip()
    try:
        tokens = [t.spelling for t in cursor.get_tokens()]
        if tokens:
            return " ".join(tokens[:token_limit]).strip()
    except Exception:
        pass
    return cursor.kind.name


def _looks_like_assignment(cursor: "clang_cindex.Cursor") -> bool:
    try:
        tokens = [t.spelling for t in cursor.get_tokens()]
    except Exception:
        return False
    joined = " ".join(tokens)
    if "=" not in joined:
        return False
    if "==" in joined or "!=" in joined or ">=" in joined or "<=" in joined:
        return False
    return True
def _type_is_const(ctype: Optional["clang_cindex.Type"]) -> bool:
    if ctype is None:
        return False
    checker = getattr(ctype, "is_const_qualified", None)
    if checker is None:
        return False
    try:
        return bool(checker())
    except Exception:
        return False


def _type_is_volatile(ctype: Optional["clang_cindex.Type"]) -> bool:
    if ctype is None:
        return False
    checker = getattr(ctype, "is_volatile_qualified", None)
    if checker is None:
        return False
    try:
        return bool(checker())
    except Exception:
        return False


def _type_is_atomic(ctype: Optional["clang_cindex.Type"]) -> bool:
    if ctype is None:
        return False
    checker = getattr(ctype, "is_atomic_qualified", None)
    if checker is None:
        return False
    try:
        return bool(checker())
    except Exception:
        return False


# ============================================================
# ==================== VIOLATION OUTPUT ======================
# ============================================================

def violation_to_json_obj(v: Violation) -> Dict[str, Any]:
    """
    Convert a Violation dataclass into a JSON-friendly dict.
    We'll keep this explicit so we can guarantee stable field order over time if needed.
    """
    return {
        "rule_id": v.rule_id,
        "severity": v.severity,
        "message": v.message,
        "location": v.location,
        "context": v.context,
        "suggested_fix": v.suggested_fix,
        "extras": v.extras,
        "tool": "Nomic",
        "version": NOMIC_VERSION,
    }


def emit_violations_json(violations: List[Violation], out: Optional[str] = None) -> None:
    """
    Serialize all violations to JSON (list of violation objects).
    """
    as_json = [violation_to_json_obj(v) for v in violations]
    text = json.dumps(as_json, indent=2, sort_keys=False)
    if out:
        with open(out, "w", encoding="utf-8") as f:
            f.write(text + "\n")
    else:
        print(text)


def emit_violations_text(violations: List[Violation], out: Optional[str] = None, context: int = 0) -> None:
    """
    Output violations in human-readable text format.
    """
    lines = []

    if not violations:
        lines.append("No violations found.")
    else:
        # Group violations by file
        by_file: Dict[str, List[Violation]] = {}
        for v in violations:
            file_path = v.location.file if hasattr(v, 'location') else 'unknown'
            if file_path not in by_file:
                by_file[file_path] = []
            by_file[file_path].append(v)

        for file_path, file_violations in sorted(by_file.items()):
            lines.append(f"\n{file_path}:")
            lines.append("-" * len(file_path))

            for v in sorted(file_violations, key=lambda x: (x.location.line if hasattr(x.location, 'line') else 0)):
                location = f"  Line {v.location.line}" if hasattr(v.location, 'line') else "  Unknown location"
                if hasattr(v.location, 'column'):
                    location += f", Col {v.location.column}"

                lines.append(f"{location}: [{v.rule_id}] {v.message}")

                # Add context lines if requested
                if context > 0 and hasattr(v, 'location') and hasattr(v.location, 'line'):
                    # This would need actual file reading implementation
                    pass

    text = "\n".join(lines)
    if out:
        with open(out, "w", encoding="utf-8") as f:
            f.write(text + "\n")
    else:
        print(text)


def emit_violations_gcc(violations: List[Violation], out: Optional[str] = None) -> None:
    """
    Output violations in GCC-style format for IDE integration.
    Format: filename:line:column: severity: message [rule_id]
    """
    lines = []

    for v in violations:
        # Build GCC-style diagnostic line
        parts = []

        # File location
        if hasattr(v, 'location'):
            parts.append(v.location.file if hasattr(v.location, 'file') else 'unknown')
            parts.append(str(v.location.line) if hasattr(v.location, 'line') else '0')
            parts.append(str(v.location.column) if hasattr(v.location, 'column') else '0')
        else:
            parts.extend(['unknown', '0', '0'])

        # Severity (would need to be added to Violation class)
        severity = getattr(v, 'severity', 'warning')

        # Message with rule ID
        message = f"{severity}: {v.message} [{v.rule_id}]"

        lines.append(":".join(parts[:3]) + ": " + message)

    text = "\n".join(lines)
    if out:
        with open(out, "w", encoding="utf-8") as f:
            f.write(text + "\n")
    else:
        print(text)


def emit_violations_xml(violations: List[Violation], out: Optional[str] = None) -> None:
    """
    Output violations in XML format.
    """
    lines = ['<?xml version="1.0" encoding="UTF-8"?>']
    lines.append('<violations>')

    for v in violations:
        lines.append('  <violation>')
        lines.append(f'    <rule_id>{v.rule_id}</rule_id>')
        lines.append(f'    <message>{v.message}</message>')

        if hasattr(v, 'location'):
            lines.append('    <location>')
            if hasattr(v.location, 'file'):
                lines.append(f'      <file>{v.location.file}</file>')
            if hasattr(v.location, 'line'):
                lines.append(f'      <line>{v.location.line}</line>')
            if hasattr(v.location, 'column'):
                lines.append(f'      <column>{v.location.column}</column>')
            lines.append('    </location>')

        lines.append('  </violation>')

    lines.append('</violations>')

    text = "\n".join(lines)
    if out:
        with open(out, "w", encoding="utf-8") as f:
            f.write(text + "\n")
    else:
        print(text)


def emit_violations_sarif(violations: List[Violation], out: Optional[str] = None) -> None:
    """
    Output violations in SARIF (Static Analysis Results Interchange Format).
    """
    sarif = {
        "$schema": "https://schemastore.azurewebsites.net/schemas/json/sarif-2.1.0-rtm.5.json",
        "version": "2.1.0",
        "runs": [
            {
                "tool": {
                    "driver": {
                        "name": NOMIC_PRODUCT_NAME,
                        "version": NOMIC_VERSION,
                        "informationUri": "https://github.com/HeliOS/nomic",
                        "rules": []
                    }
                },
                "results": []
            }
        ]
    }

    # Build rule set
    rule_ids = set(v.rule_id for v in violations)
    for rule_id in sorted(rule_ids):
        sarif["runs"][0]["tool"]["driver"]["rules"].append({
            "id": rule_id,
            "name": rule_id,
            "shortDescription": {"text": f"Rule {rule_id}"}
        })

    # Add results
    for v in violations:
        result = {
            "ruleId": v.rule_id,
            "message": {"text": v.message},
            "locations": []
        }

        if hasattr(v, 'location'):
            location = {
                "physicalLocation": {
                    "artifactLocation": {
                        "uri": v.location.file if hasattr(v.location, 'file') else 'unknown'
                    }
                }
            }

            if hasattr(v.location, 'line'):
                location["physicalLocation"]["region"] = {
                    "startLine": v.location.line,
                    "startColumn": getattr(v.location, 'column', 1)
                }

            result["locations"].append(location)

        sarif["runs"][0]["results"].append(result)

    text = json.dumps(sarif, indent=2)
    if out:
        with open(out, "w", encoding="utf-8") as f:
            f.write(text + "\n")
    else:
        print(text)


# ============================================================
# ==================== EXIT CODES ============================
# ============================================================

# GNU/Linux standard exit codes
EXIT_SUCCESS = 0
EXIT_FAILURE = 1
EXIT_USAGE = 2
EXIT_DATAERR = 65
EXIT_NOINPUT = 66
EXIT_SOFTWARE = 70
EXIT_OSERR = 71
EXIT_OSFILE = 72
EXIT_CANTCREAT = 73
EXIT_IOERR = 74
EXIT_CONFIG = 78

# Application-specific exit codes
EXIT_VIOLATIONS_FOUND = 3
EXIT_PARSE_ERROR = 4
EXIT_RULE_ERROR = 5


# ============================================================
# =================== OUTPUT CONTROL =========================
# ============================================================

class OutputManager:
    """Manages output verbosity and formatting."""

    QUIET = 0
    NORMAL = 1
    VERBOSE = 2
    DEBUG = 3

    def __init__(self, verbosity: int = NORMAL, use_color: bool = True):
        self.verbosity = verbosity
        self.use_color = use_color and sys.stdout.isatty()

    def info(self, message: str) -> None:
        """Print informational message to stdout."""
        if self.verbosity >= self.NORMAL:
            print(message)

    def verbose(self, message: str) -> None:
        """Print verbose message to stdout."""
        if self.verbosity >= self.VERBOSE:
            print(message)

    def debug(self, message: str) -> None:
        """Print debug message to stderr."""
        if self.verbosity >= self.DEBUG:
            print(f"DEBUG: {message}", file=sys.stderr)

    def warning(self, message: str) -> None:
        """Print warning message to stderr."""
        if self.verbosity >= self.QUIET:
            prefix = "\033[33mWARNING:\033[0m " if self.use_color else "WARNING: "
            print(f"{prefix}{message}", file=sys.stderr)

    def error(self, message: str) -> None:
        """Print error message to stderr."""
        prefix = "\033[31mERROR:\033[0m " if self.use_color else "ERROR: "
        print(f"{prefix}{message}", file=sys.stderr)

    def success(self, message: str) -> None:
        """Print success message to stdout."""
        if self.verbosity >= self.NORMAL:
            prefix = "\033[32m✓\033[0m " if self.use_color else "✓ "
            print(f"{prefix}{message}")


# Global output manager instance
output = OutputManager()


# ============================================================
# ============================ CLI ===========================
# ============================================================

def print_full_version() -> None:
    """Print full version information including build details."""
    print(f"{NOMIC_PRODUCT_NAME} {NOMIC_VERSION}")
    print(f"{NOMIC_COPYRIGHT}")
    print(f"License: {NOMIC_LICENSE}")
    print()
    print("Build information:")
    print(f"  Python version: {sys.version.split()[0]}")
    print(f"  Platform: {sys.platform}")
    if yaml:
        print(f"  PyYAML: available")
    else:
        print(f"  PyYAML: not available (YAML support disabled)")
    if clang_cindex:
        print(f"  libclang: available")
    else:
        print(f"  libclang: not available (Clang integration disabled)")


def main(argv: Optional[List[str]] = None) -> int:
    """
    CLI entry point for Nomic.

    Exit codes follow GNU conventions:
      0: Success
      1: General failure
      2: Usage error
      3: Violations found (when using --strict)
      4: Parse error in source files
      5: Rule definition error
      65: Data format error
      66: Cannot open input
      70: Internal software error
      71: System error (OS)
      72: Critical OS file missing
      73: Cannot create output file
      74: Input/output error
      78: Configuration error
    """
    global output

    parser = argparse.ArgumentParser(
        prog="nomic",
        description=_HELP_HEADER,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=textwrap.dedent("""\
            Examples:
              # Analyze C files with default rules
              %(prog)s analyze src/*.c

              # Use custom rule files
              %(prog)s analyze --rules rules.yaml --rules extra.yaml src/*.c

              # Generate report in JSON format
              %(prog)s analyze --output report.json src/*.c

              # Run in quiet mode with strict checking
              %(prog)s analyze --quiet --strict src/*.c

              # Check specific files with verbose output
              %(prog)s analyze -vv --rules helios.yaml src/main.c src/utils.c

            Report bugs to: https://github.com/HeliOS/nomic/issues
            Home page: https://github.com/HeliOS/nomic
            """),
        add_help=False,  # We'll add custom help
    )

    # Global options
    general = parser.add_argument_group('General options')
    general.add_argument(
        "-h", "--help",
        action="help",
        help="Show this help message and exit"
    )
    general.add_argument(
        "-V", "--version",
        action="store_true",
        help="Show version information and exit"
    )
    general.add_argument(
        "--full-version",
        action="store_true",
        help="Show detailed version and build information"
    )

    # Output control options
    output_group = parser.add_argument_group('Output control')
    output_group.add_argument(
        "-q", "--quiet",
        action="store_true",
        help="Suppress all but error messages"
    )
    output_group.add_argument(
        "-v", "--verbose",
        action="count",
        default=0,
        help="Increase output verbosity (can be repeated: -v, -vv, -vvv)"
    )
    output_group.add_argument(
        "--no-color",
        action="store_true",
        help="Disable colored output"
    )
    output_group.add_argument(
        "--progress",
        action="store_true",
        help="Show progress indicators during analysis"
    )

    # Configuration options
    config_group = parser.add_argument_group('Configuration')
    config_group.add_argument(
        "-c", "--config",
        metavar="FILE",
        help="Read configuration from FILE"
    )
    config_group.add_argument(
        "--include-path",
        action="append",
        metavar="DIR",
        help="Add DIR to include search path (can be repeated)"
    )

    subparsers = parser.add_subparsers(
        dest="command",
        required=False,
        title="Commands",
        description="Available commands (use '%(prog)s COMMAND --help' for command-specific help)",
        metavar="COMMAND"
    )

    # Analyze command
    analyze_p = subparsers.add_parser(
        "analyze",
        help="Analyze C source files for rule violations",
        description="Analyze one or more C translation units and report violations.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=textwrap.dedent("""\
            Exit codes:
              0: No violations found
              3: Violations found (with --strict)
              4: Parse error in source files
              5: Error in rule definitions

            Examples:
              # Basic analysis with default rules
              %(prog)s src/*.c

              # Use specific rule files
              %(prog)s --rules coding_standards.yaml src/*.c

              # Generate machine-readable report
              %(prog)s --output violations.json --format json src/*.c

              # Human-readable report with context
              %(prog)s --format text --context 3 src/*.c
            """)
    )
    analyze_p.add_argument(
        "-r", "--rules",
        action="append",
        metavar="FILE",
        help="YAML rule file (can be repeated for multiple files)"
    )
    analyze_p.add_argument(
        "-o", "--output",
        metavar="FILE",
        help="Write output to FILE instead of stdout"
    )
    analyze_p.add_argument(
        "-f", "--format",
        choices=["json", "text", "gcc", "xml", "sarif"],
        default="json",
        help="Output format (default: json)"
    )
    analyze_p.add_argument(
        "--strict",
        action="store_true",
        help="Exit with status 3 if any violations are found"
    )
    analyze_p.add_argument(
        "--max-violations",
        type=int,
        metavar="N",
        help="Stop after finding N violations"
    )
    analyze_p.add_argument(
        "--context",
        type=int,
        default=0,
        metavar="N",
        help="Show N lines of context around violations (text format only)"
    )
    analyze_p.add_argument(
        "--exclude",
        action="append",
        metavar="PATTERN",
        help="Exclude files matching PATTERN (can be repeated)"
    )
    analyze_p.add_argument(
        "--severity",
        choices=["error", "warning", "info", "all"],
        default="all",
        help="Minimum severity level to report (default: all)"
    )
    analyze_p.add_argument(
        "files",
        nargs="*",
        help="C source files to analyze (if none, reads from stdin or config)"
    )

    # Check command (simple wrapper for compatibility)
    check_p = subparsers.add_parser(
        "check",
        help="Quick check with default rules (alias for analyze)",
        description="Quick analysis with default HeliOS rules."
    )
    check_p.add_argument(
        "files",
        nargs="+",
        help="C source files to check"
    )

    # List command
    list_p = subparsers.add_parser(
        "list-rules",
        help="List available rules",
        description="Display information about available rules."
    )
    list_p.add_argument(
        "-r", "--rules",
        action="append",
        metavar="FILE",
        help="Rule file to list (if none, lists built-in rules)"
    )
    list_p.add_argument(
        "--format",
        choices=["table", "json", "yaml"],
        default="table",
        help="Output format (default: table)"
    )

    # Validate command
    validate_p = subparsers.add_parser(
        "validate",
        help="Validate rule files",
        description="Check rule files for syntax and semantic errors."
    )
    validate_p.add_argument(
        "rules",
        nargs="+",
        help="Rule files to validate"
    )

    # Parse early to handle version flags
    args = parser.parse_args(argv)

    # Handle version flags first
    if args.version:
        print(f"{NOMIC_PRODUCT_NAME} {NOMIC_VERSION}")
        print(f"{NOMIC_COPYRIGHT}")
        print(f"License: {NOMIC_LICENSE}")
        return EXIT_SUCCESS

    if args.full_version:
        print_full_version()
        return EXIT_SUCCESS

    # Set up output manager based on verbosity flags
    if args.quiet:
        output.verbosity = OutputManager.QUIET
    elif args.verbose:
        output.verbosity = min(OutputManager.NORMAL + args.verbose, OutputManager.DEBUG)

    if args.no_color:
        output.use_color = False

    # Load configuration file if specified
    config = {}
    if args.config:
        try:
            output.verbose(f"Loading configuration from {args.config}")
            with open(args.config, 'r') as f:
                if args.config.endswith('.yaml') or args.config.endswith('.yml'):
                    if yaml:
                        config = yaml.safe_load(f)
                    else:
                        output.error("PyYAML not available, cannot read YAML config")
                        return EXIT_CONFIG
                elif args.config.endswith('.json'):
                    config = json.load(f)
                else:
                    output.error(f"Unknown config format: {args.config}")
                    return EXIT_CONFIG
        except FileNotFoundError:
            output.error(f"Configuration file not found: {args.config}")
            return EXIT_NOINPUT
        except Exception as e:
            output.error(f"Error loading configuration: {e}")
            return EXIT_CONFIG

    # Default to 'analyze' command if no command specified but files are given
    if not args.command:
        parser.print_help()
        return EXIT_USAGE

    # Handle analyze command
    if args.command == "analyze":
        try:
            # Show banner in normal mode
            if output.verbosity == OutputManager.NORMAL:
                output.info(f"{NOMIC_PRODUCT_NAME} {NOMIC_VERSION} - Semantic C Code Analysis")
                output.info("")

            # Get files to analyze
            files = args.files
            if not files:
                if config.get('files'):
                    files = config['files']
                else:
                    output.error("No input files specified")
                    return EXIT_USAGE

            # Show progress if requested
            if args.progress:
                output.info(f"Analyzing {len(files)} file(s)...")

            # Build TranslationUnits
            tus: List[TranslationUnit] = []
            for i, path in enumerate(files):
                if args.progress:
                    output.info(f"  [{i+1}/{len(files)}] Parsing {path}...")
                else:
                    output.verbose(f"Parsing {path}")

                try:
                    tu = build_translation_unit_from_clang(path)
                    tus.append(tu)
                except FileNotFoundError:
                    output.error(f"File not found: {path}")
                    return EXIT_NOINPUT
                except Exception as e:
                    output.error(f"Error parsing {path}: {e}")
                    return EXIT_PARSE_ERROR

            # Build ProjectDB
            output.verbose("Building project database")
            project_db = stitch_project_db(tus)

            # Load rules
            rule_files = args.rules or config.get('rules', [])
            if not rule_files:
                output.verbose("Using default built-in rules")
                rule_list = []  # Would load built-in rules here
            else:
                output.verbose(f"Loading rules from {len(rule_files)} file(s)")
                try:
                    rule_list = load_rules_from_yaml(rule_files)
                    output.verbose(f"Loaded {len(rule_list)} rule(s)")
                except Exception as e:
                    output.error(f"Error loading rules: {e}")
                    return EXIT_RULE_ERROR

            # Run rule engine
            output.verbose("Running rule analysis")
            engine = RuleEngine(project_db, rule_list)
            violations = engine.evaluate()

            # Apply filters
            if args.severity != "all":
                # Filter by severity (would need to implement severity in Violation)
                pass

            if args.max_violations and len(violations) > args.max_violations:
                violations = violations[:args.max_violations]
                output.warning(f"Output truncated to {args.max_violations} violations")

            # Output results
            output.verbose(f"Found {len(violations)} violation(s)")

            if args.format == "json":
                emit_violations_json(violations, out=args.output)
            elif args.format == "text":
                emit_violations_text(violations, out=args.output, context=args.context)
            elif args.format == "gcc":
                emit_violations_gcc(violations, out=args.output)
            elif args.format == "xml":
                emit_violations_xml(violations, out=args.output)
            elif args.format == "sarif":
                emit_violations_sarif(violations, out=args.output)

            # Exit code
            if violations and args.strict:
                return EXIT_VIOLATIONS_FOUND
            return EXIT_SUCCESS

        except KeyboardInterrupt:
            output.error("Interrupted by user")
            return EXIT_FAILURE
        except Exception as e:
            output.error(f"Internal error: {e}")
            if output.verbosity >= OutputManager.DEBUG:
                import traceback
                traceback.print_exc()
            return EXIT_SOFTWARE

    # Handle check command (alias for analyze with defaults)
    elif args.command == "check":
        # Convert to analyze command with default rules
        args.command = "analyze"
        args.rules = ["helios.yaml"] if os.path.exists("helios.yaml") else []
        args.format = "text"
        args.strict = True
        args.output = None
        args.severity = "all"
        args.max_violations = None
        args.context = 2
        args.exclude = []
        # Recursive call with modified args
        return main(["analyze"] + args.files)

    # Handle list-rules command
    elif args.command == "list-rules":
        try:
            rule_files = args.rules or []
            if rule_files:
                rules = load_rules_from_yaml(rule_files)
            else:
                rules = []  # Would load built-in rules

            if args.format == "json":
                print(json.dumps([asdict(r) for r in rules], indent=2))
            elif args.format == "yaml":
                if yaml:
                    print(yaml.dump([asdict(r) for r in rules], default_flow_style=False))
                else:
                    output.error("PyYAML not available")
                    return EXIT_SOFTWARE
            else:  # table format
                if not rules:
                    output.info("No rules found")
                else:
                    output.info(f"{'ID':<30} {'Type':<15} {'Description'}")
                    output.info("-" * 70)
                    for rule in rules:
                        desc = getattr(rule, 'description', 'No description')[:40]
                        output.info(f"{rule.id:<30} {rule.type:<15} {desc}")

            return EXIT_SUCCESS
        except Exception as e:
            output.error(f"Error listing rules: {e}")
            return EXIT_FAILURE

    # Handle validate command
    elif args.command == "validate":
        all_valid = True
        for rule_file in args.rules:
            try:
                output.info(f"Validating {rule_file}...")
                rules = load_rules_from_yaml([rule_file])
                output.success(f"{rule_file}: {len(rules)} valid rule(s)")
            except FileNotFoundError:
                output.error(f"{rule_file}: File not found")
                all_valid = False
            except Exception as e:
                output.error(f"{rule_file}: {e}")
                all_valid = False

        return EXIT_SUCCESS if all_valid else EXIT_RULE_ERROR

    # Should not reach here if parser is configured correctly
    return EXIT_FAILURE


if __name__ == "__main__":
    sys.exit(main())
