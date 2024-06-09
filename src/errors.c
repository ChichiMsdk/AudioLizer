#include "audio.h"

void
logger(void *w, void *r, void *s, void *f, const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	fprintf(stderr, "quitting\n");
	/* TTF_CloseFont(f); */
	SDL_DestroySurface(s);
	SDL_DestroyRenderer(g_inst.renderer);
	SDL_DestroyWindow(g_inst.window);
	/* TTF_Quit(); */
	SDL_Quit();
	exit(1);
}

void
print_audio_spec_info(SDL_AudioSpec micSpec, int micSample)
{
	printf("--Audio specs--\n");
	printf("Samples: %d\nChannels: %d\nFreq: %d\nFormat: %us\n", 
			micSample, micSpec.channels, micSpec.freq, micSpec.format);

	printf("Bit size: %d\n", SDL_AUDIO_BITSIZE(micSpec.format));
	printf("Byte size: %d\n", SDL_AUDIO_BYTESIZE(micSpec.format));
	printf("Little endian: %d\n", SDL_AUDIO_ISLITTLEENDIAN(micSpec.format));
	printf("Int: %d\n", SDL_AUDIO_ISINT(micSpec.format));
	printf("Signed: %d\n\n", SDL_AUDIO_ISSIGNED(micSpec.format));
	/* exit(1); */
}

void
logExit(char *msg)
{
	fprintf(stderr, "%s: %s\n", msg, SDL_GetError());
	SDL_DestroyWindow(g_inst.window);
	SDL_Quit();
	exit(1);
}
