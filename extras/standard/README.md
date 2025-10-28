# HeliOS Coding Standard Review Tool

## Overview

This directory contains the HeliOS Coding Standard and an interactive Python tool for reviewing and maintaining the standard.

## Files

- **HELIOS_CODE_STANDARD.json** - The complete coding standard with 33 rules
- **review_rules.py** - Interactive review tool
- **README.md** - This file

## Using the Review Tool

The `review_rules.py` script provides an interactive interface for reviewing each coding standard rule, marking rules that need revision, and adding comments for later updates.

### Quick Start

```bash
cd ~/github/HeliOS/extras/standard
./review_rules.py
```

### Features

#### Navigation
- **[n]** Next rule
- **[p]** Previous rule
- **[s]** Skip to specific rule number
- **[l]** List all rules with status
- **[f]** Filter rules (show only rules with/without comments)

#### Review Actions
- **[r]** Add or replace a review comment on the current rule
- **[c]** Clear comment (mark rule as OK)
- **[h]** Show help
- **[q]** Quit (saves automatically)

#### Status Indicators
- **[NEEDS REVIEW]** (Yellow) - Rule has a review comment
- **[OK]** (Green) - Rule has no comments

### Adding Review Comments

1. Press **[r]** to add a comment
2. Enter your comment (can be multiple lines)
3. Press **Enter** on an empty line to finish
4. Type **cancel** to abort without saving

Comments are automatically saved to the JSON file as a `review_comment` field on the rule.

### Example Workflow

1. **Initial Review**: Run the script and go through each rule
   - Press **[n]** to move through rules
   - Press **[r]** to add comments on rules that need work
   - The script auto-saves after each comment

2. **Review Commented Rules**: Run the script again
   - Press **[f]** to filter to rules with comments
   - Jump to each flagged rule to review your comments
   - Update or clear comments as needed

3. **Address Comments**: Claude Code can later read the JSON file and address all `review_comment` fields

## Rule Structure

Each rule in the JSON contains:

```json
{
  "id": "FN-001",
  "category": "function-structure",
  "name": "Function Entry/Exit Macros",
  "shortName": "FUNCTION_ENTER/EXIT Required",
  "description": "...",
  "dsl": {
    "pattern": "...",
    "rule": "..."
  },
  "rationale": "...",
  "examples": {
    "valid": [...],
    "invalid": [...]
  },
  "severity": "error",
  "references": [...],
  "review_comment": "Optional field added by review tool"
}
```

## Coding Standard Categories

- **function-structure** - Function organization and macros
- **parameter-validation** - Input validation patterns
- **control-flow** - Control flow and execution patterns
- **return-handling** - Return value management
- **memory-management** - Memory allocation and cleanup
- **variable-naming** - Naming conventions
- **type-usage** - HeliOS type system
- **numeric-literals** - Numeric constant format
- **macro-usage** - Macro naming and usage
- **error-checking** - Error handling patterns
- **comments** - Code documentation
- **operation-checking** - Operation validation
- **loop-patterns** - Iteration patterns
- **conditionals** - Conditional expressions
- **formatting** - Code formatting
- **api-naming** - API function naming
- **api-scope** - API visibility and scope
- **object-validation** - Object lifecycle and validation (NEW in v1.2.0)

## Version History

### Version 1.2.0 (2025-10-28)
- Added 4 new object validation rules (OBJ-001 through OBJ-004)
- Added object validation patterns and examples
- Total rules: 33

### Version 1.1.0 (2025-10-26)
- Added API naming and scope rules
- Total rules: 27+

## Tips

- The review tool can be run multiple times - your comments persist
- Use **[f]** to quickly navigate between rules that need attention
- Comments are saved immediately after entry
- You can update or replace comments at any time
- Press **Ctrl+C** or **[q]** to exit safely at any time

## Requirements

- Python 3.6+
- ANSI-compatible terminal (for colors)
- The script works on Linux, macOS, and Windows

## Future Enhancements

Potential features for the review tool:
- Export review summary to markdown
- Batch operations (clear all comments, etc.)
- Search rules by keyword
- Compare rules across versions
- Generate compliance checklist
