"""
Copyright (c) 2019 Nitrokey UG

This file is part of libnitrokey.

libnitrokey is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

libnitrokey is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libnitrokey. If not, see <http://www.gnu.org/licenses/>.

SPDX-License-Identifier: LGPL-3.0
"""

from misc import gs
import re


def test_offline(C_offline):
    C_offline.NK_set_debug(False)
    C_offline.NK_set_debug_level(4)
    assert C_offline.NK_get_major_library_version() == 3
    assert C_offline.NK_get_minor_library_version() >= 3
    assert C_offline.NK_login_auto() == 0
    
    libnk_version = gs(C_offline.NK_get_library_version())
    assert libnk_version
    print(libnk_version)

    # v3.4.1-29-g1f3d
    search = re.search(b'v\d\.\d(\.\d)?', libnk_version)
    assert search is not None