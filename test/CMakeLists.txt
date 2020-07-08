find_package(GTest REQUIRED)

set(GTEST_LIB /usr/local/lib/)
set(GTEST_LIBS gtest gmock pthread dl)

link_directories(${GTEST_LIB})

add_subdirectory(test_util)
add_subdirectory(test_commu)
