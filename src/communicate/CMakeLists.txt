file(GLOB commuSrc *.cpp)

add_library(commu STATIC ${commuSrc})

target_link_libraries(commu utils debugTools)