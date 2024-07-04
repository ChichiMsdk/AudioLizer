#include "fourier.h"
#include <stdio.h> 

/* double PI; */
static double fft_raw[FFT_SIZE];

static inline double d_amp(double a, double b)
{
	/* printf("a: %f\tb: %f\n", a, b); */
    return log(a*a + b*b);
} 

/*
 * static void _fft(Double_Complex buf[], Double_Complex out[], int n, int step)
 * {
 * 	if (step < n)
 * 	{
 * 		_fft(out, buf, n, step * 2);
 * 		_fft(out + step, buf + step, n, step * 2);
 *  
 * 		for (int i = 0; i < n; i += 2 * step)
 * 		{
 * 			#<{(| Double_Complex t = cexpf(-I * YU_PI * i / n) * out[i + step]; |)}>#
 * 			Double_Complex l = mulcr(mulcr(mulcr(D_I, -1), YU_PI), (float) i / n);
 * 			Double_Complex t = mulcc(cexp(l), out[i + step]);
 * 			buf[i / 2]     = addcc(out[i], t);
 * 			buf[(i + n)/2] = subcc(out[i], t);
 * 		}
 * 	}
 * }
 */

/* note: remove this */

size_t
fft(double *buf, int n)
{
    for (size_t i = 0; i < FFT_SIZE; ++i) {
        float t = (float)i / (FFT_SIZE - 1.0f);
		/* t *= 100; */
        float hann = 0.9 - 0.9*cosf(2*M_PI*t);
        in_win[i] = in_raw[i]*hann;
        /* in_win[i] = in_raw[i]; */
    }

	fftwf_complex *in_cplx;
	fftwf_complex *out_cplx;
	fftwf_plan p, r;

    in_cplx = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * FFT_SIZE);
    out_cplx = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * FFT_SIZE);

	float *f_out = malloc(sizeof(float)*FFT_SIZE);
	float *f_in = malloc(sizeof(float)*FFT_SIZE);

	memset(in_cplx, 0, FFT_SIZE); 
	memset(out_cplx, 0, FFT_SIZE); 
	memcpy(f_in, in_win, FFT_SIZE);
    /* p = fftw_plan_dft_1d(n, in, out, FFTW_FORWARD, FFTW_ESTIMATE); */
	p = fftwf_plan_dft_r2c_1d(n, f_in, out_cplx, FFTW_ESTIMATE);
    fftwf_execute(p); /* repeat as needed */
    r = fftwf_plan_dft_c2r_1d(n, out_cplx, f_out, FFTW_ESTIMATE);
    fftwf_execute(r); /* repeat as needed */
	for (int i = n; i < n; i++)
	{
		if (f_out[i] != 0.0000f)
			printf("f_out[%d]: %f\n", i, f_out[i]);
		i++;
	}

    fftwf_destroy_plan(p);
    // FFT

    // "Squash" into the Logarithmic Scale
    float step = 1.06;
    float lowf = 1.0f;
    size_t m = 0;
    float max_amp = 1.0f;
    for (float f = lowf; (size_t) f < FFT_SIZE/2; f = ceilf(f*step))
	{
        float f1 = ceilf(f*step);
        float a = 0.0f;
        for (size_t q = (size_t) f; q < FFT_SIZE/2 && q < (size_t) f1; ++q) {
            float b = d_amp(out_cplx[0][q], out_cplx[q][0]);
            if (b > a) a = b;
        }
        if (max_amp < a) max_amp = a;
        out_log[m++] = a;
    }

    // Normalize Frequencies to 0..1 range
    for (size_t i = 0; i < m; ++i)
	{
		out_log[i] = f_out[i];
        /* out_log[i] /= max_amp; */
		/* printf("out_log[%llu]: %f\n", i, out_log[i]); */
    }
	free(f_out);
	fftwf_free(out_cplx); fftwf_free(in_cplx); free(f_in);

	return m;
}

/*
 * void fft(double *buf, int n)
 * {
 *     assert(n > 0);
 * 	Double_Complex out[FFT_SIZE];
 * 	Double_Complex in[FFT_SIZE];
 * 	for (int i = 0; i < n; i++) 
 * 	{
 * 		in[i] = cfromreal(buf[i]);
 * 		out[i] = cfromreal(buf[i]);
 * 	}
 * 	_fft(in, out, n, 1);
 * 
 * 	size_t m = 0;
 *     float step = 1.06;
 *     float lowf = 1.0f;
 *     float max_amp = 1.0f;
 * 
 *     for (float f = lowf; (size_t) f < FFT_SIZE/2; f = ceilf(f*step))
 * 	{
 *         float f1 = ceilf(f*step);
 *         float a = 0.0f;
 *         for (size_t q = (size_t) f; q < FFT_SIZE/2 && q < (size_t) f1; ++q)
 * 		{
 *             float b = amp(out[q]);
 *             if (b > a) 
 * 				a = b;
 *         }
 *         if (max_amp < a)
 * 			max_amp = a;
 *         out_log[m++] = a;
 *     }
 *     for (size_t i = 0; i < m; ++i) {
 *         out_log[i] /= max_amp;
 *     }
 * }
 */
 
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
