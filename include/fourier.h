#ifndef FOURIER_H
#define FOURIER_H

#define _USE_MATH_DEFINES // for C

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#    define Float_Complex _Fcomplex
#    define cfromreal(re) _FCbuild(re, 0)
#    define cfromimag(im) _FCbuild(0, im)
#    define mulcc _FCmulcc
#    define addcc(a, b) _FCbuild(crealf(a) + crealf(b), cimagf(a) + cimagf(b))
#    define subcc(a, b) _FCbuild(crealf(a) - crealf(b), cimagf(a) - cimagf(b))
#else
#    define Float_Complex float complex
#    define cfromreal(re) (re)
#    define cfromimag(im) ((im)*I)
#    define mulcc(a, b) ((a)*(b))
#    define addcc(a, b) ((a)+(b))
#    define subcc(a, b) ((a)-(b))
#endif 

#define FFT_SIZE 20000

#include <complex.h> 
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

extern float in_raw[FFT_SIZE];
extern float in_win[FFT_SIZE];
extern Float_Complex out_raw[FFT_SIZE];
extern float out_log[FFT_SIZE];
extern float out_smooth[FFT_SIZE];
extern float out_smear[FFT_SIZE];

size_t fft_analyze(float dt);

#endif
