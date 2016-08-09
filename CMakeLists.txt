cmake_minimum_required(VERSION 3.5)
project(libnitrokey)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

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
        NK_C_API.cc CommandFailedException.cpp include/CommandFailedException.h TooLongStringException.cpp include/TooLongStringException.h include/InvalidSlotException.h include/LibraryException.h)

add_executable(libnitrokey ${SOURCE_FILES})