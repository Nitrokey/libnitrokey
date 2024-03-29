"""
Copyright (c) 2015-2018 Nitrokey UG

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
from enum import Enum
from sys import stderr

from misc import to_hex, bb
from conftest import print

RFC_SECRET_HR = '12345678901234567890'
RFC_SECRET = to_hex(RFC_SECRET_HR)  # '31323334353637383930...'
bbRFC_SECRET = bb(RFC_SECRET)


# print( repr((RFC_SECRET, RFC_SECRET_, len(RFC_SECRET))) )

class DefaultPasswords:
    ADMIN = b'12345678'
    USER = b'123456'
    ADMIN_TEMP = b'123123123'
    USER_TEMP = b'234234234'
    UPDATE = b'12345678'
    UPDATE_TEMP = b'123update123'
    UPDATE_LONG = b'1234567890'*2
    UPDATE_TOO_LONG = UPDATE_LONG + b'x'
    UPDATE_TOO_SHORT = UPDATE_LONG[:7]


class DeviceErrorCode(Enum):
    STATUS_OK = 0
    BUSY = 1 # busy or busy progressbar in place of wrong_CRC status
    NOT_PROGRAMMED = 3
    WRONG_PASSWORD = 4
    STATUS_NOT_AUTHORIZED = 5
    STATUS_AES_DEC_FAILED = 10
    STATUS_WRONG_SLOT = 2
    STATUS_TIMESTAMP_WARNING = 6
    STATUS_NO_NAME_ERROR = 7
    STATUS_NOT_SUPPORTED = 8
    STATUS_UNKNOWN_COMMAND = 9
    STATUS_AES_CREATE_KEY_FAILED = 11
    STATUS_ERROR_CHANGING_USER_PASSWORD = 12
    STATUS_ERROR_CHANGING_ADMIN_PASSWORD = 13
    STATUS_ERROR_UNBLOCKING_PIN = 14

    STATUS_UNKNOWN_ERROR = 100
    STATUS_DISCONNECTED = 255

    def __eq__(self, other):
        other_name = 'Unknown'
        try:
            other_name = str(DeviceErrorCode(other).name)
        except:
            pass
        result = self.value == other
        print(f'Returned {other_name}, expected {self.name} => {result}')
        return result

class LibraryErrors(Enum):
    TOO_LONG_STRING = 200
    INVALID_SLOT = 201
    INVALID_HEX_STRING = 202
    TARGET_BUFFER_SIZE_SMALLER_THAN_SOURCE = 203

    def __eq__(self, other):
        other_name = 'Unknown'
        try:
            other_name = str(LibraryErrors(other).name)
        except:
            pass
        result = self.value == other
        print(f'Returned {other_name}, expected {self.name} => {result}')
        return result


HOTP_slot_count = 3
TOTP_slot_count = 15
PWS_SLOT_COUNT = 16
