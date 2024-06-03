#include "SDL3/SDL_audio.h"
#include "editor.h"

YUinstance			inst = {0};
int					WINDOW_WIDTH = 1200;
int					WINDOW_HEIGHT = 800;
int					running = 1;

#define AUDIO_FORMAT SDL_AUDIO_F32LE
#define AUDIO_CHANNELS 1
#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_BUFFER_SIZE 4096

typedef struct 
{
    Uint8 *buffer;
    Uint32 length;
    Uint32 position;
} AudioData;

void
getAudioDevList()
{
    SDL_AudioDeviceID *adev;
	int device_list;
	if (!(adev = SDL_GetAudioCaptureDevices(&device_list)))
	{
		fprintf(stderr, "Couldnt getaudiocapturedevices\n");
		exit(1);
	}

	 //  NOTE: JE SUIS UNE MERDE J'AI PERDU DU TEMPS POUR CA 
	void *temp = adev;
	while (*adev)
	{
		printf("Device capture %d: %s\n", *adev, SDL_GetAudioDeviceName(*adev));
		adev++;
	}
	SDL_free(temp);

	// Output
	if (!(adev = SDL_GetAudioOutputDevices(&device_list)))
	{
		fprintf(stderr, "Couldnt getaudiocapturedevices\n");
		exit(1);
	}

	temp = adev;
	while (*adev)
	{
		printf("Device output %d: %s\n", *adev, SDL_GetAudioDeviceName(*adev));
		adev++;
	}
	SDL_free(temp);
}

void 
init(void)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		fprintf(stderr, "%s\n", SDL_GetError());
		exit(1);
	}

	inst.window = SDL_CreateWindow("Key capture", WINDOW_WIDTH, WINDOW_HEIGHT,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (inst.window == NULL)
	{
		fprintf(stderr, "%s\n", SDL_GetError());
		TTF_Quit();
		SDL_Quit();
		exit(1);
	}

	getAudioDevList();

    SDL_AudioDeviceID outputDevID;
    SDL_AudioDeviceID captureDevID;

	/* use this ??
	 *
	 * SDL_OpenAudioDeviceStream(SDL_AudioDeviceID devid, const SDL_AudioSpec *spec, SDL_AudioStreamCallback callback, void *userdata);
	 *
	 */

    outputDevID = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_OUTPUT, 0);
    captureDevID = SDL_OpenAudioDevice(42, 0);
	printf("Device output chosen %s\n", SDL_GetAudioDeviceName(outputDevID));
	printf("Device capture chosen %s\n", SDL_GetAudioDeviceName(captureDevID));

    if (!outputDevID || !captureDevID)
	{
        fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
        SDL_DestroyWindow(inst.window);
		TTF_Quit();
        SDL_Quit();
        exit(1);
    }

	SDL_AudioStream *stream = SDL_CreateAudioStream(NULL, NULL);
	if (!stream)
	{
        fprintf(stderr, "Failed to create audio stream: %s\n", SDL_GetError());
        SDL_DestroyWindow(inst.window);
		TTF_Quit();
        SDL_Quit();
        exit(1);
	}

	if (SDL_BindAudioStream(captureDevID, stream) == -1)
	{
        fprintf(stderr, "Failed to bind stream: %s\n", SDL_GetError());
        SDL_DestroyWindow(inst.window);
		TTF_Quit();
        SDL_Quit();
        exit(1);
	}

	inst.renderer = SDL_CreateRenderer(inst.window,NULL);
	if (inst.renderer == NULL)
	{
		fprintf(stderr, "%s\n", SDL_GetError());
		SDL_DestroyWindow(inst.window);
		TTF_Quit();
		SDL_Quit();
		exit(1);
	}

	inst.stream = stream;
	inst.oDevID = outputDevID;
	inst.cDevID = captureDevID;
}

#define BUFLEN 1024

void
save_file(FILE *file)
{
	if (!file)
	{
		fprintf(stderr, "file is NULL..\n");
		/* exit(1); */
	}
	int bytes_read = 0;
	int bytes_written = 0;
	size_t bytes_queued = 0;
	void *audioBuf[BUFLEN];
	/* while (SDL_GetAudioStreamQueued(inst.stream)) */
	bytes_queued = SDL_GetAudioStreamQueued(inst.stream);
	printf("bytes_queued = %llu\n", bytes_queued);
	{
		bytes_read = SDL_GetAudioStreamData(inst.stream, audioBuf, BUFLEN);
		if (bytes_read < -1)
		{
			fprintf(stderr, "No bytes received from AudioStream..\n");
			return ;
		}
		bytes_written = fwrite(audioBuf, sizeof(void *), bytes_queued, file);
		if (bytes_written < 0)
		{
			perror("fwrite line 138:");
			exit(1);
		}
	}
}

int
/* WinMain() */
main()
{
	init();
	inst.audio_file = fopen("test.wav", "wb");
	if (!inst.audio_file)
	{
		perror("Error fopen line 151: ");
		exit(1);
	}
	while (running)
	{
		SDL_SetRenderDrawColor(inst.renderer, 50, 50, 50, 255);
		SDL_RenderClear(inst.renderer);
		Events(inst.e);
		SDL_RenderPresent(inst.renderer);
	}

	SDL_DestroyRenderer(inst.renderer);
	SDL_DestroyWindow(inst.window);
	SDL_Quit();
	fclose(inst.audio_file);

    /*
	 * _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE);
	 * _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	 * _CrtDumpMemoryLeaks();
     */

	return 0;
}
