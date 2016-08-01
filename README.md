# libnitrokey
Libnitrokey is a project to communicate with Nitrokey stick devices in clean and easy manner. Written in C++14, testable with `Catch` framework, with C API, Python access (through CFFI and C API, in future with Pybind11).
The development of this project is aimed to make it itself a living documentation of communication protocol between host and the Nitrokey stick device.

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
## Documentation
There is no documentation yet, but it is meant to be included in the sources (and generated with doxygen like application if requested).
Please check NK_C_API.h for high level commands and include/NitrokeyManager.h for lower level. All devices' commands are listed along with packet format in include/stick10_commands.h and include/stick20_commands.h respectively for Nitrokey Pro and Nitrokey Storage products.

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
