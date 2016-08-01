# libnitrokey
Libnitrokey is a project to communicate with Nitrokey stick devices in clean and easy manner. Written in C++14, testable with `Catch` framework, with C API, Python access (through CFFI and C API, in future with Pybind11).
The development of this project is aimed to make it itself a living documentation of communication protocol between host and the Nitrokey stick device.
A C++14 complying compiler is required.

## Getting sources
This repository uses `git submodules`.
To clone please use git's --recursive option like in:
```bash
git clone --recursive https://github.com/Nitrokey/libnitrokey.git
```
or for already cloned repository:
```bash
git clone https://github.com/Nitrokey/libnitrokey.git
cd libnitrokey
git submodule update --init --recursive
```

## Compilation
To compile library using clang please run `make`. If you have GCC and would like to use it instead you can run:
```bash
 make CXX=g++
```
This should create a library file under path build/libnitrokey.so and compile C++ tests in unittest/ directory.

## Using with Python
To use libnitrokey with Python a [CFFI](http://cffi.readthedocs.io/en/latest/overview.html) library is required (either 2.7+ or 3.0+).
Just import it and read the C API header and it is done! You have access to the library. Example code printing HOTP code:
```python
ffi = cffi.FFI()
get_string = ffi.string

def get_library():
    fp = 'NK_C_API.h' # path to C API header

    declarations = []
    with open(fp, 'r') as f:
        declarations = f.readlines()

    for declaration in declarations:
        if 'extern' in declaration and not '"C"' in declaration:
            declaration = declaration.replace('extern', '').strip()
            print(declaration)
            ffi.cdef(declaration)

    C = ffi.dlopen("build/libnitrokey.so") # path to built library
    return C

def get_hotp_code(lib, i):
    lib.NK_get_hotp_code(i)

libnitrokey = get_library()
hotp_slot_1_code = get_hotp_code(libnitrokey, 1)
print (hotp_slot_1_code)

```
All available functions for Python are listed in NK_C_API.h.

## Documentation
The documentation of C API is included in the sources (could be  generated with doxygen if requested).
Please check NK_C_API.h (C API) for high level commands and include/NitrokeyManager.h (C++ API). All devices' commands are listed along with packet format in include/stick10_commands.h and include/stick20_commands.h respectively for Nitrokey Pro and Nitrokey Storage products.

#Tests
Warning! Before you run unittests please either change both your Admin and User PINs on your Nitrostick to defaults (12345678 and 123456 respectively) or change the values in tests source code. If you do not change them the tests might lock your device. If its too late, you can always reset your Nitrokey using instructions from [homepage](https://www.nitrokey.com/de/documentation/how-reset-nitrokey).

## Python tests
Libnitrokey has a couple of tests written in Python under the path: `unittest/test_bindings.py`. The tests themselves show how to handle common requests to device.
To run them please enter `unittest` directory and execute `py.test`. For even better coverage [randomly plugin](https://pypi.python.org/pypi/pytest-randomly) could be installed.

## C++ tests
There are also some unit tests implemented in C++:
[test_HOTP.cc](https://github.com/Nitrokey/libnitrokey/blob/master/unittest/test_HOTP.cc)
[test.cc](https://github.com/Nitrokey/libnitrokey/blob/master/unittest/test.cc)
Unit tests was written and tested with Nitrokey Pro on Ubuntu 16.04. To run them just execute binaries build in unittest/build dir after initial make. You will have to add LD_LIBRARY_PATH variable to environment though, like:
```bash
LD_LIBRARY_PATH=. ./test_HOTP
```

The device's commands are here:
[stick10_commands.h](https://github.com/Nitrokey/libnitrokey/blob/hotp_tests/include/stick10_commands.h)
for Nitrokey Pro and
[stick20_commands.h](https://github.com/Nitrokey/libnitrokey/blob/hotp_tests/include/stick20_commands.h)
for Nitrokey Storage [which includes Nitrokey Pro commands set].

The documentation of how it works could be found in nitrokey-app project's README on Github:
[Nitrokey-app - internals](https://github.com/Nitrokey/nitrokey-app/blob/master/README.md#internals) - some typos might be in links there, but one can traverse to destination manually by looking at URL.

To peek/debug communication with device running nitrokey-app in debug mode [-d switch] and checking the logs
[right click on tray icon and then 'Debug'] might be helpful. Also crosschecking with
firmware code should show how things works:
[report_protocol.c](https://github.com/Nitrokey/nitrokey-pro-firmware/blob/master/src/keyboard/report_protocol.c)
[for Nitrokey Pro, for Storage similarly].

# Known issues / tasks
* Currently only one device can be connected at a time
* C++ API needs some reorganization to C++ objects (instead of pointers to arrays). This will be also preparing for integration with Pybind11,
* PIN protected OTP is currently not working,
* Factory reset and generating AES key commands are not yet tested neither covered in unittest,
* The library is not supporting Nitrokey Storage stick but it should be done in nearest future. The only working function for now (looking by Python unit tests) is getting HOTP code.

Other tasks might be listed either in `TODO` file or on project's issues page.
