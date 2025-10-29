# HeliOS Code Standard Compliance To-Do List

## Overview
Nomic semantic analyzer identified **5,273 violations** in the HeliOS codebase:
- **4,599 errors** (must fix)
- **674 warnings** (should fix)

Generated: 2025-10-29

## Phase 1: Critical Foundation (Weeks 1-2)

### 1. Function Return Type Standardization
- [ ] Convert all 374 functions to return `Return_t` type
- [ ] Add `FUNCTION_ENTER` macro at the start of each function
- [ ] Add `FUNCTION_EXIT` macro before the closing brace
- [ ] Update function declarations in header files

**Affected files priority:**
- [ ] src/fs.c (925 violations)
- [ ] src/console.c (902 violations)
- [ ] src/mem.c (434 violations)
- [ ] src/task.c (415 violations)
- [ ] src/device.c (192 violations)
- [ ] src/queue.c (188 violations)
- [ ] src/timer.c (109 violations)
- [ ] src/streams.c (97 violations)
- [ ] src/sys.c (64 violations)
- [ ] src/port.c (28 violations)

### 2. API Naming Convention Fixes (~900 functions)

#### Public API Functions (172 violations)
- [ ] Add x-prefix to all public functions (e.g., `xMemAlloc`, `xTaskCreate`)
- [ ] Update all call sites throughout the codebase
- [ ] Update documentation with new function names

#### Internal Cross-Module Functions (244 violations)
- [ ] Convert to `__FunctionName__` pattern with double underscores
- [ ] Update all internal call sites
- [ ] Verify header file declarations match

#### Module-Private Functions (272 violations)
- [ ] Add `static` keyword to all module-private functions
- [ ] Remove from header files if incorrectly exposed
- [ ] Verify no external dependencies

## Phase 2: Object Safety (Weeks 3-4)

### 3. Kernel Object Validation (1,396 violations)

#### Add Valid Field (374 violations)
- [ ] Add `Base_t valid` field to all kernel object structures
- [ ] Update structure definitions in header files
- [ ] Initialize valid field in all existing code

#### Object Lifecycle Management
- [ ] Set `valid = VALID` after successful allocation (342 locations)
- [ ] Set `valid = INVALID` before freeing objects (358 locations)
- [ ] Add `__ObjectIsValid__()` checks at function entry (322 locations)

### 4. Memory Leak Prevention (294 violations)
- [ ] Review all functions with nested allocations
- [ ] Add cleanup labels for error paths
- [ ] Implement proper unwinding on allocation failures
- [ ] Add explanatory comments for cleanup sequences
- [ ] Test with memory leak detection tools

## Phase 3: Code Quality (Week 5)

### 5. Control Flow Improvements

#### Single-Exit Pattern (90 violations)
- [ ] Remove all early return statements
- [ ] Restructure to use nested if-statements
- [ ] Ensure single FUNCTION_EXIT point

#### List Traversal Safety (332 violations)
- [ ] Initialize cursor to `null` before all list operations
- [ ] Add null checks after traversal
- [ ] Document traversal patterns

#### Loop Optimization (64 violations)
- [ ] Add `break` statement after `__ReturnOk__()` in search loops
- [ ] Review all while/for loops for early exit opportunities

### 6. Minor Code Quality Issues

#### Type Safety
- [ ] Replace all `NULL` with HeliOS `null` macro (64 occurrences)
- [ ] Fix type mismatches in function parameters

#### Variable Initialization (43 violations)
- [ ] Initialize all local variables at declaration
- [ ] Use proper initialization values (0x0u for Size_t, null for pointers)

#### Return Macro Usage (176 violations)
- [ ] Replace `ret = ReturnOK` with `__ReturnOk__()` macro (88 locations)
- [ ] Remove unnecessary `__ReturnError__()` calls (88 locations)
- [ ] Ensure default error return behavior

## Phase 4: Testing and Validation

### 7. Verification Steps
- [ ] Run full unit test suite after each subsystem update
- [ ] Verify all 667 tests still pass
- [ ] Run Nomic analyzer to confirm violation reduction
- [ ] Performance testing to ensure no regressions
- [ ] Memory leak testing with valgrind or similar

### 8. Documentation Updates
- [ ] Update API documentation with new function names
- [ ] Create migration guide for external users
- [ ] Document coding standard compliance changes
- [ ] Update example code in documentation

## Implementation Notes

### Automation Opportunities
- Script for adding FUNCTION_ENTER/EXIT macros
- Batch renaming script for API standardization
- Automated validation field addition

### Testing Strategy
- Test each subsystem independently
- Maintain backward compatibility where possible
- Use enhanced test harnesses for validation

### Risk Mitigation
- Create feature branch from `develop`
- Make incremental commits
- Tag stable checkpoints
- Keep original function names as deprecated aliases initially

## Progress Tracking

### Metrics
- Total violations: 5,273
- Violations resolved: 0
- Completion: 0%

### Subsystem Status
| Subsystem | Violations | Status | Assignee | Notes |
|-----------|------------|--------|----------|-------|
| fs.c      | 925        | Not Started | - | Highest priority |
| console.c | 902        | Not Started | - | Complex refactoring needed |
| mem.c     | 434        | Not Started | - | Critical for stability |
| task.c    | 415        | Not Started | - | Scheduler core |
| device.c  | 192        | Not Started | - | Driver interface |
| queue.c   | 188        | Not Started | - | IPC mechanism |
| timer.c   | 109        | Not Started | - | Timing subsystem |
| streams.c | 97         | Not Started | - | I/O streams |
| sys.c     | 64         | Not Started | - | System core |
| port.c    | 28         | Not Started | - | Port I/O |

## Next Actions
1. Create feature branch: `git checkout -b feature/code-standard-compliance`
2. Start with mem.c and task.c (critical subsystems)
3. Run test suite after each file completion
4. Update this checklist as items are completed

---
*This document was generated from Nomic semantic analyzer output. Update as work progresses.*