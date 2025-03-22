cmake_minimum_required(VERSION 3.10)

project(glsl-parser LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_LIBRARY_STATIC "Build as a static library"                       ON)
option(BUILD_LIBRARY_SHARED "Build as a shared library"                       OFF)
option(BUILD_EXECUTABLE     "Build the executable"                            ON)
option(BUILD_STRIP_TARGETS  "Strip both library and executable (if possible)" OFF)
option(BUILD_INSTALL        "Install the built library and executable"        OFF)

if(BUILD_LIBRARY_STATIC AND BUILD_LIBRARY_SHARED)
    set(BUILD_LIBRARY_SHARED ON)
    set(BUILD_LIBRARY_STATIC OFF)
    message("Both BUILD_LIBRARY_STATIC and BUILD_LIBRARY_SHARED was set.")
    message("Prioritizing BUILD_LIBRARY_SHARED")
endif()

if(BUILD_EXECUTABLE)
    message("Since BUILD_EXECUTABLE was set, BUILD_INSTALL was automatically set.")
    set(BUILD_INSTALL ON) # might not allow the user to turn off installing
    if (BUILD_LIBRARY_SHARED)
        message("Building executable and linking to shared library is not adviced.")
    endif()
endif()

if(NOT BUILD_LIBRARY_STATIC AND NOT BUILD_LIBRARY_SHARED)
    set(BUILD_LIBRARY_STATIC ON)
    message("Both BUILD_LIBRARY_STATIC and BUILD_LIBRARY_SHARED were off.")
    message("Defaulting to BUILD_LIBRARY_STATIC")
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
    set(BUILD_STRIP_TARGETS)
    message("Since CMAKE_BUILD_TYPE is Release.")
    message("We are stripping both library and executable.")
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

if(BUILD_INSTALL)
    include(GNUInstallDirs)

    install(FILES)
    install(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )

    if(BUILD_EXECUTABLE)
        install(TARGETS ${PROJECT_NAME}-exe
            RUNTIME DESTINATION bin
        )
    endif()
endif()