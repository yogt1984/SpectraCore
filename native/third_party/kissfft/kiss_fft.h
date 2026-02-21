/*
 * Copyright (c) 2003-2010 Mark Borgerding
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * KissFFT - A mixed-radix Fast Fourier Transform
 * https://github.com/mborgerding/kissfft
 *
 * This is a placeholder header. The actual KissFFT source should be
 * downloaded from the official repository.
 */

#ifndef KISS_FFT_H
#define KISS_FFT_H

#include <stdlib.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef kiss_fft_scalar
#define kiss_fft_scalar float
#endif

typedef struct {
    kiss_fft_scalar r;
    kiss_fft_scalar i;
} kiss_fft_cpx;

typedef struct kiss_fft_state* kiss_fft_cfg;

kiss_fft_cfg kiss_fft_alloc(int nfft, int inverse_fft, void* mem, size_t* lenmem);
void kiss_fft(kiss_fft_cfg cfg, const kiss_fft_cpx* fin, kiss_fft_cpx* fout);
void kiss_fft_free(void* cfg);

#ifdef __cplusplus
}
#endif

#endif /* KISS_FFT_H */
