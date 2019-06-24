#!/usr/bin/env python3

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

import click

from conftest import get_library, get_device_type
from constants import DefaultPasswords, DeviceErrorCode


@click.group()
def cli():
    pass


@click.command()
def update():
    libnk = get_library(None)
    device_type = get_device_type()
    print(device_type)
    assert device_type[0] == 'S'
    assert libnk.NK_enable_firmware_update(DefaultPasswords.UPDATE) == DeviceErrorCode.STATUS_OK


cli.add_command(update)

if __name__ == '__main__':
    cli()
