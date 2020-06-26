cmake_minimum_required(VERSION 3.15)
project(Net)

set(CMAKE_CXX_STANDARD 14)

include_directories(include/)
include_directories(utils/)

find_package(Boost   REQUIRED)
find_package(OpenSSL REQUIRED)

message(STATUS SSL ${OPENSSL_LIBRARIES})

include(GoogleTest)

set(OPENSSL_LIBS /usr/local/lib/libssl.dylib /usr/local/lib/libcrypto.dylib)
set(GTEST_LIB /usr/local/lib/)
set(GTEST_LIBS gtest pthread dl)

include_directories(${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${SQLiteCpp_INCLUDE_DIRS})
link_directories(${GTEST_LIB})

aux_source_directory(src SOURCE_ALL)

add_executable(NetTest test/test_main.cpp)

gtest_add_tests(TARGET         NetTest
                TEST_SUFFIX    .noArgs
                TEST_LIST      noArgsTests)

add_library(m_error STATIC src/error.cpp)
add_library(m_rsa STATIC src/rsa_cpp_binding.cpp)

target_link_libraries(NetTest m_rsa  m_error ${GTEST_LIBS} ssl crypto boost_program_options)
set_tests_properties(${noArgsTests}   PROPERTIES TIMEOUT 10)