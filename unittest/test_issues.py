from enum import Enum

import pytest

from constants import DefaultPasswords, DeviceErrorCode
from misc import gs
from test_pro import check_HOTP_RFC_codes


def test_destroy_encrypted_data_leaves_OTP_intact(C):
    """
    Make sure AES key regeneration will not remove OTP records.
    Test for Nitrokey Storage.
    Details: https://github.com/Nitrokey/libnitrokey/issues/199
    """
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    # write password safe slot
    assert C.NK_write_password_safe_slot(0, b'slotname1', b'login1', b'pass1') == DeviceErrorCode.STATUS_OK
    # read slot
    assert gs(C.NK_get_password_safe_slot_name(0)) == b'slotname1'
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK
    slot_login = C.NK_get_password_safe_slot_login(0)
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK
    assert gs(slot_login) == b'login1'

    # write OTP
    use_8_digits = False
    use_pin_protection = False
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_write_config(255, 255, 255, use_pin_protection, not use_pin_protection,
                             DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    check_HOTP_RFC_codes(C, lambda x: gs(C.NK_get_hotp_code(x)), use_8_digits=use_8_digits)

    # confirm OTP
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert gs(C.NK_get_hotp_slot_name(1)) == b'python_test'

    # destroy password safe by regenerating aes key
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK

    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_build_aes_key(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    assert gs(C.NK_get_password_safe_slot_name(0)) != b'slotname1'
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK

    # confirm OTP
    assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    assert gs(C.NK_get_hotp_slot_name(1)) == b'python_test'


class Modes(Enum):
    EmptyBody = 0
    FactoryResetWithAES = 1
    FactoryReset = 2
    AESGeneration = 3

@pytest.mark.firmware
@pytest.mark.factory_reset
@pytest.mark.parametrize("mode", map(Modes, range(4)))
def test_pro_factory_reset_breaks_update_password(C, mode: Modes):
    from test_pro_bootloader import test_bootloader_password_change_pro, test_bootloader_password_change_pro_length
    from test_pro import test_factory_reset

    func = {
        0: lambda: True,
        1: lambda: test_factory_reset(C) or True,
        2: lambda: C.NK_factory_reset(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK,
        3: lambda: C.NK_build_aes_key(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK,
    }

    def boot_test(C):
        test_bootloader_password_change_pro(C)
        test_bootloader_password_change_pro_length(C)

    boot_test(C)
    func[mode.value]()
    boot_test(C)
