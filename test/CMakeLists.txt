find_package(GTest REQUIRED)

set(CMAKE_CXX_FLAGS_DEBUG "-g")

set(GTEST_LIB /usr/local/lib/)
set(GTEST_LIBS gtest gmock pthread dl)

link_directories(${GTEST_LIB})

add_subdirectory(test_util)
add_subdirectory(test_commu)
add_subdirectory(test_commu_modules)
