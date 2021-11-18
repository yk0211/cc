cmake_minimum_required (VERSION 3.10.2)
project(cc)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)

add_executable(${PROJECT_NAME} src/main.cpp)
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)
link_directories(
    ${PROJECT_SOURCE_DIR}/lib/opencc
)
target_link_libraries(${PROJECT_NAME} opencc)