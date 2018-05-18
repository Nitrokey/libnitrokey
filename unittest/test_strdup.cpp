/*
 * Copyright (c) 2015-2018 Nitrokey UG
 *
 * This file is part of libnitrokey.
 *
 * libnitrokey is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * libnitrokey is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libnitrokey. If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0
 */

// issue: https://github.com/Nitrokey/libnitrokey/issues/110
// tests according to the issue's author, Robin Krahl (robinkrahl)
// suggested run command: valgrind --tool=memcheck --leak-check=full ./test_strdup

#include <cstdio>
#include <memory.h>
#include "../NK_C_API.h"
#include "catch.hpp"


static const int SHORT_STRING_LENGTH = 10;

TEST_CASE("Test strdup memory free error", "[BASIC]")
{
  NK_set_debug(false);
  char *c = NK_status(); /* error --> string literal */
  REQUIRE(c != nullptr);
  REQUIRE(strnlen(c, SHORT_STRING_LENGTH) == 0);
  puts(c);
  free(c);
}

TEST_CASE("Test strdup memory leak", "[BASIC]")
{
  NK_set_debug(false);
  bool connected = NK_login_auto() == 1;
  if (!connected) return;

  REQUIRE(connected);
  char *c = NK_status();  /* no error --> dynamically allocated */
  REQUIRE(c != nullptr);
  REQUIRE(strnlen(c, SHORT_STRING_LENGTH) > 0);
  puts(c);
  free(c);
}

