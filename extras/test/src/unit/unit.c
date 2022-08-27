/**
 * @file unit.c
 * @author Manny Peterson (mannymsp@gmail.com)
 * @brief A simple unit testing framework for HeliOS
 * @version 0.3.5
 * @date 2022-08-23
 * 
 * @copyright
 * HeliOS Embedded Operating System
 * Copyright (C) 2020-2022 Manny Peterson <mannymsp@gmail.com>
 *  
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */
#include "unit.h"

static unit_t *unit = NULL;

void unit_init(void) {




  if (NULL == unit) {


    unit = (unit_t *)calloc(1, sizeof(unit_t));

    printf("unit: intializing...\n");
  }



  return;
}

void unit_begin(const char *name_) {


  if ((NULL != unit) & (NULL != name_) & (false == unit->begun)) {


    strncpy(unit->name, name_, UNIT_NAME_LENGTH);


    unit->begun = true;


    unit->failed = false;

    printf("unit: begin: %s\n", unit->name);
  }

  return;
}

void unit_try(int expr_) {


  if ((NULL != unit) & (true == unit->begun) & (false == expr_)) {

    unit->failed = true;
  }


  return;
}

void unit_end(void) {


  if ((NULL != unit) & (true == unit->begun)) {


    if (true == unit->failed) {


      printf("unit: end: %s failed\n", unit->name);



      unit->fail++;


    } else {

      printf("unit: end: %s\n", unit->name);

      unit->pass++;
    }


    memset(unit->name, 0x0, UNIT_NAME_LENGTH);


    unit->begun = false;


    unit->failed = false;
  }



  return;
}


void unit_exit(void) {


  if (NULL != unit) {

    printf("unit: failed = %" PRId32 "\n", unit->fail);


    printf("unit: passed = %" PRId32 "\n", unit->pass);


    printf("unit: total = %" PRId32 "\n", unit->fail + unit->pass);


    printf("unit: exiting...\n");

    free(unit);

    if (0x0 < unit->fail) {


      exit(0x1);


    } else {


      exit(0x0);
    }
  }

  return;
}