import pytest

from misc import ffi, gs, wait, cast_pointer_to_tuple
from constants import DefaultPasswords, DeviceErrorCode, RFC_SECRET, LibraryErrors

import pprint
pprint = pprint.PrettyPrinter(indent=4).pprint


def get_dict_from_dissect(status):
    x = []
    for s in status.split('\n'):
        try:
            if not ':' in s: continue
            ss = s.replace('\t', '').replace(' (int) ', '').split(':')
            if not len(ss) == 2: continue
            x.append(ss)
        except:
            pass
    d = {k.strip(): v.strip() for k, v in x}
    return d


def test_get_status_storage(C):
    status_pointer = C.NK_get_status_storage()
    status_string = gs(status_pointer)
    status_dict = get_dict_from_dissect(status_string)
    default_admin_password_retry_count = 3
    assert int(status_dict['AdminPwRetryCount']) == default_admin_password_retry_count


def test_sd_card_usage(C):
    data_pointer = C.NK_get_SD_usage_data()
    data_string = gs(data_pointer)
    assert len(data_string) > 0
    data_dict = get_dict_from_dissect(data_string)
    assert int(data_dict['WriteLevelMax']) <= 100


def test_encrypted_volume_unlock(C):
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK


def test_encrypted_volume_unlock_hidden(C):
    hidden_volume_password = 'hiddenpassword'
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    assert C.NK_create_hidden_volume(0, 20, 21, hidden_volume_password) == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_hidden_volume(hidden_volume_password) == DeviceErrorCode.STATUS_OK


def test_encrypted_volume_setup_multiple_hidden(C):
    hidden_volume_password = 'hiddenpassword'
    p = lambda i: hidden_volume_password + str(i)
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    for i in range(4):
        assert C.NK_create_hidden_volume(i, 20+i*10, 20+i*10+i+1, p(i) ) == DeviceErrorCode.STATUS_OK
    for i in range(4):
        assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
        assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
        assert C.NK_unlock_hidden_volume(p(i)) == DeviceErrorCode.STATUS_OK


def test_unencrypted_volume_set_read_only(C):
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_set_unencrypted_read_only(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK


def test_unencrypted_volume_set_read_write(C):
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_set_unencrypted_read_write(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK


def test_export_firmware(C):
    assert C.NK_export_firmware(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK


def test_clear_new_sd_card_notification(C):
    assert C.NK_clear_new_sd_card_warning(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK


@pytest.mark.skip
def test_fill_SD_card(C):
    status = C.NK_fill_SD_card_with_random_data(DefaultPasswords.ADMIN)
    assert status == DeviceErrorCode.STATUS_OK or status == DeviceErrorCode.BUSY
    while 1:
        value = C.NK_get_progress_bar_value()
        if value == -1: break
        assert 0 <= value <= 100
        assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK
        wait(5)


def test_get_busy_progress_on_idle(C):
    value = C.NK_get_progress_bar_value()
    assert value == -1
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK


def test_change_update_password(C):
    wrong_password = 'aaaaaaaaaaa'
    assert C.NK_change_update_password(wrong_password, DefaultPasswords.UPDATE_TEMP) == DeviceErrorCode.WRONG_PASSWORD
    assert C.NK_change_update_password(DefaultPasswords.UPDATE, DefaultPasswords.UPDATE_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_change_update_password(DefaultPasswords.UPDATE_TEMP, DefaultPasswords.UPDATE) == DeviceErrorCode.STATUS_OK


def test_send_startup(C):
    time_seconds_from_epoch = 0 # FIXME set proper date
    assert C.NK_send_startup(time_seconds_from_epoch) == DeviceErrorCode.STATUS_OK
