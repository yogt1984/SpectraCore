/*
 * Copyright (c) 2003-2010 Mark Borgerding
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * PLACEHOLDER IMPLEMENTATION
 * Replace with actual KissFFT source from:
 * https://github.com/mborgerding/kissfft
 */

#include "kiss_fftr.h"
#include <string.h>

struct kiss_fftr_state {
    kiss_fft_cfg substate;
    kiss_fft_cpx* tmpbuf;
    kiss_fft_cpx* super_twiddles;
    int nfft;
};

kiss_fftr_cfg kiss_fftr_alloc(int nfft, int inverse_fft, void* mem, size_t* lenmem) {
    kiss_fftr_cfg cfg = (kiss_fftr_cfg)malloc(sizeof(struct kiss_fftr_state));
    if (!cfg) return NULL;

    cfg->nfft = nfft;
    cfg->substate = kiss_fft_alloc(nfft / 2, inverse_fft, NULL, NULL);
    cfg->tmpbuf = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * nfft);
    cfg->super_twiddles = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * (nfft / 2));

    // Initialize super twiddles
    for (int i = 0; i < nfft / 2; ++i) {
        double phase = -3.14159265358979323846 * (i + 1) / nfft;
        if (inverse_fft) phase = -phase;
        cfg->super_twiddles[i].r = (float)cos(phase);
        cfg->super_twiddles[i].i = (float)sin(phase);
    }

    return cfg;
}

void kiss_fftr(kiss_fftr_cfg cfg, const kiss_fft_scalar* timedata, kiss_fft_cpx* freqdata) {
    int N = cfg->nfft;
    int Nhalf = N / 2;

    // Pack real data into complex
    kiss_fft_cpx* tmpbuf = cfg->tmpbuf;
    for (int i = 0; i < Nhalf; ++i) {
        tmpbuf[i].r = timedata[2 * i];
        tmpbuf[i].i = timedata[2 * i + 1];
    }

    // FFT of half-length complex
    kiss_fft(cfg->substate, tmpbuf, freqdata);

    // Unpack
    freqdata[Nhalf].r = freqdata[0].r - freqdata[0].i;
    freqdata[Nhalf].i = 0;
    freqdata[0].r = freqdata[0].r + freqdata[0].i;
    freqdata[0].i = 0;

    for (int k = 1; k < Nhalf; ++k) {
        kiss_fft_cpx fk = freqdata[k];
        kiss_fft_cpx fnk;
        fnk.r = freqdata[Nhalf - k].r;
        fnk.i = -freqdata[Nhalf - k].i;

        kiss_fft_cpx fek, fok;
        fek.r = (fk.r + fnk.r) * 0.5f;
        fek.i = (fk.i + fnk.i) * 0.5f;
        fok.r = (fk.r - fnk.r) * 0.5f;
        fok.i = (fk.i - fnk.i) * 0.5f;

        // Multiply by twiddle
        kiss_fft_cpx tw = cfg->super_twiddles[k - 1];
        kiss_fft_cpx tmp;
        tmp.r = fok.r * tw.r - fok.i * tw.i;
        tmp.i = fok.r * tw.i + fok.i * tw.r;

        freqdata[k].r = fek.r + tmp.r;
        freqdata[k].i = fek.i + tmp.i;
    }
}

void kiss_fftri(kiss_fftr_cfg cfg, const kiss_fft_cpx* freqdata, kiss_fft_scalar* timedata) {
    int N = cfg->nfft;
    int Nhalf = N / 2;

    // Inverse of unpacking
    cfg->tmpbuf[0].r = freqdata[0].r + freqdata[Nhalf].r;
    cfg->tmpbuf[0].i = freqdata[0].r - freqdata[Nhalf].r;

    for (int k = 1; k < Nhalf; ++k) {
        kiss_fft_cpx fk = freqdata[k];
        kiss_fft_cpx tw = cfg->super_twiddles[k - 1];
        tw.i = -tw.i;  // Conjugate for inverse

        kiss_fft_cpx tmp;
        tmp.r = fk.r * tw.r - fk.i * tw.i;
        tmp.i = fk.r * tw.i + fk.i * tw.r;

        cfg->tmpbuf[k].r = fk.r + tmp.r;
        cfg->tmpbuf[k].i = fk.i + tmp.i;
    }

    // Inverse FFT
    kiss_fft(cfg->substate, cfg->tmpbuf, cfg->tmpbuf);

    // Unpack to real
    for (int i = 0; i < Nhalf; ++i) {
        timedata[2 * i] = cfg->tmpbuf[i].r;
        timedata[2 * i + 1] = cfg->tmpbuf[i].i;
    }
}
