cmake_minimum_required (VERSION 3.10.2)
project(cc)
set(CMAKE_CXX_STANDARD 11)
if(UNIX)
    set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -ggdb -O2 -Wl,-rpath=./:./lib")
endif()
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)
link_directories(${PROJECT_SOURCE_DIR}/lib)
add_executable(${PROJECT_NAME} src/main.cpp)
include_directories(${PROJECT_SOURCE_DIR}/include)    
target_link_libraries(${PROJECT_NAME} 
    opencc
    uchardet
    iconv
    yaml-cpp)

if(WIN32)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION windows)
elseif(UNIX)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION linux)
endif()
