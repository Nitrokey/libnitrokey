#!/usr/bin/env python
import cffi
from enum import Enum

"""
This example will print 10 HOTP codes from just written HOTP#2 slot.
For more examples of use please refer to unittest/test_bindings.py file.
"""

ffi = cffi.FFI()
get_string = ffi.string

class DeviceErrorCode(Enum):
    STATUS_OK = 0
    NOT_PROGRAMMED = 3
    WRONG_PASSWORD = 4
    STATUS_NOT_AUTHORIZED = 5
    STATUS_AES_DEC_FAILED = 0xa


def get_library():
    fp = 'NK_C_API.h'  # path to C API header

    declarations = []
    with open(fp, 'r') as f:
        declarations = f.readlines()

    a = iter(declarations)
    for declaration in a:
        if declaration.startswith('extern') and not '"C"' in declaration:
            declaration = declaration.replace('extern', '').strip()
            while not ';' in declaration:
                declaration += (next(a)).strip()
            # print(declaration)
            ffi.cdef(declaration)

    C = ffi.dlopen("build/libnitrokey.so")  # path to built library
    return C


def get_hotp_code(lib, i):
    return lib.NK_get_hotp_code(i)

print('Warning!')
print('This example will change your configuration on inserted stick and overwrite your HOTP#2 slot.')
print('Please write "continue" to continue or any other string to quit')
a = raw_input()

if not a == 'continue':
    exit()

ADMIN = raw_input('Please enter your admin PIN (empty string uses 12345678)')
ADMIN = ADMIN or '12345678'  # use default if empty string

libnitrokey = get_library()
libnitrokey.NK_set_debug(False)  # do not show debug messages

ADMIN_TEMP = '123123123'
RFC_SECRET = '12345678901234567890'

# libnitrokey.NK_login('S')  # connect only to Nitrokey Storage device
# libnitrokey.NK_login('P')  # connect only to Nitrokey Pro device
device_connected = libnitrokey.NK_login_auto()  # connect to any Nitrokey Stick
if device_connected:
    print('Connected to Nitrokey device!')
else:
    print('Could not connect to Nitrokey device!')
    exit()
use_8_digits = True
pin_correct = libnitrokey.NK_first_authenticate(ADMIN, ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
if pin_correct:
    print('Your PIN is correct!')
else:
    print('Your PIN is not correct! Please try again. Please be careful to not lock your stick!')
    retry_count_left = libnitrokey.NK_get_admin_retry_count()
    print('Retry count left: %d' % retry_count_left )
    exit()

# For function parameters documentation please check NK_C_API.h
assert libnitrokey.NK_write_config(255, 255, 255, False, True, ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
libnitrokey.NK_first_authenticate(ADMIN, ADMIN_TEMP)
libnitrokey.NK_write_hotp_slot(1, 'python_test', RFC_SECRET, 0, use_8_digits, False, False, "",
                            ADMIN_TEMP)
# RFC test according to: https://tools.ietf.org/html/rfc4226#page-32
test_data = [
    1284755224, 1094287082, 137359152, 1726969429, 1640338314, 868254676, 1918287922, 82162583, 673399871,
    645520489,
]
print('Getting HOTP code from Nitrokey Pro (RFC test, 8 digits): ')
for i in range(10):
    hotp_slot_1_code = get_hotp_code(libnitrokey, 1)
    print('%d: %d, should be %s' % (i, hotp_slot_1_code, str(test_data[i])[-8:] ))
libnitrokey.NK_logout()  # disconnect device
