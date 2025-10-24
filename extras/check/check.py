#!/usr/bin/env python3
"""
HeliOS Compliance Checker - Report-Only Version
A sophisticated static analysis tool for coding standards enforcement

Features:
- YAML-driven rule engine
- AST and regex-based parsing
- Multi-level caching for performance
- Parallel processing
- Detailed violation reporting
- No auto-fix functionality (report-only)
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
from collections import defaultdict
from concurrent.futures import ProcessPoolExecutor, as_completed
from typing import List, Dict, Tuple, Optional, Set, Any
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
# Data Models
# ============================================================================

class Severity(Enum):
    """Violation severity levels"""
    CRITICAL = 1
    IMPORTANT = 2
    RECOMMENDED = 3
    INFO = 4

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

@dataclass
class Violation:
    """Coding standard violation"""
    rule_id: str
    severity: Severity
    location: SourceLocation
    message: str
    category: str = ""
    fix_suggestion: str = ""

@dataclass
class FileAnalysis:
    """Analysis results for a single file"""
    filepath: str
    violations: List[Violation]
    functions: List[Function]
    metrics: Dict[str, Any]
    parse_errors: List[str]
    analysis_time: float

# ============================================================================
# Parser
# ============================================================================

class CodeParser:
    """Unified code parser with multiple strategies"""

    def parse_file(self, filepath: str, content: str) -> Dict:
        """Parse file and extract code entities"""
        clean_content = self._remove_comments_strings(content)

        return {
            'content': content,
            'clean_content': clean_content,
            'functions': self._extract_functions(content, clean_content),
            'filepath': filepath
        }

    def _remove_comments_strings(self, content: str) -> str:
        """Remove comments and string literals"""
        result = []
        i = 0
        while i < len(content):
            # Skip single-line comments
            if i < len(content) - 1 and content[i:i+2] == '//':
                while i < len(content) and content[i] != '\n':
                    result.append(' ')
                    i += 1
            # Skip multi-line comments
            elif i < len(content) - 1 and content[i:i+2] == '/*':
                result.append(' ')
                result.append(' ')
                i += 2
                while i < len(content) - 1:
                    if content[i:i+2] == '*/':
                        result.append(' ')
                        result.append(' ')
                        i += 2
                        break
                    result.append(' ')
                    i += 1
            # Skip string literals
            elif content[i] in '"\'':
                quote = content[i]
                result.append(' ')
                i += 1
                while i < len(content):
                    if content[i] == '\\' and i + 1 < len(content):
                        result.append(' ')
                        result.append(' ')
                        i += 2
                    elif content[i] == quote:
                        result.append(' ')
                        i += 1
                        break
                    else:
                        result.append(' ')
                        i += 1
            else:
                result.append(content[i])
                i += 1

        return ''.join(result)

    def _extract_functions(self, content: str, clean_content: str) -> List[Function]:
        """Extract functions from code"""
        functions = []

        # Pattern to match C functions
        func_pattern = r'''
            (?:^|\n)                             # Start of line
            (?P<qualifiers>(?:static|inline|extern|const|volatile|\s)+)?
            (?P<return>[\w_]+(?:\s*\*)?)\s+     # Return type
            (?P<name>[\w_]+)\s*                 # Function name
            \((?P<params>[^)]*)\)\s*            # Parameters
            (?=\{)                               # Followed by opening brace
        '''

        for match in re.finditer(func_pattern, clean_content, re.VERBOSE | re.MULTILINE):
            # Find complete function body
            start = match.start()
            brace_start = content.find('{', match.end())
            if brace_start == -1:
                continue

            # Match braces to find function end
            brace_count = 1
            pos = brace_start + 1
            while pos < len(content) and brace_count > 0:
                if content[pos] == '{':
                    brace_count += 1
                elif content[pos] == '}':
                    brace_count -= 1
                pos += 1

            body = content[brace_start:pos]
            clean_body = self._remove_comments_strings(body)

            # Parse parameters
            params = []
            if match.group('params') and match.group('params').strip() != 'void':
                for param in match.group('params').split(','):
                    param = param.strip()
                    if param:
                        tokens = re.findall(r'\b[\w_]+\b', param)
                        if tokens:
                            param_name = tokens[-1]
                            param_type = param.replace(param_name, '').strip()
                            params.append({'name': param_name, 'type': param_type})

            # Extract function attributes for rule checking
            attributes = {
                'has_function_enter': 'FUNCTION_ENTER' in clean_body[:200],
                'has_function_exit': 'FUNCTION_EXIT' in clean_body[-200:],
                'is_static': bool(match.group('qualifiers') and 'static' in match.group('qualifiers')),
                'is_public_api': match.group('name').startswith('x') if match.group('name') else False
            }

            location = SourceLocation(
                file='',
                line=content[:start].count('\n') + 1
            )

            func = Function(
                name=match.group('name'),
                location=location,
                return_type=match.group('return'),
                parameters=params,
                body=body,
                complexity=self._calculate_complexity(body),
                attributes=attributes
            )

            functions.append(func)

        return functions

    def _calculate_complexity(self, body: str) -> int:
        """Calculate cyclomatic complexity"""
        complexity = 1

        # Count decision points
        patterns = [
            r'\bif\s*\(',
            r'\belse\s+if\s*\(',
            r'\bwhile\s*\(',
            r'\bfor\s*\(',
            r'\bcase\s+',
            r'\?\s*[^:]+:',  # Ternary operator
            r'&&',
            r'\|\|'
        ]

        for pattern in patterns:
            complexity += len(re.findall(pattern, body))

        return complexity

# ============================================================================
# Rule Engine
# ============================================================================

class RuleEngine:
    """YAML-driven rule engine"""

    def __init__(self, rules_file: str):
        self.rules = self._load_rules(rules_file)
        self.parser = CodeParser()

    def _load_rules(self, rules_file: str) -> Dict:
        """Load rules from YAML file"""
        try:
            with open(rules_file, 'r') as f:
                data = yaml.safe_load(f)
                return data.get('rules', {})
        except Exception as e:
            logging.error(f"Error loading rules from {rules_file}: {e}")
            return {}

    def check_function(self, func: Function, rule_id: str, rule_config: Dict) -> List[Violation]:
        """Check a function against a single rule"""
        violations = []
        check_type = rule_config.get('check_type', 'pattern')

        if check_type == 'pattern':
            violations.extend(self._check_pattern_rule(func, rule_id, rule_config))
        elif check_type == 'builtin':
            violations.extend(self._check_builtin_rule(func, rule_id, rule_config))
        elif check_type == 'semantic':
            violations.extend(self._check_semantic_rule(func, rule_id, rule_config))
        elif check_type == 'composite':
            violations.extend(self._check_composite_rule(func, rule_id, rule_config))

        return violations

    def _check_pattern_rule(self, func: Function, rule_id: str, config: Dict) -> List[Violation]:
        """Check pattern-based rules"""
        violations = []
        patterns = config.get('patterns', {})
        exclude_patterns = config.get('exclude_patterns', [])

        for pattern_name, pattern_str in patterns.items():
            try:
                # Check if pattern should be excluded
                should_exclude = False
                for exclude in exclude_patterns:
                    if re.search(exclude, func.body):
                        should_exclude = True
                        break

                if not should_exclude and re.search(pattern_str, func.body):
                    violations.append(Violation(
                        rule_id=rule_id,
                        severity=Severity[config.get('severity', 'RECOMMENDED')],
                        location=func.location,
                        message=config.get('message', f"Pattern '{pattern_name}' matched in {func.name}"),
                        category=config.get('category', 'General'),
                        fix_suggestion=config.get('fix_suggestion', '')
                    ))
            except re.error as e:
                logging.warning(f"Invalid regex pattern '{pattern_name}' in rule {rule_id}: {e}")

        return violations

    def _check_builtin_rule(self, func: Function, rule_id: str, config: Dict) -> List[Violation]:
        """Check built-in rules based on function attributes"""
        violations = []
        builtin_type = config.get('builtin_type', '')

        if builtin_type == 'function_structure':
            # Check FUNCTION_ENTER/EXIT
            if config.get('require_enter') and not func.attributes.get('has_function_enter'):
                violations.append(Violation(
                    rule_id=rule_id,
                    severity=Severity[config.get('severity', 'CRITICAL')],
                    location=func.location,
                    message=f"Function '{func.name}' missing FUNCTION_ENTER",
                    category=config.get('category', 'Function Structure'),
                    fix_suggestion=config.get('fix_suggestion', 'Add FUNCTION_ENTER after opening brace')
                ))

            if config.get('require_exit') and not func.attributes.get('has_function_exit'):
                violations.append(Violation(
                    rule_id=rule_id,
                    severity=Severity[config.get('severity', 'CRITICAL')],
                    location=func.location,
                    message=f"Function '{func.name}' missing FUNCTION_EXIT",
                    category=config.get('category', 'Function Structure'),
                    fix_suggestion=config.get('fix_suggestion', 'Add FUNCTION_EXIT before closing brace')
                ))

        elif builtin_type == 'parameter_naming':
            # Check parameter naming conventions
            suffix = config.get('require_suffix', '_')
            exclude_types = config.get('exclude_types', [])

            for param in func.parameters:
                param_name = param.get('name', '')
                param_type = param.get('type', '')

                # Check if parameter type should be excluded
                should_exclude = any(excl in param_type for excl in exclude_types)

                if param_name and not should_exclude and not param_name.endswith(suffix):
                    violations.append(Violation(
                        rule_id=rule_id,
                        severity=Severity[config.get('severity', 'IMPORTANT')],
                        location=func.location,
                        message=f"Parameter '{param_name}' missing '{suffix}' suffix",
                        category=config.get('category', 'Naming Convention'),
                        fix_suggestion=f"Rename to '{param_name}{suffix}'"
                    ))

        elif builtin_type == 'early_returns':
            # Check for early returns
            return_pattern = r'^\s*return\s+[^;]+;'
            allowed_patterns = config.get('allowed_patterns', ['__ReturnOk__'])

            for match in re.finditer(return_pattern, func.body, re.MULTILINE):
                return_line = func.body[max(0, match.start()-50):match.end()]

                # Check if return is allowed
                is_allowed = any(pattern in return_line for pattern in allowed_patterns)

                if not is_allowed:
                    line_offset = func.body[:match.start()].count('\n')
                    violations.append(Violation(
                        rule_id=rule_id,
                        severity=Severity[config.get('severity', 'CRITICAL')],
                        location=SourceLocation(
                            file=func.location.file,
                            line=func.location.line + line_offset
                        ),
                        message=config.get('message', 'Early return detected'),
                        category=config.get('category', 'Function Structure'),
                        fix_suggestion=config.get('fix_suggestion', 'Use __ReturnOk__() instead')
                    ))

        return violations

    def _check_semantic_rule(self, func: Function, rule_id: str, config: Dict) -> List[Violation]:
        """Check semantic rules based on code meaning"""
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
                    fix_suggestion=config.get('fix_suggestion', 'Refactor into smaller functions')
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
                    fix_suggestion=config.get('fix_suggestion', 'Split into smaller functions')
                ))

        return violations

    def _check_composite_rule(self, func: Function, rule_id: str, config: Dict) -> List[Violation]:
        """Check composite rules that combine multiple conditions"""
        violations = []

        # Composite rules reference other rules
        required_rules = config.get('requires_all', [])
        any_of_rules = config.get('requires_any', [])

        # For now, just check if all required patterns exist
        all_matched = True
        for req_rule_id in required_rules:
            if req_rule_id in self.rules:
                req_violations = self.check_function(func, req_rule_id, self.rules[req_rule_id])
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
                fix_suggestion=config.get('fix_suggestion', 'Check component rules')
            ))

        return violations

# ============================================================================
# Cache Manager
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
# Main Checker
# ============================================================================

class ComplianceChecker:
    """Main compliance checker"""

    def __init__(self, rules_file: str, cache_enabled: bool = True, parallel: bool = True):
        self.rules_file = rules_file
        self.rule_engine = RuleEngine(rules_file)
        self.parser = CodeParser()
        self.cache = CacheManager() if cache_enabled else None
        self.parallel = parallel
        self.rules_hash = self._get_rules_hash()

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
        ast = self.parser.parse_file(filepath, content)
        functions = ast.get('functions', [])

        # Check all rules
        violations = []
        for func in functions:
            func.location.file = filepath
            for rule_id, rule_config in self.rule_engine.rules.items():
                if rule_config.get('enabled', True):
                    violations.extend(
                        self.rule_engine.check_function(func, rule_id, rule_config)
                    )

        # Calculate metrics
        metrics = {
            'total_functions': len(functions),
            'total_lines': len(content.split('\n')),
            'avg_complexity': sum(f.complexity for f in functions) / len(functions) if functions else 0
        }

        analysis = FileAnalysis(
            filepath=filepath,
            violations=violations,
            functions=functions,
            metrics=metrics,
            parse_errors=[],
            analysis_time=time.time() - start_time
        )

        # Cache result
        if self.cache:
            self.cache.set(filepath, self.rules_hash, analysis)

        return analysis

    def check_files(self, filepaths: List[str]) -> List[FileAnalysis]:
        """Check multiple files"""
        results = []

        if self.parallel and len(filepaths) > 1:
            with ProcessPoolExecutor(max_workers=multiprocessing.cpu_count()) as executor:
                futures = {executor.submit(self.check_file, fp): fp for fp in filepaths}

                iterator = tqdm(as_completed(futures), total=len(futures), desc="Checking files") if TQDM_AVAILABLE else as_completed(futures)

                for future in iterator:
                    try:
                        result = future.result(timeout=30)
                        results.append(result)
                    except Exception as e:
                        filepath = futures[future]
                        logging.error(f"Error checking {filepath}: {e}")
        else:
            iterator = tqdm(filepaths, desc="Checking files") if TQDM_AVAILABLE else filepaths
            for filepath in iterator:
                try:
                    result = self.check_file(filepath)
                    results.append(result)
                except Exception as e:
                    logging.error(f"Error checking {filepath}: {e}")

        return results

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
        lines.append("Compliance Report")
        lines.append("=" * 80)

        # Summary
        total_files = len(analyses)
        total_violations = sum(len(a.violations) for a in analyses)

        severity_counts = defaultdict(int)
        category_counts = defaultdict(int)

        for analysis in analyses:
            for v in analysis.violations:
                severity_counts[v.severity.name] += 1
                category_counts[v.category] += 1

        lines.append(f"\nFiles analyzed: {total_files}")
        lines.append(f"Total violations: {total_violations}")

        if severity_counts:
            lines.append("\nBy Severity:")
            for severity in ['CRITICAL', 'IMPORTANT', 'RECOMMENDED', 'INFO']:
                if severity in severity_counts:
                    lines.append(f"  {severity}: {severity_counts[severity]}")

        if category_counts:
            lines.append("\nBy Category:")
            for category, count in sorted(category_counts.items(), key=lambda x: x[1], reverse=True)[:5]:
                lines.append(f"  {category}: {count}")

        # Cache stats
        if self.cache:
            stats = self.cache.get_stats()
            lines.append(f"\nCache hit rate: {stats['hit_rate']:.1f}%")

        # Detailed violations
        if analyses:
            lines.append("\n" + "-" * 80)
            lines.append("Violations by file:")
            lines.append("-" * 80)

            for analysis in sorted(analyses, key=lambda a: len(a.violations), reverse=True):
                if analysis.violations:
                    lines.append(f"\n{analysis.filepath}: {len(analysis.violations)} violations")

                    # Group by severity
                    by_severity = defaultdict(list)
                    for v in analysis.violations:
                        by_severity[v.severity].append(v)

                    for severity in [Severity.CRITICAL, Severity.IMPORTANT, Severity.RECOMMENDED, Severity.INFO]:
                        if severity in by_severity:
                            lines.append(f"  {severity.name}:")
                            for v in by_severity[severity][:5]:  # Show first 5 of each severity
                                lines.append(f"    - Line {v.location.line}: {v.message}")
                            if len(by_severity[severity]) > 5:
                                lines.append(f"    ... and {len(by_severity[severity]) - 5} more")

        # Performance metrics
        total_time = sum(a.analysis_time for a in analyses)
        lines.append("\n" + "-" * 80)
        lines.append("Performance:")
        lines.append("-" * 80)
        lines.append(f"Total analysis time: {total_time:.2f}s")
        if total_files > 0:
            lines.append(f"Average time per file: {total_time/total_files:.3f}s")

        return '\n'.join(lines)

    def _generate_json_report(self, analyses: List[FileAnalysis]) -> str:
        """Generate JSON report"""
        report = {
            'timestamp': datetime.now().isoformat(),
            'summary': {
                'total_files': len(analyses),
                'total_violations': sum(len(a.violations) for a in analyses)
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
                        'fix_suggestion': v.fix_suggestion
                    }
                    for v in analysis.violations
                ],
                'metrics': analysis.metrics
            }
            report['files'].append(file_report)

        return json.dumps(report, indent=2)

    def _generate_html_report(self, analyses: List[FileAnalysis]) -> str:
        """Generate HTML report"""
        # Simple HTML report
        html = ['<html><head><title>Compliance Report</title>']
        html.append('<style>')
        html.append('body { font-family: Arial, sans-serif; margin: 20px; }')
        html.append('table { border-collapse: collapse; width: 100%; }')
        html.append('th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }')
        html.append('th { background-color: #f2f2f2; }')
        html.append('.critical { color: #d32f2f; font-weight: bold; }')
        html.append('.important { color: #f57c00; }')
        html.append('.recommended { color: #388e3c; }')
        html.append('.info { color: #1976d2; }')
        html.append('</style></head><body>')

        html.append('<h1>Compliance Report</h1>')

        # Summary
        total_violations = sum(len(a.violations) for a in analyses)
        html.append(f'<p>Files analyzed: {len(analyses)}</p>')
        html.append(f'<p>Total violations: {total_violations}</p>')

        # Violations table
        html.append('<h2>Violations</h2>')
        html.append('<table>')
        html.append('<tr><th>File</th><th>Line</th><th>Severity</th><th>Rule</th><th>Message</th></tr>')

        for analysis in analyses:
            for v in analysis.violations:
                severity_class = v.severity.name.lower()
                html.append(f'<tr>')
                html.append(f'<td>{analysis.filepath}</td>')
                html.append(f'<td>{v.location.line}</td>')
                html.append(f'<td class="{severity_class}">{v.severity.name}</td>')
                html.append(f'<td>{v.rule_id}</td>')
                html.append(f'<td>{v.message}</td>')
                html.append(f'</tr>')

        html.append('</table>')
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
# Main Entry Point
# ============================================================================

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description='Compliance Checker - Report-Only Version',
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
        parallel=not args.no_parallel
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
        if v.severity == Severity.CRITICAL
    )

    return 1 if total_critical > 0 else 0

if __name__ == '__main__':
    sys.exit(main())