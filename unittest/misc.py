import cffi

ffi = cffi.FFI()
gs = ffi.string


def to_hex(s):
    return "".join("{:02x}".format(ord(c)) for c in s)


def wait(t):
    import time
    msg = 'Waiting for %d seconds' % t
    print(msg.center(40, '='))
    time.sleep(t)


def cast_pointer_to_tuple(obj, typen, len):
    # usage:
    #     config = cast_pointer_to_tuple(config_raw_data, 'uint8_t', 5)
    return tuple(ffi.cast("%s [%d]" % (typen, len), obj)[0:len])

def get_firmware_version_from_status(C):
    status = gs(C.NK_status())
    status = [s if 'firmware_version' in s else '' for s in status.split('\n')]
    firmware = status[0].split(':')[1]
    return firmware


def is_pro_rtm_07(C):
    firmware = get_firmware_version_from_status(C)
    return '07 00' in firmware


def is_storage(C):
    """
    exact firmware storage is sent by other function
    """
    firmware = get_firmware_version_from_status(C)
    return '01 00' in firmware