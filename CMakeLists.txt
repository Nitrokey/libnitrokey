cmake_minimum_required(VERSION 3.5)

IF (UNIX)
    OPTION(ADD_ASAN "Use ASAN to show memory issues" FALSE)
    OPTION(USE_CLANG "Use CLang" FALSE)
    IF(USE_CLANG)
        set(CMAKE_CXX_COMPILER "/usr/bin/clang++-3.8" CACHE string "clang++ compiler" FORCE)
    ELSE()
        set(CMAKE_CXX_COMPILER)
    ENDIF()
    IF(ADD_ASAN)
        SET(EXTRA_LIBS ${EXTRA_LIBS} asan )
        ADD_DEFINITIONS(-fsanitize=address -fno-omit-frame-pointer)
        #    ADD_DEFINITIONS(-fsanitize=thread -fno-omit-frame-pointer -fPIE -pie -g)
    ENDIF()
ENDIF()

project(libnitrokey CXX)
set(CMAKE_CXX_STANDARD 14)

OPTION(COMPILE_TESTS "Compile tests" FALSE)
IF (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
ENDIF()
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

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
    target_link_libraries (test_C_API ${EXTRA_LIBS} nitrokey catch)

    add_executable (test2 unittest/test2.cc)
    target_link_libraries (test2 ${EXTRA_LIBS} nitrokey catch)

    add_executable (test3 unittest/test3.cc)
    target_link_libraries (test3 ${EXTRA_LIBS} nitrokey catch)

    add_executable (test_HOTP unittest/test_HOTP.cc)
    target_link_libraries (test_HOTP ${EXTRA_LIBS} nitrokey catch)

    add_executable (test1 unittest/test.cc)
    target_link_libraries (test1 ${EXTRA_LIBS} nitrokey catch)

    #run with 'make test' or 'ctest'
    #needs connected PRO device for success
    #warning: it may delete data on the device
    include (CTest)
    add_test (runs test_C_API)
ENDIF()

