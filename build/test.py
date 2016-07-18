import cffi

ffi = cffi.FFI()
fp = '../NK_C_API.h'

declarations = []
with open(fp, 'r') as f:
    declarations = f.readlines()

for declaration in declarations:
    # extern int NK_write_totp_slot(int slot_number, char* secret, int time_window);
    if 'extern' in declaration and not '"C"' in declaration:
        declaration = declaration.replace('extern', '').strip()
        print(declaration)
        ffi.cdef(declaration)

C = ffi.dlopen("./libnitrokey.so")

if __name__ == "__main__":
    C.NK_set_debug(False)
    C.NK_set_debug(True)
    a = C.NK_login('12345678', '123123123')
    # a = C.NK_logout()
    print(a)
    C.NK_set_debug(False)

    # print(''.center(40, '#'))
    print(ffi.string(C.NK_status()))
    # print(''.center(40, '#'))

    # print(C.NK_get_hotp_code(0))
    # print(C.NK_get_totp_code(0, 0, 0, 0))
    # print(ffi.string(C.NK_get_totp_slot_name(0)))

    s = []
    for i in range(16):
        s.append(ffi.string(C.NK_get_totp_slot_name(i)))
    for i in range(3):
        s.append(ffi.string(C.NK_get_hotp_slot_name(i)))
    print(repr(s))
    print((s))

    s = []
    for i in range(16):
        s.append(C.NK_get_totp_code(i, 0, 0, 0))
    for i in range(3):
        s.append(C.NK_get_hotp_code(i))
    print(repr(s))
    print((s))
    C.NK_set_debug(True)

    s = []
    C.NK_write_hotp_slot(1, 'python_test', '12345678901234567890', 0, '123123123')
    C.NK_set_debug(False)
    for i in range(3):
        s.append(C.NK_get_hotp_code(1))
    print((s))
