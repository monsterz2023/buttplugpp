cmake_minimum_required(VERSION 3.15)
project(buttplugpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(BUILD_TESTS "Build all tests." OFF) # Turn off by default
# If testing is enabled, include the tests subdirectory
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED)
file(GLOB SOURCES "src/*.cpp")

add_library(${CMAKE_PROJECT_NAME} ${SOURCES})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
add_executable("buttpp_main" ${SOURCES} "examples/main.cpp")
target_link_libraries("buttpp_main" PRIVATE nlohmann_json::nlohmann_json)