import pytest
import cffi
from enum import Enum

ffi = cffi.FFI()
gs = ffi.string

RFC_SECRET = '12345678901234567890'


class DefaultPasswords(Enum):
    ADMIN = '12345678'
    USER = '123456'
    ADMIN_TEMP = '123123123'


class DeviceErrorCode(Enum):
    STATUS_OK = 0
    NOT_PROGRAMMED = 3
    WRONG_PASSWORD = 4
    STATUS_NOT_AUTHORIZED = 5


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
    C.NK_login(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP)

    # C.NK_set_debug(True)

    def fin():
        print ('\nFinishing connection to device')
        C.NK_logout()
        print ('Finished')

    request.addfinalizer(fin)

    return C


def test_enable_password_safe(C):
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_enable_password_safe('wrong_password') == DeviceErrorCode.WRONG_PASSWORD
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK


def test_write_password_safe_slot(C):
    C.NK_set_debug(True)
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_write_password_safe_slot(0, 'slotname1', 'login1', 'pass1') == DeviceErrorCode.STATUS_NOT_AUTHORIZED
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_password_safe_slot(0, 'slotname1', 'login1', 'pass1') == DeviceErrorCode.STATUS_OK
    C.NK_set_debug(False)


def test_get_password_safe_slot_name(C):
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert gs(C.NK_get_password_safe_slot_name(0, DefaultPasswords.ADMIN_TEMP)) == ''
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_NOT_AUTHORIZED

    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    assert gs(C.NK_get_password_safe_slot_name(0, DefaultPasswords.ADMIN_TEMP)) == 'slotname1'
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK


def test_get_password_safe_slot_login_password(C):
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    slot_login = C.NK_get_password_safe_slot_login(0, DefaultPasswords.ADMIN_TEMP)
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK
    assert gs(slot_login) == 'login1'
    slot_password = gs(C.NK_get_password_safe_slot_password(0, DefaultPasswords.ADMIN_TEMP))
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK
    assert slot_password == 'pass1'


def test_erase_password_safe_slot(C):
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    assert C.NK_erase_password_safe_slot(0) == DeviceErrorCode.STATUS_OK
    assert gs(C.NK_get_password_safe_slot_name(0, DefaultPasswords.ADMIN_TEMP)) == ''
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK  # TODO shouldn't be DeviceErrorCode.NOT_PROGRAMMED ?


def test_password_safe_slot_status(C):
    C.NK_set_debug(True)
    assert C.NK_get_password_safe_slot_status() == DeviceErrorCode.STATUS_OK
    C.NK_set_debug(False)


def test_admin_PIN_change(C):
    assert C.NK_change_admin_PIN('wrong_password', DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.WRONG_PASSWORD
    assert C.NK_change_admin_PIN(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_change_admin_PIN(DefaultPasswords.ADMIN_TEMP, DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK


def test_user_PIN_change(C):
    assert C.NK_change_user_PIN('wrong_password', DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.WRONG_PASSWORD
    assert C.NK_change_user_PIN(DefaultPasswords.USER, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_change_user_PIN(DefaultPasswords.ADMIN_TEMP, DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK


def test_admin_retry_counts(C):
    assert C.NK_get_admin_retry_count() == 3
    assert C.NK_change_admin_PIN('wrong_password', DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.WRONG_PASSWORD
    assert C.NK_get_admin_retry_count() == 3 - 1
    assert C.NK_change_admin_PIN(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK
    assert C.NK_get_admin_retry_count() == 3


def test_user_retry_counts(C):
    assert C.NK_get_user_retry_count() == 3
    assert C.NK_enable_password_safe('wrong_password') == DeviceErrorCode.WRONG_PASSWORD
    assert C.NK_get_user_retry_count() == 3 - 1
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    assert C.NK_get_user_retry_count() == 3


def test_HOTP_RFC(C):
    # https://tools.ietf.org/html/rfc4226#page-32
    C.NK_write_hotp_slot(1, 'python_test', RFC_SECRET, 0, DefaultPasswords.ADMIN_TEMP)
    test_data = [
        755224, 287082, 359152, 969429, 338314, 254676, 287922, 162583, 399871, 520489,
    ]
    for code in test_data:
        r = C.NK_get_hotp_code(1)
        assert code == r


def test_TOTP_RFC(C):
    # test according to https://tools.ietf.org/html/rfc6238#appendix-B
    C.NK_write_totp_slot(1, 'python_test', RFC_SECRET, 30, True, DefaultPasswords.ADMIN_TEMP)
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
    # TODO add setup to have at least one slot not programmed
    for i in range(16):
        name = ffi.string(C.NK_get_totp_slot_name(i))
        if name == '':
            assert C.NK_get_last_command_status() == DeviceErrorCode.NOT_PROGRAMMED
    for i in range(3):
        name = ffi.string(C.NK_get_hotp_slot_name(i))
        if name == '':
            assert C.NK_get_last_command_status() == DeviceErrorCode.NOT_PROGRAMMED


def test_get_OTP_codes(C):
    # TODO add setup to have at least one slot not programmed
    for i in range(16):
        code = C.NK_get_totp_code(i, 0, 0, 0)
        if code == 0:
            assert C.NK_get_last_command_status() == DeviceErrorCode.NOT_PROGRAMMED

    for i in range(3):
        code = C.NK_get_hotp_code(i)
        if code == 0:
            assert C.NK_get_last_command_status() == DeviceErrorCode.NOT_PROGRAMMED
