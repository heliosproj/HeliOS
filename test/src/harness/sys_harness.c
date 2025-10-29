/*UNCRUSTIFY-OFF*/
/**
 * @file sys_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Comprehensive unit test harness for system (sys)
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "sys_harness.h"
#include "../../src/sys.h"
#include "../../src/console.h"


/* Test constants */
#define ASSERT_TEST_VALUE_256 0x100
#define ASSERT_TEST_VALUE_512 0x200
#define INIT_CALL_COUNT 5 /* Number of times to call xSystemInit */
#define STRESS_INIT_COUNT 100 /* Stress test initialization count */
#define TEST_POINTER_ADDR 0x12345678u /* Arbitrary test address */
#define PRODUCT_NAME_MIN_LENGTH 1 /* Minimum product name length */
#define MAX_CONCURRENT_CALLS 50 /* Maximum concurrent system calls */

/* Helper function prototypes */
static void test_system_initialization(void);
static void test_system_information(void);
static void test_system_critical_functions(void);
static void test_enhanced_assertions(void);
static void test_null_pointer_validation(void);
static void test_system_info_stress(void);
static void test_system_info_content_validation(void);
static void test_concurrent_system_operations(void);
static void test_assertion_boundary_conditions(void);
static void test_scheduler_state_integration(void);


void sys_harness(void) {
  unit_print("=== COMPREHENSIVE SYSTEM TEST SUITE ===");
  test_system_initialization();
  test_system_information();
  test_null_pointer_validation();
  test_system_info_stress();
  test_system_info_content_validation();
  test_system_critical_functions();
  test_enhanced_assertions();
  test_assertion_boundary_conditions();
  test_concurrent_system_operations();
  test_scheduler_state_integration();
  unit_print("=== SYSTEM TEST SUITE COMPLETE ===");
}


/* ============================================================================
 * SECTION 1: SYSTEM INITIALIZATION
 * ============================================================================
 */
static void test_system_initialization(void) {
  Base_t i;


  unit_print("--- Section 1: System Initialization ---");


  /* Test 1.1: Single initialization call succeeds */
  unit_begin("System initialization succeeds on first call");
  unit_assert_ok(xSystemInit());
  unit_end();


  /* Test 1.2: Multiple initialization calls succeed (idempotent) */
  unit_begin("System initialization is idempotent across multiple calls");

  for(i = 0x0u; i < INIT_CALL_COUNT; i++) {
    unit_assert_ok(xSystemInit());
  }

  unit_end();


  /* Test 1.3: Stress test - many rapid initializations */
  unit_begin("System handles stress test of rapid initializations");

  for(i = 0x0u; i < STRESS_INIT_COUNT; i++) {
    unit_assert_ok(xSystemInit());
  }

  unit_end();


  /* Test 1.4: Initialization after memory operations */
  unit_begin("System initialization works after memory allocations");
  {
    volatile Addr_t *ptr = null;


    /* Allocate some memory */
    unit_assert_ok(xMemAlloc(&ptr, 1024));

    /* Initialize system */
    unit_assert_ok(xSystemInit());

    /* Free memory */
    unit_assert_ok(xMemFree(ptr));
  } unit_end();


  /* Test 1.5: Initialization state persistence */
  unit_begin("System maintains initialization state across calls");
  unit_assert_ok(xSystemInit());
  unit_assert_ok(xSystemInit());
  unit_assert_ok(xSystemInit());
  unit_end();
}


/* ============================================================================
 * SECTION 2: SYSTEM INFORMATION
 * ============================================================================
 */
static void test_system_information(void) {
  SystemInfo_t *info = null;


  unit_print("--- Section 2: System Information ---");


  /* Test 2.1: System information retrieval */
  unit_begin("System information retrieval returns valid product data");
  unit_assert_ok(xSystemGetSystemInfo(&info));
  unit_assert_not_null(info);


  /* Verify system info contains valid data */
  unit_assert_true(info->productName[0] != '\0');


  /* Free allocated memory */
  unit_assert_ok(xMemFree(info));
  unit_end();


  /* Test 2.2: System info structure validity */
  unit_begin("System info structure contains valid fields");
  unit_assert_ok(xSystemGetSystemInfo(&info));
  unit_assert_not_null(info);


  /* Check product name is not empty */
  unit_assert_true(__strlen__(info->productName) >= PRODUCT_NAME_MIN_LENGTH);


  /* Check version components exist */
  unit_assert_true(info->majorVersion >= 0x0u);
  unit_assert_true(info->minorVersion >= 0x0u);
  unit_assert_true(info->patchVersion >= 0x0u);

  unit_assert_ok(xMemFree(info));
  unit_end();


  /* Test 2.3: Multiple consecutive info retrievals */
  unit_begin("Multiple system info retrievals work correctly");
  {
    SystemInfo_t *info1 = null;
    SystemInfo_t *info2 = null;
    SystemInfo_t *info3 = null;


    unit_assert_ok(xSystemGetSystemInfo(&info1));
    unit_assert_ok(xSystemGetSystemInfo(&info2));
    unit_assert_ok(xSystemGetSystemInfo(&info3));

    unit_assert_not_null(info1);
    unit_assert_not_null(info2);
    unit_assert_not_null(info3);

    /* Verify all return same product name */
    unit_assert_equal(__strcmp__(info1->productName, info2->productName), 0x0u);
    unit_assert_equal(__strcmp__(info2->productName, info3->productName), 0x0u);

    /* Verify version numbers match */
    unit_assert_equal(info1->majorVersion, info2->majorVersion);
    unit_assert_equal(info2->majorVersion, info3->majorVersion);

    unit_assert_ok(xMemFree(info1));
    unit_assert_ok(xMemFree(info2));
    unit_assert_ok(xMemFree(info3));
  } unit_end();


  /* Test 2.4: System info after initialization */
  unit_begin("System info retrieval works after reinitialization");
  unit_assert_ok(xSystemInit());
  unit_assert_ok(xSystemGetSystemInfo(&info));
  unit_assert_not_null(info);
  unit_assert_true(info->productName[0] != '\0');
  unit_assert_ok(xMemFree(info));
  unit_end();
}


/* ============================================================================
 * SECTION 3: NULL POINTER VALIDATION
 * ============================================================================
 */
static void test_null_pointer_validation(void) {
  unit_print("--- Section 3: NULL Pointer Validation ---");


  /* Test 3.1: NULL pointer in xSystemGetSystemInfo */
  unit_begin("xSystemGetSystemInfo rejects NULL pointer");
  unit_assert_not_ok(xSystemGetSystemInfo(null));
  unit_end();


  /* Test 3.2: Multiple NULL pointer calls */
  unit_begin("Multiple NULL pointer calls handled consistently");
  unit_assert_not_ok(xSystemGetSystemInfo(null));
  unit_assert_not_ok(xSystemGetSystemInfo(null));
  unit_assert_not_ok(xSystemGetSystemInfo(null));
  unit_end();


  /* Test 3.3: NULL pointer alternating with valid calls */
  unit_begin("NULL pointer validation doesn't affect valid calls");
  {
    SystemInfo_t *info = null;


    unit_assert_not_ok(xSystemGetSystemInfo(null));
    unit_assert_ok(xSystemGetSystemInfo(&info));
    unit_assert_not_null(info);
    unit_assert_ok(xMemFree(info));

    info = null;
    unit_assert_not_ok(xSystemGetSystemInfo(null));
    unit_assert_ok(xSystemGetSystemInfo(&info));
    unit_assert_not_null(info);
    unit_assert_ok(xMemFree(info));
  } unit_end();
}


/* ============================================================================
 * SECTION 4: SYSTEM INFO STRESS TESTING
 * ============================================================================
 */
static void test_system_info_stress(void) {
  Base_t i;
  SystemInfo_t *infoArray[50];


  unit_print("--- Section 4: System Info Stress Testing ---");


  /* Test 4.1: Rapid sequential info retrievals */
  unit_begin("Rapid sequential system info retrievals work correctly");

  for(i = 0x0u; i < 50; i++) {
    infoArray[i] = null;
    unit_assert_ok(xSystemGetSystemInfo(&infoArray[i]));
    unit_assert_not_null(infoArray[i]);
  }


  /* Verify all have valid data */
  for(i = 0x0u; i < 50; i++) {
    unit_assert_true(infoArray[i]->productName[0] != '\0');
  }


  /* Free all */
  for(i = 0x0u; i < 50; i++) {
    unit_assert_ok(xMemFree(infoArray[i]));
  }

  unit_end();


  /* Test 4.2: Interleaved allocate and free */
  unit_begin("Interleaved system info allocate and free operations");

  for(i = 0x0u; i < 25; i++) {
    SystemInfo_t *info1 = null;
    SystemInfo_t *info2 = null;


    unit_assert_ok(xSystemGetSystemInfo(&info1));
    unit_assert_ok(xSystemGetSystemInfo(&info2));

    unit_assert_not_null(info1);
    unit_assert_not_null(info2);

    unit_assert_ok(xMemFree(info1));
    unit_assert_ok(xMemFree(info2));
  }

  unit_end();


  /* Test 4.3: System info with memory pressure */
  unit_begin("System info retrieval under memory pressure");
  {
    volatile Addr_t *memBlocks[20];
    SystemInfo_t *info = null;


    /* Allocate some memory to create pressure */
    for(i = 0x0u; i < 20; i++) {
      if(OK(xMemAlloc(&memBlocks[i], 512))) {
        /* Memory allocated */
      } else {
        memBlocks[i] = null;
      }
    }


    /* Try to get system info */
    unit_assert_ok(xSystemGetSystemInfo(&info));
    unit_assert_not_null(info);
    unit_assert_ok(xMemFree(info));

    /* Free memory blocks */
    for(i = 0x0u; i < 20; i++) {
      if(null != memBlocks[i]) {
        xMemFree(memBlocks[i]);
      }
    }
  } unit_end();
}


/* ============================================================================
 * SECTION 5: SYSTEM INFO CONTENT VALIDATION
 * ============================================================================
 */
static void test_system_info_content_validation(void) {
  SystemInfo_t *info = null;


  unit_print("--- Section 5: System Info Content Validation ---");


  /* Test 5.1: Product name format validation */
  unit_begin("Product name has valid format");
  unit_assert_ok(xSystemGetSystemInfo(&info));
  unit_assert_not_null(info);


  /* Check product name length */
  {
    Size_t nameLen = __strlen__(info->productName);


    unit_assert_true(nameLen > 0x0u);
    unit_assert_true(nameLen <= OS_PRODUCT_NAME_SIZE);
  }


  /* Check for null termination */
  unit_assert_equal(info->productName[OS_PRODUCT_NAME_SIZE - 1], '\0');

  unit_assert_ok(xMemFree(info));
  unit_end();


  /* Test 5.2: Version number consistency */
  unit_begin("Version numbers are consistent across calls");
  {
    SystemInfo_t *info1 = null;
    SystemInfo_t *info2 = null;


    unit_assert_ok(xSystemGetSystemInfo(&info1));
    unit_assert_ok(xSystemGetSystemInfo(&info2));

    unit_assert_equal(info1->majorVersion, info2->majorVersion);
    unit_assert_equal(info1->minorVersion, info2->minorVersion);
    unit_assert_equal(info1->patchVersion, info2->patchVersion);

    unit_assert_ok(xMemFree(info1));
    unit_assert_ok(xMemFree(info2));
  } unit_end();


  /* Test 5.3: Little endian flag consistency */
  unit_begin("Little endian flag remains consistent");
  {
    SystemInfo_t *info1 = null;
    SystemInfo_t *info2 = null;


    unit_assert_ok(xSystemGetSystemInfo(&info1));
    unit_assert_ok(xSystemGetSystemInfo(&info2));

    /* Little endian flag should be consistent */
    unit_assert_equal(info1->littleEndian, info2->littleEndian);

    unit_assert_ok(xMemFree(info1));
    unit_assert_ok(xMemFree(info2));
  } unit_end();


  /* Test 5.4: Valid flag validation */
  unit_begin("Valid flag is set correctly");
  unit_assert_ok(xSystemGetSystemInfo(&info));
  unit_assert_not_null(info);

  /* Check valid flag is set */
  unit_assert_true(info->valid != 0x0u);

  unit_assert_ok(xMemFree(info));
  unit_end();
}


/* ============================================================================
 * SECTION 6: SYSTEM CRITICAL FUNCTIONS
 * ============================================================================
 */
static void test_system_critical_functions(void) {
  unit_print("--- Section 6: System Critical Functions ---");


  /* Test 6.1: System initialization continues normal operation */
  unit_begin("System initialization allows continued operation");

  /* Initialize system */
  unit_assert_ok(xSystemInit());

  /* Verify operations work after init */
  {
    SystemInfo_t *info = null;


    unit_assert_ok(xSystemGetSystemInfo(&info));
    unit_assert_not_null(info);
    unit_assert_ok(xMemFree(info));
  }

  unit_end();


  /* Test 6.2: System functions available after initialization */
  unit_begin("All system functions available after initialization");
  unit_assert_ok(xSystemInit());

  /* Test that we can call various system functions */
  {
    SystemInfo_t *info = null;


    unit_assert_ok(xSystemGetSystemInfo(&info));
    unit_assert_not_null(info);
    unit_assert_ok(xMemFree(info));
  }

  unit_end();


  /* Test 6.3: System state maintained across operations */
  unit_begin("System state maintained across various operations");
  unit_assert_ok(xSystemInit());

  /* Do some operations */
  {
    volatile Addr_t *ptr = null;
    SystemInfo_t *info = null;


    unit_assert_ok(xMemAlloc(&ptr, 100));
    unit_assert_ok(xSystemGetSystemInfo(&info));

    unit_assert_not_null(ptr);
    unit_assert_not_null(info);

    unit_assert_ok(xMemFree(ptr));
    unit_assert_ok(xMemFree(info));
  }

  unit_end();
}


/* ============================================================================
 * SECTION 7: ENHANCED ASSERTION MACROS
 * ============================================================================
 */
static void test_enhanced_assertions(void) {
  Size_t size1 = ASSERT_TEST_VALUE_256;
  Size_t size2 = ASSERT_TEST_VALUE_256;
  Size_t size3 = ASSERT_TEST_VALUE_512;
  Base_t *ptr1 = null;
  Base_t *ptr2 = (Base_t *) TEST_POINTER_ADDR;
  Base_t zero = 0x0u;
  Base_t one = 0x1u;


  unit_print("--- Section 7: Enhanced Assertion Macros ---");


  /* Test 7.1: Equality checks */
  unit_begin("Assertion equality checks work correctly");

  /* These should pass */
  unit_assert_equal(size1, size2);
  unit_assert_equal(ASSERT_TEST_VALUE_256, size1);
  unit_assert_equal(0x100, ASSERT_TEST_VALUE_256);

  unit_end();


  /* Test 7.2: Inequality checks */
  unit_begin("Assertion inequality checks work correctly");

  /* These should pass */
  unit_assert_not_equal(size1, size3);
  unit_assert_not_equal(ASSERT_TEST_VALUE_256, ASSERT_TEST_VALUE_512);
  unit_assert_not_equal(0x100, 0x200);

  unit_end();


  /* Test 7.3: Pointer checks */
  unit_begin("Assertion pointer checks work correctly");

  /* These should pass */
  unit_assert_null(ptr1);
  unit_assert_not_null(ptr2);

  unit_end();


  /* Test 7.4: Boolean checks */
  unit_begin("Assertion boolean checks work correctly");

  /* These should pass */
  unit_assert_true(size1 == size2);
  unit_assert_false(size1 == size3);
  unit_assert_true(one);
  unit_assert_false(zero);
  unit_assert_true(1);
  unit_assert_false(0);

  unit_end();


  /* Test 7.5: Complex expressions */
  unit_begin("Assertion macros handle complex expressions");

  unit_assert_true((size1 + size2) == (ASSERT_TEST_VALUE_256 * 2));
  unit_assert_false((size1 + size3) == size1);
  unit_assert_equal((size1 + size2), (ASSERT_TEST_VALUE_256 + ASSERT_TEST_VALUE_256));

  unit_end();
}


/* ============================================================================
 * SECTION 8: ASSERTION BOUNDARY CONDITIONS
 * ============================================================================
 */
static void test_assertion_boundary_conditions(void) {
  Size_t maxSize = 0xFFFFFFFF;
  Size_t minSize = 0x0u;
  Base_t *nullPtr = null;


  unit_print("--- Section 8: Assertion Boundary Conditions ---");


  /* Test 8.1: Maximum value comparisons */
  unit_begin("Assertions handle maximum value comparisons");
  unit_assert_equal(maxSize, 0xFFFFFFFF);
  unit_assert_not_equal(maxSize, minSize);
  unit_assert_true(maxSize > minSize);
  unit_end();


  /* Test 8.2: Minimum value comparisons */
  unit_begin("Assertions handle minimum value comparisons");
  unit_assert_equal(minSize, 0x0u);
  unit_assert_not_equal(minSize, maxSize);
  unit_assert_false(minSize > maxSize);
  unit_end();


  /* Test 8.3: NULL pointer edge cases */
  unit_begin("Assertions handle NULL pointer edge cases");
  unit_assert_null(nullPtr);
  unit_assert_null(null);
  unit_assert_true(nullPtr == null);
  unit_end();


  /* Test 8.4: Boundary arithmetic */
  unit_begin("Assertions handle boundary arithmetic correctly");
  {
    Size_t a = 0xFFFFFFFF;
    Size_t b = 0x1u;


    /* Test overflow conditions - values should wrap */
    unit_assert_not_equal((a + b), a);
    unit_assert_equal((minSize - b), maxSize);
  } unit_end();


  /* Test 8.5: Zero comparisons */
  unit_begin("Assertions handle zero comparisons");
  unit_assert_equal(0x0u, minSize);
  unit_assert_true(minSize == 0x0u);
  unit_assert_false(minSize != 0x0u);
  unit_assert_equal(0, 0x0u);
  unit_end();
}


/* ============================================================================
 * SECTION 9: CONCURRENT SYSTEM OPERATIONS
 * ============================================================================
 */
static void test_concurrent_system_operations(void) {
  Base_t i;


  unit_print("--- Section 9: Concurrent System Operations ---");


  /* Test 9.1: Concurrent initialization calls */
  unit_begin("Concurrent initialization calls work correctly");

  for(i = 0x0u; i < MAX_CONCURRENT_CALLS; i++) {
    unit_assert_ok(xSystemInit());
  }

  unit_end();


  /* Test 9.2: Concurrent system info retrievals */
  unit_begin("Concurrent system info retrievals work correctly");
  {
    SystemInfo_t *infoArray[25];
    Base_t j;


    /* Allocate multiple info structures */
    for(i = 0x0u; i < 25; i++) {
      infoArray[i] = null;
      unit_assert_ok(xSystemGetSystemInfo(&infoArray[i]));
      unit_assert_not_null(infoArray[i]);
    }


    /* Verify all have consistent data */
    for(i = 0x0u; i < 24; i++) {
      unit_assert_equal(__strcmp__(infoArray[i]->productName, infoArray[i + 1]->productName), 0x0u);
    }


    /* Free all */
    for(j = 0x0u; j < 25; j++) {
      unit_assert_ok(xMemFree(infoArray[j]));
    }
  } unit_end();


  /* Test 9.3: Interleaved init and info calls */
  unit_begin("Interleaved init and info calls work correctly");

  for(i = 0x0u; i < 10; i++) {
    SystemInfo_t *info = null;


    unit_assert_ok(xSystemInit());
    unit_assert_ok(xSystemGetSystemInfo(&info));
    unit_assert_not_null(info);
    unit_assert_ok(xMemFree(info));
  }

  unit_end();


  /* Test 9.4: System operations with memory operations */
  unit_begin("System operations work concurrently with memory operations");

  for(i = 0x0u; i < 20; i++) {
    volatile Addr_t *ptr = null;
    SystemInfo_t *info = null;


    unit_assert_ok(xMemAlloc(&ptr, 256));
    unit_assert_ok(xSystemInit());
    unit_assert_ok(xSystemGetSystemInfo(&info));

    unit_assert_not_null(ptr);
    unit_assert_not_null(info);

    unit_assert_ok(xMemFree(ptr));
    unit_assert_ok(xMemFree(info));
  }

  unit_end();
}


/* ============================================================================
 * SECTION 10: SCHEDULER STATE INTEGRATION
 * ============================================================================
 */
static void test_scheduler_state_integration(void) {
  unit_print("--- Section 10: Scheduler State Integration ---");


  /* Test 10.1: System info retrieval with scheduler operations */
  unit_begin("System info works with scheduler state checks");
  {
    SystemInfo_t *info = null;
    SchedulerState_t schedState;


    unit_assert_ok(xSystemInit());
    unit_assert_ok(xSystemGetSystemInfo(&info));
    unit_assert_not_null(info);

    /* Try to get scheduler state */
    if(OK(xTaskGetSchedulerState(&schedState))) {
      /* Scheduler state available - verify it's valid */
      unit_assert_true((schedState == SchedulerStateRunning) || (schedState == SchedulerStateSuspended));
    }

    unit_assert_ok(xMemFree(info));
  } unit_end();


  /* Test 10.2: System operations across scheduler state changes */
  unit_begin("System operations work across scheduler state changes");
  {
    SystemInfo_t *info1 = null;
    SystemInfo_t *info2 = null;


    /* Get info in initial state */
    unit_assert_ok(xSystemGetSystemInfo(&info1));
    unit_assert_not_null(info1);

    /* Suspend scheduler */
    unit_assert_ok(xTaskSuspendAll());

    /* Get info while suspended */
    unit_assert_ok(xSystemGetSystemInfo(&info2));
    unit_assert_not_null(info2);

    /* Verify info is consistent */
    unit_assert_equal(__strcmp__(info1->productName, info2->productName), 0x0u);

    /* Resume scheduler */
    unit_assert_ok(xTaskResumeAll());

    unit_assert_ok(xMemFree(info1));
    unit_assert_ok(xMemFree(info2));
  } unit_end();


  /* Test 10.3: System initialization with scheduler state */
  unit_begin("System initialization works with various scheduler states");

  /* Initialize in running state */
  unit_assert_ok(xSystemInit());

  /* Suspend scheduler and initialize */
  unit_assert_ok(xTaskSuspendAll());
  unit_assert_ok(xSystemInit());
  unit_assert_ok(xTaskResumeAll());

  /* Initialize again after resume */
  unit_assert_ok(xSystemInit());

  unit_end();


  /* Test 10.4: Rapid system calls with scheduler toggling */
  unit_begin("Rapid system calls work with scheduler state toggling");
  {
    Base_t i;


    for(i = 0x0u; i < 10; i++) {
      SystemInfo_t *info = null;


      unit_assert_ok(xTaskSuspendAll());
      unit_assert_ok(xSystemInit());
      unit_assert_ok(xSystemGetSystemInfo(&info));
      unit_assert_not_null(info);
      unit_assert_ok(xTaskResumeAll());

      unit_assert_ok(xMemFree(info));
    }
  } unit_end();
}
