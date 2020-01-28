def bb(x):
    return bytes(x, encoding='ascii')


def helper_fill(str_to_fill, target_width):
    assert target_width >= len(str_to_fill)
    numbers = '1234567890' * 4
    str_to_fill += numbers[:target_width - len(str_to_fill)]
    assert len(str_to_fill) == target_width
    return bb(str_to_fill)


def helper_PWS_get_pass(suffix):
    return helper_fill('pass' + suffix, 20)


def helper_PWS_get_loginname(suffix):
    return helper_fill('login' + suffix, 32)


def helper_PWS_get_slotname(suffix):
    return helper_fill('slotname' + suffix, 11)