cmake_minimum_required(VERSION 3.15)
project(Net)

set(CMAKE_CXX_STANDARD 14)

include_directories(include/)
include_directories(utils/)

find_package(sqlite3         REQUIRED)
find_package(Boost           COMPONENTS
             program_options REQUIRED)
find_package(SQLiteCpp       REQUIRED)
find_package(gtest           REQUIRED)
find_package(OpenSSL REQUIRED)

message(STATUS SSL ${OPENSSL_LIBRARIES})

include(GoogleTest)

set(OPENSSL_LIBS /usr/local/lib/libssl.dylib /usr/local/lib/libcrypto.dylib)
set(GTEST_LIB /usr/local/lib/)
set(GTEST_LIBS gtest pthread dl)

include_directories(${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${SQLiteCpp_INCLUDE_DIRS})
link_directories(${GTEST_LIB})

aux_source_directory(src SOURCE_ALL)


# add_executable(Net ${SOURCE} src/rsacpp.cpp include/rsacpp.h src/init.cpp include/init.h)
add_executable(NetRSATest test/rsa_test.cpp)

gtest_add_tests(TARGET         NetRSATest
                TEST_SUFFIX    .noArgs
                TEST_LIST      noArgsTests)

add_executable(NetOptsPceTest test/opts_processor_test.cpp)

gtest_add_tests(TARGET         NetOptsPceTest
        TEST_SUFFIX    .noArgs
        TEST_LIST      noArgsTests)

add_library(m_error STATIC src/error.cpp)
add_library(m_rsa STATIC src/rsa_cpp_binding.cpp)
add_library(test_main STATIC test/test_main.cpp)



target_link_libraries(NetRSATest test_main m_rsa m_error ${GTEST_LIBS} ${OPENSSL_LIBS})
target_link_libraries(NetOptsPceTest test_main m_error Boost::program_options)
set_tests_properties(${noArgsTests}   PROPERTIES TIMEOUT 10)
# target_link_libraries(Net SQLiteCpp sqlite3 gtest ${Boost_LIBRARIES}  pthread dl ssl )