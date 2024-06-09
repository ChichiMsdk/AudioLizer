#include "audio.h"

void *g_buffer;

void
init_audio(void)
{
	g_buffer = malloc(FIRST_ALLOC); assert(g_buffer);
	g_inst.sample_size = 0;
	g_inst.current_buff_size = FIRST_ALLOC;

	/* sets the global var for the capture and output logical dev */
	SDL_AudioSpec a_capture_spec = set_capture_device(g_inst.capture_name);
	SDL_AudioSpec a_output_spec = set_output_device(g_inst.output_name);

	/* same here stream is set in global g_inst.stream */
	stream_capture_init(a_capture_spec, g_inst.capture_id);

	/* should probably do this just before saving the file instead */
	wav_header_init(a_capture_spec);
}

void
adjust_volume_for_file(float factor)
{
	size_t		i;
	int16_t		*data;
	size_t		number_samples;

	i = 0;
	data = (int16_t *)g_buffer;
	/* checks the number of samples; total size / size of 1 sample (2 byteshere) */
	number_samples= g_wav_header.dlength / sizeof(int16_t);
	while (i < number_samples)
	{
		/* i dont really know whats the limit here for the volume ... */
		int32_t check_sample = (int32_t)(data[i] * factor);
		if (check_sample > INT16_MAX)
			check_sample = INT16_MAX;
		else if (check_sample < INT16_MIN)
			check_sample = INT16_MIN;
		data[i] = check_sample;
		i++;
	}
}

/* TODO: make appending file possible and volume choice in argv[] or ui */
void
save_file(FILE *file, char *file_name)
{
	static size_t	bytes_written;
	float			volume_f = 50.0f;

	g_inst.audio_file = fopen(file_name, "wb");
	if (!g_inst.audio_file) { perror("Error fopen line 151: "); exit(1); }

	/* adapt file length to new buffer length */
	g_wav_header.flength = g_wav_header.dlength + 44;
	bytes_written = fwrite(&g_wav_header, sizeof(t_wav), 1, g_inst.audio_file);
	if (bytes_written < 0)
	{ perror("bytes_written is < 0;"); exit(1); }

	/* volume is too low when recording for some reason .. */
	adjust_volume_for_file(volume_f);

	bytes_written += fwrite(g_buffer, 1, g_wav_header.dlength, g_inst.audio_file);
	if (bytes_written < 0)
	{ perror("bytes_written is < 0;"); exit(1); }

	fclose(g_inst.audio_file);

	printf("file_size is: %fKB\n", (double) g_wav_header.flength/1000);
	printf("data_size is: %fKB\n", (double) g_wav_header.dlength/1000);
}
