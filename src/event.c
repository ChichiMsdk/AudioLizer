#include "audio.h"

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
key_up(SDL_Keycode key, AudioData *a_data)
{
	char *filename = "audio2.wav";
	/* record_released(key); */
	switch (key)
	{
		case SDLK_l:
			printf("l is pressed\n");
			g_sending = 0;
			break;
		case SDLK_s:
			printf("Writing to file...\n");
			/* retrieve_stream_data(); */
			/*
			 * should be prompted to change the name of the file
			 * maybe a global? 
			 */
			save_file(filename, a_data);
			break;
		case SDLK_r:
			{
				if (!SDL_AudioDevicePaused(g_inst.capture_id))
				{
					printf("Paused recording..\n");
					SDL_PauseAudioDevice(g_inst.capture_id);
					SDL_FlushAudioStream(g_inst.stream);
					g_retrieving = 1;
					g_vizualizing = 1;
				}
				else
				{
					printf("Resume recording!\n");
					SDL_ResumeAudioDevice(g_inst.capture_id);
					g_retrieving = 0;
					g_vizualizing = 0;
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
Events(SDL_Event e, AudioData *a_data)
{
	while (SDL_PollEvent(&e) != 0)
	{
		switch (e.type)
		{
			case SDL_EVENT_QUIT:
				{
					g_running = 0;
					break;
				}
			case SDL_EVENT_MOUSE_BUTTON_UP:
				{
					button_check(get_mouse_state(), &g_inst.button);
					break;
				}
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				{
					Mouse_state mouse = get_mouse_state();
					button_check(get_mouse_state(), &g_inst.button);
					debug_mouse_state(mouse);
					break;
				}
			case SDL_EVENT_MOUSE_MOTION:
				{
					button_check(get_mouse_state(), &g_inst.button);
					break;
				}
			case SDL_EVENT_KEY_DOWN:
				{
					/* should I break here ? */
					key_down(e.key.keysym.sym);
					break;
				}
			case SDL_EVENT_KEY_UP:
				{
					key_up(e.key.keysym.sym, a_data);
					break;
				}
		}
	}
}
