cmake_minimum_required(VERSION 3.10)

project(glsl-parser LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_STATIC_LIB "Build as a static library" ON)
option(BUILD_SHARED_LIB "Build as a shared library" OFF)
option(BUILD_EXECUTABLE "Build the executable" ON)
option(INSTALL_BUILD "Install the built library and executable" OFF)

if(BUILD_STATIC_LIB AND BUILD_SHARED_LIB)
    set(BUILD_SHARED_LIB OFF)
endif()

if(BUILD_EXECUTABLE)
    set(INSTALL_BUILD ON)
    set(BUILD_STATIC_LIB ON)
endif()

if(NOT BUILD_STATIC_LIB AND NOT BUILD_SHARED_LIB)
    set(BUILD_STATIC_LIB ON)
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /EHs- /W4 /O2")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(COMPILER_SPECIFIC_OPTIONS /permissive- /Zc:preprocessor /W4)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -Wall -Wextra -Wformat -O3")
    set(COMPILER_SPECIFIC_OPTIONS "")
endif()

set(LIB_SOURCES
    library/src/ast.cpp
    library/src/lexer.cpp
    library/src/parser.cpp
    library/src/util.cpp
)

set(LIB_HEADERS
    library/include/glsl-parser/ast.h
    library/include/glsl-parser/lexemes.h
    library/include/glsl-parser/lexer.h
    library/include/glsl-parser/parser.h
    library/include/glsl-parser/util.h
)

set(EXE_SOURCES
    executable/src/main.cpp
)

set(LIB_NAME ${PROJECT_NAME}-static)

if(BUILD_SHARED_LIB)
    set(LIB_NAME ${PROJECT_NAME}-shared)
    add_library(${LIB_NAME} SHARED ${LIB_SOURCES} ${LIB_HEADERS})
else()
    add_library(${LIB_NAME} STATIC ${LIB_SOURCES} ${LIB_HEADERS})
endif()

target_include_directories(${LIB_NAME} PUBLIC library/include)

if(BUILD_EXECUTABLE)
    add_executable(${PROJECT_NAME}-exe ${EXE_SOURCES})
    target_link_libraries(${PROJECT_NAME}-exe PRIVATE ${LIB_NAME})
endif()

add_custom_target(test
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.py
    DEPENDS ${PROJECT_NAME}-exe
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running tests..."
)
