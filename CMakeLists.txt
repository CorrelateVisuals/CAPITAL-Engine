cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME CapitalEngine)
project(${PROJECT_NAME})
message("==== Projectname: ${PROJECT_NAME}")

set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Wpedantic \
    -Winit-self -Wno-unused -Wshadow \
    -Wno-parentheses -Wlogical-op -Wredundant-decls \
    -Wcast-align -Wsign-promo -Wmissing-include-dirs \
    -Woverloaded-virtual -Wctor-dtor-privacy ") 

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

message("==== Source dir: ${CMAKE_SOURCE_DIR}")
message("====   EXE path: ${EXECUTABLE_OUTPUT_PATH}")

file(
GLOB_RECURSE CAPITALENGINE_SOURCES
RELATIVE "${CMAKE_SOURCE_DIR}"
src/*.cpp
src/*.h
)

message("=== SOURCE dir: ${CMAKE_SOURCE_DIR}  ${CAPITALENGINE_SOURCES}")

# list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# find_package(glfw3)
# target_link_libraries(${PROJECTNAME} glfw3)

link_libraries(glfw)
link_libraries(vulkan)

add_executable(CapitalEngine ${CAPITALENGINE_SOURCES})
# target_link_libraries(CapitalEngine ${Vulkan_LIBRARIES})

