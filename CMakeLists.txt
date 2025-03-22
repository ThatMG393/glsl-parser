cmake_minimum_required(VERSION 3.10)

project(glsl-parser LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_STATIC_LIB "Build as a static library" ON)
option(BUILD_SHARED_LIB "Build as a shared library" OFF)
option(BUILD_EXECUTABLE "Build the executable" ON)
option(INSTALL_BUILD "Install the built library and executable" OFF)

# If executable is built, enable installation automatically
if(BUILD_EXECUTABLE)
    set(INSTALL_BUILD ON)
endif()

# Ensure at least one library type is built
if(NOT BUILD_STATIC_LIB AND NOT BUILD_SHARED_LIB)
    set(BUILD_STATIC_LIB ON)
endif()

# Compiler flags
if(MSVC)
    # MSVC-specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /EHs- /W4 /O2")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(COMPILER_SPECIFIC_OPTIONS /permissive- /Zc:preprocessor /W4)
else()
    # Non-MSVC flags (fixing the typo in -fno-exceptions)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -Wall -Wextra -Wformat -O3")
    set(COMPILER_SPECIFIC_OPTIONS "")
endif()

# Library source files
set(LIB_SOURCES
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

# Executable source files
set(EXE_SOURCES
    main.cpp
)

# First, create the library target(s)
if(BUILD_STATIC_LIB)
    add_library(${PROJECT_NAME}_static STATIC ${LIB_SOURCES})
    set_target_properties(${PROJECT_NAME}_static PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
        POSITION_INDEPENDENT_CODE ON
    )
    target_compile_options(${PROJECT_NAME}_static PRIVATE ${COMPILER_SPECIFIC_OPTIONS})
    target_include_directories(${PROJECT_NAME}_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    )
endif()

if(BUILD_SHARED_LIB)
    add_library(${PROJECT_NAME}_shared SHARED ${LIB_SOURCES})
    set_target_properties(${PROJECT_NAME}_shared PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
        POSITION_INDEPENDENT_CODE ON
    )
    target_compile_options(${PROJECT_NAME}_shared PRIVATE ${COMPILER_SPECIFIC_OPTIONS})
    target_include_directories(${PROJECT_NAME}_shared PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    )
endif()

# Set the main library target for other targets to link against
if(BUILD_STATIC_LIB)
    set(MAIN_LIB_TARGET ${PROJECT_NAME}_static)
else()
    set(MAIN_LIB_TARGET ${PROJECT_NAME}_shared)
endif()

# Build the executable if requested
if(BUILD_EXECUTABLE)
    add_executable(${PROJECT_NAME}_exe ${EXE_SOURCES})
    set_target_properties(${PROJECT_NAME}_exe PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
    )
    target_link_libraries(${PROJECT_NAME}_exe PRIVATE ${MAIN_LIB_TARGET})
    target_compile_options(${PROJECT_NAME}_exe PRIVATE ${COMPILER_SPECIFIC_OPTIONS})
endif()

# Create an alias target for use in other projects
if(BUILD_STATIC_LIB)
    add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME}_static)
elseif(BUILD_SHARED_LIB)
    add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME}_shared)
endif()

# Installation rules
if(INSTALL_BUILD)
    include(GNUInstallDirs)
    
    # Install the libraries
    if(BUILD_STATIC_LIB)
        install(TARGETS ${PROJECT_NAME}_static
            EXPORT ${PROJECT_NAME}Targets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    endif()
    
    if(BUILD_SHARED_LIB)
        install(TARGETS ${PROJECT_NAME}_shared
            EXPORT ${PROJECT_NAME}Targets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    endif()
    
    # Install the executable if built
    if(BUILD_EXECUTABLE)
        install(TARGETS ${PROJECT_NAME}_exe
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()

    # Install the headers
    install(FILES ${HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    )

    # Export the targets for use by other projects
    install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}
    )
    
    # Create and install a config file
    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}
    )
    
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}
    )
endif()

# Add testing
add_custom_target(test
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/test.py
    DEPENDS ${PROJECT_NAME}_exe
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running tests..."
)
