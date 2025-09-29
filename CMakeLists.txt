cmake_minimum_required(VERSION 3.21)

project(
        iris
        DESCRIPTION "Intermediate Representation Internal Structure "
        LANGUAGES CXX
        )

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
  message(STATUS "CMAKE_BUILD_TYPE is not specified, using Release by default")
endif()

# set(DEBUG_CXX_FLAGS " -O0 -g -Wpedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wswitch-default -Wundef -Wunused -Wshadow")

string(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra")
string(APPEND CMAKE_CXX_FLAGS_DEBUG ${DEBUG_CXX_FLAGS})
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O2")

set(LIB_SRC_DIR lib)
set(LIB_INC_DIR include)

file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS ${LIB_SRC_DIR}/*.cpp)

add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${LIB_INC_DIR})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

enable_testing()
add_subdirectory(tests)

add_subdirectory(examples)

find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        *.cpp
        *.hpp
    )
    
    add_custom_target(
        format
        COMMAND ${CLANG_FORMAT} -style=file -i ${ALL_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()
