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
			g_running = 0;
		}
		else if (e.type == SDL_EVENT_KEY_DOWN)
		{
			/* record_pressed(e.key.keysym.sym); */
			switch (e.key.keysym.sym)
			{
				case SDLK_ESCAPE:
					SDL_FlushAudioStream(g_inst.stream);
					save_file(g_inst.audio_file);
					g_running = 0;
					break;
			}
		}
		else if (e.type == SDL_EVENT_KEY_UP)
		{
			/* record_released(e.key.keysym.sym); */
			switch (e.key.keysym.sym)
			{
				case SDLK_k:
					printf("Writing to file...\n");
					save_file(g_inst.audio_file);
					break;
				case SDLK_r:
					{
						if (!SDL_AudioDevicePaused(g_inst.capture_id))
						{
							printf("Paused recording..\n");
							SDL_PauseAudioDevice(g_inst.capture_id);
						}
						else
						{
							printf("Resume recording!\n");
							SDL_ResumeAudioDevice(g_inst.capture_id);
						}
						break;
					}
				case SDLK_f:
					{
						if (!SDL_AudioDevicePaused(g_inst.capture_id))
						{
							printf("Stopped recording..\n");
							SDL_PauseAudioDevice(g_inst.capture_id);
						}
						printf("Clearing the stream!\n");
						/* SDL_ClearAudioStream(inst.stream); */
						break;
					}
			}
		}
	}
}
