#include "editor.h"
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
key_down(SDL_Keycode key)
{
	/* record_pressed(key); */
	switch (key)
	{
		case SDLK_ESCAPE:
			/* save_file(g_inst.audio_file); */
			g_running = 0;
			break;
	}
}

void
key_up(SDL_Keycode key)
{
	/* record_released(key); */
	switch (key)
	{
		case SDLK_s:
			printf("Writing to file...\n");
			SDL_FlushAudioStream(g_inst.stream);
			retrieve_stream_data();
			save_file(g_inst.audio_file, "audio.wav");
			printf("file_size is: %fKB\n", (double) g_wav_header.flength/1000);
			printf("data_size is: %fKB\n", (double) g_wav_header.dlength/1000);
			break;
		case SDLK_r:
			{
				if (!SDL_AudioDevicePaused(g_inst.capture_id))
				{
					printf("Paused recording..\n");
					SDL_PauseAudioDevice(g_inst.capture_id);
					g_retrieving = 1;
				}
				else
				{
					printf("Resume recording!\n");
					SDL_ResumeAudioDevice(g_inst.capture_id);
					g_retrieving = 0;
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
				SDL_ClearAudioStream(g_inst.stream);
				break;
			}
	}
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
			key_down(e.key.keysym.sym);
		}
		else if (e.type == SDL_EVENT_KEY_UP)
		{
			key_up(e.key.keysym.sym);
		}
	}
}
