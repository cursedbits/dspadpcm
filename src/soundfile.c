#include <stdio.h>
#include "types.h"
#include "soundfile.h"
#include "endian.h"
#include "aifffile.h"
#include "wavfile.h"


#define SOUND_FILE_WAVE 0         
#define SOUND_FILE_AIFF 1


/*--------------------------------------------------------------------------*/
int getAiffInfo(char* path, SOUNDINFO* soundinfo, void* buffer)
{
	FILE*    file;
	AIFFINFO aiffinfo;

	if(!(file = fopen(path, "rb"))) {
		return SOUND_FILE_FOPEN_ERROR;
	}

	aiffReadHeader(&aiffinfo, file);

	soundinfo->channels      = aiffGetChannels(&aiffinfo);
	soundinfo->bitsPerSample = aiffGetBitsPerSample(&aiffinfo);
	soundinfo->sampleRate    = aiffGetSampleRate(&aiffinfo);
	soundinfo->samples       = aiffGetSamples(&aiffinfo);
	soundinfo->loopStart     = aiffGetLoopStart(&aiffinfo);
	soundinfo->loopEnd       = aiffGetLoopEnd(&aiffinfo);
	soundinfo->bufferLength  = 0;

	switch(soundinfo->bitsPerSample) {
	case 8:
		soundinfo->bufferLength = soundinfo->samples * soundinfo->channels;
		break;

	case 16:
		soundinfo->bufferLength = soundinfo->samples * soundinfo->channels * 2;
		break;
	}

	if(buffer) {
		fread(buffer, soundinfo->bufferLength, 1, file);

		if(soundinfo->bitsPerSample == 16) {
			reverse_buffer_16(buffer, soundinfo->bufferLength / 2);
		}
	}

	fclose(file);

	return SOUND_FILE_SUCCESS;
}


/*--------------------------------------------------------------------------*/
int getWaveInfo(char* path, SOUNDINFO* soundinfo, void* buffer)
{
	FILE*     file;
	WAVECHUNK wavechunk;

	if(!(file = fopen(path, "rb"))) {
		return SOUND_FILE_FOPEN_ERROR;
	}

	wavReadHeader(&wavechunk, file);

	soundinfo->channels      = wavGetChannels(&wavechunk);
	soundinfo->bitsPerSample = wavGetBitsPerSample(&wavechunk);
	soundinfo->sampleRate    = wavGetSampleRate(&wavechunk);
	soundinfo->samples       = wavGetSamples(&wavechunk);
	soundinfo->loopStart     = 0;
	soundinfo->loopEnd       = 0;

	soundinfo->bufferLength  = 0;

	switch (soundinfo->bitsPerSample) {
	case 8:
		soundinfo->bufferLength = soundinfo->samples * soundinfo->channels;
		break;

	case 16:
		soundinfo->bufferLength = soundinfo->samples * soundinfo->channels * 2;
		break;
	}

	if(buffer) {
		fread(buffer, soundinfo->bufferLength, 1, file);

		if(soundinfo->bitsPerSample == 8) {
			int i;
			char* p = buffer;

			for(i = 0; i < soundinfo->bufferLength; i++) {
				*p = *p + 0x79;
				p++;
			}
		}
	}

	fclose(file);

	return SOUND_FILE_SUCCESS;
}


/*--------------------------------------------------------------------------*/
int getFileType(char* path, SOUNDINFO* soundinfo)
{
	FILE* file;
	u32 data, result;

	if(!(file = fopen(path, "rb"))) {
		return SOUND_FILE_FOPEN_ERROR;
	} else {
		result = SOUND_FILE_FORMAT_ERROR;
	}

	fread(&data, 1, sizeof(u32), file);

	switch(data) {
	case CHUNK_FORM:
		fread(&data, 1, sizeof(u32), file);
		fread(&data, 1, sizeof(u32), file);

		if(data == CHUNK_AIFF) {
			result = SOUND_FILE_AIFF;
		}

		break;

	case CHUNK_RIFF:
		fread(&data, 1, sizeof(u32), file);
		fread(&data, 1, sizeof(u32), file);

		if(data == CHUNK_WAVE) {
			result = SOUND_FILE_WAVE;
		}

		break;
	}

	fclose(file);

	return result;
}


/*--------------------------------------------------------------------------*/
int getSoundInfo(char* path, SOUNDINFO* soundinfo)
{
	u32 result = getFileType(path, soundinfo);

	switch(result) {
	case SOUND_FILE_AIFF:
		result = getAiffInfo(path, soundinfo, NULL);
		break;

	case SOUND_FILE_WAVE:
		result = getWaveInfo(path, soundinfo, NULL);
		break;
	}

	return result;
}


/*--------------------------------------------------------------------------*/
int getSoundSamples(char* path, SOUNDINFO* soundinfo, void* dest)
{
	u32 result = getFileType(path, soundinfo);

	switch(result) {
	case SOUND_FILE_AIFF:
		result = getAiffInfo(path, soundinfo, dest);
		break;

	case SOUND_FILE_WAVE:
		result = getWaveInfo(path, soundinfo, dest);
		break;
	}

	return result;
}


/*--------------------------------------------------------------------------*/
int writeWaveFile(char* path, SOUNDINFO* info, void* samples)
{
	WAVECHUNK wavechunk;
	FILE*     file;

	if(!(file = fopen(path, "wb"))) {
		return SOUND_FILE_FOPEN_ERROR;
	}

	wavCreateHeader(&wavechunk, info->samples, info->channels,
			info->bitsPerSample, info->sampleRate);

	wavWriteHeader(&wavechunk, file);

	fwrite(samples, info->bufferLength, sizeof(char), file);

	fclose(file);

	return SOUND_FILE_SUCCESS;
}


/*--------------------------------------------------------------------------*/
int writeAiffFile(char* path, SOUNDINFO* info, void* samples)
{
	AIFFINFO aiffinfo;
	FILE*    file;


	if(!(file = fopen(path, "wb"))) {
		return SOUND_FILE_FOPEN_ERROR;
	}

	aiffCreateHeader(&aiffinfo, info->channels, info->samples,
			info->bitsPerSample, info->sampleRate);

	aiffWriteHeader(&aiffinfo, file, info->loopStart, info->loopEnd,
			info->bitsPerSample, info->samples, info->channels);

	if(info->bitsPerSample == 16) {
		reverse_buffer_16(samples, info->bufferLength / 2);
	}

	fwrite(samples, info->bufferLength, sizeof(char), file);

	if(info->loopEnd) {
		aiffCreateMark(&aiffinfo, info->loopStart, info->loopEnd);
		aiffWriteMark(&aiffinfo, file);
	}

	fclose(file);

	return SOUND_FILE_SUCCESS;
}
