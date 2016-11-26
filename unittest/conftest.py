import pytest

from misc import ffi

@pytest.fixture(scope="module")
def C(request):
    fp = '../NK_C_API.h'

    declarations = []
    with open(fp, 'r') as f:
        declarations = f.readlines()

    a = iter(declarations)
    for declaration in a:
        if declaration.startswith('extern') and not '"C"' in declaration:
            declaration = declaration.replace('extern', '').strip()
            while not ';' in declaration:
                declaration += (next(a)).strip()
            print(declaration)
            ffi.cdef(declaration, override=True)

    C = ffi.dlopen("../build/libnitrokey.so")
    C.NK_set_debug(False)
    nk_login = C.NK_login_auto()
    if nk_login != 1:
        print('No devices detected!')
    assert nk_login == 1  # returns 0 if not connected or wrong model or 1 when connected

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
