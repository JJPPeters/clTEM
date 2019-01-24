#
# CMake module for Easylogging++ logging library
#
# Defines ${EASYLOGGINGPP_INCLUDE_DIR}
#
# If ${EASYLOGGINGPP_USE_STATIC_LIBS} is ON then static libs are searched.
# In these cases ${EASYLOGGINGPP_LIBRARY} is also defined
#
# (c) 2017-2018 Zuhd Web Services
#
# https://github.com/zuhd-org/easyloggingpp
# https://zuhd.org
# https://muflihun.com
#

message ("-- Easylogging++: Searching...")
set(EASYLOGGINGPP_PATHS ${EASYLOGGINGPP_ROOT} $ENV{EASYLOGGINGPP_ROOT})

find_path(EASYLOGGINGPP_INCLUDE_DIR
        easylogging++.h
        PATH_SUFFIXES include
        PATHS ${EASYLOGGINGPP_PATHS}
)

if (EASYLOGGINGPP_USE_STATIC_LIBS)
    message ("-- Easylogging++: Static linking is preferred")
    if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
        find_library(EASYLOGGINGPP_LIBRARY
                NAMES libeasyloggingppd.a libeasyloggingppd.dylib libeasyloggingppd
                HINTS "${CMAKE_PREFIX_PATH}/lib"
                )
    else ()
        find_library(EASYLOGGINGPP_LIBRARY
                NAMES libeasyloggingpp.a libeasyloggingpp.dylib libeasyloggingpp
                HINTS "${CMAKE_PREFIX_PATH}/lib"
                )
    endif ()
elseif (EASYLOGGINGPP_USE_SHARED_LIBS)
    message ("-- Easylogging++: Dynamic linking is preferred")
    if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
        find_library(EASYLOGGINGPP_LIBRARY
            NAMES libeasyloggingppd.dylib libeasyloggingppd libeasyloggingppd.a
            HINTS "${CMAKE_PREFIX_PATH}/lib"
        )
    else ()
        find_library(EASYLOGGINGPP_LIBRARY
                NAMES libeasyloggingpp.dylib libeasyloggingpp libeasyloggingpp.a
                HINTS "${CMAKE_PREFIX_PATH}/lib"
                )
    endif ()
endif()

find_package_handle_standard_args(EASYLOGGINGPP REQUIRED_VARS EASYLOGGINGPP_INCLUDE_DIR)
