
file(GLOB testSrc *.cpp)

add_executable(testUtils ${testSrc})

message(${GTEST_LIBS})

target_link_libraries(testUtils utils debugTools ssl crypto ${GTEST_LIBS})

add_test(Name testUtils
         COMMAND testUtils)