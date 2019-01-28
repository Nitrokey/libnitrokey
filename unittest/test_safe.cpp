/*
 * Copyright (c) 2019 Nitrokey UG
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

#include "catch2/catch.hpp"

#include <iostream>
#include <cstring>
#include "log.h"
#include "../NK_C_API.h"

int login;

TEST_CASE("C API connect", "[BASIC]") {
    INFO("This test set assumes either Pro or Storage device is connected.");
    INFO("Here should be implemented only tests not changing the device's state, "
         "and safe to user data.");

    login = NK_login_auto();
    REQUIRE(login != 0);
    NK_set_debug_level(3);
}

TEST_CASE("Check retry count", "[BASIC]") {
    INFO("This test assumes your PINs' attempt counters are set to 3");
    REQUIRE(login != 0);
    REQUIRE(NK_get_admin_retry_count() == 3);
    REQUIRE(NK_get_user_retry_count() == 3);
}

void validate_cstring(char *s){
    constexpr uint16_t max_length = 8*1024;
    REQUIRE(s != nullptr);
    REQUIRE(strnlen(s, max_length) > 0);
    REQUIRE(strnlen(s, max_length) < max_length);
    std::cout << s << std::endl;
}

TEST_CASE("Status command for Pro or Storage", "[BASIC]") {
    REQUIRE(login != 0);
    char* s = nullptr;

    auto const m = NK_get_device_model();
    REQUIRE(m != NK_DISCONNECTED);
    if (m == NK_PRO)
        s = NK_get_status_as_string();
    else if (m == NK_STORAGE){
        s = NK_get_status_storage_as_string();
    }

    validate_cstring(s);
    free(s);
    s = nullptr;
}

TEST_CASE("Device serial", "[BASIC]") {
    REQUIRE(login != 0);
    char* s = nullptr;
    s = NK_device_serial_number();
    validate_cstring(s);
    free(s);
    s = nullptr;
}

TEST_CASE("Firmware version", "[BASIC]") {
    REQUIRE(login != 0);
    // Currently all devices has major version '0'
    // No firmware ever had a minor equal to '0'
    REQUIRE(NK_get_major_firmware_version() == 0);
    REQUIRE(NK_get_minor_firmware_version() != 0);
}

TEST_CASE("Library version", "[BASIC]") {
    REQUIRE(NK_get_major_library_version() == 3);
    REQUIRE(NK_get_minor_library_version() >= 4);
}
