/*
 * Copyright (c) 2020 Nitrokey UG
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

#include <stdlib.h>
#include "../NK_C_API.h"

// This test should be run with valgrind to make sure that there are no
// memory leaks in the tested functions:
//     valgrind ./test_memory
int main() {
	int result = NK_login_auto();
	if (result != 1)
		return 1;
  
	int retry_count = NK_get_admin_retry_count();
	if (retry_count != 3)
		return 1;
	retry_count = NK_get_user_retry_count();
	if (retry_count != 3)
		return 1;

	enum NK_device_model model = NK_get_device_model();
	if (model != NK_PRO && model != NK_STORAGE)
		return 1;

	uint8_t *config = NK_read_config();
	if (config == NULL)
		return 1;
	NK_free_config(config);

	result = NK_enable_password_safe("123456");
	if (result != 0)
		return 1;

	uint8_t *slot_status = NK_get_password_safe_slot_status();
	if (slot_status == NULL) {
		return 1;
	}
	NK_free_password_safe_slot_status(slot_status);

	NK_logout();

	return 0;
}

