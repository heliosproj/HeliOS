/*
 * Test file for enhanced compliance checker
 */

#include "helios.h"

/* Test function with proper structure */
Return_t xTestFunction(Base_t param_) {
    FUNCTION_ENTER;

    /* Check pointer parameter */
    if (__PointerIsNull__(param_)) {
        __ReturnWithError__(ERROR_NULL_POINTER);
    }

    /* Memory allocation with check */
    void *memory_ = null;
    if (OK(__KernelAllocateMemory__(&memory_, 100))) {
        /* Use memory */

        /* Free memory */
        __KernelFreeMemory__(memory_);
        __ReturnOk__();
    } else {
        __SetFlag__(MEMFAULT);
        __AssertOnElse__();
    }

    FUNCTION_EXIT;
}

/* Static function with proper naming */
static Return_t __InternalHelper__(void) {
    FUNCTION_ENTER;

    __ReturnOk__();

    FUNCTION_EXIT;
}

/* Function with early return violation */
int badFunction(void) {
    if (true) {
        return 0; /* This should be flagged */
    }
    return 1;
}

/* Function missing ENTER/EXIT */
void missingMacros(void) {
    /* This should be flagged */
    int x = 0;
    x++;
}