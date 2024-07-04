#ifndef FFT_H
#define FFT_H

#define _USE_MATH_DEFINES // for C

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#	define Float_Complex _Fcomplex
#	define cfromreal(re) _FCbuild(re, 0)
#	define cfromimag(im) _FCbuild(0, im)
#	define mulcc _FCmulcc
#	define mulcr _FCmulcr
#	define addcc(a, b) _FCbuild(crealf(a) + crealf(b), cimagf(a) + cimagf(b))
#	define subcc(a, b) _FCbuild(crealf(a) - crealf(b), cimagf(a) - cimagf(b))
#else
#	define Float_Complex float complex
#	define cfromreal(re) (re)
#	define cfromimag(im) ((im)*I)
#	define mulcc(a, b) ((a)*(b))
#	define addcc(a, b) ((a)+(b))
#	define subcc(a, b) ((a)-(b))
#endif 

#include <stdio.h>
#include <math.h>
#include <complex.h>

#endif
