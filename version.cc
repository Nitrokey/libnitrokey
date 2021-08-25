/*
 * Copyright (c) 2018 Nitrokey UG
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

#include "version.h"

namespace nitrokey {
    unsigned int get_major_library_version() {
#ifdef LIBNK_VERSION_MAJOR
        return LIBNK_VERSION_MAJOR;
#endif
        return 3;
    }

    unsigned int get_minor_library_version() {
#ifdef LIBNK_VERSION_MINOR
        return LIBNK_VERSION_MINOR;
#endif
        return 0;
    }

    const char* get_library_version() {
        return "unknown";
    }
}

