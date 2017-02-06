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
        include/LongOperationInProgressException.h
        include/stick10_commands_0.8.h
        )

#add_library(libnitrokey STATIC ${SOURCE_FILES})
add_library(nitrokey SHARED ${SOURCE_FILES})
add_library(catch STATIC unittest/catch_main.cpp )

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
