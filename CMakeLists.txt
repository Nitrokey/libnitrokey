cmake_minimum_required(VERSION 3.5)

IF (UNIX)
    OPTION(USE_CLANG "Use CLang" TRUE)
    IF(USE_CLANG)
        set(CMAKE_CXX_COMPILER "/usr/bin/clang++-3.8" CACHE string "clang++ compiler" FORCE)
    ENDIF()
ENDIF()

project(libnitrokey CXX)
set(CMAKE_CXX_STANDARD 14)

#OPTION(COMPILE_TESTS "Compile tests" FALSE)
OPTION(COMPILE_TESTS "Compile tests" TRUE)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

include_directories(include)
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
    include/CommandFailedException.h
    include/LibraryException.h
    include/LongOperationInProgressException.h
    include/stick10_commands_0.8.h
    command_id.cc
    device.cc
    log.cc
    misc.cc
    NitrokeyManager.cc
    NK_C_API.h
    NK_C_API.cc
)


add_library(nitrokey SHARED ${SOURCE_FILES})
target_link_libraries(nitrokey hidapi-libusb)

IF (COMPILE_TESTS)
    include_directories(unittest/Catch/include)

    add_library(catch SHARED unittest/catch_main.cpp )

    add_executable (test_C_API unittest/test_C_API.cpp)
    target_link_libraries (test_C_API PUBLIC nitrokey catch)

    add_executable (test2 unittest/test2.cc)
    target_link_libraries (test2 PUBLIC nitrokey catch)

    add_executable (test3 unittest/test3.cc)
    target_link_libraries (test3 PUBLIC nitrokey catch)

    add_executable (test_HOTP unittest/test_HOTP.cc)
    target_link_libraries (test_HOTP PUBLIC nitrokey catch)

    add_executable (test1 unittest/test.cc)
    target_link_libraries (test1 PUBLIC nitrokey catch)

    #run with 'make test' or 'ctest'
    #needs connected PRO device for success
    #warning: it may delete data on the device
    include (CTest)
    add_test (runs test_C_API)
ENDIF()

