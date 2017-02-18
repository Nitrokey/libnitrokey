import pytest

from misc import ffi

device_type = None

def skip_if_device_version_lower_than(allowed_devices):
    global device_type
    model, version = device_type
    infinite_version_number = 999
    if allowed_devices.get(model, infinite_version_number) > version:
        pytest.skip('This device model is not applicable to run this test')


@pytest.fixture(scope="module")
def C(request):
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
            print(declaration)
            ffi.cdef(declaration, override=True)

    C = ffi.dlopen("../build/libnitrokey.so")
    C.NK_set_debug(False)
    nk_login = C.NK_login_auto()
    if nk_login != 1:
        print('No devices detected!')
    assert nk_login != 0  # returns 0 if not connected or wrong model or 1 when connected
    global device_type
    firmware_version = C.NK_get_major_firmware_version()
    model = 'P' if firmware_version in [7,8] else 'S'
    device_type = (model, firmware_version)

    # assert C.NK_first_authenticate(DefaultPasswords.ADMIN, DefaultPasswords.ADMIN_TEMP) == DeviceErrorCode.STATUS_OK
    # assert C.NK_user_authenticate(DefaultPasswords.USER, DefaultPasswords.USER_TEMP) == DeviceErrorCode.STATUS_OK

    # C.NK_status()

    def fin():
        print('\nFinishing connection to device')
        C.NK_logout()
        print('Finished')

    request.addfinalizer(fin)
    C.NK_set_debug(True)

    return C
