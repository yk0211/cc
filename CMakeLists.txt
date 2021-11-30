cmake_minimum_required (VERSION 3.10.2)
project(cc)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/bin")

if(WIN32)
elseif(UNIX)
    set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wl,-rpath=./:./lib")
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -ggdb -O0 -Wl,-rpath=./:./lib")
endif()

link_directories(
    "${PROJECT_SOURCE_DIR}/lib"
)

add_executable(${PROJECT_NAME} "src/main.cpp")

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
)
target_link_libraries(${PROJECT_NAME} 
    opencc
    uchardet
    iconv
    yaml-cpp
)