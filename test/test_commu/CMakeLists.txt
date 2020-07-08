
file(GLOB testSrc *.cpp)

add_compile_definitions(DEBUG)

add_executable(testCommu ${testSrc})

target_link_libraries(testCommu commu debugTools Boost::thread)

