file(GLOB testSrc *.cpp)

add_executable(testCommuModules ${testSrc})

target_link_libraries(testCommuModules commu utils ssl crypto debugTools ${GTEST_LIBS})

add_test(Name testCommuModules
         COMMAND testCommuModules)
