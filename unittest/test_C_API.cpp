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

static const int TOO_LONG_STRING = 200;

#include "catch.hpp"

#include <iostream>
#include <string>
#include "log.h"
#include "../NK_C_API.h"

TEST_CASE("C API connect", "[BASIC]") {
  auto login = NK_login_auto();
  REQUIRE(login != 0);
  NK_logout();
  login = NK_login_auto();
  REQUIRE(login != 0);
  NK_logout();
  login = NK_login_auto();
  REQUIRE(login != 0);
}

TEST_CASE("Check retry count", "[BASIC]") {
  REQUIRE(NK_get_admin_retry_count() == 3);
  REQUIRE(NK_get_user_retry_count() == 3);
}

TEST_CASE("Check long strings", "[STANDARD]") {
  const char* longPin = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  const char* pin = "123123123";
  auto result = NK_change_user_PIN(longPin, pin);
  REQUIRE(result == TOO_LONG_STRING);
  result = NK_change_user_PIN(pin, longPin);
  REQUIRE(result == TOO_LONG_STRING);
  CAPTURE(result);
}