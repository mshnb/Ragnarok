cmake_minimum_required(VERSION 3.5)

project(Ragnarok)

include_directories(include/)
link_directories(${CMAKE_SOURCE_DIR}/lib)

file(GLOB PROJECT_HEADERS include/*.h)
file(GLOB PROJECT_SOURCES src/*.cpp src/*.c src/*.cc)

file(GLOB PROJECT_CONFIGS CMakeLists.txt
                          Readme.md
                          .gitignore)

source_group("include" FILES ${PROJECT_HEADERS})
source_group("src" FILES ${PROJECT_SOURCES})

add_executable(${PROJECT_NAME} ${PROJECT_HEADERS} ${PROJECT_SOURCES} ${PROJECT_CONFIGS})

if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()

find_package (OpenMP)

if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
endif()