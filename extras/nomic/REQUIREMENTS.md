# Nomic Commercial C Semantic Code Analyzer - Requirements Specification

## Executive Summary

Nomic is a commercial-grade, extensible C semantic code analyzer designed for enterprise deployment. It provides a powerful DSL, SQL query language, and plugin architecture for comprehensive code analysis without coupling to any specific project.

Old Nomic source code is at ~/github/HeliOS/extras/nomic. DO NOT USE THIS SOURCE CODE - IT IS ONLY AN EXAMPLE OF HOW NOT TO IMPLEMENT A COMMERCIAL GRADE EXTENSIBLE C SEMANTIC CODE ANALYZER. IT WAS A FAILURE!

### REQ-000: Version
- Do not put the Nomic version number anywhere.
- Nomic is called "Nomic C Semantic Code Analyzer"

## Core Architecture Requirements

### REQ-001: Modular Architecture
The system SHALL be designed with a modular, layered architecture:
- **Core Layer**: AST abstraction, semantic model, base types
- **Query Layer**: DSL engine, SQL engine, pattern matching
- **Analysis Layer**: Rule engine, violation detection, metrics
- **Plugin Layer**: Plugins, extension points
- **Output Layer**: Multiple format generators
- **API Layer**: programmatic interfaces

### REQ-002: Language Abstraction Layer
The system SHALL abstract all compiler-specific details:
- No direct Clang/LLVM types in public APIs
- Compiler-agnostic semantic model

### REQ-003: Zero External Coupling
The system SHALL have no coupling to specific projects:
- Generic implementation without project-specific code
- Configurable for any C codebase
- No hardcoded paths or project assumptions
- Self-contained with all dependencies managed

## DSL Requirements

### REQ-004: Commercial-Grade DSL
The system SHALL provide a powerful, intuitive DSL with:
- Type-safe expressions
- Variable binding and scoping
- Control flow (if/else, loops, switch)
- User-defined functions
- Lambda expressions
- Pattern matching syntax
- Regular expression support

### REQ-005: DSL Built-in Functions
The system SHALL provide comprehensive built-in functions:
- **AST Navigation**: parent(), children(), ancestors(), descendants(), siblings()
- **Type Analysis**: typeof(), sizeof(), isPointer(), isArray(), isConst()
- **Control Flow**: complexity(), paths(), dominates(), reaches()
- **Data Flow**: defines(), uses(), taints(), flows()
- **Metrics**: loc(), cyclomatic(), coupling(), cohesion()
- **Pattern Matching**: matches(), contains(), startsWith(), endsWith()
- **Collection Operations**: map(), filter(), reduce(), any(), all(), count()
- **String Operations**: regex(), format(), split(), join(), replace()
- MORE FUNCTIONS!! Should be around 100 functions built-in to DSL!


## SQL Query Language Requirements

### REQ-007: Full SQL Support
The system SHALL support SQL queries over the AST:
- SELECT, FROM, WHERE, JOIN, GROUP BY, HAVING, ORDER BY
- Subqueries and CTEs (Common Table Expressions)
- Window functions (ROW_NUMBER, RANK, LEAD, LAG)
- Aggregates (COUNT, SUM, AVG, MIN, MAX)
- Set operations (UNION, INTERSECT, EXCEPT)

### REQ-008: SQL Virtual Tables
The system SHALL expose AST as SQL tables:
- **functions**: All function declarations
- **variables**: All variable declarations
- **statements**: All statements
- **expressions**: All expressions
- **calls**: All function calls
- **types**: All type definitions
- **files**: All source files
- **macros**: All preprocessor macros


## Pattern Matching Requirements

### REQ-010: Multi-line Pattern Matching
The system SHALL support sophisticated pattern matching:
- Multi-line patterns with wildcards
- Context-aware matching (indentation, scope)
- Negative patterns (must not match)
- Ordered and unordered patterns
- Pattern variables and backreferences

### REQ-011: Pattern Syntax
The system SHALL support multiple pattern syntaxes:
- Regular expressions (PCRE compatible)
- Glob patterns
- Structural patterns (AST-based)
- Semantic patterns (type-aware)
- XPath-like patterns

## Plugin System Requirements


### REQ-014: Plugin Capabilities
Plugins SHALL be able to:
- Add custom rules
- Extend DSL with new functions
- Add output formats
- Provide custom analyzers
- Hook into analysis pipeline
- Add SQL virtual tables

## Configuration Requirements

### REQ-016: Configuration Format
The system SHALL support multiple configuration formats:
- YAML
- Command-line overrides

## Output Format Requirements

### REQ-019: Output Formats
The system SHALL support multiple output formats:
- **JSON**: Structured, machine-readable
- **SARIF**: Static Analysis Results Interchange Format

## Rule System Requirements

### REQ-022: Rule Definition
Rules SHALL support multiple definition methods:
- YAML
- DSL expressions
- SQL queries
- Plugin code

### REQ-023: Rule Capabilities
Rules SHALL support:
- Multiple severity levels
- Categories and tags
- Suppression directives
- Fix suggestions
- Code examples
- Documentation links
- Custom properties

## Analysis Engine Requirements

### REQ-026: Analysis Features
The analysis engine SHALL provide:
- Inter-procedural analysis
- Context-sensitive analysis
- Path-sensitive analysis
- Taint analysis
- Control flow analysis
- Type inference

## Integration Requirements

### REQ-028: Build System Integration
The system SHALL integrate with:
- CMake
- Download dependencies/libraries automatically

### REQ-029: Version Control Integration
The system SHALL integrate with:
- Git

## API Requirements

### REQ-032: Language Bindings
The system SHALL provide bindings for:
- C API

## Security Requirements


### REQ-043: Maintainability
The system SHALL be maintainable:
- Modular architecture
- Unit test coverage > 80%
- Integration test suite

### REQ-044: Usability
The system SHALL be user-friendly:
- Intuitive CLI interface
- Clear error messages
- Helpful suggestions

## Testing Requirements

### REQ-045: Test Coverage
The system SHALL include:
- Unit tests for all modules

### REQ-046: Test Framework
Testing SHALL use:
- Google Test for C++ tests




