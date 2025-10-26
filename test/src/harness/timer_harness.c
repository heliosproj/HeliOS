/*UNCRUSTIFY-OFF*/
/**
 * @file timer_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Comprehensive unit test harness for timers
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 HeliOS Project <license@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "timer_harness.h"


/* Test constants for timer periods (in ticks/milliseconds) */
#define TIMER_PERIOD_1_SECOND 0x3E8 /* 1000 ms */
#define TIMER_PERIOD_2_SECONDS 0x7D0 /* 2000 ms */
#define TIMER_WAIT_SECONDS 3 /* Sleep duration for expiration tests */
/* Helper function prototypes */
static void test_error_handling_and_null_pointers(void);
static void test_timer_creation_and_period(void);
static void test_timer_activation_and_expiration(void);
static void test_timer_control_operations(void);
static void test_timer_delete(void);


void timer_harness(void) {
  unit_print("=== COMPREHENSIVE TIMER TEST SUITE ===");
  test_error_handling_and_null_pointers();
  test_timer_creation_and_period();
  test_timer_activation_and_expiration();
  test_timer_control_operations();
  test_timer_delete();
  unit_print("=== TIMER TEST SUITE COMPLETE ===");
}


/* ============================================================================
 * SECTION 1: ERROR HANDLING AND NULL POINTER VALIDATION
 * ============================================================================
 */
static void test_error_handling_and_null_pointers(void) {
  Timer_t *timer = null;
  Timer_t *nullTimer = null;
  Base_t result;
  Ticks_t period;


  unit_print("--- Section 1: Error Handling and NULL Pointer Tests ---");


  /* Test 1.1: NULL pointer in xTimerCreate */
  unit_begin("xTimerCreate with NULL pointer returns error");
  unit_assert_not_ok(xTimerCreate(null, TIMER_PERIOD_1_SECOND));
  unit_end();


  /* Test 1.2: Create a valid timer for error tests */
  unit_begin("Create valid timer for error testing");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_1_SECOND));
  unit_assert_not_null(timer);
  unit_end();


  /* Test 1.3: NULL pointer in xTimerDelete */
  unit_begin("xTimerDelete with NULL pointer returns error");
  unit_assert_not_ok(xTimerDelete(nullTimer));
  unit_end();


  /* Test 1.4: NULL pointer in xTimerChangePeriod */
  unit_begin("xTimerChangePeriod with NULL timer returns error");
  unit_assert_not_ok(xTimerChangePeriod(nullTimer, TIMER_PERIOD_2_SECONDS));
  unit_end();


  /* Test 1.5: NULL pointer in xTimerGetPeriod */
  unit_begin("xTimerGetPeriod with NULL timer returns error");
  unit_assert_not_ok(xTimerGetPeriod(nullTimer, &period));
  unit_end();


  /* Test 1.6: NULL pointer in xTimerIsTimerActive */
  unit_begin("xTimerIsTimerActive with NULL timer returns error");
  unit_assert_not_ok(xTimerIsTimerActive(nullTimer, &result));
  unit_end();


  /* Test 1.7: NULL pointer in xTimerReset */
  unit_begin("xTimerReset with NULL timer returns error");
  unit_assert_not_ok(xTimerReset(nullTimer));
  unit_end();


  /* Test 1.8: NULL pointer in xTimerStart */
  unit_begin("xTimerStart with NULL timer returns error");
  unit_assert_not_ok(xTimerStart(nullTimer));
  unit_end();


  /* Test 1.9: NULL pointer in xTimerStop */
  unit_begin("xTimerStop with NULL timer returns error");
  unit_assert_not_ok(xTimerStop(nullTimer));
  unit_end();


  /* Cleanup */
  unit_begin("Cleanup error test timer");
  unit_assert_ok(xTimerDelete(timer));
  unit_end();
}


/* ============================================================================
 * SECTION 2: TIMER CREATION AND PERIOD MANAGEMENT
 * ============================================================================
 */
static void test_timer_creation_and_period(void) {
  Timer_t *timer = null;
  Ticks_t retrievedPeriod = 0x0u;
  Ticks_t newPeriod = 0x0u;


  unit_print("--- Section 2: Timer Creation and Period Management ---");


  /* Test 1.1: Timer creation */
  unit_begin("Timer creation with specified period succeeds");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_1_SECOND));
  unit_assert_not_null(timer);
  unit_end();


  /* Test 1.2: Period retrieval */
  unit_begin("Timer period retrieval returns configured value");
  unit_assert_ok(xTimerGetPeriod(timer, &retrievedPeriod));
  unit_assert_equal(retrievedPeriod, TIMER_PERIOD_1_SECOND);
  unit_end();


  /* Test 1.3: Period change */
  unit_begin("Timer period change updates period value");
  unit_assert_ok(xTimerChangePeriod(timer, TIMER_PERIOD_2_SECONDS));
  unit_assert_ok(xTimerGetPeriod(timer, &newPeriod));
  unit_assert_equal(newPeriod, TIMER_PERIOD_2_SECONDS);
  unit_end();


  /* Cleanup for next section */
  xTimerDelete(timer);
}


/* ============================================================================
 * SECTION 3: TIMER ACTIVATION AND EXPIRATION
 * ============================================================================
 */
static void test_timer_activation_and_expiration(void) {
  Timer_t *timer = null;
  Base_t isActive = 0x0u;
  Base_t hasExpiredBefore = 0x0u;
  Base_t hasExpiredAfter = 0x0u;


  unit_print("--- Section 2: Timer Activation and Expiration ---");


  /* Test 2.1: Inactive timer check */
  unit_begin("Timer active check returns false for inactive timer");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_2_SECONDS));
  unit_assert_ok(xTimerIsTimerActive(timer, &isActive));
  unit_assert_false(isActive);
  unit_end();


  /* Test 2.2: Timer expiration detection */
  unit_begin("Timer expiration check detects timeout after period elapses");


  /* Before starting, should not be expired */
  unit_assert_not_ok(xTimerHasTimerExpired(timer, &hasExpiredBefore));
  unit_assert_false(hasExpiredBefore);


  /* Start and wait for expiration */
  unit_assert_ok(xTimerStart(timer));
  sleep(TIMER_WAIT_SECONDS);


  /* After waiting, should be expired */
  unit_assert_ok(xTimerHasTimerExpired(timer, &hasExpiredAfter));
  unit_assert_true(hasExpiredAfter);
  unit_end();


  /* Cleanup for next section */
  xTimerDelete(timer);
}


/* ============================================================================
 * SECTION 4: TIMER CONTROL OPERATIONS
 * ============================================================================
 */
static void test_timer_control_operations(void) {
  Timer_t *timer = null;
  Base_t isActiveAfterStop = 0x0u;
  Base_t isActiveAfterStart = 0x0u;
  Base_t hasExpiredBeforeReset = 0x0u;
  Base_t hasExpiredAfterReset = 0x0u;


  unit_print("--- Section 3: Timer Control Operations ---");


  /* Test 3.1: Timer stop */
  unit_begin("Timer stop deactivates running timer");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_2_SECONDS));
  unit_assert_ok(xTimerStart(timer));
  unit_assert_ok(xTimerStop(timer));
  unit_assert_ok(xTimerIsTimerActive(timer, &isActiveAfterStop));
  unit_assert_false(isActiveAfterStop);
  unit_end();


  /* Test 3.2: Timer start */
  unit_begin("Timer start activates timer");
  unit_assert_ok(xTimerStart(timer));
  unit_assert_ok(xTimerIsTimerActive(timer, &isActiveAfterStart));
  unit_assert_true(isActiveAfterStart);
  unit_end();


  /* Test 3.3: Timer reset */
  unit_begin("Timer reset clears expiration status");


  /* Wait for expiration */
  sleep(TIMER_WAIT_SECONDS);
  unit_assert_ok(xTimerHasTimerExpired(timer, &hasExpiredBeforeReset));
  unit_assert_true(hasExpiredBeforeReset);


  /* Reset and verify not expired */
  unit_assert_ok(xTimerReset(timer));
  unit_assert_ok(xTimerHasTimerExpired(timer, &hasExpiredAfterReset));
  unit_assert_false(hasExpiredAfterReset);
  unit_end();


  /* Cleanup for next section */
  xTimerDelete(timer);
}


/* ============================================================================
 * SECTION 5: TIMER DELETE AND CLEANUP
 * ============================================================================
 */
static void test_timer_delete(void) {
  Timer_t *timer = null;
  Ticks_t period = 0x0u;


  unit_print("--- Section 5: Timer Delete and Cleanup ---");


  /* Test 4.1: Timer delete invalidates handle */
  unit_begin("Timer delete invalidates timer handle");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_1_SECOND));


  /* Delete should succeed */
  unit_assert_ok(xTimerDelete(timer));


  /* Operations on deleted timer should fail */
  unit_assert_not_ok(xTimerGetPeriod(timer, &period));
  unit_end();
}