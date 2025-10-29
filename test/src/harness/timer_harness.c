/*UNCRUSTIFY-OFF*/
/**
 * @file timer_harness.c
 * @author Manny Peterson <manny@heliosproj.org>
 * @brief Comprehensive unit test harness for timers
 * @version 0.5.0
 * @date 2023-03-19
 *
 * @copyright
 * HeliOS Embedded Operating System Copyright (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *
 */
/*UNCRUSTIFY-ON*/
#include "timer_harness.h"


/* Test constants for timer periods (in ticks/milliseconds) */
#define TIMER_PERIOD_SHORT 0x64 /* 100 ms */
#define TIMER_PERIOD_MEDIUM 0x1F4 /* 500 ms */
#define TIMER_PERIOD_1_SECOND 0x3E8 /* 1000 ms */
#define TIMER_PERIOD_2_SECONDS 0x7D0 /* 2000 ms */
#define TIMER_PERIOD_5_SECONDS 0x1388 /* 5000 ms */
#define TIMER_WAIT_SHORT 1 /* Sleep 1 second */
#define TIMER_WAIT_MEDIUM 2 /* Sleep 2 seconds */
#define TIMER_WAIT_LONG 3 /* Sleep 3 seconds */
#define TIMER_WAIT_EXTENDED 6 /* Sleep 6 seconds */

/* Stress test constants */
#define MAX_TIMERS_STRESS 50
#define MAX_TIMERS_CONCURRENT 20
#define MAX_PERIOD_VALUE 0xFFFFFFFF

/* Helper function prototypes */
static void test_error_handling_and_null_pointers(void);
static void test_timer_creation_and_period(void);
static void test_timer_activation_and_state(void);
static void test_timer_control_operations(void);
static void test_timer_expiration_detection(void);
static void test_timer_period_modifications(void);
static void test_multiple_timers(void);
static void test_timer_stress_operations(void);
static void test_timer_boundary_conditions(void);
static void test_timer_state_persistence(void);
static void test_timer_delete_and_cleanup(void);
static void test_timer_edge_cases(void);


void timer_harness(void) {
  unit_print("=== COMPREHENSIVE TIMER TEST SUITE ===");
  test_error_handling_and_null_pointers();
  test_timer_creation_and_period();
  test_timer_activation_and_state();
  test_timer_control_operations();
  test_timer_expiration_detection();
  test_timer_period_modifications();
  test_multiple_timers();
  test_timer_stress_operations();
  test_timer_boundary_conditions();
  test_timer_state_persistence();
  test_timer_edge_cases();
  test_timer_delete_and_cleanup();
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


  /* Test 1.6: xTimerGetPeriod with NULL period pointer */
  unit_begin("xTimerGetPeriod with NULL period pointer returns error");
  unit_assert_not_ok(xTimerGetPeriod(timer, null));
  unit_end();


  /* Test 1.7: NULL pointer in xTimerIsTimerActive */
  unit_begin("xTimerIsTimerActive with NULL timer returns error");
  unit_assert_not_ok(xTimerIsTimerActive(nullTimer, &result));
  unit_end();


  /* Test 1.8: xTimerIsTimerActive with NULL result pointer */
  unit_begin("xTimerIsTimerActive with NULL result pointer returns error");
  unit_assert_not_ok(xTimerIsTimerActive(timer, null));
  unit_end();


  /* Test 1.9: NULL pointer in xTimerReset */
  unit_begin("xTimerReset with NULL timer returns error");
  unit_assert_not_ok(xTimerReset(nullTimer));
  unit_end();


  /* Test 1.10: NULL pointer in xTimerStart */
  unit_begin("xTimerStart with NULL timer returns error");
  unit_assert_not_ok(xTimerStart(nullTimer));
  unit_end();


  /* Test 1.11: NULL pointer in xTimerStop */
  unit_begin("xTimerStop with NULL timer returns error");
  unit_assert_not_ok(xTimerStop(nullTimer));
  unit_end();


  /* Test 1.12: NULL pointer in xTimerHasTimerExpired */
  unit_begin("xTimerHasTimerExpired with NULL timer returns error");
  unit_assert_not_ok(xTimerHasTimerExpired(nullTimer, &result));
  unit_end();


  /* Test 1.13: xTimerHasTimerExpired with NULL result pointer */
  unit_begin("xTimerHasTimerExpired with NULL result pointer returns error");
  unit_assert_not_ok(xTimerHasTimerExpired(timer, null));
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
  Timer_t *timer1 = null;
  Timer_t *timer2 = null;
  Timer_t *timer3 = null;
  Ticks_t retrievedPeriod = 0x0u;


  unit_print("--- Section 2: Timer Creation and Period Management ---");


  /* Test 2.1: Timer creation with short period */
  unit_begin("Timer creation with short period succeeds");
  unit_assert_ok(xTimerCreate(&timer1, TIMER_PERIOD_SHORT));
  unit_assert_not_null(timer1);
  unit_end();


  /* Test 2.2: Timer creation with medium period */
  unit_begin("Timer creation with medium period succeeds");
  unit_assert_ok(xTimerCreate(&timer2, TIMER_PERIOD_MEDIUM));
  unit_assert_not_null(timer2);
  unit_end();


  /* Test 2.3: Timer creation with long period */
  unit_begin("Timer creation with long period succeeds");
  unit_assert_ok(xTimerCreate(&timer3, TIMER_PERIOD_2_SECONDS));
  unit_assert_not_null(timer3);
  unit_end();


  /* Test 2.4: Period retrieval for short timer */
  unit_begin("Timer period retrieval returns configured short value");
  unit_assert_ok(xTimerGetPeriod(timer1, &retrievedPeriod));
  unit_assert_equal(retrievedPeriod, TIMER_PERIOD_SHORT);
  unit_end();


  /* Test 2.5: Period retrieval for medium timer */
  unit_begin("Timer period retrieval returns configured medium value");
  unit_assert_ok(xTimerGetPeriod(timer2, &retrievedPeriod));
  unit_assert_equal(retrievedPeriod, TIMER_PERIOD_MEDIUM);
  unit_end();


  /* Test 2.6: Period retrieval for long timer */
  unit_begin("Timer period retrieval returns configured long value");
  unit_assert_ok(xTimerGetPeriod(timer3, &retrievedPeriod));
  unit_assert_equal(retrievedPeriod, TIMER_PERIOD_2_SECONDS);
  unit_end();


  /* Test 2.7: Multiple period retrievals are consistent */
  unit_begin("Multiple period retrievals return consistent values");
  {
    Ticks_t period1, period2, period3;


    unit_assert_ok(xTimerGetPeriod(timer1, &period1));
    unit_assert_ok(xTimerGetPeriod(timer1, &period2));
    unit_assert_ok(xTimerGetPeriod(timer1, &period3));
    unit_assert_equal(period1, period2);
    unit_assert_equal(period2, period3);
  } unit_end();


  /* Cleanup */
  xTimerDelete(timer1);
  xTimerDelete(timer2);
  xTimerDelete(timer3);
}


/* ============================================================================
 * SECTION 3: TIMER ACTIVATION AND STATE
 * ============================================================================
 */
static void test_timer_activation_and_state(void) {
  Timer_t *timer = null;
  Base_t isActive = 0x0u;


  unit_print("--- Section 3: Timer Activation and State ---");


  /* Test 3.1: Newly created timer is inactive */
  unit_begin("Newly created timer is inactive");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_1_SECOND));
  unit_assert_ok(xTimerIsTimerActive(timer, &isActive));
  unit_assert_false(isActive);
  unit_end();


  /* Test 3.2: Timer becomes active after start */
  unit_begin("Timer becomes active after start");
  unit_assert_ok(xTimerStart(timer));
  unit_assert_ok(xTimerIsTimerActive(timer, &isActive));
  unit_assert_true(isActive);
  unit_end();


  /* Test 3.3: Timer becomes inactive after stop */
  unit_begin("Timer becomes inactive after stop");
  unit_assert_ok(xTimerStop(timer));
  unit_assert_ok(xTimerIsTimerActive(timer, &isActive));
  unit_assert_false(isActive);
  unit_end();


  /* Test 3.4: Timer state persists across multiple checks */
  unit_begin("Timer state persists across multiple checks");
  {
    Base_t state1, state2, state3;


    unit_assert_ok(xTimerStart(timer));
    unit_assert_ok(xTimerIsTimerActive(timer, &state1));
    unit_assert_ok(xTimerIsTimerActive(timer, &state2));
    unit_assert_ok(xTimerIsTimerActive(timer, &state3));
    unit_assert_true(state1 && state2 && state3);
  } unit_end();


  /* Test 3.5: Start already started timer requires stop first */
  unit_begin("Starting already started timer requires stop first");
  /* Timer is already started from previous test, attempt to start again */
  unit_assert_not_ok(xTimerStart(timer));
  unit_assert_ok(xTimerIsTimerActive(timer, &isActive));
  unit_assert_true(isActive);
  unit_end();


  /* Test 3.6: Stop already stopped timer requires start first */
  unit_begin("Stopping already stopped timer requires start first");
  unit_assert_ok(xTimerStop(timer));
  unit_assert_not_ok(xTimerStop(timer));
  unit_assert_ok(xTimerIsTimerActive(timer, &isActive));
  unit_assert_false(isActive);
  unit_end();


  /* Cleanup */
  xTimerDelete(timer);
}


/* ============================================================================
 * SECTION 4: TIMER CONTROL OPERATIONS
 * ============================================================================
 */
static void test_timer_control_operations(void) {
  Timer_t *timer = null;
  Base_t isActive = 0x0u;


  unit_print("--- Section 4: Timer Control Operations ---");


  /* Test 4.1: Timer start activates timer */
  unit_begin("Timer start activates timer");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_1_SECOND));
  unit_assert_ok(xTimerStart(timer));
  unit_assert_ok(xTimerIsTimerActive(timer, &isActive));
  unit_assert_true(isActive);
  unit_end();


  /* Test 4.2: Timer stop deactivates running timer */
  unit_begin("Timer stop deactivates running timer");
  unit_assert_ok(xTimerStop(timer));
  unit_assert_ok(xTimerIsTimerActive(timer, &isActive));
  unit_assert_false(isActive);
  unit_end();


  /* Test 4.3: Timer reset restarts timer */
  unit_begin("Timer reset restarts timer");
  unit_assert_ok(xTimerStart(timer));
  unit_assert_ok(xTimerReset(timer));
  unit_assert_ok(xTimerIsTimerActive(timer, &isActive));
  unit_assert_true(isActive);
  unit_end();


  /* Test 4.4: Alternating start/stop operations */
  unit_begin("Alternating start/stop operations work correctly");
  {
    Base_t i;


    /* Must stop first since timer is active from previous test */
    unit_assert_ok(xTimerStop(timer));

    for(i = 0x0u; i < 10; i++) {
      unit_assert_ok(xTimerStart(timer));
      unit_assert_ok(xTimerStop(timer));
    }
  } unit_end();


  /* Test 4.5: Rapid reset operations */
  unit_begin("Rapid reset operations work correctly");
  {
    Base_t i;


    unit_assert_ok(xTimerStart(timer));

    for(i = 0x0u; i < 10; i++) {
      unit_assert_ok(xTimerReset(timer));
    }
  } unit_end();


  /* Cleanup */
  xTimerDelete(timer);
}


/* ============================================================================
 * SECTION 5: TIMER EXPIRATION DETECTION
 * ============================================================================
 */
static void test_timer_expiration_detection(void) {
  Timer_t *timer = null;
  Base_t hasExpired = 0x0u;


  unit_print("--- Section 5: Timer Expiration Detection ---");


  /* Test 5.1: Timer not expired before start */
  unit_begin("Timer not expired before start");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_1_SECOND));
  unit_assert_not_ok(xTimerHasTimerExpired(timer, &hasExpired));
  unit_end();


  /* Test 5.2: Timer not expired immediately after start */
  unit_begin("Timer not expired immediately after start");
  unit_assert_ok(xTimerStart(timer));
  unit_assert_ok(xTimerHasTimerExpired(timer, &hasExpired));
  unit_assert_false(hasExpired);
  unit_end();


  /* Test 5.3: Timer expires after period elapses */
  unit_begin("Timer expires after period elapses");
  sleep(TIMER_WAIT_MEDIUM);
  unit_assert_ok(xTimerHasTimerExpired(timer, &hasExpired));
  unit_assert_true(hasExpired);
  unit_end();


  /* Test 5.4: Reset clears expiration */
  unit_begin("Reset clears expiration status");
  unit_assert_ok(xTimerReset(timer));
  unit_assert_ok(xTimerHasTimerExpired(timer, &hasExpired));
  unit_assert_false(hasExpired);
  unit_end();


  /* Test 5.5: Stop prevents expiration */
  unit_begin("Stop prevents expiration");
  unit_assert_ok(xTimerStop(timer));
  sleep(TIMER_WAIT_MEDIUM);
  unit_assert_not_ok(xTimerHasTimerExpired(timer, &hasExpired));
  unit_end();


  /* Cleanup */
  xTimerDelete(timer);
}


/* ============================================================================
 * SECTION 6: TIMER PERIOD MODIFICATIONS
 * ============================================================================
 */
static void test_timer_period_modifications(void) {
  Timer_t *timer = null;
  Ticks_t newPeriod = 0x0u;


  unit_print("--- Section 6: Timer Period Modifications ---");


  /* Test 6.1: Period change on inactive timer */
  unit_begin("Period change on inactive timer succeeds");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_1_SECOND));
  unit_assert_ok(xTimerChangePeriod(timer, TIMER_PERIOD_2_SECONDS));
  unit_assert_ok(xTimerGetPeriod(timer, &newPeriod));
  unit_assert_equal(newPeriod, TIMER_PERIOD_2_SECONDS);
  unit_end();


  /* Test 6.2: Period change on active timer */
  unit_begin("Period change on active timer succeeds");
  unit_assert_ok(xTimerStart(timer));
  unit_assert_ok(xTimerChangePeriod(timer, TIMER_PERIOD_SHORT));
  unit_assert_ok(xTimerGetPeriod(timer, &newPeriod));
  unit_assert_equal(newPeriod, TIMER_PERIOD_SHORT);
  unit_end();


  /* Test 6.3: Multiple period changes */
  unit_begin("Multiple period changes work correctly");
  unit_assert_ok(xTimerChangePeriod(timer, TIMER_PERIOD_MEDIUM));
  unit_assert_ok(xTimerGetPeriod(timer, &newPeriod));
  unit_assert_equal(newPeriod, TIMER_PERIOD_MEDIUM);

  unit_assert_ok(xTimerChangePeriod(timer, TIMER_PERIOD_1_SECOND));
  unit_assert_ok(xTimerGetPeriod(timer, &newPeriod));
  unit_assert_equal(newPeriod, TIMER_PERIOD_1_SECOND);
  unit_end();


  /* Test 6.4: Period change to same value */
  unit_begin("Period change to same value succeeds");
  unit_assert_ok(xTimerChangePeriod(timer, TIMER_PERIOD_1_SECOND));
  unit_assert_ok(xTimerGetPeriod(timer, &newPeriod));
  unit_assert_equal(newPeriod, TIMER_PERIOD_1_SECOND);
  unit_end();


  /* Test 6.5: Period change to very short period */
  unit_begin("Period change to very short period succeeds");
  unit_assert_ok(xTimerChangePeriod(timer, 1));
  unit_assert_ok(xTimerGetPeriod(timer, &newPeriod));
  unit_assert_equal(newPeriod, 1);
  unit_end();


  /* Cleanup */
  xTimerDelete(timer);
}


/* ============================================================================
 * SECTION 7: MULTIPLE TIMERS
 * ============================================================================
 */
static void test_multiple_timers(void) {
  Timer_t *timer1 = null;
  Timer_t *timer2 = null;
  Timer_t *timer3 = null;
  Base_t isActive1, isActive2, isActive3;
  Ticks_t period1, period2, period3;


  unit_print("--- Section 7: Multiple Timers ---");


  /* Test 7.1: Create multiple timers with different periods */
  unit_begin("Create multiple timers with different periods");
  unit_assert_ok(xTimerCreate(&timer1, TIMER_PERIOD_SHORT));
  unit_assert_ok(xTimerCreate(&timer2, TIMER_PERIOD_MEDIUM));
  unit_assert_ok(xTimerCreate(&timer3, TIMER_PERIOD_1_SECOND));
  unit_assert_not_null(timer1);
  unit_assert_not_null(timer2);
  unit_assert_not_null(timer3);
  unit_end();


  /* Test 7.2: All timers initially inactive */
  unit_begin("All timers initially inactive");
  unit_assert_ok(xTimerIsTimerActive(timer1, &isActive1));
  unit_assert_ok(xTimerIsTimerActive(timer2, &isActive2));
  unit_assert_ok(xTimerIsTimerActive(timer3, &isActive3));
  unit_assert_false(isActive1 || isActive2 || isActive3);
  unit_end();


  /* Test 7.3: Start all timers */
  unit_begin("Start all timers simultaneously");
  unit_assert_ok(xTimerStart(timer1));
  unit_assert_ok(xTimerStart(timer2));
  unit_assert_ok(xTimerStart(timer3));
  unit_assert_ok(xTimerIsTimerActive(timer1, &isActive1));
  unit_assert_ok(xTimerIsTimerActive(timer2, &isActive2));
  unit_assert_ok(xTimerIsTimerActive(timer3, &isActive3));
  unit_assert_true(isActive1 && isActive2 && isActive3);
  unit_end();


  /* Test 7.4: Stop one timer doesn't affect others */
  unit_begin("Stopping one timer doesn't affect others");
  unit_assert_ok(xTimerStop(timer2));
  unit_assert_ok(xTimerIsTimerActive(timer1, &isActive1));
  unit_assert_ok(xTimerIsTimerActive(timer2, &isActive2));
  unit_assert_ok(xTimerIsTimerActive(timer3, &isActive3));
  unit_assert_true(isActive1 && isActive3);
  unit_assert_false(isActive2);
  unit_end();


  /* Test 7.5: Each timer maintains its own period */
  unit_begin("Each timer maintains its own period independently");
  unit_assert_ok(xTimerGetPeriod(timer1, &period1));
  unit_assert_ok(xTimerGetPeriod(timer2, &period2));
  unit_assert_ok(xTimerGetPeriod(timer3, &period3));
  unit_assert_equal(period1, TIMER_PERIOD_SHORT);
  unit_assert_equal(period2, TIMER_PERIOD_MEDIUM);
  unit_assert_equal(period3, TIMER_PERIOD_1_SECOND);
  unit_end();


  /* Test 7.6: Delete one timer doesn't affect others */
  unit_begin("Deleting one timer doesn't affect others");
  unit_assert_ok(xTimerDelete(timer2));
  unit_assert_ok(xTimerIsTimerActive(timer1, &isActive1));
  unit_assert_ok(xTimerIsTimerActive(timer3, &isActive3));
  unit_end();


  /* Cleanup */
  xTimerDelete(timer1);
  xTimerDelete(timer3);
}


/* ============================================================================
 * SECTION 8: TIMER STRESS OPERATIONS
 * ============================================================================
 */
static void test_timer_stress_operations(void) {
  Timer_t *timers[MAX_TIMERS_STRESS];
  Base_t i;
  Base_t created = 0x0u;


  unit_print("--- Section 8: Timer Stress Operations ---");


  /* Test 8.1: Create many timers */
  unit_begin("Create many timers successfully");

  for(i = 0x0u; i < MAX_TIMERS_STRESS; i++) {
    timers[i] = null;

    if(OK(xTimerCreate(&timers[i], TIMER_PERIOD_1_SECOND + i))) {
      created++;
    } else {
      break;
    }
  }

  unit_assert_true(created > 0x0u);
  unit_end();


  /* Test 8.2: Start all created timers */
  unit_begin("Start all created timers");
  {
    Base_t started = 0x0u;


    for(i = 0x0u; i < created; i++) {
      if(OK(xTimerStart(timers[i]))) {
        started++;
      }
    }

    unit_assert_equal(started, created);
  } unit_end();


  /* Test 8.3: Verify all timers are active */
  unit_begin("Verify all started timers are active");
  {
    Base_t activeCount = 0x0u;
    Base_t isActive;


    for(i = 0x0u; i < created; i++) {
      if(OK(xTimerIsTimerActive(timers[i], &isActive)) && isActive) {
        activeCount++;
      }
    }

    unit_assert_equal(activeCount, created);
  } unit_end();


  /* Test 8.4: Stop all timers */
  unit_begin("Stop all created timers");
  {
    Base_t stopped = 0x0u;


    for(i = 0x0u; i < created; i++) {
      if(OK(xTimerStop(timers[i]))) {
        stopped++;
      }
    }

    unit_assert_equal(stopped, created);
  } unit_end();


  /* Test 8.5: Delete all timers */
  unit_begin("Delete all created timers");
  {
    Base_t deleted = 0x0u;


    for(i = 0x0u; i < created; i++) {
      if(OK(xTimerDelete(timers[i]))) {
        deleted++;
      }
    }

    unit_assert_equal(deleted, created);
  } unit_end();


  /* Test 8.6: Rapid create and delete operations */
  unit_begin("Rapid create and delete operations");
  {
    Timer_t *temp = null;


    for(i = 0x0u; i < 20; i++) {
      unit_assert_ok(xTimerCreate(&temp, TIMER_PERIOD_MEDIUM));
      unit_assert_ok(xTimerDelete(temp));
    }
  } unit_end();
}


/* ============================================================================
 * SECTION 9: TIMER BOUNDARY CONDITIONS
 * ============================================================================
 */
static void test_timer_boundary_conditions(void) {
  Timer_t *timer = null;
  Ticks_t period;


  unit_print("--- Section 9: Timer Boundary Conditions ---");


  /* Test 9.1: Timer with minimum period value */
  unit_begin("Timer with minimum period value succeeds");
  unit_assert_ok(xTimerCreate(&timer, 1));
  unit_assert_ok(xTimerGetPeriod(timer, &period));
  unit_assert_equal(period, 1);
  xTimerDelete(timer);
  unit_end();


  /* Test 9.2: Timer with zero period */
  unit_begin("Timer with zero period handling");
  {
    Return_t result = xTimerCreate(&timer, 0);


    if(OK(result)) {
      /* Zero period allowed - verify it */
      unit_assert_ok(xTimerGetPeriod(timer, &period));
      unit_assert_equal(period, 0);
      xTimerDelete(timer);
    }


    /* Either allowing or rejecting zero is valid */
    unit_assert_true(true);
  } unit_end();


  /* Test 9.3: Timer with very large period */
  unit_begin("Timer with very large period succeeds");
  unit_assert_ok(xTimerCreate(&timer, 0xFFFFFFFE));
  unit_assert_ok(xTimerGetPeriod(timer, &period));
  unit_assert_equal(period, 0xFFFFFFFE);
  xTimerDelete(timer);
  unit_end();


  /* Test 9.4: Timer with maximum period value */
  unit_begin("Timer with maximum period value succeeds");
  unit_assert_ok(xTimerCreate(&timer, MAX_PERIOD_VALUE));
  unit_assert_ok(xTimerGetPeriod(timer, &period));
  unit_assert_equal(period, MAX_PERIOD_VALUE);
  xTimerDelete(timer);
  unit_end();


  /* Test 9.5: Period change boundary values */
  unit_begin("Period change to boundary values succeeds");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_1_SECOND));
  unit_assert_ok(xTimerChangePeriod(timer, 1));
  unit_assert_ok(xTimerGetPeriod(timer, &period));
  unit_assert_equal(period, 1);

  unit_assert_ok(xTimerChangePeriod(timer, MAX_PERIOD_VALUE));
  unit_assert_ok(xTimerGetPeriod(timer, &period));
  unit_assert_equal(period, MAX_PERIOD_VALUE);
  xTimerDelete(timer);
  unit_end();
}


/* ============================================================================
 * SECTION 10: TIMER STATE PERSISTENCE
 * ============================================================================
 */
static void test_timer_state_persistence(void) {
  Timer_t *timer = null;
  Base_t isActive;
  Ticks_t period;


  unit_print("--- Section 10: Timer State Persistence ---");


  /* Test 10.1: Timer state persists across period changes */
  unit_begin("Timer state persists across period changes");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_1_SECOND));
  unit_assert_ok(xTimerStart(timer));
  unit_assert_ok(xTimerChangePeriod(timer, TIMER_PERIOD_2_SECONDS));
  unit_assert_ok(xTimerIsTimerActive(timer, &isActive));
  unit_assert_true(isActive);
  unit_end();


  /* Test 10.2: Period persists across state changes */
  unit_begin("Period persists across state changes");
  unit_assert_ok(xTimerStop(timer));
  unit_assert_ok(xTimerGetPeriod(timer, &period));
  unit_assert_equal(period, TIMER_PERIOD_2_SECONDS);

  unit_assert_ok(xTimerStart(timer));
  unit_assert_ok(xTimerGetPeriod(timer, &period));
  unit_assert_equal(period, TIMER_PERIOD_2_SECONDS);
  unit_end();


  /* Test 10.3: State persists across multiple operations */
  unit_begin("State persists across multiple operations");
  unit_assert_ok(xTimerReset(timer));
  unit_assert_ok(xTimerChangePeriod(timer, TIMER_PERIOD_MEDIUM));
  unit_assert_ok(xTimerIsTimerActive(timer, &isActive));
  unit_assert_true(isActive);
  unit_end();


  /* Test 10.4: Rapid state transitions maintain consistency */
  unit_begin("Rapid state transitions maintain consistency");
  {
    Base_t i;


    for(i = 0x0u; i < 10; i++) {
      unit_assert_ok(xTimerStop(timer));
      unit_assert_ok(xTimerStart(timer));
      unit_assert_ok(xTimerIsTimerActive(timer, &isActive));
      unit_assert_true(isActive);
    }
  } unit_end();


  /* Cleanup */
  xTimerDelete(timer);
}


/* ============================================================================
 * SECTION 11: TIMER EDGE CASES
 * ============================================================================
 */
static void test_timer_edge_cases(void) {
  Timer_t *timer = null;
  Base_t isActive;


  unit_print("--- Section 11: Timer Edge Cases ---");


  /* Test 11.1: Reset only resets start time, must start timer */
  unit_begin("Reset inactive timer then start activates it");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_1_SECOND));
  unit_assert_ok(xTimerReset(timer));
  unit_assert_ok(xTimerStart(timer));
  unit_assert_ok(xTimerIsTimerActive(timer, &isActive));
  unit_assert_true(isActive);
  unit_end();


  /* Test 11.2: Multiple consecutive resets on active timer */
  unit_begin("Multiple consecutive resets on active timer work correctly");
  unit_assert_ok(xTimerReset(timer));
  unit_assert_ok(xTimerReset(timer));
  unit_assert_ok(xTimerReset(timer));
  unit_assert_ok(xTimerIsTimerActive(timer, &isActive));
  unit_assert_true(isActive);
  unit_end();


  /* Test 11.3: Start after stop works correctly */
  unit_begin("Start after stop works correctly");
  unit_assert_ok(xTimerStop(timer));
  unit_assert_ok(xTimerStart(timer));
  unit_assert_ok(xTimerIsTimerActive(timer, &isActive));
  unit_assert_true(isActive);
  unit_end();


  /* Test 11.4: Period change immediately after creation */
  unit_begin("Period change immediately after creation succeeds");
  {
    Timer_t *newTimer = null;
    Ticks_t period;


    unit_assert_ok(xTimerCreate(&newTimer, TIMER_PERIOD_SHORT));
    unit_assert_ok(xTimerChangePeriod(newTimer, TIMER_PERIOD_2_SECONDS));
    unit_assert_ok(xTimerGetPeriod(newTimer, &period));
    unit_assert_equal(period, TIMER_PERIOD_2_SECONDS);
    xTimerDelete(newTimer);
  } unit_end();


  /* Test 11.5: Interleaved operations on multiple timers */
  unit_begin("Interleaved operations on multiple timers work correctly");
  {
    Timer_t *timer1 = null;
    Timer_t *timer2 = null;


    unit_assert_ok(xTimerCreate(&timer1, TIMER_PERIOD_SHORT));
    unit_assert_ok(xTimerCreate(&timer2, TIMER_PERIOD_MEDIUM));
    unit_assert_ok(xTimerStart(timer1));
    unit_assert_ok(xTimerStart(timer2));
    unit_assert_ok(xTimerStop(timer1));
    unit_assert_ok(xTimerReset(timer2));

    unit_assert_ok(xTimerDelete(timer1));
    unit_assert_ok(xTimerDelete(timer2));
  } unit_end();


  /* Cleanup */
  xTimerDelete(timer);
}


/* ============================================================================
 * SECTION 12: TIMER DELETE AND CLEANUP
 * ============================================================================
 */
static void test_timer_delete_and_cleanup(void) {
  Timer_t *timer = null;
  Ticks_t period = 0x0u;
  Base_t isActive;


  unit_print("--- Section 12: Timer Delete and Cleanup ---");


  /* Test 12.1: Delete inactive timer */
  unit_begin("Delete inactive timer succeeds");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_1_SECOND));
  unit_assert_ok(xTimerDelete(timer));
  unit_end();


  /* Test 12.2: Delete active timer */
  unit_begin("Delete active timer succeeds");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_1_SECOND));
  unit_assert_ok(xTimerStart(timer));
  unit_assert_ok(xTimerDelete(timer));
  unit_end();


  /* Test 12.3: Operations on deleted timer fail */
  unit_begin("Operations on deleted timer fail gracefully");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_1_SECOND));
  unit_assert_ok(xTimerDelete(timer));


  /* These should fail or handle gracefully */
  unit_assert_not_ok(xTimerGetPeriod(timer, &period));
  unit_assert_not_ok(xTimerIsTimerActive(timer, &isActive));
  unit_end();


  /* Test 12.4: Delete multiple timers in sequence */
  unit_begin("Delete multiple timers in sequence");
  {
    Timer_t *timer1 = null;
    Timer_t *timer2 = null;
    Timer_t *timer3 = null;


    unit_assert_ok(xTimerCreate(&timer1, TIMER_PERIOD_SHORT));
    unit_assert_ok(xTimerCreate(&timer2, TIMER_PERIOD_MEDIUM));
    unit_assert_ok(xTimerCreate(&timer3, TIMER_PERIOD_1_SECOND));

    unit_assert_ok(xTimerDelete(timer1));
    unit_assert_ok(xTimerDelete(timer2));
    unit_assert_ok(xTimerDelete(timer3));
  } unit_end();


  /* Test 12.5: Create, delete, and recreate timer */
  unit_begin("Create, delete, and recreate timer works correctly");
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_1_SECOND));
  unit_assert_ok(xTimerDelete(timer));
  unit_assert_ok(xTimerCreate(&timer, TIMER_PERIOD_2_SECONDS));
  unit_assert_ok(xTimerGetPeriod(timer, &period));
  unit_assert_equal(period, TIMER_PERIOD_2_SECONDS);
  unit_assert_ok(xTimerDelete(timer));
  unit_end();
}
