# iOS CMake Toolchain File
# Based on https://github.com/leetal/ios-cmake
#
# Usage:
#   cmake -B build-ios -G Xcode \
#       -DCMAKE_TOOLCHAIN_FILE=cmake/ios.toolchain.cmake \
#       -DPLATFORM=OS64
#   cmake --build build-ios --config Release

# Platform options:
# - OS64: iOS device (arm64)
# - SIMULATOR64: iOS simulator (x86_64)
# - SIMULATORARM64: iOS simulator (arm64 for M1/M2 Macs)

if(NOT DEFINED PLATFORM)
    set(PLATFORM "OS64" CACHE STRING "iOS platform: OS64, SIMULATOR64, SIMULATORARM64")
endif()

# Set CMAKE_SYSTEM_NAME to iOS
set(CMAKE_SYSTEM_NAME iOS)

# Deployment target
if(NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum iOS deployment target")
endif()

# Architectures based on platform
if(PLATFORM STREQUAL "OS64")
    # iOS device (arm64)
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architecture")
    set(CMAKE_OSX_SYSROOT "iphoneos" CACHE STRING "SDK name")
    set(IOS_DEVICE TRUE)
elseif(PLATFORM STREQUAL "SIMULATOR64")
    # iOS simulator (x86_64 for Intel Macs)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architecture")
    set(CMAKE_OSX_SYSROOT "iphonesimulator" CACHE STRING "SDK name")
    set(IOS_SIMULATOR TRUE)
elseif(PLATFORM STREQUAL "SIMULATORARM64")
    # iOS simulator (arm64 for Apple Silicon Macs)
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architecture")
    set(CMAKE_OSX_SYSROOT "iphonesimulator" CACHE STRING "SDK name")
    set(IOS_SIMULATOR TRUE)
else()
    message(FATAL_ERROR "Unknown PLATFORM: ${PLATFORM}. Use OS64, SIMULATOR64, or SIMULATORARM64")
endif()

# Disable bitcode (deprecated since Xcode 14)
set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE NO)

# Only build active architecture in Xcode
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO)

# Code signing (for device builds)
if(IOS_DEVICE)
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer" CACHE STRING "Code signing identity")
endif()

# Standard library
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++17")
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

# Build type defaults
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif()

# Compiler flags for iOS
set(CMAKE_C_FLAGS_INIT "-ffast-math")
set(CMAKE_CXX_FLAGS_INIT "-ffast-math")

# Define iOS platform macro
add_definitions(-DIOS -DTARGET_OS_IOS)

# Hide symbols by default (export only what's needed)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_C_VISIBILITY_PRESET hidden)

# Set platform name
set(IOS TRUE)

# Print configuration
message(STATUS "iOS Toolchain Configuration:")
message(STATUS "  Platform: ${PLATFORM}")
message(STATUS "  Architectures: ${CMAKE_OSX_ARCHITECTURES}")
message(STATUS "  Deployment Target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
message(STATUS "  SDK: ${CMAKE_OSX_SYSROOT}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
