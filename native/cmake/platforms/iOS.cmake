# iOS platform configuration

set(CMAKE_SYSTEM_NAME iOS)
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum iOS version")

# Build for device and simulator
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for iOS")

# Disable bitcode (deprecated)
set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE NO)

# Compiler flags
add_compile_options(-Wall -Wextra)
add_compile_options(-ffast-math)

# Hide symbols by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
