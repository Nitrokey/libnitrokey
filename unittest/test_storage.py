"""
Copyright (c) 2015-2019 Nitrokey UG

This file is part of libnitrokey.

libnitrokey is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

libnitrokey is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libnitrokey. If not, see <http://www.gnu.org/licenses/>.

SPDX-License-Identifier: LGPL-3.0
"""

import pprint
import pytest

from conftest import skip_if_device_version_lower_than
from constants import DefaultPasswords, DeviceErrorCode, bb
from misc import gs, wait, ffi

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


@pytest.mark.other
@pytest.mark.info
def test_get_status_storage(C):
    skip_if_device_version_lower_than({'S': 43})
    status_pointer = C.NK_get_status_storage_as_string()
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK
    status_string = gs(status_pointer)
    assert len(status_string) > 0
    status_dict = get_dict_from_dissect(status_string.decode('ascii'))
    default_admin_password_retry_count = 3
    assert int(status_dict['AdminPwRetryCount']) == default_admin_password_retry_count
    print('C.NK_get_major_firmware_version(): {}'.format(C.NK_get_major_firmware_version()))
    print('C.NK_get_minor_firmware_version(): {}'.format(C.NK_get_minor_firmware_version()))


@pytest.mark.other
@pytest.mark.info
def test_sd_card_usage(C):
    skip_if_device_version_lower_than({'S': 43})
    data_pointer = C.NK_get_SD_usage_data_as_string()
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK
    data_string = gs(data_pointer)
    assert len(data_string) > 0
    data_dict = get_dict_from_dissect(data_string.decode("ascii"))
    assert int(data_dict['WriteLevelMax']) <= 100


@pytest.mark.encrypted
def test_encrypted_volume_unlock(C):
    skip_if_device_version_lower_than({'S': 43})
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK


@pytest.mark.hidden
def test_encrypted_volume_unlock_hidden(C):
    skip_if_device_version_lower_than({'S': 43})
    hidden_volume_password = b'hiddenpassword'
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    assert C.NK_create_hidden_volume(0, 20, 21, hidden_volume_password) == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_hidden_volume(hidden_volume_password) == DeviceErrorCode.STATUS_OK


@pytest.mark.hidden
def test_encrypted_volume_setup_multiple_hidden_lock(C):
    import random
    skip_if_device_version_lower_than({'S': 45}) #hangs device on lower version
    hidden_volume_password = b'hiddenpassword' + bb(str(random.randint(0,100)))
    p = lambda i: hidden_volume_password + bb(str(i))
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    for i in range(4):
        assert C.NK_create_hidden_volume(i, 20+i*10, 20+i*10+i+1, p(i) ) == DeviceErrorCode.STATUS_OK
    for i in range(4):
        assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
        assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
        assert C.NK_unlock_hidden_volume(p(i)) == DeviceErrorCode.STATUS_OK


@pytest.mark.hidden
@pytest.mark.parametrize("volumes_to_setup", range(1, 5))
def test_encrypted_volume_setup_multiple_hidden_no_lock_device_volumes(C, volumes_to_setup):
    skip_if_device_version_lower_than({'S': 43})
    hidden_volume_password = b'hiddenpassword'
    p = lambda i: hidden_volume_password + bb(str(i))
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    for i in range(volumes_to_setup):
        assert C.NK_create_hidden_volume(i, 20+i*10, 20+i*10+i+1, p(i)) == DeviceErrorCode.STATUS_OK

    assert C.NK_lock_encrypted_volume() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK

    for i in range(volumes_to_setup):
        assert C.NK_unlock_hidden_volume(p(i)) == DeviceErrorCode.STATUS_OK
        # TODO mount and test for files
        assert C.NK_lock_hidden_volume() == DeviceErrorCode.STATUS_OK


@pytest.mark.hidden
@pytest.mark.parametrize("volumes_to_setup", range(1, 5))
def test_encrypted_volume_setup_multiple_hidden_no_lock_device_volumes_unlock_at_once(C, volumes_to_setup):
    skip_if_device_version_lower_than({'S': 43})
    hidden_volume_password = b'hiddenpassword'
    p = lambda i: hidden_volume_password + bb(str(i))
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    for i in range(volumes_to_setup):
        assert C.NK_create_hidden_volume(i, 20+i*10, 20+i*10+i+1, p(i)) == DeviceErrorCode.STATUS_OK
        assert C.NK_unlock_hidden_volume(p(i)) == DeviceErrorCode.STATUS_OK
        assert C.NK_lock_hidden_volume() == DeviceErrorCode.STATUS_OK

    assert C.NK_lock_encrypted_volume() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK

    for i in range(volumes_to_setup):
        assert C.NK_unlock_hidden_volume(p(i)) == DeviceErrorCode.STATUS_OK
        # TODO mount and test for files
        assert C.NK_lock_hidden_volume() == DeviceErrorCode.STATUS_OK


@pytest.mark.hidden
@pytest.mark.parametrize("use_slot", range(4))
def test_encrypted_volume_setup_one_hidden_no_lock_device_slot(C, use_slot):
    skip_if_device_version_lower_than({'S': 43})
    hidden_volume_password = b'hiddenpassword'
    p = lambda i: hidden_volume_password + bb(str(i))
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    i = use_slot
    assert C.NK_create_hidden_volume(i, 20+i*10, 20+i*10+i+1, p(i)) == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_hidden_volume(p(i)) == DeviceErrorCode.STATUS_OK
    assert C.NK_lock_hidden_volume() == DeviceErrorCode.STATUS_OK

    assert C.NK_lock_encrypted_volume() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK

    for j in range(3):
        assert C.NK_unlock_hidden_volume(p(i)) == DeviceErrorCode.STATUS_OK
        # TODO mount and test for files
        assert C.NK_lock_hidden_volume() == DeviceErrorCode.STATUS_OK


@pytest.mark.hidden
@pytest.mark.PWS
def test_password_safe_slot_name_corruption(C):
    skip_if_device_version_lower_than({'S': 43})
    volumes_to_setup = 4
    # connected with encrypted volumes, possible also with hidden
    def fill(s, wid):
        assert wid >= len(s)
        numbers = '1234567890' * 4
        s += numbers[:wid - len(s)]
        assert len(s) == wid
        return bb(s)

    def get_pass(suffix):
        return fill('pass' + suffix, 20)

    def get_loginname(suffix):
        return fill('login' + suffix, 32)

    def get_slotname(suffix):
        return fill('slotname' + suffix, 11)

    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_enable_password_safe(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    PWS_slot_count = 16
    for i in range(0, PWS_slot_count):
        iss = str(i)
        assert C.NK_write_password_safe_slot(i,
                                             get_slotname(iss), get_loginname(iss),
                                             get_pass(iss)) == DeviceErrorCode.STATUS_OK

    def check_PWS_correctness(C):
        for i in range(0, PWS_slot_count):
            iss = str(i)
            assert gs(C.NK_get_password_safe_slot_name(i)) == get_slotname(iss)
            assert gs(C.NK_get_password_safe_slot_login(i)) == get_loginname(iss)
            assert gs(C.NK_get_password_safe_slot_password(i)) == get_pass(iss)

    hidden_volume_password = b'hiddenpassword'
    p = lambda i: hidden_volume_password + bb(str(i))
    def check_volumes_correctness(C):
        for i in range(volumes_to_setup):
            assert C.NK_unlock_hidden_volume(p(i)) == DeviceErrorCode.STATUS_OK
            # TODO mount and test for files
            assert C.NK_lock_hidden_volume() == DeviceErrorCode.STATUS_OK

    check_PWS_correctness(C)

    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    for i in range(volumes_to_setup):
        assert C.NK_create_hidden_volume(i, 20+i*10, 20+i*10+i+1, p(i)) == DeviceErrorCode.STATUS_OK
        assert C.NK_unlock_hidden_volume(p(i)) == DeviceErrorCode.STATUS_OK
        assert C.NK_lock_hidden_volume() == DeviceErrorCode.STATUS_OK

    assert C.NK_lock_encrypted_volume() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK

    check_volumes_correctness(C)
    check_PWS_correctness(C)
    check_volumes_correctness(C)
    check_PWS_correctness(C)

    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    check_volumes_correctness(C)
    check_PWS_correctness(C)
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    check_volumes_correctness(C)
    check_PWS_correctness(C)


@pytest.mark.hidden
def test_hidden_volume_corruption(C):
    # bug: this should return error without unlocking encrypted volume each hidden volume lock, but it does not
    skip_if_device_version_lower_than({'S': 43})
    hidden_volume_password = b'hiddenpassword'
    p = lambda i: hidden_volume_password + bb(str(i))
    volumes_to_setup = 4
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    for i in range(volumes_to_setup):
        assert C.NK_create_hidden_volume(i, 20 + i * 10, 20 + i * 10 + i + 1, p(i)) == DeviceErrorCode.STATUS_OK
        assert C.NK_unlock_hidden_volume(p(i)) == DeviceErrorCode.STATUS_OK
        assert C.NK_lock_hidden_volume() == DeviceErrorCode.STATUS_OK

    assert C.NK_lock_encrypted_volume() == DeviceErrorCode.STATUS_OK

    assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
    for i in range(volumes_to_setup):
        assert C.NK_unlock_encrypted_volume(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK
        assert C.NK_unlock_hidden_volume(p(i)) == DeviceErrorCode.STATUS_OK
        wait(2)
        assert C.NK_lock_hidden_volume() == DeviceErrorCode.STATUS_OK


@pytest.mark.unencrypted
def test_unencrypted_volume_set_read_only(C):
    skip_if_device_version_lower_than({'S': 43})
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_set_unencrypted_read_only(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK


@pytest.mark.unencrypted
def test_unencrypted_volume_set_read_write(C):
    skip_if_device_version_lower_than({'S': 43})
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_set_unencrypted_read_write(DefaultPasswords.USER) == DeviceErrorCode.STATUS_OK


@pytest.mark.unencrypted
def test_unencrypted_volume_set_read_only_admin(C):
    skip_if_device_version_lower_than({'S': 51})
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_set_unencrypted_read_only_admin(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK


@pytest.mark.unencrypted
def test_unencrypted_volume_set_read_write_admin(C):
    skip_if_device_version_lower_than({'S': 51})
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_set_unencrypted_read_write_admin(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK


@pytest.mark.encrypted
@pytest.mark.skip(reason='not supported on recent firmware, except v0.49')
def test_encrypted_volume_set_read_only(C):
    skip_if_device_version_lower_than({'S': 99})
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_set_encrypted_read_only(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK


@pytest.mark.encrypted
@pytest.mark.skip(reason='not supported on recent firmware, except v0.49')
def test_encrypted_volume_set_read_write(C):
    skip_if_device_version_lower_than({'S': 99})
    assert C.NK_lock_device() == DeviceErrorCode.STATUS_OK
    assert C.NK_set_encrypted_read_write(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK


@pytest.mark.other
def test_export_firmware(C):
    skip_if_device_version_lower_than({'S': 43})
    assert C.NK_export_firmware(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK


@pytest.mark.other
def test_clear_new_sd_card_notification(C):
    skip_if_device_version_lower_than({'S': 43})
    assert C.NK_clear_new_sd_card_warning(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK


@pytest.mark.encrypted
@pytest.mark.slowtest
@pytest.mark.skip(reason='long test (about 1h)')
def test_fill_SD_card(C):
    skip_if_device_version_lower_than({'S': 43})
    status = C.NK_fill_SD_card_with_random_data(DefaultPasswords.ADMIN)
    assert status == DeviceErrorCode.STATUS_OK or status == DeviceErrorCode.BUSY
    while 1:
        value = C.NK_get_progress_bar_value()
        if value == -1: break
        assert 0 <= value <= 100
        assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK
        wait(5)


@pytest.mark.other
@pytest.mark.info
def test_get_busy_progress_on_idle(C):
    skip_if_device_version_lower_than({'S': 43})
    value = C.NK_get_progress_bar_value()
    assert value == -1
    assert C.NK_get_last_command_status() == DeviceErrorCode.STATUS_OK


@pytest.mark.update
def test_change_update_password(C):
    skip_if_device_version_lower_than({'S': 43})
    wrong_password = b'aaaaaaaaaaa'
    assert C.NK_change_update_password(wrong_password, DefaultPasswords.UPDATE_TEMP) == DeviceErrorCode.WRONG_PASSWORD
    assert C.NK_change_update_password(DefaultPasswords.UPDATE, DefaultPasswords.UPDATE_TEMP) == DeviceErrorCode.STATUS_OK
    assert C.NK_change_update_password(DefaultPasswords.UPDATE_TEMP, DefaultPasswords.UPDATE) == DeviceErrorCode.STATUS_OK


# @pytest.mark.skip(reason='no reversing method added yet')
@pytest.mark.update
def test_enable_firmware_update(C):
    skip_if_device_version_lower_than({'S': 50})
    wrong_password = b'aaaaaaaaaaa'
    assert C.NK_enable_firmware_update(wrong_password) == DeviceErrorCode.WRONG_PASSWORD
    # skip actual test - reason: no reversing method added yet
    # assert C.NK_enable_firmware_update(DefaultPasswords.UPDATE) == DeviceErrorCode.STATUS_OK


@pytest.mark.other
def test_send_startup(C):
    skip_if_device_version_lower_than({'S': 43})
    time_seconds_from_epoch = 0 # FIXME set proper date
    assert C.NK_send_startup(time_seconds_from_epoch) == DeviceErrorCode.STATUS_OK


@pytest.mark.other
def test_struct_multiline_prodtest(C):
    info_st = ffi.new('struct NK_storage_ProductionTest *')
    if info_st is None: raise Exception('Invalid value')
    err = C.NK_get_storage_production_info(info_st)
    assert err == 0
    assert info_st.SD_Card_ManufacturingYear_u8 != 0
    assert info_st.SD_Card_ManufacturingMonth_u8 != 0
    assert info_st.SD_Card_Size_u8 != 0
    assert info_st.FirmwareVersion_au8[0] == 0
    assert info_st.FirmwareVersion_au8[1] >= 50

    info = 'CPU:{CPU},SC:{SC},SD:{SD},' \
           'SCM:{SCM},SCO:{SCO},DAT:{DAT},Size:{size},Firmware:{fw} - {fwb}'.format(
        CPU='0x{:08x}'.format(info_st.CPU_CardID_u32),
        SC='0x{:08x}'.format(info_st.SmartCardID_u32),
        SD='0x{:08x}'.format(info_st.SD_CardID_u32),
        SCM='0x{:02x}'.format(info_st.SD_Card_Manufacturer_u8),
        SCO='0x{:04x}'.format(info_st.SD_Card_OEM_u16),
        DAT='20{}.{}'.format(info_st.SD_Card_ManufacturingYear_u8, info_st.SD_Card_ManufacturingMonth_u8),
        size=info_st.SD_Card_Size_u8,
        fw='{}.{}'.format(info_st.FirmwareVersion_au8[0], info_st.FirmwareVersion_au8[1]),
        fwb=info_st.FirmwareVersionInternal_u8
        )
    print(info)

@pytest.mark.other
@pytest.mark.firmware
def test_export_firmware_extended_fedora29(C):
    """
    Check, whether the firmware file is exported correctly, and in correct size.
    Apparently, the auto-remounting side effect of the v0.46 change, is disturbing the export process.
    Unmounting the UV just before the export gives the device 20/20 success rate.
    Test case for issue https://github.com/Nitrokey/nitrokey-app/issues/399
    """

    skip_if_device_version_lower_than({'S': 43})
    skip_if_not_fedora('Tested on Fedora only. To check on other distros.')

    from time import sleep
    import os
    from os.path import exists as exist
    import re
    try:
        import pyudev as pu
        import pexpect
    except:
        pytest.skip('Skipping due to missing required packages: pyudev and pexpect.')

    ctx = pu.Context()
    devices = ctx.list_devices(subsystem='block', ID_VENDOR='Nitrokey')
    device = None
    for d in devices:
        if d.device_type == 'partition':
            device = '/dev/{}'.format(d.sys_name)
            break
    assert device, 'Device could not be found'

    pexpect.run(f'udisksctl unmount -b {device}').decode()
    sleep(1)
    _res = pexpect.run(f'udisksctl mount -b {device}').decode()
    firmware_abs_path = re.findall('at (/.*)\.', _res)
    assert firmware_abs_path, 'Cannot get mount point'
    firmware_abs_path = firmware_abs_path[0]

    print('path: {}, device: {}'.format(firmware_abs_path, device))
    assert firmware_abs_path, 'Cannot get mount point'
    firmware_abs_path = firmware_abs_path + '/firmware.bin'

    checks = 0
    checks_add = 0

    if exist(firmware_abs_path):
        os.remove(firmware_abs_path)

    assert not exist(firmware_abs_path)

    ATTEMPTS = 20
    for i in range(ATTEMPTS):
        # if umount is disabled, success rate is 3/10, enabled: 10/10
        pexpect.run(f'udisksctl unmount -b {device}')
        assert C.NK_export_firmware(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK
        pexpect.run(f'udisksctl mount -b {device}')
        sleep(1)
        firmware_file_exist = exist(firmware_abs_path)
        if firmware_file_exist:
            checks += 1
            getsize = os.path.getsize(firmware_abs_path)
            print('Firmware file exist, size: {}'.format(getsize))
            checks_add += 1 if getsize >= 100 * 1024 else 0
            # checks_add += 1 if os.path.getsize(firmware_abs_path) == 256*1024 else 0
            os.remove(firmware_abs_path)
        assert not exist(firmware_abs_path)

    print('CHECK {} ; CHECK ADDITIONAL {}'.format(checks, checks_add))

    assert checks == ATTEMPTS
    assert checks_add == checks


def skip_if_not_fedora(message:str) -> None:
    import os
    from os.path import exists as exist

    def skip():
        pytest.skip(message)

    os_release_fp = '/etc/os-release'
    if not exist(os_release_fp):
        skip()
    with open(os_release_fp) as f:
        os_release_lines = f.readlines()
    if 'Fedora' not in os_release_lines[0]:
        skip()


@pytest.mark.other
@pytest.mark.firmware
def test_export_firmware_extended_macos(C):
    """
    Check, whether the firmware file is exported correctly, and in correct size.
    Apparently, the auto-remounting side effect of the v0.46 change, is disturbing the export process.
    Unmounting the UV just before the export gives the device 20/20 success rate.
    Test case for issue https://github.com/Nitrokey/nitrokey-app/issues/399
    """

    skip_if_device_version_lower_than({'S': 43})
    skip_if_not_macos('macOS specific test, due to the mount path and command.')

    import pexpect
    from time import sleep
    import os
    from os.path import exists as exist
    import plistlib

    usb_devices = pexpect.run('system_profiler -xml SPUSBDataType')
    assert b'Nitrokey' in usb_devices, 'No Nitrokey devices connected'
    usb_devices_parsed = plistlib.loads(usb_devices)

    assert isinstance(usb_devices_parsed, list), 'usb_devices_parsed has unexpected type'

    # Try to get all USB devices
    try:
        devices = usb_devices_parsed[0]['_items'][0]['_items']
    except KeyError:
        devices = None

    assert devices is not None, 'could not list USB devices'

    device_item = None

    for item in devices:
        if '_items' in item:
            # Fix for macOS 10.13.6, Python 3.6.2
            item = item['_items'][0]
        if 'manufacturer' in item and item['manufacturer'] == 'Nitrokey':
            device_item = item

    # Try to get first volume of USB device
    try:
        volume = device_item['Media'][0]['volumes'][0]
    except (KeyError, TypeError):
        volume = None

    assert volume is not None, 'could not determine volume'
    assert 'bsd_name' in volume, 'could not get BSD style device name'

    device = '/dev/' + volume['bsd_name']
    pexpect.run(f'diskutil mount {device}')
    sleep(3)
    assert 'mount_point' in volume, 'could not get mount point'
    firmware_abs_path = volume['mount_point'] + '/firmware.bin'
    checks = 0
    print('path: {}, device: {}'.format(firmware_abs_path, device))
    checks_add = 0

    if exist(firmware_abs_path):
        os.remove(firmware_abs_path)

    assert not exist(firmware_abs_path)

    ATTEMPTS = 20
    for i in range(ATTEMPTS):
        # if umount is disabled, success rate is 3/10, enabled: 10/10
        pexpect.run(f'diskutil unmount {device}')
        assert C.NK_export_firmware(DefaultPasswords.ADMIN) == DeviceErrorCode.STATUS_OK
        pexpect.run(f'diskutil mount {device}')
        sleep(1)
        firmware_file_exist = exist(firmware_abs_path)
        if firmware_file_exist:
            checks += 1
            getsize = os.path.getsize(firmware_abs_path)
            print('Firmware file exist, size: {}'.format(getsize))
            checks_add += 1 if getsize >= 100 * 1024 else 0
            # checks_add += 1 if os.path.getsize(firmware_abs_path) == 256*1024 else 0
            os.remove(firmware_abs_path)
        assert not exist(firmware_abs_path)

    print('CHECK {} ; CHECK ADDITIONAL {}'.format(checks, checks_add))

    assert checks == ATTEMPTS
    assert checks_add == checks


def skip_if_not_macos(message:str) -> None:
    import platform

    if platform.system() != 'Darwin':
        pytest.skip(message)
