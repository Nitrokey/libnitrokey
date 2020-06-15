import pytest

from conftest import skip_if_device_version_lower_than, library_device_reconnect
from constants import DefaultPasswords, DeviceErrorCode, LibraryErrors
from helpers import helper_populate_device, helper_check_device_for_data


@pytest.mark.firmware
def test_bootloader_password_change_pro_length(C):
    skip_if_device_version_lower_than({'P': 11})

    # Test whether the correct password is set
    assert C.NK_change_firmware_password_pro(DefaultPasswords.UPDATE, DefaultPasswords.UPDATE) == DeviceErrorCode.STATUS_OK
    # Change to the longest possible password
    assert C.NK_change_firmware_password_pro(DefaultPasswords.UPDATE, DefaultPasswords.UPDATE_LONG) == DeviceErrorCode.STATUS_OK
    assert C.NK_change_firmware_password_pro(DefaultPasswords.UPDATE_LONG, DefaultPasswords.UPDATE) == DeviceErrorCode.STATUS_OK
    # Use longer or shorter passwords than possible
    assert C.NK_change_firmware_password_pro(DefaultPasswords.UPDATE, DefaultPasswords.UPDATE_TOO_LONG) == LibraryErrors.TOO_LONG_STRING
    assert C.NK_change_firmware_password_pro(DefaultPasswords.UPDATE, DefaultPasswords.UPDATE_TOO_SHORT) == DeviceErrorCode.WRONG_PASSWORD



@pytest.mark.firmware
def test_bootloader_password_change_pro(C):
    skip_if_device_version_lower_than({'P': 11})
    assert C.NK_change_firmware_password_pro(b'zxcasd', b'zxcasd') == DeviceErrorCode.WRONG_PASSWORD

    # Revert effects of broken test run, if needed
    C.NK_change_firmware_password_pro(DefaultPasswords.UPDATE_TEMP, DefaultPasswords.UPDATE)

    # Change to the same password
    assert C.NK_change_firmware_password_pro(DefaultPasswords.UPDATE, DefaultPasswords.UPDATE) == DeviceErrorCode.STATUS_OK
    assert C.NK_change_firmware_password_pro(DefaultPasswords.UPDATE, DefaultPasswords.UPDATE) == DeviceErrorCode.STATUS_OK
    # Change password
    assert C.NK_change_firmware_password_pro(DefaultPasswords.UPDATE, DefaultPasswords.UPDATE_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_change_firmware_password_pro(DefaultPasswords.UPDATE_TEMP, DefaultPasswords.UPDATE) == DeviceErrorCode.STATUS_OK


@pytest.mark.firmware
def test_bootloader_run_pro_wrong_password(C):
    skip_if_device_version_lower_than({'P': 11})
    assert C.NK_enable_firmware_update_pro(DefaultPasswords.UPDATE_TEMP) == DeviceErrorCode.WRONG_PASSWORD


@pytest.mark.skip_by_default
@pytest.mark.firmware
def test_bootloader_run_pro_real(C):
    skip_if_device_version_lower_than({'P': 11})
    # Not enabled due to lack of side-effect removal at this point
    assert C.NK_enable_firmware_update_pro(DefaultPasswords.UPDATE) == DeviceErrorCode.STATUS_DISCONNECTED


@pytest.mark.firmware
def test_bootloader_password_change_pro_too_long(C):
    skip_if_device_version_lower_than({'P': 11})
    long_string = b'a' * 100
    assert C.NK_change_firmware_password_pro(long_string, long_string) == LibraryErrors.TOO_LONG_STRING
    assert C.NK_change_firmware_password_pro(DefaultPasswords.UPDATE, long_string) == LibraryErrors.TOO_LONG_STRING


@pytest.mark.skip_by_default
@pytest.mark.firmware
def test_bootloader_data_rention(C):
    skip_if_device_version_lower_than({'P': 11})

    assert helper_populate_device(C)
    assert C.NK_enable_firmware_update_pro(DefaultPasswords.UPDATE) == DeviceErrorCode.STATUS_DISCONNECTED
    input('Please press ENTER after uploading new firmware to the device')
    C = library_device_reconnect(C)
    assert helper_check_device_for_data(C)

