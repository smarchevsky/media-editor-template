cmake_minimum_required(VERSION 3.5)

project(media-editor-template LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DPROJECT_DIR="${PROJECT_SOURCE_DIR}")
add_definitions(-DRESOURCE_DIR="${PROJECT_SOURCE_DIR}/resources")

string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

add_definitions(-DHOME_DIR="$ENV{HOME}")

include_directories(src/)
include_directories(3party/)

FILE(GLOB_RECURSE ALL_HEADERS ${CMAKE_SOURCE_DIR}/*.h)
FILE(GLOB_RECURSE ALL_CPP  "src/*.cpp" "3party/*.cpp" "3party/*.c")
# set_source_files_properties("3party/miniz.c" PROPERTIES LANGUAGE CXX )

FILE(GLOB_RECURSE ALL_SHADERS "resources/shaders/*.frag")
FILE(GLOB_RECURSE ALL_SHADERS "resources/shaders/*.vert")

# add_library(${PROJECT_NAME} ${ALL_CPP} ${ALL_SHADERS} ${ALL_HEADERS})
add_executable(${PROJECT_NAME} ${ALL_CPP} ${ALL_SHADERS} ${ALL_HEADERS} "main.cpp")

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
#  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC GL SDL2)


# Hints for CMake noobs like me.
# this is your child project, the parent project is your "main"
# where you try to attach this lib

# CMake in your root project:

# set(MET "3party/media-editor-template")   # something like set variable, to get it write ${MET}
# add_subdirectory(${MET})

# set(SOURCE_FILES main.cpp)
# add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# include_directories(${MET}/src)         # to find #include "..." do this
# target_link_libraries(${PROJECT_NAME} media-editor-template)  # undefined reference without this
