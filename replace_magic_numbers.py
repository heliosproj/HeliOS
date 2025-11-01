#!/usr/bin/env python3
"""
Replace magic number literals with defined constants in C source files.
Processes all .c files in src/ and drivers/ directories (excludes header files).
"""

import os
import re
from pathlib import Path


REPLACEMENTS = {
    '0xBu': 'FAT_83_NAME_LENGTH',
    '0x8u': 'FAT_83_BASENAME_LENGTH',
    '0x3u': 'FAT_83_EXTENSION_LENGTH',
    '0xE5u': 'FAT_ENTRY_DELETED',
    '0xFFu': 'FAT_BYTE_MASK',
    '0x8': 'FAT_BYTE_SHIFT_8',
    '0x10': 'FAT_BYTE_SHIFT_16',
    '0x18': 'FAT_BYTE_SHIFT_24',
    '0x10u': 'FAT_CLUSTER_HIGH_SHIFT',
    '0xFFFFu': 'FAT_CLUSTER_LOW_MASK',
    '0x2u': 'FAT_MIN_VALID_CLUSTER',
    '0x1u': 'FAT_CLUSTER_INCREMENT',
    '0x3u': 'FAT_MIN_SEARCH_CLUSTER',
    '0x10000u': 'FAT_MAX_SEARCHABLE_CLUSTERS',
    '0x100u': 'FAT_MAX_PATH_COMPONENT',
    '0x200u': 'FAT_DEFAULT_SECTOR_SIZE',
    '0x0FFFFFFFu': 'FAT32_ENTRY_MASK',
    '0xF0000000u': 'FAT32_RESERVED_BITS_MASK',
    '0x0FFFFFF8u': 'FAT32_MEDIA_DESCRIPTOR_ENTRY',
    '0x4u': 'FAT32_ENTRY_SIZE_BYTES',
    '0xEBu': 'FAT_BOOT_JMP_EB',
    '0x58u': 'FAT_BOOT_JMP_58',
    '0x90u': 'FAT_BOOT_JMP_90',
    '0x55u': 'FAT_BOOT_SIG_55',
    '0xAAu': 'FAT_BOOT_SIG_AA',
    '0xF8u': 'FAT_MEDIA_TYPE_HARD_DISK',
    '0x3Fu': 'FAT_DEFAULT_SECTORS_PER_TRACK',
    '0x80u': 'FAT_DRIVE_NUMBER_HDD',
    '0x29u': 'FAT_EXTENDED_BOOT_SIG',
    '0x1u': 'FAT_FSINFO_SECTOR',
    '0x6u': 'FAT_BACKUP_BOOT_SECTOR',
    '0x1': 'FAT_LE16_HIGH_BYTE_OFFSET',
    '0x2': 'FAT_CLUSTER_DATA_OFFSET',
    '0xB16B00B5u': 'CHECKSUM_XOR_CONSTANT',
    '0x3E8u': 'MS_PER_SECOND',
    '0x3E8': 'MS_PER_SECOND',
    '0x800u': 'FAT_DEFAULT_TOTAL_SECTORS',
    '0x12345678u': 'FAT_DEFAULT_VOLUME_ID',
    '0x1000u': 'FS_VOL_INFO_CLUSTER_LIMIT',
    '0x20u': 'FS_FORMAT_RESERVED_SECTORS',
    '0xFu': 'HEX_DIGIT_MASK',
    '0x1C200u': 'USART_MAX_BAUD_RATE',
    '0x5u': 'USART_IRQ_PRIORITY',
}


CONTEXT_SENSITIVE_REPLACEMENTS = {
    'fat.c': {
        '0x8u': 'FAT_83_BASENAME_LENGTH',
        '0x3u': 'FAT_83_EXTENSION_LENGTH',
        '0xBu': 'FAT_83_NAME_LENGTH',
        '0x100u': 'FAT_MAX_PATH_COMPONENT',
        '0x2u': 'FAT_MIN_VALID_CLUSTER',
        '0x10': 'FAT_BYTE_SHIFT_16',
        '0x8': 'FAT_BYTE_SHIFT_8',
        '0x18': 'FAT_BYTE_SHIFT_24',
        '0xFFu': 'FAT_BYTE_MASK',
        '0xE5u': 'FAT_ENTRY_DELETED',
        '0x0FFFFFFFu': 'FAT32_ENTRY_MASK',
        '0xF0000000u': 'FAT32_RESERVED_BITS_MASK',
        '0x4u': 'FAT32_ENTRY_SIZE_BYTES',
        '0x10000u': 'FAT_MAX_SEARCHABLE_CLUSTERS',
        '0x1': 'FAT_LE16_HIGH_BYTE_OFFSET',
    },
    'fs.c': {
        '0x200u': 'FS_DEFAULT_SECTOR_SIZE',
        '0xBu': 'FAT_83_NAME_LENGTH',
        '0x8u': 'FAT_83_BASENAME_LENGTH',
        '0x2u': 'FAT_MIN_VALID_CLUSTER',
        '0x100u': 'FS_PATH_BUFFER_SIZE',
        '0x1000u': 'FS_VOL_INFO_CLUSTER_LIMIT',
        '0xEBu': 'FAT_BOOT_JMP_EB',
        '0x58u': 'FAT_BOOT_JMP_58',
        '0x90u': 'FAT_BOOT_JMP_90',
        '0x55u': 'FAT_BOOT_SIG_55',
        '0xAAu': 'FAT_BOOT_SIG_AA',
        '0xF8u': 'FAT_MEDIA_TYPE_HARD_DISK',
        '0x3Fu': 'FAT_DEFAULT_SECTORS_PER_TRACK',
        '0x10u': 'FAT_DEFAULT_NUM_HEADS',
        '0x800u': 'FAT_DEFAULT_TOTAL_SECTORS',
        '0x80u': 'FAT_DRIVE_NUMBER_HDD',
        '0x29u': 'FAT_EXTENDED_BOOT_SIG',
        '0x12345678u': 'FAT_DEFAULT_VOLUME_ID',
        '0x0FFFFFF8u': 'FAT32_MEDIA_DESCRIPTOR_ENTRY',
        '0x0FFFFFFFu': 'FAT32_EOC_MAX',
        '0xFFFFu': 'FAT_CLUSTER_LOW_MASK',
        '0x10': 'FAT_CLUSTER_HIGH_SHIFT',
    },
    'mem.c': {
        '0xFFFFu': 'FLETCHER_MASK',
        '0x10u': 'CHECKSUM_WORD_SHIFT',
        '0x20u': 'CHECKSUM_DWORD_SHIFT',
        '0xB16B00B5u': 'CHECKSUM_XOR_CONSTANT',
        '0x4': 'MEM_ALIGN_SHIFT_4BIT',
        '0x100': 'BYTE_ORDER_TEST_VALUE',
        '0x10': 'CHECKSUM_WORD_SHIFT',
    },
    'console.c': {
        '0x10': 'CONSOLE_NUM_BUFFER_SIZE',
        '0x2': 'CONSOLE_ECHO_BUFFER_SIZE',
        '0x2u': 'PATH_SEGMENTS_DIVISOR',
        '0xFu': 'HEX_DIGIT_MASK',
    },
    'port.c': {
        '0x3E8': 'MS_PER_SECOND',
    },
    'usart_stm32_driver.c': {
        '0x1C200u': 'USART_MAX_BAUD_RATE',
        '0x5u': 'USART_IRQ_PRIORITY',
        '0x1u': 'USART_SINGLE_BYTE_TRANSFER',
        '0x3E8u': 'USART_DEFAULT_TIMEOUT_MS',
    },
    'char_driver.c': {
        '0x3E8u': 'CHAR_DEFAULT_TIMEOUT_MS',
        '0x1u': 'USART_SINGLE_BYTE_TRANSFER',
    },
}


def is_identifier_char(c):
    """Check if character can be part of an identifier."""
    return c.isalnum() or c == '_'


def should_skip_file(filepath):
    """Determine if a file should be skipped."""
    filename = os.path.basename(filepath)
    return filename in ['HeliOS.h', 'config.h']


def replace_in_line(line, filename):
    """
    Replace magic numbers with defined constants in a line.
    Only replaces literals, not parts of identifiers or hex numbers.
    """
    replacements_for_file = CONTEXT_SENSITIVE_REPLACEMENTS.get(filename, {})

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

        if char == '0' and i + 1 < len(line) and line[i+1] in 'xX':
            hex_start = i
            i += 2
            while i < len(line) and (line[i] in '0123456789ABCDEFabcdef' or line[i] in 'uUlL'):
                i += 1

            next_char = line[i] if i < len(line) else ''
            if is_identifier_char(next_char):
                result.append(line[hex_start:i])
                continue

            hex_literal = line[hex_start:i]

            replacement = replacements_for_file.get(hex_literal)
            if replacement:
                result.append(replacement)
            else:
                result.append(hex_literal)
            continue

        result.append(char)
        i += 1

    return ''.join(result)


def process_file(filepath):
    """Process a single file and replace magic numbers."""
    filename = os.path.basename(filepath)

    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()

        modified = False
        new_lines = []

        for line in lines:
            original = line.rstrip('\n')
            processed_line = replace_in_line(original, filename)
            new_lines.append(processed_line + '\n' if line.endswith('\n') else processed_line)

            if processed_line != original:
                modified = True

        if modified:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.writelines(new_lines)
            return True

        return False

    except Exception as e:
        print(f"Error processing {filepath}: {e}")
        return False


def process_directory(directory):
    """Process all C source files in the directory."""
    directory = Path(directory)

    if not directory.exists():
        print(f"Warning: Directory {directory} does not exist")
        return 0, 0

    processed_count = 0
    skipped_count = 0

    for root, dirs, files in os.walk(directory):
        for filename in files:
            if not filename.endswith('.c'):
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
    """Main entry point."""
    print("Replacing magic numbers with defined constants...")
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
