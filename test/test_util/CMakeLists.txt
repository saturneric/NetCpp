
add_executable(testUtils main.cpp)

message(${GTEST_LIBS})

link_libraries(testUtils utils debugTools ${GTEST_LIBS})

add_test(Name testUtils
         COMMAND testUtils)