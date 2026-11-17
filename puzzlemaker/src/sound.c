
#ifdef ENABLE_SOUND
#include <stdlib.h>
#include <time.h>
#define MINIAUDIO_IMPLEMENTATION

#include "miniaudio/miniaudio.h"

#include <stdio.h>

#define SAMPLE_FORMAT ma_format_f32
#define CHANNEL_COUNT 2
#define SAMPLE_RATE 48000

ma_device device;
int decoderCount;
ma_decoder* decoders;
char* decodersAtEnd;

char areAllDecodersAtEnd()
{
	for (int i = 0; i < decoderCount; i++)
	{
		if (decodersAtEnd[i] == 0)
			return 0;
	}

	return 1;
}

unsigned int mixframes(ma_decoder* decoder, float* output, unsigned int frameCount)
{
	ma_result result;
	float temp[4096];
	unsigned int tempCapInFrames = 4096 / CHANNEL_COUNT;
	unsigned int totalFramesRead = 0;

	while (totalFramesRead < frameCount)
	{
		unsigned long long framesReadThisIteration;
		unsigned int totalFramesRemaining = frameCount - totalFramesRead;
		unsigned int framesToReadThisIteration = tempCapInFrames;
		if (framesToReadThisIteration > totalFramesRemaining)
			framesToReadThisIteration = totalFramesRemaining;

		result = ma_decoder_read_pcm_frames(decoder, temp, framesToReadThisIteration, &framesReadThisIteration);
		if (result != MA_SUCCESS || framesReadThisIteration == 0)
			break;

		for (unsigned long long sample = 0; sample < framesReadThisIteration * CHANNEL_COUNT; sample++)
			output[totalFramesRead * CHANNEL_COUNT + sample] += temp[sample];

		totalFramesRead += (ma_uint32)framesReadThisIteration;

		if (framesReadThisIteration < (ma_uint32)framesToReadThisIteration)
			break;
	}

	return totalFramesRead;
}

void data_callback(ma_device* device, void* output, const void* input, ma_uint32 frameCount)
{
	float* pOutputF32 = (float*)output;

	for (int i = 0; i < decoderCount; i++)
	{
		if (!decodersAtEnd[i])
		{
			unsigned int framesRead = mixframes(&decoders[i], output, frameCount);
			if (framesRead < frameCount)
				decodersAtEnd[i] = 1;
		}
	}
}

void initSounds()
{
	ma_result result;
	ma_decoder_config decoderConfig;
	ma_device_config deviceConfig;

	decoderCount = 2;
	decoders = (ma_decoder*)malloc(sizeof(ma_decoder) * decoderCount);
	decodersAtEnd = (char*)malloc(sizeof(char) * decoderCount);

	decoderConfig = ma_decoder_config_init(SAMPLE_FORMAT, CHANNEL_COUNT, SAMPLE_RATE);

	char* sounds[2] = {"carve2.wav", "carve3.wav"};

	for (int i = 0; i < decoderCount; i++)
	{
		result = ma_decoder_init_file(sounds[i], &decoderConfig, &decoders[i]);
		if (result != MA_SUCCESS)
		{
			for (int j = 0; j < i; j++)
				ma_decoder_uninit(&decoders[j]);
			free(decoders);
			free(decodersAtEnd);

			printf("Failed to load %s.\n", sounds[i]);
			return;
		}
		decodersAtEnd[i] = 1;
	}

	deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.format = SAMPLE_FORMAT;
	deviceConfig.playback.channels = CHANNEL_COUNT;
	deviceConfig.sampleRate = SAMPLE_RATE;
	deviceConfig.dataCallback = data_callback;
	deviceConfig.pUserData = NULL;

	if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS)
	{
		for (int i = 0; i < decoderCount; i++)
			ma_decoder_uninit(&decoders[i]);
		free(decoders);
		free(decodersAtEnd);

		printf("Failed to open playback device.\n");
		return;
	}

	if (ma_device_start(&device) != MA_SUCCESS)
	{
		ma_device_uninit(&device);
		for (int i = 0; i < decoderCount; i++)
			ma_decoder_uninit(&decoders[i]);
		free(decoders);
		free(decodersAtEnd);

		printf("Failed to start playback device.\n");
		return;
	}

	srand(time(0));
}

void playVoxelSfx()
{
	int r = rand() % 2;
	for (int i = 0; i < decoderCount; i++)
		decodersAtEnd[i] = 1;
	ma_decoder_seek_to_pcm_frame(&decoders[r], 0);
	decodersAtEnd[r] = 0;
}

void shutdownSounds()
{
	ma_device_uninit(&device);

	for (int i = 0; i < decoderCount; i++)
		ma_decoder_uninit(&decoders[i]);
	free(decoders);
	free(decodersAtEnd);
}
#else
void initSounds()
{
}
void playVoxelSfx()
{
}
void shutdownSounds()
{
}
#endif
