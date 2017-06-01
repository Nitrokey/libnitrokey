#!/usr/bin/env python

"""
Use the following to run locally:

$ sudo pip install virtualenv
$ virtualenv venv
$ . venv/bin/activate
$ pip install --editable .
$ libnitrokey --help
"""

import click
import itertools
import cffi

ffi = cffi.FFI()

gs = ffi.string
device_type = None


def connect():
    fp = '../NK_C_API.h'

    declarations = []
    with open(fp, 'r') as f:
        declarations = f.readlines()

    a = iter(declarations)
    for declaration in a:
        if declaration.startswith('NK_C_API'):
            declaration = declaration.replace('NK_C_API', '').strip()
            while not ';' in declaration:
                declaration += (next(a)).strip()
            ffi.cdef(declaration, override=True)

    C = None
    import os, sys

    path_build = [".", os.path.join(".", "build"), os.path.join("..", "build")]
    names = ["libnitrokey-log.so", "libnitrokey.so"]
    paths = itertools.product(path_build, names)
    tested = []
    for p in paths:
        p = os.path.join(p[0], p[1])
        tested.append(p)
        if os.path.exists(p):
            C = ffi.dlopen(p)
            break
    if not C:
        print("No library file found")
        print("Tested paths:" + repr(tested))
        sys.exit(1)

    C.NK_set_debug(False)

    nk_login = C.NK_login_auto()
    if nk_login != 1:
        print('No devices detected!')
    assert nk_login != 0  # returns 0 if not connected or wrong model or 1 when connected
    global device_type
    firmware_version = C.NK_get_major_firmware_version()
    model = 'P' if firmware_version in [7, 8] else 'S'
    device_type = (model, firmware_version)

    # assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    # assert C.NK_user_authenticate(DefaultPasswords.USER, DefaultPasswords.USER_TEMP) == DeviceErrorCode.STATUS_OK

    # C.NK_status()

    C.NK_set_debug(True)
    return C


C = connect()


@click.group()
@click.option('--debug', default=0, help='debug level')
def cli(debug):
    C.NK_set_debug(debug != 0)
    pass


@click.command()
def status():
    """Show device's status"""
    print (gs(C.NK_status()))
    if device_type[0] == 'S':
        print (gs(C.NK_get_status_storage_as_string()))
    return C.NK_get_last_command_status()


@click.command()
def device_serial_number():
    """Show device's serial number"""
    print (gs(C.NK_device_serial_number()))
    return C.NK_get_last_command_status()


def abort_callback(ctx, param, value):
    if not value:
        ctx.abort()


@click.command()
@click.option('--yes', is_flag=True, callback=abort_callback,
              expose_value=False, prompt='Do you want to continue?')
def lock_device():
    """Lock the device - lock volumes and password safe"""
    print C.NK_lock_device() or "Locked"
    return C.NK_get_last_command_status()


#
# @click.command()
# def NK_first_authenticate():
#     """Show device status"""
#     print C.NK_first_authenticate()
#     return C.NK_get_last_command_status()
#
#
# @click.command()
# def NK_user_authenticate():
#     """Show device status"""
#     print C.NK_user_authenticate()
#     return C.NK_get_last_command_status()


# @click.command()
# def NK_factory_reset():
#     """Show device status"""
#     print C.NK_factory_reset()
#     return C.NK_get_last_command_status()
#
#
# @click.command()
# def NK_build_aes_key():
#     """Show device status"""
#     print C.NK_build_aes_key()
#     return C.NK_get_last_command_status()
#
#
# @click.command()
# @click.argument('slot_number')
# def NK_unlock_user_password(slot_number):
#     """Show device status"""
#     print C.NK_unlock_user_password()
#     return C.NK_get_last_command_status()


@click.command()
@click.argument('slot_number')
def get_totp_slot_name(slot_number):
    """Show TOTP slot name"""
    print gs(C.NK_get_totp_slot_name(int(slot_number))) or "OTP slot not programmed"
    return C.NK_get_last_command_status()


@click.command()
@click.argument('slot_number')
def get_hotp_slot_name(slot_number):
    """Show HOTP slot name"""
    print gs(C.NK_get_hotp_slot_name(int(slot_number))) or "OTP slot not programmed"
    return C.NK_get_last_command_status()


#
# @click.command()
# def NK_erase_hotp_slot():
#     """Show device status"""
#     print C.NK_erase_hotp_slot()
#     return C.NK_get_last_command_status()
#
#
# @click.command()
# def NK_erase_totp_slot():
#     """NK_erase_totp_slot"""
#     print C.NK_erase_totp_slot()
#     return C.NK_get_last_command_status()


@click.command()
@click.argument('slot_number')
def get_hotp_code(slot_number):
    """Get HOTP code"""
    print gs(C.NK_get_hotp_code(int(slot_number))) or "OTP slot not programmed"
    return C.NK_get_last_command_status()


@click.command()
@click.argument('slot_number')
def get_totp_code(slot_number):
    """Get TOTP code"""
    print gs(C.NK_get_totp_code(int(slot_number), 0, 0, 0)) or "OTP slot not programmed"
    return C.NK_get_last_command_status()


@click.command()
@click.option('--user_pin', prompt=True, confirmation_prompt=True,
              hide_input=True)
def unlock_encrypted_volume(user_pin):
    """Unlock encrypted volume"""
    print C.NK_unlock_encrypted_volume(str(user_pin)) or "Unlocked"
    return C.NK_get_last_command_status()


@click.command()
@click.option('--hidden_volume_password', prompt=True, confirmation_prompt=True,
              hide_input=True)
def unlock_hidden_volume(hidden_volume_password):
    """Unlock hidden volume"""
    print C.NK_unlock_hidden_volume(str(hidden_volume_password)) or "Unlocked"
    return C.NK_get_last_command_status()


cli.add_command(status)
cli.add_command(device_serial_number)
cli.add_command(lock_device)
cli.add_command(unlock_encrypted_volume)
cli.add_command(unlock_hidden_volume)

cli.add_command(get_hotp_code)
cli.add_command(get_totp_code)
cli.add_command(get_hotp_slot_name)
cli.add_command(get_totp_slot_name)

if __name__ == '__main__':
    cli()
    C.NK_logout()

