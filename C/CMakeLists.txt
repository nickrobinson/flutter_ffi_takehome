cmake_minimum_required(VERSION 3.15)
project(dittoffi VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find Threads package (for pthread)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Define the shared library
add_library(dittoffi SHARED
    src/ditto.c
    include/ditto.h
)

# Set output name based on platform
if(WIN32)
    set_target_properties(dittoffi PROPERTIES OUTPUT_NAME "dittoffi")
    set_target_properties(dittoffi PROPERTIES PREFIX "")
elseif(APPLE)
    set_target_properties(dittoffi PROPERTIES OUTPUT_NAME "dittoffi")
    set_target_properties(dittoffi PROPERTIES PREFIX "lib")
else()
    set_target_properties(dittoffi PROPERTIES OUTPUT_NAME "dittoffi")
    set_target_properties(dittoffi PROPERTIES PREFIX "lib")
endif()

# Include directories
target_include_directories(dittoffi PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link pthread
target_link_libraries(dittoffi PRIVATE Threads::Threads)

# Define DITTOFFI_EXPORTS for Windows DLL export
target_compile_definitions(dittoffi PRIVATE DITTOFFI_EXPORTS)

# Platform-specific settings
if(WIN32)
    # Windows-specific flags
    target_compile_options(dittoffi PRIVATE /W4)
elseif(APPLE)
    # macOS-specific flags
    target_compile_options(dittoffi PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(dittoffi PROPERTIES
        MACOSX_RPATH ON
        INSTALL_RPATH "@loader_path"
    )
else()
    # Linux-specific flags
    target_compile_options(dittoffi PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(dittoffi PROPERTIES
        INSTALL_RPATH "$ORIGIN"
    )
endif()

# Installation rules
if(APPLE)
    install(TARGETS dittoffi
        LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/macos
    )
elseif(UNIX)
    install(TARGETS dittoffi
        LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/linux
    )
elseif(WIN32)
    install(TARGETS dittoffi
        RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/windows
    )
endif()

# Print build information
message(STATUS "Building dittoffi v${PROJECT_VERSION}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
