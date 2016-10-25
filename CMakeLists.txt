cmake_minimum_required(VERSION 3.5)
project(libnitrokey)

set(CMAKE_CXX_COMPILER "/usr/bin/clang++-3.8" CACHE string "clang++ compiler" FORCE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC -Wno-gnu-variable-sized-type-not-at-end -g3" )
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -lhidapi-libusb" )

include_directories(include unittest/Catch/include)

set(SOURCE_FILES
    include/command.h
    include/command_id.h
    include/cxx_semantics.h
    include/device.h
    include/device_proto.h
    include/dissect.h
    include/inttypes.h
    include/log.h
    include/misc.h
    include/NitrokeyManager.h
    include/stick10_commands.h
    include/stick20_commands.h
        NK_C_API.h
    command_id.cc
    device.cc
    log.cc
    misc.cc
    NitrokeyManager.cc
        NK_C_API.cc include/CommandFailedException.h include/LibraryException.h
        unittest/test_C_API.cpp
        unittest/catch_main.cpp
        unittest/test2.cc
        include/LongOperationInProgressException.h
        )

add_executable(libnitrokey ${SOURCE_FILES})