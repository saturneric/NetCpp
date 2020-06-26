
add_executable(testUtils main.cpp test_rsa.cpp)

message(${GTEST_LIBS})

target_link_libraries(testUtils utils debugTools ${GTEST_LIBS})

add_test(Name testUtils
         COMMAND testUtils)