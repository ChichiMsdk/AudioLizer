#ifndef FOURIER_H
#define FOURIER_H

#define _USE_MATH_DEFINES // for C

#include <fftw3.h>
#include <complex.h> 
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define YU_E        2.71828182845904523536   // e
#define YU_LOG2E    1.44269504088896340736   // log2(e)
#define YU_LOG10E   0.434294481903251827651  // log10(e)
#define YU_LN2      0.693147180559945309417  // ln(2)
#define YU_LN10     2.30258509299404568402   // ln(10)
#define YU_PI       3.14159265358979323846   // pi
#define YU_PI_2     1.57079632679489661923   // pi/2
#define YU_PI_4     0.785398163397448309616  // pi/4
#define YU_1_PI     0.318309886183790671538  // 1/pi
#define YU_2_PI     0.636619772367581343076  // 2/pi
#define YU_2_SQRTPI 1.12837916709551257390   // 2/sqrt(pi)
#define YU_SQRT2    1.41421356237309504880   // sqrt(2)
#define YU_SQRT1_2  0.707106781186547524401  // 1/sqrt(2)

#ifdef _MSC_VER

#include "windows.h"
extern LARGE_INTEGER		wfreq;
extern LARGE_INTEGER		wstart;
extern LARGE_INTEGER		wend;
void print_timer(LARGE_INTEGER start, LARGE_INTEGER end, LARGE_INTEGER freq);

#	define WIN32_LEAN_AND_MEAN
#	define _CRT_SECURE_NO_WARNINGS
#	define Float_Complex _Fcomplex
#	define f_cfromreal(re) _FCbuild(re, 0)
#	define f_cfromimag(im) _FCbuild(0, im)
#	define mulcc_f _FCmulcc
#	define mulcr_f _FCmulcr
#	define addcc_f(a, b) _FCbuild(crealf(a) + crealf(b), cimagf(a) + cimagf(b))
#	define subcc_f(a, b) _FCbuild(crealf(a) - crealf(b), cimagf(a) - cimagf(b))

#	define Double_Complex _Dcomplex
#	define cfromreal(re) _Cbuild(re, 0)
#	define cfromimag(im) _Cbuild(0, im)
#	define mulcc _Cmulcc
#	define mulcr _Cmulcr
#	define addcc(a, b) _Cbuild(creal(a) + creal(b), cimag(a) + cimag(b))
#	define subcc(a, b) _Cbuild(creal(a) - creal(b), cimag(a) - cimag(b))
#	define D_I _Cbuild(0.0f, 1.0f)
#else
#	define Float_Complex float complex
#	define cfromreal(re) (re)
#	define cfromimag(im) ((im)*I)
#	define mulcc(a, b) ((a)*(b))
#	define addcc(a, b) ((a)+(b))
#	define subcc(a, b) ((a)-(b))
#endif 

#define FFT_SIZE 20000

extern float in_raw[FFT_SIZE];
extern float in_win[FFT_SIZE];
extern Float_Complex out_raw[FFT_SIZE];
extern float out_log[FFT_SIZE];
extern float out_smooth[FFT_SIZE];
extern float out_smear[FFT_SIZE];

size_t	fft_analyze(float dt);
size_t	fft(double *buf, int n);

#endif
