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
    USER_TEMP = '234234234'


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
    C.NK_set_debug(False)
    C.NK_login('P')
    # assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    # assert C.NK_user_authenticate(DefaultPasswords.USER, DefaultPasswords.USER_TEMP) == DeviceErrorCode.STATUS_OK

    # C.NK_status()

    def fin():
        print ('\nFinishing connection to device')
        C.NK_logout()
        print ('Finished')

    request.addfinalizer(fin)
    C.NK_set_debug(True)

    return C


def test_enable_password_safe(C):
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_enable_password_safe('wrong_password') == DeviceErrorCode.WRONG_PASSWORD
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK


def test_write_password_safe_slot(C):
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_write_password_safe_slot(0, 'slotname1', 'login1', 'pass1') == DeviceErrorCode.STATUS_NOT_AUTHORIZED
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_password_safe_slot(0, 'slotname1', 'login1', 'pass1') == DeviceErrorCode.STATUS_OK


def test_get_password_safe_slot_name(C):
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_password_safe_slot(0, 'slotname1', 'login1', 'pass1') == DeviceErrorCode.STATUS_OK
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert gs(C.NK_get_password_safe_slot_name(0)) == ''
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_NOT_AUTHORIZED

    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    assert gs(C.NK_get_password_safe_slot_name(0)) == 'slotname1'
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK


def test_get_password_safe_slot_login_password(C):
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_password_safe_slot(0, 'slotname1', 'login1', 'pass1') == DeviceErrorCode.STATUS_OK
    slot_login = C.NK_get_password_safe_slot_login(0)
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK
    assert gs(slot_login) == 'login1'
    slot_password = gs(C.NK_get_password_safe_slot_password(0))
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK
    assert slot_password == 'pass1'


def test_erase_password_safe_slot(C):
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    assert C.NK_erase_password_safe_slot(0) == DeviceErrorCode.STATUS_OK
    assert gs(C.NK_get_password_safe_slot_name(0)) == ''
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK  # TODO CHECK shouldn't this be DeviceErrorCode.NOT_PROGRAMMED ?


def test_password_safe_slot_status(C):
    C.NK_set_debug(True)
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    assert C.NK_erase_password_safe_slot(0) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_password_safe_slot(1, 'slotname2', 'login2', 'pass2') == DeviceErrorCode.STATUS_OK
    safe_slot_status = C.NK_get_password_safe_slot_status()
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK
    is_slot_programmed = list(ffi.cast("uint8_t [16]", safe_slot_status)[0:16])
    print((is_slot_programmed, len(is_slot_programmed)))
    assert is_slot_programmed[0] == 0
    assert is_slot_programmed[1] == 1


@pytest.mark.skip(reason="issue to register, skipping for now")
def test_issue_device_locks_on_second_key_generation_in_sequence(C):
    assert C.NK_build_aes_key(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK
    assert C.NK_build_aes_key(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK


def test_regenerate_aes_key(C):
    C.NK_set_debug(True)
    assert C.NK_build_aes_key(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK


def test_destroy_password_safe(C):
    """
    Sometimes fails on NK Pro - slot name is not cleared ergo key generation has not succeed despite the success result
    returned from the device
    """
    C.NK_set_debug(True)
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    # write password safe slot
    assert C.NK_write_password_safe_slot(0, 'slotname1', 'login1', 'pass1') == DeviceErrorCode.STATUS_OK
    # read slot
    assert gs(C.NK_get_password_safe_slot_name(0)) == 'slotname1'
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK
    slot_login = C.NK_get_password_safe_slot_login(0)
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK
    assert gs(slot_login) == 'login1'
    # destroy password safe by regenerating aes key
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK

    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_build_aes_key(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK

    assert gs(C.NK_get_password_safe_slot_name(0)) != 'slotname1'
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK

    # check was slot status cleared
    safe_slot_status = C.NK_get_password_safe_slot_status()
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK
    is_slot_programmed = list(ffi.cast("uint8_t [16]", safe_slot_status)[0:16])
    assert is_slot_programmed[0] == 0



def test_admin_PIN_change(C):
    new_password = '123123123'
    assert C.NK_change_admin_PIN('wrong_password', new_password) == DeviceErrorCode.WRONG_PASSWORD
    assert C.NK_change_admin_PIN(DefaultPasswords.ADMIN, new_password) == DeviceErrorCode.STATUS_OK
    assert C.NK_change_admin_PIN(new_password, DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK


def test_user_PIN_change(C):
    new_password = '123123123'
    assert C.NK_change_user_PIN('wrong_password', new_password) == DeviceErrorCode.WRONG_PASSWORD
    assert C.NK_change_user_PIN(DefaultPasswords.USER, new_password) == DeviceErrorCode.STATUS_OK
    assert C.NK_change_user_PIN(new_password, DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK


def test_admin_retry_counts(C):
    default_admin_retry_count = 3
    assert C.NK_get_admin_retry_count() == default_admin_retry_count
    assert C.NK_change_admin_PIN('wrong_password', DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.WRONG_PASSWORD
    assert C.NK_get_admin_retry_count() == default_admin_retry_count - 1
    assert C.NK_change_admin_PIN(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK
    assert C.NK_get_admin_retry_count() == default_admin_retry_count


def test_user_retry_counts(C):
    default_user_retry_count = 3
    assert C.NK_get_user_retry_count() == default_user_retry_count
    assert C.NK_enable_password_safe('wrong_password') == DeviceErrorCode.WRONG_PASSWORD
    assert C.NK_get_user_retry_count() == default_user_retry_count - 1
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    assert C.NK_get_user_retry_count() == default_user_retry_count

def test_admin_auth(C):
    assert C.NK_first_authenticate('wrong_password', DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.WRONG_PASSWORD
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK

def test_user_auth(C):
    assert C.NK_user_authenticate('wrong_password', DefaultPasswords.USER_TEMP) == DeviceErrorCode.WRONG_PASSWORD
    assert C.NK_user_authenticate(DefaultPasswords.USER, DefaultPasswords.USER_TEMP) == DeviceErrorCode.STATUS_OK


def check_RFC_codes(C, func, prep=None):
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_hotp_slot(1, 'python_test', RFC_SECRET, 0, False, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    test_data = [
        755224, 287082, 359152, 969429, 338314, 254676, 287922, 162583, 399871, 520489,
    ]
    for code in test_data:
        if prep:
            prep()
        r = func(1)
        assert code == r


@pytest.mark.skip(reason="not working correctly, skipping for now")
def test_HOTP_RFC_pin_protection(C):
    C.NK_set_debug(True)
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_config(True, True, True, True, False, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_hotp_slot(1, 'python_test', RFC_SECRET, 0, False, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    # check_RFC_codes(C, lambda x: C.NK_get_hotp_code_PIN(x, DefaultPasswords.USER_TEMP), lambda: C.NK_user_authenticate(DefaultPasswords.USER, DefaultPasswords.USER_TEMP))
    assert C.NK_user_authenticate(DefaultPasswords.USER, DefaultPasswords.USER_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_get_hotp_code_PIN(1, DefaultPasswords.USER_TEMP) == 755224
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK


@pytest.mark.skip(reason="not implemented yet")
def test_HOTP_RFC_no_pin_protection_8digits(C):
    assert False # TODO to write

def test_HOTP_RFC_no_pin_protection(C):
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_hotp_slot(1, 'python_test', RFC_SECRET, 0, False, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_config(True, True, True, False, True, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    # https://tools.ietf.org/html/rfc4226#page-32
    check_RFC_codes(C, C.NK_get_hotp_code)


def test_TOTP_RFC_no_pin_protection(C):
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_config(True, True, True, False, True, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    # test according to https://tools.ietf.org/html/rfc6238#appendix-B
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_totp_slot(1, 'python_test', RFC_SECRET, 30, True, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
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
    C.NK_set_debug(True)
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_erase_totp_slot(0, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    # erasing slot invalidates temporary password, so requesting authentication
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_erase_hotp_slot(0, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK

    for i in range(15):
        name = ffi.string(C.NK_get_totp_slot_name(i))
        if name == '':
            assert C.NK_get_last_command_status() == DeviceErrorCode.NOT_PROGRAMMED
    for i in range(3):
        name = ffi.string(C.NK_get_hotp_slot_name(i))
        if name == '':
            assert C.NK_get_last_command_status() == DeviceErrorCode.NOT_PROGRAMMED


def test_get_OTP_codes(C):
    C.NK_set_debug(True)
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_erase_hotp_slot(0, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_erase_totp_slot(0, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    for i in range(15):
        code = C.NK_get_totp_code(i, 0, 0, 0)
        if code == 0:
            assert C.NK_get_last_command_status() == DeviceErrorCode.NOT_PROGRAMMED

    for i in range(3):
        code = C.NK_get_hotp_code(i)
        if code == 0:
            assert C.NK_get_last_command_status() == DeviceErrorCode.NOT_PROGRAMMED


def test_get_code_user_authorize(C):
    C.NK_set_debug(True)
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_totp_slot(0, 'python_otp_auth', RFC_SECRET, 30, True,
                                DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    # enable PIN protection of OTP codes with write_config
    # TODO create convinience function on C API side to enable/disable OTP USER_PIN protection
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_config(True, True, True, True, False, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    code = C.NK_get_totp_code(0, 0, 0, 0)
    assert code == 0
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_NOT_AUTHORIZED
    # disable PIN protection with write_config
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_config(True, True, True, False, True, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    code = C.NK_get_totp_code(0, 0, 0, 0)
    assert code != 0
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK


def cast_pointer_to_tuple(obj, typen, len):
    # usage:
    #     config = cast_pointer_to_tuple(config_raw_data, 'uint8_t', 5)
    return tuple(ffi.cast("%s [%d]" % (typen, len), obj)[0:len])


def test_read_write_config(C):
    C.NK_set_debug(True)

    # let's set sample config with pin protection and disabled capslock
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_config(True, False, True, True, False, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    config_raw_data = C.NK_read_config()
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK
    config = cast_pointer_to_tuple(config_raw_data, 'uint8_t', 5)
    assert config == (True, False, True, True, False)

    # restore defaults and check
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_config(True, True, True, False, True, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    config_raw_data = C.NK_read_config()
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK
    config = cast_pointer_to_tuple(config_raw_data, 'uint8_t', 5)
    assert config == (True, True, True, False, True)
