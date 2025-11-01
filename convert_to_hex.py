#!/usr/bin/env python3
"""
Convert decimal integer literals to hexadecimal format with 'u' suffix.
Processes all .c and .h files in src/ and drivers/ directories.
This version properly handles identifiers and only converts actual numeric literals.
"""

import os
import re
import sys
from pathlib import Path


def is_identifier_char(c):
    """Check if character can be part of an identifier."""
    return c.isalnum() or c == '_'


def process_line(line):
    """
    Process a single line and convert decimal literals to hex.
    Only converts standalone numeric literals, not numbers within identifiers.
    """
    result = []
    i = 0
    in_string = False
    in_char = False
    escape_next = False

    while i < len(line):
        if escape_next:
            result.append(line[i])
            escape_next = False
            i += 1
            continue

        char = line[i]

        if char == '\\':
            escape_next = True
            result.append(char)
            i += 1
            continue

        if char == '"' and not in_char:
            in_string = not in_string
            result.append(char)
            i += 1
            continue

        if char == "'" and not in_string:
            in_char = not in_char
            result.append(char)
            i += 1
            continue

        if in_string or in_char:
            result.append(char)
            i += 1
            continue

        if len(result) > 0 and is_identifier_char(result[-1]):
            result.append(char)
            i += 1
            continue

        if i + 1 < len(line) and char == '0' and line[i+1] in 'xX':
            result.append(char)
            result.append(line[i+1])
            i += 2
            while i < len(line) and (line[i] in '0123456789ABCDEFabcdef' or line[i] in 'uUlL'):
                result.append(line[i])
                i += 1
            continue

        if char.isdigit():
            num_str = ''
            j = i

            while j < len(line) and line[j].isdigit():
                num_str += line[j]
                j += 1

            suffix_chars = ''
            while j < len(line) and line[j] in 'uUlL':
                suffix_chars += line[j]
                j += 1

            next_char = line[j] if j < len(line) else ''

            if is_identifier_char(next_char):
                result.append(num_str)
                result.append(suffix_chars)
                i = j
                continue

            value = int(num_str)
            hex_str = f"0x{value:X}u"
            result.append(hex_str)
            i = j
            continue

        result.append(char)
        i += 1

    return ''.join(result)


def process_file(filepath):
    """
    Process a single file and convert decimal literals to hex.
    """
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()

        modified = False
        new_lines = []

        for line in lines:
            original = line.rstrip('\n')
            processed_line = process_line(original)
            new_lines.append(processed_line + '\n' if line.endswith('\n') else processed_line)

            if processed_line != original:
                modified = True

        if modified:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.writelines(new_lines)
            return True

        return False

    except Exception as e:
        print(f"Error processing {filepath}: {e}", file=sys.stderr)
        return False


def should_skip_file(filepath):
    """
    Determine if a file should be skipped.
    """
    filename = os.path.basename(filepath)
    return filename in ['HeliOS.h', 'config.h']


def process_directory(directory):
    """
    Process all C source files in the directory.
    """
    directory = Path(directory)

    if not directory.exists():
        print(f"Warning: Directory {directory} does not exist")
        return 0, 0

    processed_count = 0
    skipped_count = 0

    for root, dirs, files in os.walk(directory):
        for filename in files:
            if not filename.endswith(('.c', '.h')):
                continue

            filepath = os.path.join(root, filename)

            if should_skip_file(filepath):
                print(f"Skipping: {filepath}")
                skipped_count += 1
                continue

            if process_file(filepath):
                print(f"Modified: {filepath}")
                processed_count += 1
            else:
                print(f"No changes: {filepath}")

    return processed_count, skipped_count


def main():
    """
    Main entry point.
    """
    print("Converting decimal integer literals to hexadecimal...")
    print("=" * 60)

    total_processed = 0
    total_skipped = 0

    for directory in ['src', 'drivers']:
        print(f"\nProcessing {directory}/ directory:")
        print("-" * 60)
        processed, skipped = process_directory(directory)
        total_processed += processed
        total_skipped += skipped

    print("\n" + "=" * 60)
    print(f"Summary: {total_processed} files modified, {total_skipped} files skipped")


if __name__ == '__main__':
    main()
