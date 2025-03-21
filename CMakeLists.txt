cmake_minimum_required(VERSION 3.10)

project(glsl-parser LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_STATIC_LIB "Build as a static library" ON)
option(BUILD_SHARED_LIB "Build as a shared library" OFF)
option(INSTALL_BUILD    "Install the built library" OFF)
if(BUILD_STATIC_LIB)
    set(BUILD_SHARED_LIB OFF)
endif()

if(NOT BUILD_STATIC_LIB AND NOT BUILD_SHARED_LIB)
    set(BUILD_STATIC_LIB ON)
endif()

# Compiler flags
if (MSVC)
    # MSVC-specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /EHs- /W4 /O2")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
else()
    # Non-MSVC flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptoins -Wall -Wextra -Wformat -O3")
endif()

# Source files
set(SOURCES
    ast.cpp
    lexer.cpp
    parser.cpp
    util.cpp
)

# Header files
set(HEADERS
    ast.h
    lexemes.h
    lexer.h
    parser.h
    util.h
)

# Library target

if(BUILD_STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC ${SOURCES})
elseif(BUILD_SHARED_LIB)
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
endif()

# Public include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# MSVC-specific setting
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /permissive- /Zc:preprocessor /W4)
endif()

if(INSTALL_BUILD)
    # Installation rules
    include(GNUInstallDirs)

    # Install the library
    install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # Install the headers
    install(FILES ${HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    )

    # Export the target for use by other projects
    install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE glsl-parser::
        DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}
    )
endif()
