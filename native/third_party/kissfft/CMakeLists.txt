# KissFFT library

add_library(kissfft STATIC
    kiss_fft.c
    kiss_fftr.c
)

target_include_directories(kissfft PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Disable warnings for third-party code
if(MSVC)
    target_compile_options(kissfft PRIVATE /W0)
else()
    target_compile_options(kissfft PRIVATE -w)
endif()
