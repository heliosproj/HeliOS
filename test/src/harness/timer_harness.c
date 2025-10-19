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
static void test_timer_creation_and_period(void);
static void test_timer_activation_and_expiration(void);
static void test_timer_control_operations(void);
static void test_timer_delete(void);


void timer_harness(void) {
  unit_print("=== COMPREHENSIVE TIMER TEST SUITE ===");

  test_timer_creation_and_period();
  test_timer_activation_and_expiration();
  test_timer_control_operations();
  test_timer_delete();

  unit_print("=== TIMER TEST SUITE COMPLETE ===");
}


/* ============================================================================
 * SECTION 1: TIMER CREATION AND PERIOD MANAGEMENT
 * ============================================================================ */
static void test_timer_creation_and_period(void) {
  Timer_t *timer = null;
  Ticks_t retrievedPeriod = nil;
  Ticks_t newPeriod = nil;


  unit_print("--- Section 1: Timer Creation and Period Management ---");

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
 * SECTION 2: TIMER ACTIVATION AND EXPIRATION
 * ============================================================================ */
static void test_timer_activation_and_expiration(void) {
  Timer_t *timer = null;
  Base_t isActive = nil;
  Base_t hasExpiredBefore = nil;
  Base_t hasExpiredAfter = nil;


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
 * SECTION 3: TIMER CONTROL OPERATIONS
 * ============================================================================ */
static void test_timer_control_operations(void) {
  Timer_t *timer = null;
  Base_t isActiveAfterStop = nil;
  Base_t isActiveAfterStart = nil;
  Base_t hasExpiredBeforeReset = nil;
  Base_t hasExpiredAfterReset = nil;


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
 * SECTION 4: TIMER DELETE AND CLEANUP
 * ============================================================================ */
static void test_timer_delete(void) {
  Timer_t *timer = null;
  Ticks_t period = nil;


  unit_print("--- Section 4: Timer Delete and Cleanup ---");

  /* Test 4.1: Timer delete invalidates handle */
  unit_begin("Timer delete invalidates timer handle");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_1_SECOND));

  /* Delete should succeed */
  unit_assert_ok(xTimerDelete(timer));

  /* Operations on deleted timer should fail */
  unit_assert_not_ok(xTimerGetPeriod(timer, &period));
  unit_end();
}
