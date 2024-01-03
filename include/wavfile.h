#ifndef __WAVFILE_H__
#define __WAVFILE_H__

#include "types.h"
#include "chunkname.h"

/*--------------------------------------------------------------------------*
    RIFF chunk names
 *--------------------------------------------------------------------------*/
#define	CHUNK_RIFF	chunk_name('R','I','F','F')
#define	CHUNK_WAVE	chunk_name('W','A','V','E')

typedef struct {
	char		chunkId[4];
	u32		chunkSize;
	u16		waveFormatType;
	u16		channel;
	u32		samplesPerSec;
	u32		bytesPerSec;
	u16		blockSize;
	u16		bitsPerSample;
} FMTCHUNK;

typedef struct {
	char		chunkId[4];
	u32		chunkSize;
} DATACHUNK;

typedef struct {
	char		chunkId[4];
	u32		chunkSize;
	char		formType[4];
	FMTCHUNK	fmt;
	DATACHUNK	data;
} WAVECHUNK;


void	wavCreateHeader(WAVECHUNK* wc, int numOfSamples, int channel,
		int bitsPerSample, int frequency);
void	wavWriteHeader(WAVECHUNK* wc, FILE* outfile);
int	wavReadHeader(WAVECHUNK* wc, FILE* infile);
u32	wavGetSamples(WAVECHUNK* wc);
u32	wavGetChannels(WAVECHUNK* wc); 
u32	wavGetBitsPerSample(WAVECHUNK* wc);  
u32	wavGetSampleRate(WAVECHUNK* wc);

#endif
