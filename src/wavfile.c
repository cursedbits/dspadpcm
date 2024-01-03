#include <stdio.h>
#include <stdlib.h>
#include "wavfile.h"

const u32* str_riff = (const u32*) "RIFF";
const u32* str_wave = (const u32*) "WAVE";
const u32* str_fmt  = (const u32*) "fmt ";
const u32* str_data = (const u32*) "data";


/*--------------------------------------------------------------------------*/
void wavCreateHeader(WAVECHUNK* wc, int numOfSamples, int channel,
		int bitsPerSample, int frequency)
{
	u32 datasize, riffchunksize;
	u16 blocksize;


	*((u32*) wc->chunkId)      = *str_riff;
	*((u32*) wc->formType)     = *str_wave;
	*((u32*) wc->fmt.chunkId)  = *str_fmt;
	*((u32*) wc->data.chunkId) = *str_data;

	blocksize = channel * (bitsPerSample / 8);
	datasize  = blocksize * numOfSamples;
	riffchunksize = datasize + 36;

	wc->chunkSize = riffchunksize;

	wc->fmt.chunkSize = 16;
	wc->fmt.waveFormatType = 1;
	wc->fmt.channel = channel;
	wc->fmt.samplesPerSec = frequency;
	wc->fmt.bytesPerSec = frequency * blocksize;
	wc->fmt.blockSize = blocksize;
	wc->fmt.bitsPerSample = bitsPerSample;

	wc->data.chunkSize = datasize;
}


/*--------------------------------------------------------------------------*/
void wavWriteHeader(WAVECHUNK* wc, FILE* outfile)
{
	fwrite(wc, 1, sizeof(WAVECHUNK), outfile);
}


/*--------------------------------------------------------------------------*/
int wavReadHeader(WAVECHUNK* wc, FILE* infile)
{
	u32 d, riffchunksize, size;
	int state;

	state = FALSE;  

	if(fseek(infile, 0, SEEK_SET) != 0) {
		return FALSE;
	}

	fread(&d, 1, sizeof(u32), infile);

	if(d != *str_riff) {
		return FALSE;
	}

	*(u32*) wc->chunkId = d;

	fread(&riffchunksize, 1, sizeof(u32), infile);
	wc->chunkSize = riffchunksize;

	fread(&d, 1, sizeof(u32), infile);

	if(d != *str_wave) {
		return FALSE;
	}

	*(u32*) wc->formType = d;

	riffchunksize -= 4;

	while(riffchunksize > 8) {
		fread(&d, 1, sizeof(u32), infile);
		fread(&size, 1, sizeof(u32), infile);

		if(d == *str_fmt) {
			int remaining;

			*(u32*) wc->fmt.chunkId = d;
			wc->fmt.chunkSize = size;

			fread(&wc->fmt.waveFormatType, 1, sizeof(FMTCHUNK) - 8, infile);

			remaining = size - (sizeof(FMTCHUNK) - 8);

			if(remaining) {
				fseek(infile, remaining, SEEK_CUR);
			}

			riffchunksize -= (2 * sizeof(int)) + size;
		} else if (d == *str_data) {
			*(u32*) wc->data.chunkId = d;
			wc->data.chunkSize = size;

			state = TRUE;
			riffchunksize -= sizeof(DATACHUNK);

			break;
		} else {
			fseek(infile, size, SEEK_CUR);

			riffchunksize = riffchunksize - size - 8;
		}      
	}

	return state;
}


/*--------------------------------------------------------------------------*/
u32 wavGetSamples(WAVECHUNK* wc)
{
	u32 a, bps;

	bps = wc->fmt.channel * wc->fmt.bitsPerSample / 8;
	a = wc->data.chunkSize / bps;

	return a;
}


/*--------------------------------------------------------------------------*/
u32 wavGetChannels(WAVECHUNK* wc)
{
	return (u32) wc->fmt.channel;
}


/*--------------------------------------------------------------------------*/
u32 wavGetBitsPerSample(WAVECHUNK* wc)
{
	return (u32) wc->fmt.bitsPerSample;  
}


/*--------------------------------------------------------------------------*/
u32 wavGetSampleRate(WAVECHUNK* wc)
{
	return (u32) wc->fmt.samplesPerSec;
}
