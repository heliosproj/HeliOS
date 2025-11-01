#!/usr/bin/env python3
"""
Strip all comments and blank lines from C source files in src/ and drivers/ directories.
Preserves HeliOS.h and config.h files.
"""

import os
import re
import sys
from pathlib import Path


def strip_comments_and_blanks(content):
    """
    Remove all C-style comments (// and /* */) and blank lines from content.
    """
    result = []
    in_block_comment = False
    in_string = False
    escape_next = False

    lines = content.split('\n')

    for line in lines:
        new_line = []
        i = 0

        while i < len(line):
            char = line[i]

            if escape_next:
                if not in_block_comment:
                    new_line.append(char)
                escape_next = False
                i += 1
                continue

            if char == '\\' and in_string:
                escape_next = True
                if not in_block_comment:
                    new_line.append(char)
                i += 1
                continue

            if char == '"' and not in_block_comment:
                in_string = not in_string
                new_line.append(char)
                i += 1
                continue

            if in_string:
                new_line.append(char)
                i += 1
                continue

            if in_block_comment:
                if i + 1 < len(line) and line[i:i+2] == '*/':
                    in_block_comment = False
                    i += 2
                    continue
                i += 1
                continue

            if i + 1 < len(line) and line[i:i+2] == '//':
                break

            if i + 1 < len(line) and line[i:i+2] == '/*':
                in_block_comment = True
                i += 2
                continue

            new_line.append(char)
            i += 1

        processed_line = ''.join(new_line).rstrip()

        if processed_line:
            result.append(processed_line)

    return '\n'.join(result)


def should_process_file(filepath):
    """
    Determine if a file should be processed.
    Returns True if file should be processed, False otherwise.
    """
    filename = os.path.basename(filepath)

    if filename in ['HeliOS.h', 'config.h']:
        return False

    if filepath.endswith(('.c', '.h')):
        return True

    return False


def process_directory(directory):
    """
    Process all C source files in the given directory and its subdirectories.
    """
    directory = Path(directory)

    if not directory.exists():
        print(f"Warning: Directory {directory} does not exist")
        return

    processed_count = 0
    skipped_count = 0

    for root, dirs, files in os.walk(directory):
        for filename in files:
            filepath = os.path.join(root, filename)

            if not should_process_file(filepath):
                if filename in ['HeliOS.h', 'config.h']:
                    print(f"Skipping: {filepath}")
                    skipped_count += 1
                continue

            try:
                with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                    original_content = f.read()

                processed_content = strip_comments_and_blanks(original_content)

                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(processed_content)

                if processed_content != original_content:
                    print(f"Processed: {filepath}")
                    processed_count += 1
                else:
                    print(f"No changes: {filepath}")

            except Exception as e:
                print(f"Error processing {filepath}: {e}", file=sys.stderr)

    return processed_count, skipped_count


def main():
    """
    Main entry point.
    """
    print("Stripping comments and blank lines from C source files...")
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
