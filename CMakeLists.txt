# https://cmake.org/pipermail/cmake/2011-May/044166.html
IF(NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
    SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
ENDIF()

cmake_minimum_required(VERSION 3.1)
IF (UNIX)
    OPTION(ADD_ASAN "Use ASAN to show memory issues" FALSE)
    OPTION(ADD_TSAN "Use TSAN to show thread issues" FALSE)
    IF(ADD_ASAN)
        SET(EXTRA_LIBS ${EXTRA_LIBS} asan )
        ADD_COMPILE_OPTIONS(-fsanitize=address -fno-omit-frame-pointer)
    ENDIF()
    IF(ADD_TSAN)
        SET(EXTRA_LIBS ${EXTRA_LIBS} tsan )
        SET(USE_CLANG TRUE)
        ADD_COMPILE_OPTIONS(-fsanitize=thread -fno-omit-frame-pointer -fPIC -g) #use with clang
    ENDIF()
    IF(ADD_TSAN AND ADD_ASAN)
        message(FATAL_ERROR "TSAN and ASAN cannot be used at the same time")
    ENDIF()
ENDIF()

project(libnitrokey LANGUAGES C CXX VERSION 3.5.0)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


include(GNUInstallDirs)



IF (NOT CMAKE_BUILD_TYPE)
    IF(APPLE)
        # Issues occur when build with enabled optimizations
        set(CMAKE_BUILD_TYPE Debug)
    ELSE()
        set(CMAKE_BUILD_TYPE RelWithDebInfo)
    ENDIF()
ENDIF()
MESSAGE("${PROJECT_NAME}: Build type: ${CMAKE_BUILD_TYPE}")

include_directories(hidapi)
include_directories(libnitrokey)
set(SOURCE_FILES
    libnitrokey/command.h
    libnitrokey/command_id.h
    libnitrokey/cxx_semantics.h
    libnitrokey/device.h
    libnitrokey/device_proto.h
    libnitrokey/dissect.h
    libnitrokey/log.h
    libnitrokey/misc.h
    libnitrokey/NitrokeyManager.h
    libnitrokey/stick10_commands.h
    libnitrokey/stick20_commands.h
    libnitrokey/CommandFailedException.h
    libnitrokey/LibraryException.h
    libnitrokey/LongOperationInProgressException.h
    libnitrokey/stick10_commands_0.8.h
    command_id.cc
    device.cc
    log.cc
    misc.cc
    NitrokeyManager.cc
    NK_C_API.h
    NK_C_API.cc
    DeviceCommunicationExceptions.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/version.cc
    )

set(BUILD_SHARED_LIBS ON CACHE BOOL "Build all libraries as shared")
add_library(nitrokey ${SOURCE_FILES})

IF(APPLE)
    include_directories(hidapi/hidapi)
    add_library(hidapi-libusb STATIC hidapi/mac/hid.c )
    target_link_libraries(hidapi-libusb "-framework CoreFoundation" "-framework IOKit")
    target_link_libraries(nitrokey hidapi-libusb)
ELSEIF(UNIX)
#    add_library(hidapi-libusb STATIC hidapi/libusb/hid.c )
    find_package(PkgConfig)
    pkg_search_module(HIDAPI_LIBUSB REQUIRED hidapi-libusb)
    target_compile_options(nitrokey PRIVATE ${HIDAPI_LIBUSB_CFLAGS})
    target_link_libraries(nitrokey ${HIDAPI_LIBUSB_LDFLAGS})
ELSEIF(WIN32)
    include_directories(hidapi/hidapi)
    add_library(hidapi-libusb STATIC hidapi/windows/hid.c )
    target_link_libraries(hidapi-libusb setupapi)
    target_link_libraries(nitrokey hidapi-libusb)
ENDIF()

set_target_properties(nitrokey PROPERTIES
	VERSION ${libnitrokey_VERSION}
	SOVERSION ${libnitrokey_VERSION_MAJOR})

OPTION(ERROR_ON_WARNING "Stop compilation on warning found (not supported for MSVC)" OFF)
if (NOT MSVC)
    set(COMPILE_FLAGS "-Wall -Wno-unused-function -Wcast-qual -Woverloaded-virtual -Wsign-compare -Wformat -Wformat-security")
    IF(NOT APPLE)
        if (ERROR_ON_WARNING)
            set(COMPILE_FLAGS "${COMPILE_FLAGS} -Werror")
        endif()
    ENDIF()
    SET_TARGET_PROPERTIES(nitrokey PROPERTIES COMPILE_FLAGS ${COMPILE_FLAGS} )
endif()

OPTION(NO_LOG "Compile without logging functionality and its strings (decreases size)" OFF)
IF (NO_LOG)
    SET_TARGET_PROPERTIES(nitrokey PROPERTIES COMPILE_DEFINITIONS "NO_LOG")
ENDIF()

OPTION(LOG_VOLATILE_DATA "Log volatile data (debug)" OFF)
IF (LOG_VOLATILE_DATA)
    SET_TARGET_PROPERTIES(nitrokey PROPERTIES COMPILE_DEFINITIONS "LOG_VOLATILE_DATA")
ENDIF()


OPTION(ADD_GIT_INFO "Add information about source code version from Git repository" TRUE)
# generate version.h
IF(ADD_GIT_INFO)
execute_process(
	COMMAND git describe --always --abbrev=4 HEAD
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	RESULT_VARIABLE PROJECT_VERSION_GIT_RETURN_CODE
	OUTPUT_VARIABLE PROJECT_VERSION_GIT
	OUTPUT_STRIP_TRAILING_WHITESPACE
	ERROR_QUIET
)
ENDIF()
IF((NOT ${ADD_GIT_INFO}) OR (${PROJECT_VERSION_GIT_RETURN_CODE}))
	SET(PROJECT_VERSION_GIT "v${PROJECT_VERSION}")
ENDIF()
MESSAGE(STATUS "Setting Git library version to: " ${PROJECT_VERSION_GIT} )
configure_file("version.cc.in" "version.cc" @ONLY)


file(GLOB LIB_INCLUDES "libnitrokey/*.h" "NK_C_API.h")
install (FILES ${LIB_INCLUDES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
install (TARGETS nitrokey DESTINATION ${CMAKE_INSTALL_LIBDIR})

IF(NOT WIN32)
    # Install Nitrokey udev rules
    IF(NOT DEFINED CMAKE_INSTALL_UDEVRULESDIR)
        set(PKG_GET_UDEV_DIR ${PKG_CONFIG_EXECUTABLE} --variable=udevdir udev)
        execute_process(COMMAND ${PKG_GET_UDEV_DIR} RESULT_VARIABLE ERR OUTPUT_VARIABLE CMAKE_INSTALL_UDEVRULESDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
        IF(${ERR})
            set(CMAKE_INSTALL_UDEVRULESDIR "lib/udev/rules.d")
        ELSE()
            set(CMAKE_INSTALL_UDEVRULESDIR "${CMAKE_INSTALL_UDEVRULESDIR}/rules.d")
        ENDIF()
        string(REGEX REPLACE "^/" "" CMAKE_INSTALL_UDEVRULESDIR "${CMAKE_INSTALL_UDEVRULESDIR}")
        string(REGEX REPLACE "^usr/" "" CMAKE_INSTALL_UDEVRULESDIR "${CMAKE_INSTALL_UDEVRULESDIR}") # usual prefix is usr/local
        message(STATUS "Setting udev rules dir to ${CMAKE_INSTALL_UDEVRULESDIR}")
    ENDIF()

    install(FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/data/41-nitrokey.rules
            DESTINATION ${CMAKE_INSTALL_UDEVRULESDIR}
    )
ENDIF()

# configure and install pkg-config file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libnitrokey.pc.in ${CMAKE_CURRENT_BINARY_DIR}/libnitrokey-1.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libnitrokey-1.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

OPTION(COMPILE_TESTS "Compile tests" FALSE)
OPTION(COMPILE_OFFLINE_TESTS "Compile offline tests" FALSE)

IF(COMPILE_OFFLINE_TESTS OR COMPILE_TESTS)
    find_package(PkgConfig)
    IF(PKG_CONFIG_FOUND)
        pkg_check_modules(CATCH2 catch2)
    ENDIF()

    if (CATCH2_FOUND)
        message(STATUS "Found system Catch2, not using bundled version")
        add_compile_options(${CATCH2_CFLAGS})
    ELSE()
        message(STATUS "Did NOT find system Catch2, instead using bundled version")
        include_directories(unittest/Catch/single_include)
    ENDIF()

    add_library(catch STATIC unittest/catch_main.cpp )
ENDIF()

IF(COMPILE_OFFLINE_TESTS)
    add_executable (test_offline unittest/test_offline.cc)
    target_link_libraries (test_offline ${EXTRA_LIBS} nitrokey catch)
    SET_TARGET_PROPERTIES(test_offline PROPERTIES COMPILE_FLAGS ${COMPILE_FLAGS} )
    #run with 'make test' or 'ctest'
    include (CTest)
    add_test (runs test_offline)

    add_executable(test_minimal unittest/test_minimal.c)
    target_link_libraries(test_minimal ${EXTRA_LIBS} nitrokey)
    add_test(minimal test_minimal)
ENDIF()

IF (COMPILE_TESTS)
    #needs connected Pro/Storage devices for success
    #WARNING: it may delete data on the device

        SET(TESTS
                unittest/test_C_API.cpp
                unittest/test2.cc
                unittest/test3.cc
                unittest/test_HOTP.cc
                unittest/test1.cc
                unittest/test_issues.cc
                unittest/test_multiple_devices.cc
                unittest/test_strdup.cpp
                unittest/test_safe.cpp
        )

    foreach(testsourcefile ${TESTS} )
        get_filename_component(testname ${testsourcefile} NAME_WE )
        add_executable(${testname} ${testsourcefile} )
        target_link_libraries(${testname} ${EXTRA_LIBS} nitrokey catch )
        SET_TARGET_PROPERTIES(${testname} PROPERTIES COMPILE_FLAGS ${COMPILE_FLAGS} )
    endforeach(testsourcefile)

ENDIF()



#SET(CPACK_GENERATOR
#        "DEB;RPM")
# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
        "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
include (CPack)

# Build Doxygen documentation for the C API
find_package(Doxygen)
if (DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc ${DOXYGEN_EXECUTABLE} Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating C API documentation with Doxygen" VERBATIM)
endif(DOXYGEN_FOUND)
