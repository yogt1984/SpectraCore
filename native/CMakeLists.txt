cmake_minimum_required(VERSION 3.20)
project(spectra VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Options
option(SPECTRA_BUILD_TESTS "Build unit tests" ON)
option(SPECTRA_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(SPECTRA_ENABLE_SANITIZERS "Enable sanitizers in debug builds" OFF)

# Platform-specific configuration
include(cmake/platforms/${CMAKE_SYSTEM_NAME}.cmake OPTIONAL)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Third-party dependencies
add_subdirectory(third_party/kissfft)

# Core library sources
set(SPECTRA_SOURCES
    src/core/types.cpp
    src/core/ring_buffer.cpp
    src/core/window_functions.cpp
    src/fft/fft.cpp
    src/fft/stft.cpp
    src/filters/fir_filter.cpp
    src/filters/iir_filter.cpp
    src/filters/filter_design.cpp
    src/filters/streaming_filter.cpp
    src/analysis/correlation.cpp
    src/analysis/psd.cpp
    src/analysis/hilbert.cpp
    src/analysis/resample.cpp
    src/analysis/onset_detection.cpp
    src/abi/spectra_abi.cpp
)

# Create shared library
add_library(spectra SHARED ${SPECTRA_SOURCES})

target_include_directories(spectra
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(spectra PRIVATE kissfft)

# Export symbols - ensure all symbols are visible for shared library
set_target_properties(spectra PROPERTIES
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN OFF
)
target_compile_definitions(spectra PRIVATE SPECTRA_BUILD)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(spectra PRIVATE _USE_MATH_DEFINES)
elseif(APPLE)
    set_target_properties(spectra PROPERTIES
        FRAMEWORK FALSE
        MACOSX_RPATH ON
    )
endif()

# Sanitizers (debug builds)
if(SPECTRA_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(spectra PRIVATE -fsanitize=address,undefined)
        target_link_options(spectra PRIVATE -fsanitize=address,undefined)
    endif()
endif()

# Testing
if(SPECTRA_BUILD_TESTS)
    enable_testing()
    include(cmake/testing.cmake)
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS spectra
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/spectra DESTINATION include)
install(FILES include/spectra_abi.h DESTINATION include)
