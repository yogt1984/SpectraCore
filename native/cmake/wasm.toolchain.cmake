# WebAssembly/Emscripten CMake Toolchain File
#
# Usage:
#   emcmake cmake -B build-wasm \
#       -DCMAKE_TOOLCHAIN_FILE=cmake/wasm.toolchain.cmake \
#       -DCMAKE_BUILD_TYPE=Release
#   cmake --build build-wasm
#
# Requirements:
#   - Emscripten SDK installed and activated
#   - Source emsdk_env.sh before building
#
# Build profiles:
#   - Release: Balanced size/speed (-O3)
#   - MinSizeRel: Minimum size (-Oz, recommended for web)
#   - Debug: Debug symbols, no optimization

# This toolchain file is minimal because Emscripten's toolchain
# handles most configuration. We just add SpectraCore-specific flags.

# Verify Emscripten is being used
if(NOT EMSCRIPTEN)
    message(FATAL_ERROR "This toolchain requires Emscripten. Use: emcmake cmake ...")
endif()

# Set platform
set(CMAKE_SYSTEM_NAME Emscripten)
set(CMAKE_SYSTEM_PROCESSOR wasm32)

# Build type defaults
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE MinSizeRel CACHE STRING "Build type (MinSizeRel for web)" FORCE)
endif()

# C++ standard (must match native builds)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization flags per build type
set(CMAKE_C_FLAGS_RELEASE "-O3" CACHE STRING "C flags for Release")
set(CMAKE_CXX_FLAGS_RELEASE "-O3" CACHE STRING "C++ flags for Release")

set(CMAKE_C_FLAGS_MINSIZEREL "-Oz" CACHE STRING "C flags for MinSizeRel")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Oz" CACHE STRING "C++ flags for MinSizeRel")

set(CMAKE_C_FLAGS_DEBUG "-O0 -g" CACHE STRING "C flags for Debug")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE STRING "C++ flags for Debug")

# Common compiler flags
add_compile_options(
    -ffast-math          # Fast floating-point math (like native builds)
    -msimd128            # Enable WASM SIMD (2-4× speedup, 95%+ browser support)
)

# Linker flags for WASM
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
    -s WASM=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MODULARIZE=1 \
    -s EXPORT_ES6=0 \
    -s ENVIRONMENT=web \
    -s FILESYSTEM=0 \
    -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap'] \
    -s EXPORTED_FUNCTIONS=['_malloc','_free'] \
    -s DISABLE_EXCEPTION_CATCHING=1 \
    -s SUPPORT_LONGJMP=0"
)

# Additional flags for MinSizeRel (aggressive size optimization)
if(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
        -s MINIMAL_RUNTIME=0 \
        -s IGNORE_MISSING_MAIN=1"
    )
endif()

# Additional flags for Debug (debugging symbols, source maps)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
        -s ASSERTIONS=2 \
        -s SAFE_HEAP=1 \
        -s STACK_OVERFLOW_CHECK=1 \
        -gsource-map"
    )
endif()

# Disable features not needed for SpectraCore
add_compile_definitions(
    NO_FILESYSTEM=1      # No file I/O needed
)

# Output configuration
message(STATUS "WebAssembly/Emscripten Toolchain Configuration:")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Flags: ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}}")
message(STATUS "  C++ Flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")
message(STATUS "  SIMD: Enabled (-msimd128)")
message(STATUS "  Memory: Dynamic growth allowed")
message(STATUS "  Target: Web browsers (Chrome, Firefox, Safari, Edge)")

# Expected output size (compressed):
# - MinSizeRel: ~150-250 KB (gzip)
# - Release: ~200-300 KB (gzip)
# - Debug: ~500-800 KB (with symbols)
