#include "fourier.h"
#include <stdio.h> 

/* double PI; */
static double fft_raw[FFT_SIZE];
float in_raw[FFT_SIZE];
float in_win[FFT_SIZE];
float out_log[FFT_SIZE];
float out_smooth[FFT_SIZE];
float out_smear[FFT_SIZE];

static inline float d_amp(fftwf_complex *nb, int i)
{
	/* printf("a: %f\tb: %f\n", a, b); */
	float a1 = nb[i][0];
	float a2 = nb[0][i];
	float b1 = nb[i][1];
	float b2 = nb[1][i];
	
    return logf(sqrt(a1) + sqrt(b2));
} 
/* note: remove this */

static float hann;
static float f = 1.0f;
static float fst = 0.9;
static float scd = 0.9;
static float mult = 1.0f;

size_t
fft(double *buf, int n)
{
	/* memset(in_win, 0, n); */
    for (size_t i = 0; i < n; ++i) 
	{
        float t = (float)i / (n - 1.0f);
        hann = fst - scd*cosf(2*M_PI*t);
        in_win[i] = in_raw[i]*hann;
        /* in_win[i] = in_raw[i]; */
    }

	fftwf_complex *in_cplx;
	fftwf_complex *out_cplx;
	fftwf_plan p, r;

    out_cplx = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * FFT_SIZE);

	float *f_in = malloc(sizeof(float)*FFT_SIZE);
	for (int i = 0; i < n; i++)
		f_in[i] = in_win[i];

	p = fftwf_plan_dft_r2c_1d(n, f_in, out_cplx, FFTW_ESTIMATE);
    fftwf_execute(p); /* repeat as needed */

    size_t m = n / 2 + 1;
	float threshold = 1.0f / 1.0f;
	for (int i = 0; i < m; i++)
	{
		float magnitude = (float)sqrt(out_cplx[i][0] * out_cplx[i][0] + out_cplx[i][1] * out_cplx[i][1]);
		/* printf("%f\n", magnitude); */
		/* if ( i % 2) */
			out_log[i] = (magnitude >= threshold) ? magnitude : 0.0f;
	}

    // "Squash" into the Logarithmic Scale
    float step = 1.06;
    float lowf = 1.0f;
    float max_amp = 1.0f;

	static float max_temp;
	if (max_amp > max_temp)
	{ max_temp = max_amp; printf("max_amp: %f\n", max_temp); }

    // Normalize Frequencies to 0..1 range
	size_t i = 0;
	size_t j = 0;
    while (i < m && j < m)
	{
		out_log[i] /= 240.0f;
		if (j >= 130)
		{
			break;
			if (out_log[i] > threshold)
				out_log[j++] = out_log[i];
			i++;
			continue;
		}
		out_log[j++] = out_log[i++];
		/* printf("out_log[%llu]: %f\n", i, out_log[i]); */
    }

    fftwf_destroy_plan(p);
	fftwf_free(out_cplx); free(f_in);

	return j;
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
