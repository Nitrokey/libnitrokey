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

import pytest

from misc import ffi, gs

device_type = None

from logging import getLogger, basicConfig, DEBUG

basicConfig(format='* %(relativeCreated)6d %(filename)s:%(lineno)d %(message)s',level=DEBUG)
log = getLogger('conftest')
print = log.debug

def get_device_type():
    return device_type


def skip_if_device_version_lower_than(allowed_devices):
    global device_type
    model, version = device_type
    infinite_version_number = 999
    if allowed_devices.get(model, infinite_version_number) > version:
        pytest.skip('This device model is not applicable to run this test')


class AtrrCallProx(object):
    def __init__(self, C, name):
        self.C = C
        self.name = name

    def __call__(self, *args, **kwargs):
        print('Calling {}{}'.format(self.name, args))
        res = self.C(*args, **kwargs)
        res_s = res
        try:
            res_s = '{} => '.format(res) + '{}'.format(gs(res))
        except Exception as e:
            pass
        print('Result of {}: {}'.format(self.name, res_s))
        return res


class AttrProxy(object):
    def __init__(self, C, name):
        self.C = C
        self.name = name

    def __getattr__(self, attr):
        return AtrrCallProx(getattr(self.C, attr), attr)


@pytest.fixture(scope="module")
def C_offline(request=None):
    print("Getting library without initializing connection")
    return get_library(request, allow_offline=True)


@pytest.fixture(scope="module")
def C(request=None):
    print("Getting library with connection initialized")
    return get_library(request)


def get_library(request, allow_offline=False):
    fp = '../NK_C_API.h'

    declarations = []
    with open(fp, 'r') as f:
        declarations = f.readlines()

    cnt = 0
    a = iter(declarations)
    for declaration in a:
        if declaration.strip().startswith('NK_C_API') \
                or declaration.strip().startswith('struct'):
            declaration = declaration.replace('NK_C_API', '').strip()
            while ');' not in declaration and '};' not in declaration:
                declaration += (next(a)).strip()+'\n'
            ffi.cdef(declaration, override=True)
            cnt += 1
    print('Imported {} declarations'.format(cnt))

    C = None
    import os, sys
    path_build = os.path.join("..", "build")
    paths = [
            os.environ.get('LIBNK_PATH', None),
            os.path.join(path_build,"libnitrokey.so"),
            os.path.join(path_build,"libnitrokey.dylib"),
            os.path.join(path_build,"libnitrokey.dll"),
            os.path.join(path_build,"nitrokey.dll"),
    ]
    for p in paths:
        if not p: continue
        print("Trying " +p)
        p = os.path.abspath(p)
        if os.path.exists(p):
            print("Found: "+p)
            C = ffi.dlopen(p)
            break
        else:
            print("File does not exist: " + p)
    if not C:
        print("No library file found")
        sys.exit(1)

    C.NK_set_debug_level(int(os.environ.get('LIBNK_DEBUG', 2)))

    nk_login = C.NK_login_auto()
    if nk_login != 1:
        print('No devices detected!')
    if not allow_offline:
        assert nk_login != 0  # returns 0 if not connected or wrong model or 1 when connected
        global device_type
        firmware_version = C.NK_get_minor_firmware_version()
        model = C.NK_get_device_model()
        model = 'P' if model == 1 else 'S' if model == 2 else 'U'
        device_type = (model, firmware_version)
        print('Connected device: {} {}'.format(model, firmware_version))

    # assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    # assert C.NK_user_authenticate(DefaultPasswords.USER, DefaultPasswords.USER_TEMP) == DeviceErrorCode.STATUS_OK

    # C.NK_status()

    def fin():
        print('\nFinishing connection to device')
        C.NK_logout()
        print('Finished')

    if request:
        request.addfinalizer(fin)
    # C.NK_set_debug(True)
    C.NK_set_debug_level(int(os.environ.get('LIBNK_DEBUG', 3)))

    return AttrProxy(C, "libnitrokey C")


def pytest_addoption(parser):
    parser.addoption("--run-skipped", action="store_true",
                     help="run the tests skipped by default, e.g. adding side effects")

def pytest_runtest_setup(item):
    if 'skip_by_default' in item.keywords and not item.config.getoption("--run-skipped"):
        pytest.skip("need --run-skipped option to run this test")