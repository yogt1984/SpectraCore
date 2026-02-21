# Android platform configuration

# Requires Android NDK toolchain file to be set via CMAKE_TOOLCHAIN_FILE

# Minimum API level
if(NOT ANDROID_PLATFORM)
    set(ANDROID_PLATFORM android-24)
endif()

# STL
if(NOT ANDROID_STL)
    set(ANDROID_STL c++_shared)
endif()

# Compiler flags
add_compile_options(-Wall -Wextra)
add_compile_options(-ffast-math)

# NEON optimization for ARM
if(ANDROID_ABI STREQUAL "armeabi-v7a")
    add_compile_options(-mfpu=neon)
endif()

# Hide symbols by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
