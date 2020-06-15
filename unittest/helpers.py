from constants import DeviceErrorCode, PWS_SLOT_COUNT, DefaultPasswords
from misc import gs, bb


def helper_fill(str_to_fill, target_width):
    assert target_width >= len(str_to_fill)
    numbers = '1234567890' * 4
    str_to_fill += numbers[:target_width - len(str_to_fill)]
    assert len(str_to_fill) == target_width
    return bb(str_to_fill)


def helper_PWS_get_pass(suffix):
    return helper_fill('pass' + suffix, 20)


def helper_PWS_get_loginname(suffix):
    return helper_fill('login' + suffix, 32)


def helper_PWS_get_slotname(suffix):
    return helper_fill('slotname' + suffix, 11)


def helper_check_device_for_data(C):
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK

    for i in range(0, PWS_SLOT_COUNT):
        iss = str(i)
        assert gs(C.NK_get_password_safe_slot_name(i)) == helper_PWS_get_slotname(iss)
        assert gs(C.NK_get_password_safe_slot_login(i)) == helper_PWS_get_loginname(iss)
        assert gs(C.NK_get_password_safe_slot_password(i)) == helper_PWS_get_pass(iss)
    return True


def helper_populate_device(C):
    # FIXME use object with random data, and check against it
    # FIXME generate OTP as well, and check codes against its secrets
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    res = C.NK_enable_password_safe(DefaultPasswords.USER)
    if res != DeviceErrorCode.STATUS_OK:
        assert C.NK_build_aes_key(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK
        assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK

    for i in range(0, PWS_SLOT_COUNT):
        iss = str(i)
        assert C.NK_write_password_safe_slot(i,
                                             helper_PWS_get_slotname(iss), helper_PWS_get_loginname(iss),
                                             helper_PWS_get_pass(iss)) == DeviceErrorCode.STATUS_OK
    return True
