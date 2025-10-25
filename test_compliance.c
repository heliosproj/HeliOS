/*
 * Test file for compliance checker rules
 * This file intentionally contains violations for testing
 */

#include "types.h"
#include <stdint.h>

void test_numeric_literals(void) {
    /* These should be flagged by MN-002 */
    int bad1 = 5;           /* Should be: Word_t bad1 = 0x5u; */
    int bad2 = 255;         /* Should be: Word_t bad2 = 0xFFu; */
    unsigned bad3 = 100;    /* Should be: Word_t bad3 = 0x64u; */

    /* These should be OK for MN-002 */
    Word_t good1 = 0x5u;
    Word_t good2 = 0xFFu;
    Byte_t good3 = 0x64u;

    /* Edge cases that shouldn't be flagged */
    Word_t ok1 = 0;  /* 0 and 1 are allowed */
    Word_t ok2 = 1;
    Word_t shift = 1 << 4;  /* Shift operations excluded */
    Word_t arr[10];  /* Array size excluded */
}

void test_type_system(void) {
    /* These should be flagged by TS-001 */
    int bad_int = 0x5u;             /* Should use Word_t */
    unsigned int bad_uint = 0x10u;  /* Should use Word_t */
    char bad_char = 'A';            /* Should use Byte_t */
    uint32_t bad_u32 = 0x100u;      /* Should use Word_t */
    uint16_t bad_u16 = 0x10u;       /* Should use HalfWord_t */
    uint8_t bad_u8 = 0x5u;          /* Should use Byte_t */
    size_t bad_size = 0x100u;       /* Should use Size_t */
    long bad_long = 0x1000l;        /* Should use Word_t */

    /* These should be OK */
    Word_t good_word = 0x100u;
    Byte_t good_byte = 0x5u;
    HalfWord_t good_half = 0x100u;
    Size_t good_size = 0x1000u;
    Return_t result = ReturnOK;
}

/* Function parameters should also be checked */
void test_parameters(int bad_param, unsigned bad_param2, char *bad_str) {
    /* Should use: Word_t, Word_t, Byte_t* */
}

/* Correct function */
void good_function(Word_t param1, Byte_t *param2, Size_t size) {
    /* This is correct */
}