# libnitrokey

Warning! Before you run unittests please either change both your Admin and User PINs on your Nitrostick to defaults (12345678 and 123456 respectively) or change the values in tests source code. If its too late, you can always reset your Nitrokey using instructions from [homepage](
https://www.nitrokey.com/de/documentation/how-reset-nitrokey).

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

Libnitrokey do not have at the moment any documentation. Most helpful for
now would be unittests [currently hotp_tests branch - still in
development, but they should show how to handle operations with Nitrokeys and
they should be frequently updated]:
[test_HOTP.cc](https://github.com/Nitrokey/libnitrokey/blob/hotp_tests/unittest/test_HOTP.cc)
[test.cc](https://github.com/Nitrokey/libnitrokey/blob/hotp_tests/unittest/test.cc)
Unit tests was written with Nitrokey Pro on Ubuntu 15.04. To run them just execute binaries build in unittest/build dir after initial make. You will have to add LD_LIBRARY_PATH variable to environment though, like:
```bash
LD_LIBRARY_PATH=. ./test_HOTP
```

The commands are here:
[stick10_commands.h](https://github.com/Nitrokey/libnitrokey/blob/hotp_tests/include/stick10_commands.h)
for Nitrokey Pro and
[stick20_commands.h](https://github.com/Nitrokey/libnitrokey/blob/hotp_tests/include/stick20_commands.h)
for Nitrokey Storage [which includes Nitrokey Pro commands set].

For the documentation of how it works one could check readme for
nitrokey-app project on Github:
[Nitrokey-app - internals](https://github.com/Nitrokey/nitrokey-app/blob/master/README.md#internals) - some typos might be in links there, but one can traverse to destination manually by looking at url.

One could check which commands are needed for particular operation with
running nitrokey-app in debug mode [-d switch] and checking the logs
[right click on tray icon and then 'Debug']. Also crosschecking with
firmware code should show how things works:
[report_protocol.c](https://github.com/Nitrokey/nitrokey-pro-firmware/blob/master/src/keyboard/report_protocol.c)
[for Nitrokey Pro, for Storage similarly].
