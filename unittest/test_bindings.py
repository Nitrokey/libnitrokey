import pytest
import cffi

ffi = cffi.FFI()


@pytest.fixture(scope="module")
def C(request):

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

    C = ffi.dlopen("../build/libnitrokey.so")
    C.NK_login('12345678', '123123123')
    C.NK_set_debug(True)
    def fin():
        C.NK_logout()
        request.addfinalizer(fin)
    return C


def test_HOTP_RFC(C):
    # https://tools.ietf.org/html/rfc4226#page-32
    C.NK_write_hotp_slot(1, 'python_test', '12345678901234567890', 0, '123123123')
    test_data = [
        755224, 287082, 359152, 969429, 338314, 254676, 287922, 162583, 399871, 520489,
    ]
    for code in test_data:
        r = C.NK_get_hotp_code(1)
        assert code == r


def test_TOTP_RFC(C):
    # test according to https://tools.ietf.org/html/rfc6238#appendix-B
    C.NK_write_totp_slot(1, 'python_test', '12345678901234567890', 30, True, '123123123')
    test_data = [
        (59, 1, 94287082),
        (1111111109, 0x00000000023523EC, 7081804),
        (1111111111, 0x00000000023523ED, 14050471),
        (1234567890, 0x000000000273EF07, 89005924),
    ]
    for t, T, code in test_data:
        C.NK_totp_set_time(t)
        r = C.NK_get_totp_code(1, T, 0, 30)  # FIXME T is not changing the outcome
        assert code == r


def test_get_slot_names(C):
    s = []
    for i in range(16):
        s.append(ffi.string(C.NK_get_totp_slot_name(i)))
    for i in range(3):
        s.append(ffi.string(C.NK_get_hotp_slot_name(i)))
    print(repr(s))
    print(s)

def test_get_OTP_codes(C):
    s = []
    for i in range(16):
        s.append(C.NK_get_totp_code(i, 0, 0, 0))
    for i in range(3):
        s.append(C.NK_get_hotp_code(i))
    print(repr(s))
    print(s)
