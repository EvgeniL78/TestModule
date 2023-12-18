cmake_minimum_required(VERSION 3.0.0)

project(TestModule CXX)

set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)
include_directories(/lib /include)

add_executable(TestModule main.cpp automat.h client.h client.cpp service.h service.cpp)
target_link_libraries(TestModule libpaho-mqtt3a.so)
add_executable(TestModule_tester main_tester.cpp client.h client.cpp service.h service.cpp)
target_link_libraries(TestModule_tester libpaho-mqtt3a.so)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

