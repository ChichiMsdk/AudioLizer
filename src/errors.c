#include "editor.h"

void
logger(void *w, void *r, void *s, void *f, const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	fprintf(stderr, "quitting\n");
	/* TTF_CloseFont(f); */
	SDL_DestroySurface(s);
	SDL_DestroyRenderer(inst.renderer);
	SDL_DestroyWindow(inst.window);
	/* TTF_Quit(); */
	SDL_Quit();
	exit(1);
}
