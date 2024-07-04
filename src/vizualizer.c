#include "app.h"
#include "fourier.h"

static const Uint8 mix8[] = 
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24,
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A,
    0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45,
    0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
    0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B,
    0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71,
    0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C,
    0x7D, 0x7E, 0x7F, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92,
    0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D,
    0x9E, 0x9F, 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8,
    0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3,
    0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE,
    0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9,
    0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4,
    0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA,
    0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5,
    0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

// The volume ranges from 0 - 128
static inline SDL_FPoint
make_wave(size_t i, int w, int h, int length, int value, Uint8 *dst)
{
	int x1 = 0;
	int y1 = 0;

	if (length == 0)
		length = 1;
	/* x1 = i / w * length; */
	x1 = i * length / w;
	y1 = (h / 2) - ( value * h / 2) / length;
	/* y1 = (h / 2) - ((dst[i]) * h / 2); */
	/* printf("x1: %d\tw:%d\ti: %llu\tlength:%d\n", x1, w, i, length); */
	return (SDL_FPoint){.x = x1, .y = y1};
}

static inline SDL_FRect
make_plot(size_t i, float c_w, float c_h, float n_rect, float fft_value, float rect_w, float space)
{
	float x1 = 0;
	float y1 = 0;
	float w1 = 0;
	float h1 = 0;

	/* x1 = i * rect_w + space; */
	/* y1 = (c_h - c_h / 4.0f); */
	x1 = i * (rect_w + space);
	y1 = c_h - (c_h / 4.0f);
	h1 = fft_value * (c_h / 2.0f) * -1;
    /*
	 * if (fft_value > 0.0f)
	 * 	printf("h1: %f\tx1: %f\ty1: %f\t fft_value: %f\n", h1, x1, y1, fft_value);
     */
	return (SDL_FRect){.x = x1, .y = y1, .w = rect_w, .h = h1};
}

float easeInOutQuad(float t) {
    if (t < 0.5f) {
        return 2.0f * t * t;
    } else {
        return -1.0f + (4.0f - 2.0f * t) * t;
    }
}

float
interpolate2(float start, float end, float factor)
{
    float easedFactor = easeInOutQuad(factor);
    return start + easedFactor * (end - start);
}

float 
interpolate(float start, float end, float factor)
{ 
	return start + factor * (end - start); 
}

#define MIX_MAXVOLUME 128
#define ADJUST_VOLUME(type, s, v) ((s) = (type)(((s) * (v)) / MIX_MAXVOLUME))
#define ADJUST_VOLUME_U8(s, v)    ((s) = (Uint8)(((((s) - 128) * (v)) / MIX_MAXVOLUME) + 128))

#include <float.h>
static float previous[200] = {0};
static float current[200] = {0};
void
apply_fft(Uint8 *dst, Uint8 *src, Uint32 length, Audio_wave *wave, float adjust)
{
	int i = 0;
	Uint32 len = length;

	int volume = (int)SDL_roundf(adjust * MIX_MAXVOLUME);
	if (volume == 0)
		return ;

	const float *src32 = (float *)src;
	float *dst32 = (float *)dst;
	float src1, src2;
	float dst_sample;
	const float max_audioval = 1.0f;
	const float min_audioval = -1.0f;

	len /= 4;
	while (len--) 
	{
		src1 = SDL_SwapFloatLE(*src32) * adjust;
		i++;
		src2 = SDL_SwapFloatLE(*dst32);
		src32++;
		dst_sample = src1 + src2;
		if (dst_sample > max_audioval) {
			dst_sample = max_audioval;
		} else if (dst_sample < min_audioval) {
			dst_sample = min_audioval;
		}
		*(dst32++) = SDL_SwapFloatLE(dst_sample);
	}

	memset(in_raw, 0, FFT_SIZE);
	memcpy(in_raw, dst, length);
	size_t m = fft_analyze(((float)(g_end - g_start) / (1000 * 1000 * 1000)));
	SDL_FRect *rects= malloc(sizeof(SDL_FRect) * m);
    
	double space = g_win_w / ((m - 1) * 2.0f);
	double w_space = (m - 1) * space;
	double remaining_w = g_win_w - w_space;
    double rect_w = remaining_w / m;
	/* printf("space %d\tw_space: %d\tremaining_w: %d\trect_w: %d\n", space, w_space, remaining_w, rect_w); */
	SDL_SetRenderTarget(g_inst.r, wave->text);
	YU_SetRenderDrawColor(g_inst.r, YU_GRAY);
	SDL_RenderClear(g_inst.r);
    /*
	 * YU_SetRenderDrawColor(g_inst.r, YU_BLUE_ATOLL);
	 * SDL_RenderFillRects(g_inst.r, rects, m);
     */
	i = -1;
	while (++i < m)
	{
		if (i == 7 || i == 16)
			rects[i] = make_plot(i, g_win_w, g_win_h, m, out_log[i-1], rect_w, space);
		else
			rects[i] = make_plot(i, g_win_w, g_win_h, m, out_log[i], rect_w, space);
    
		current[i] = interpolate2(previous[i], rects[i].h, 0.3f);
		SDL_SetRenderDrawColorFloat(g_inst.r, i/100.0f, i/30.0f, 255.0f, 255);
		SDL_FRect r = {.x = rects[i].x, .y = rects[i].y, .w = rects[i].w, .h = current[i]};
		SDL_RenderFillRect(g_inst.r, &r);
	}
	memcpy(previous, current, sizeof(previous));
	free(rects);
	return ;
}

void
YU_MixAudio(Uint8 *dst, const Uint8 *src, SDL_AudioFormat format,
                 Uint32 len, float fvolume, Audio_wave *wave)
{
	int 		factor = fvolume;
	size_t		i = 0;
	Uint32 		length = len;
	int			w = wave->w;
	static		size_t tmp;

	SDL_FPoint	*points;
	SDL_FPoint  first = {.x = 0, .y = wave->h / 2.0f};
	SDL_FPoint  scd = {.x = wave->w, .y = wave->h / 2.0f};

	/* if (len == 0 || len <= 150000) */
	if (len == 0 )
	{
		return;
	}
	/* dst = malloc(sizeof(Uint8) * len); */
	points = malloc(sizeof(SDL_FPoint) * wave->w);
	memset(points, 0, wave->w);
	/* memset(dst, 0, len); */
	if (format == SDL_AUDIO_F32)
	{
		const float *src32 = (float *)src;
		float *dst32 = (float *)dst;
		float src1, src2;
		float dst_sample;

		len /= 4;
		while (len-- && w--) 
		{
			src1 = SDL_SwapFloatLE(*src32) * fvolume;
			points[i] = make_wave(i, wave->w, wave->h, length, src1, dst);
			i++;
			src2 = SDL_SwapFloatLE(*dst32);
			src32++;

			dst_sample = src1 + src2;
			*(dst32++) = SDL_SwapFloatLE(dst_sample);
		}
	}
	SDL_SetRenderTarget(g_inst.r, wave->text);
	YU_SetRenderDrawColor(g_inst.r, YU_GRAY);
	SDL_RenderClear(g_inst.r);
	YU_SetRenderDrawColor(g_inst.r, YU_BLUE_ATOLL);
	SDL_RenderLines(g_inst.r, points, i);
	/* printf("length = %d\n", length); */
    /*
	 * if (len > 150000)
	 * 	g_inst.w_form.buflen = 0;
     */
	/* sprintf(txt, "%d", length); */
freer:
	free(points);
	/* free(dst); */

	return ;
}
