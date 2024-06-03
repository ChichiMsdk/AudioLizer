#include "editor.h"
#include "SDL3/SDL_keycode.h"
#include <io.h>

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

void
Events(SDL_Event e)
{
	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_EVENT_QUIT)
		{
			running = 0;
		}
		else if (e.type == SDL_EVENT_KEY_DOWN)
		{
			/* record_pressed(e.key.keysym.sym); */
			switch (e.key.keysym.sym)
			{
				case SDLK_ESCAPE:
					running = 0;
					break;
			}
		}
		else if (e.type == SDL_EVENT_KEY_UP)
		{
			/* record_released(e.key.keysym.sym); */
			switch (e.key.keysym.sym)
			{
				case SDLK_k:
                    /*
					 * printf("Writing to file...\n");
					 * save_file(inst.audio_file);
                     */
					break;
				case SDLK_r:
					{
						if (!SDL_AudioDevicePaused(inst.cDevID))
						{
							printf("Paused recording..\n");
							SDL_PauseAudioDevice(inst.cDevID);
						}
						else
						{
							printf("Resume recording!\n");
							SDL_ResumeAudioDevice(inst.cDevID);
						}
						break;
					}
				case SDLK_f:
					{
						if (!SDL_AudioDevicePaused(inst.cDevID))
						{
							printf("Stopped recording..\n");
							SDL_PauseAudioDevice(inst.cDevID);
						}
						printf("Clearing the stream!\n");
						SDL_ClearAudioStream(inst.stream);
						break;
					}
			}
		}
	}
}
