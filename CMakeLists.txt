cmake_minimum_required(VERSION 3.13)
project(Net)

set(CMAKE_CXX_STANDARD 14)

include_directories(include/)
include_directories(utils/)



IF (WIN32)

ELSEIF (APPLE)
    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl@1.1)
    set(OPENSSL_LIBRARIES /usr/local/opt/openssl@1.1/lib)
ELSEIF (UNIX)

ENDIF ()

find_package(Boost   REQUIRED thread)
find_package(OpenSSL 1.1.1 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${SQLiteCpp_INCLUDE_DIRS})

enable_testing()

add_subdirectory(src)

add_subdirectory(test)