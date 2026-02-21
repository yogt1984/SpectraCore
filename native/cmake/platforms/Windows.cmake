# Windows platform configuration

if(MSVC)
    # Use static runtime for easier deployment
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    # Warning level
    add_compile_options(/W4)

    # Disable specific warnings
    add_compile_options(/wd4251)  # DLL export warnings for STL

    # Enable parallel compilation
    add_compile_options(/MP)

    # Fast floating point
    add_compile_options(/fp:fast)
endif()

# Windows-specific definitions
add_compile_definitions(
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
)
