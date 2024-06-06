#include "editor.h"

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
		int32_t check_sample = (int32_t)(data[i] * factor);
		if (check_sample > INT16_MAX)
			check_sample = INT16_MAX;
		else if (check_sample < INT16_MIN)
			check_sample = INT16_MIN;
		data[i] = check_sample;
		i++;
	}
}

/* TODO: make appending file possible  */
void
save_file(FILE *file, char *file_name)
{
	static size_t	bytes_written;
	float			volume_f = 16.0f;

	g_inst.audio_file = fopen(file_name, "ab");
	if (!g_inst.audio_file) { perror("Error fopen line 151: "); exit(1); }

	/* adapt file length to new buffer length */
	g_wav_header.flength = g_wav_header.dlength + 44;
	bytes_written = fwrite(&g_wav_header, sizeof(t_wav), 1, g_inst.audio_file);
	if (bytes_written < 0)
	{ perror("bytes_written is < 0;"); exit(1); }

	int *print = (int *) g_buffer;
	adjust_volume_for_file(volume_f);

	bytes_written += fwrite(g_buffer, 1, g_wav_header.dlength, g_inst.audio_file);
	if (bytes_written < 0)
	{ perror("bytes_written is < 0;"); exit(1); }
	fclose(g_inst.audio_file);
}
