# SpectraCore Tests

# Unit tests
spectra_add_test(test_ring_buffer unit/test_ring_buffer.cpp)
spectra_add_test(test_window_functions unit/test_window_functions.cpp)
spectra_add_test(test_fft unit/test_fft.cpp)
spectra_add_test(test_stft unit/test_stft.cpp)
spectra_add_test(test_onset_detection unit/test_onset_detection.cpp)
spectra_add_test(test_butterworth unit/test_butterworth.cpp)
spectra_add_test(test_chebyshev unit/test_chebyshev.cpp)
spectra_add_test(test_elliptic unit/test_elliptic.cpp)
spectra_add_test(test_lfilter unit/test_lfilter.cpp)
spectra_add_test(test_filters unit/test_filters.cpp)
spectra_add_test(test_correlation unit/test_correlation.cpp)
spectra_add_test(test_streaming_filter unit/test_streaming_filter.cpp)

# Benchmarks (optional)
if(SPECTRA_BUILD_BENCHMARKS)
    add_executable(bench_fft benchmarks/bench_fft.cpp)
    target_link_libraries(bench_fft PRIVATE spectra)
endif()
