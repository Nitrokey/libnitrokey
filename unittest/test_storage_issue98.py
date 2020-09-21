import os
from time import sleep

import pexpect
import pytest

from conftest import skip_if_device_version_lower_than
from constants import DefaultPasswords, DeviceErrorCode
from helpers import helper_populate_device, \
    helper_check_device_for_data
from misc import gs


def reinsert():
    uhub_path = os.getenv('LIBNK_UHUBPATH', 'uhubctl')
    uhub_loc = os.getenv('LIBNK_UHUBLOC', None)  # e.g. -p 1 --loc 5-4
    assert uhub_path
    assert uhub_loc
    uhub_path = f'{uhub_path} {uhub_loc} '
    param_on = ' -a 1 '
    param_off = ' -a 0 '
    print(f"Reinsert device: {uhub_path + param_off}")
    pexpect.run(uhub_path + param_off)
    my_sleep(1)
    print(f"Reinsert device: {uhub_path + param_on}")
    pexpect.run(uhub_path + param_on)
    my_sleep(1)

def get_status(C) -> str:
    status = C.NK_get_status_as_string()
    s = gs(status)
    return s

def my_sleep(t:int):
    print(f"Sleep for {t} seconds")
    sleep(t)

@pytest.mark.other
@pytest.mark.firmware
def test_data_lost(C):
    """
    Test case for issue: https://github.com/Nitrokey/nitrokey-storage-firmware/issues/98
    """

    skip_if_device_version_lower_than({'S': 54})

    # print(f"Status: {get_status(C)}")

    # helper_populate_device(C)
    helper_check_device_for_data(C)

    commands = [
        lambda: C.NK_unlock_encrypted_volume(DefaultPasswords.USER),
        lambda: C.NK_set_unencrypted_read_only_admin(DefaultPasswords.ADMIN),
        lambda: C.NK_set_unencrypted_read_write_admin(DefaultPasswords.ADMIN),
        lambda: C.NK_set_encrypted_read_only(DefaultPasswords.ADMIN),
        lambda: C.NK_set_encrypted_read_write(DefaultPasswords.ADMIN),
        lambda: C.NK_clear_new_sd_card_warning(DefaultPasswords.ADMIN),
        lambda: C.NK_send_startup(4242),
        lambda: C.NK_lock_encrypted_volume(),
    ]

    iterations = 0
    failed = []
    while True:
        iterations += 1
        print(f"*** Starting iteration {iterations} (failed [{len(failed)}]{failed})")

        for cmd_i , c in enumerate(commands):
            assert c() == DeviceErrorCode.STATUS_OK
            my_sleep(1)
            reinsert()

            try:
                helper_check_device_for_data(C)
            except Exception as e:
                print(f"Check failed: {str(e)}")
                print("".center(80, "*"))
                print("".center(80, "*"))
                print("".center(80, "*"))
                my_sleep(3)
                failed += cmd_i
        # print(f"Status: {get_status(C)}")
        assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
        my_sleep(1)
