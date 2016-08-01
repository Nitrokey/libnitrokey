#!/usr/bin/env python
import cffi

ffi = cffi.FFI()
get_string = ffi.string


def get_library():
    fp = 'NK_C_API.h'  # path to C API header

    declarations = []
    with open(fp, 'r') as f:
        declarations = f.readlines()

    for declaration in declarations:
        if 'extern' in declaration and not '"C"' in declaration:
            declaration = declaration.replace('extern', '').strip()
            # print(declaration)
            ffi.cdef(declaration)

    C = ffi.dlopen("build/libnitrokey.so")  # path to built library
    return C


def get_hotp_code(lib, i):
    return lib.NK_get_hotp_code(i)


libnitrokey = get_library()
libnitrokey.NK_set_debug(False)  # do not show debug messages

libnitrokey.NK_login('P')  # connect to Nitrokey Pro device
hotp_slot_1_code = get_hotp_code(libnitrokey, 1)
print('Getting HOTP code from Nitrokey Pro: ')
print(hotp_slot_1_code)
libnitrokey.NK_logout()  # disconnect device

libnitrokey.NK_login('S')  # connect to Nitrokey Storage device
hotp_slot_1_code_nitrokey_storage = get_hotp_code(libnitrokey, 1)
print('Getting HOTP code from Nitrokey Storage: ')
print(hotp_slot_1_code_nitrokey_storage)
libnitrokey.NK_logout()  # disconnect device
