cmake_minimum_required(VERSION 3.0.0)
add_compile_options(-std=c++17)
project(TestModule)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

add_executable(TestModule main.cpp automat.h client.h client.cpp service.h service.cpp)
include_directories(/lib /include)
target_link_libraries(TestModule libpaho-mqtt3a.so)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

