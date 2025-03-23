cmake_minimum_required(VERSION 3.10)

project(glsl-parser LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_LIBRARY_STATIC "Build as a static library"                       ON)
option(BUILD_LIBRARY_SHARED "Build as a shared library"                       OFF)
option(BUILD_EXECUTABLE     "Build the executable"                            ON)
option(BUILD_STRIP_TARGETS  "Strip both library and executable (if possible)" OFF)

if(BUILD_LIBRARY_STATIC AND BUILD_LIBRARY_SHARED)
    set(BUILD_LIBRARY_SHARED ON)
    set(BUILD_LIBRARY_STATIC OFF)
    message(STATUS "Both BUILD_LIBRARY_STATIC and BUILD_LIBRARY_SHARED was set.")
    message(STATUS "Prioritizing BUILD_LIBRARY_SHARED")
endif()

if(BUILD_EXECUTABLE)
    message(STATUS "Since BUILD_EXECUTABLE was set, BUILD_INSTALL was automatically set.")
    if (BUILD_LIBRARY_SHARED)
        message(STATUS "Building executable and linking to shared library is not adviced.")
    endif()
endif()

if(NOT BUILD_LIBRARY_STATIC AND NOT BUILD_LIBRARY_SHARED)
    set(BUILD_LIBRARY_STATIC ON)
    message(STATUS "Both BUILD_LIBRARY_STATIC and BUILD_LIBRARY_SHARED were off.")
    message(STATUS "Defaulting to BUILD_LIBRARY_STATIC")
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
    set(BUILD_STRIP_TARGETS)
    message(STATUS "Since CMAKE_BUILD_TYPE is Release.")
    message(STATUS "We are stripping both library and executable.")

    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    endif()
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /EHs- /W4")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(COMPILER_SPECIFIC_OPTIONS /permissive- /Zc:preprocessor /W4)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -Wall -Wextra -Wformat")
endif()

set(LIB_SOURCES
    library/src/ast.cpp
    library/src/converter.cpp
    library/src/lexer.cpp
    library/src/parser.cpp
    library/src/util.cpp
)

set(LIB_HEADERS
    library/include/glsl-parser/ast.h
    library/include/glsl-parser/converter.h
    library/include/glsl-parser/lexemes.h
    library/include/glsl-parser/lexer.h
    library/include/glsl-parser/parser.h
    library/include/glsl-parser/util.h
)

set(EXE_SOURCES
    executable/src/main.cpp
)

if(BUILD_LIBRARY_SHARED)
    add_library(${PROJECT_NAME} SHARED ${LIB_SOURCES} ${LIB_HEADERS})
    add_library(${PROJECT_NAME}-shared ALIAS ${PROJECT_NAME})
else()
    add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES} ${LIB_HEADERS})
    add_library(${PROJECT_NAME}-static ALIAS ${PROJECT_NAME})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC library/include)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if(BUILD_EXECUTABLE)
    add_executable(${PROJECT_NAME}-exe ${EXE_SOURCES})
    target_link_libraries(${PROJECT_NAME}-exe PRIVATE ${PROJECT_NAME}::${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME}-exe PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

    if(BUILD_LIBRARY_SHARED)
        set_target_properties(${PROJECT_NAME}-exe PROPERTIES
            INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
            BUILD_WITH_INSTALL_RPATH TRUE
        )
    endif()
endif()

add_custom_target(test
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.py
    DEPENDS ${PROJECT_NAME}-exe
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running tests..."
)

if(BUILD_STRIP_TARGETS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${PROJECT_NAME}>
        COMMENT "Stripping symbols from ${PROJECT_NAME}"
    )

    if(BUILD_EXECUTABLE)
        add_custom_command(TARGET ${PROJECT_NAME}-exe POST_BUILD
            COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${PROJECT_NAME}-exe>
            COMMENT "Stripping symbols from ${PROJECT_NAME}-exe"
        )
    endif()
endif()

include(GNUInstallDirs)

install(FILES ${LIB_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/glsl-parser
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(BUILD_EXECUTABLE)
    install(TARGETS ${PROJECT_NAME}-exe
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

message(STATUS "Build Configuration:")
message(STATUS "  BUILD_LIBRARY_STATIC: ${BUILD_LIBRARY_STATIC}")
message(STATUS "  BUILD_LIBRARY_SHARED: ${BUILD_LIBRARY_SHARED}")
message(STATUS "  BUILD_EXECUTABLE: ${BUILD_EXECUTABLE}")
message(STATUS "  BUILD_STRIP_TARGETS: ${BUILD_STRIP_TARGETS}")
message(STATUS "  BUILD_INSTALL: ${BUILD_INSTALL}")