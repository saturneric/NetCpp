file(GLOB utilSrc *.cpp)

add_library(utils STATIC ${utilSrc})

target_link_libraries(utils debugTools ssl crypto)