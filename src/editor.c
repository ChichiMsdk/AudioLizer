#include "app.h"

void
record_released(int key)
{
	printf("key pressed: %d or %s\n", key, SDL_GetKeyName(key));
}

void
record_pressed(int key)
{
	static int old = 0;
	if (key == old)
		return;
	printf("key pressed: %d or %s\n", key, SDL_GetKeyName(key));
	old = key;
}
