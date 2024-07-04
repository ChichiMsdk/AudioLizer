#include "fourier.h"

/* double PI; */

static inline double amp(Double_Complex z)
{
    double a = creal(z);
    double b = cimag(z);
    return log(a*a + b*b);
} 

static void _fft(Double_Complex buf[], Double_Complex out[], int n, int step)
{
	if (step < n)
	{
		_fft(out, buf, n, step * 2);
		_fft(out + step, buf + step, n, step * 2);
 
		for (int i = 0; i < n; i += 2 * step)
		{
			/* Double_Complex t = cexpf(-I * YU_PI * i / n) * out[i + step]; */
			Double_Complex l = mulcr(mulcr(mulcr(D_I, -1), YU_PI), (float) i / n);
			Double_Complex t = mulcc(cexp(l), out[i + step]);
			buf[i / 2]     = addcc(out[i], t);
			buf[(i + n)/2] = subcc(out[i], t);
		}
	}
}

/* note: remove this */
#include <stdlib.h> 
void fft(Double_Complex buf[], int n)
{
    assert(n > 0);
	Double_Complex *out = malloc(sizeof(Double_Complex)*n);
	for (int i = 0; i < n; i++) 
		out[i] = buf[i];
	_fft(buf, out, n, 1);
	free(out);
}
 
#include <stdio.h> 
void show(const char * s, Double_Complex buf[]) 
{
	printf("%s", s);
	for (int i = 0; i < 8; i++)
	{
		if (!cimag(buf[i]))
			printf("%f ", creal(buf[i]));
		else
			printf("(%f, %f) ", creal(buf[i]), cimag(buf[i]));
	}
}

/*
 * int main()
 * {
 * 	Double_Complex buf[] = {1, 1, 1, 1, 0, 0, 0, 0};
 *  
 * 	show("Data: ", buf);
 * 	fft(buf, 8);
 * 	show("\nFFT : ", buf);
 * 	return 0;
 * }
 */
