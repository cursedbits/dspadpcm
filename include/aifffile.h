#ifndef __AIFFFILE_H__
#define __AIFFFILE_H__

#include "types.h"
#include "chunkname.h"

/*--------------------------------------------------------------------------*
    AIFF chunk names
 *--------------------------------------------------------------------------*/
#define CHUNK_FORM  chunk_name('F','O','R','M')
#define CHUNK_AIFF  chunk_name('A','I','F','F')    
#define CHUNK_AIFC  chunk_name('A','I','F','C')    
#define CHUNK_FVER  chunk_name('F','V','E','R')
#define CHUNK_COMM  chunk_name('C','O','M','M')
#define CHUNK_SSND  chunk_name('S','S','N','D')
#define CHUNK_MARK  chunk_name('M','A','R','K')
#define CHUNK_COMT  chunk_name('C','O','M','T')
#define CHUNK_INST  chunk_name('I','N','S','T')
#define CHUNK_MIDI  chunk_name('M','I','D','I')
#define CHUNK_AESD  chunk_name('A','E','S','D')
#define CHUNK_APPL  chunk_name('A','P','P','L')
#define CHUNK_NAME  chunk_name('N','A','M','E')
#define CHUNK_AUTH  chunk_name('A','U','T','H')
#define CHUNK_COPY  chunk_name('(','c',')',' ')
#define CHUNK_ANNO  chunk_name('A','N','N','O')

typedef struct {
	u8	chunk[4];
	u8	bytes[4];
	u8	channels[2];
	u8	samples[4];
	u8	bitsPerSample[2];
	u8	samplesPerSec[10];
} AIFFCOMM;

typedef struct {
	u8	chunk[4];
	u8	bytes[4];
	u8	normalKey;
	u8	detune;
	u8	lowKey;
	u8	hiKey;
	u8	loVel;
	u8	hiVel;
	u8	gain[2];
	u8	playMode0[2];
	u8	begLoop0[2];
	u8	endLoop0[2];
	u8	playMode1[2];
	u8	begLoop1[2];
	u8	endLoop1[2];
} AIFFINST;

typedef struct {
	u8	chunk[4];
	u8	bytes[4];

	u8	count[2];

	u8	id0[2];
	u8	position0[4];
	u8	ch0[10];

	u8	id1[2];
	u8	position1[4];
	u8	ch1[11];
} AIFFMARK;

typedef struct {
	AIFFCOMM comm;
	AIFFINST inst;
	AIFFMARK mark;
} AIFFINFO;

void aiffCreateHeader(
	AIFFINFO*	aiffinfo,
	int		channels,
	int		samples,
	int		bitsPerSample,
	int		sampleRate
);

void aiffWriteHeader(
	AIFFINFO*	aiffinfo,
	FILE*		outfile,
	int		loopStart,
	int		loopEnd,
	int		bitsPerSample,
	int		samples,
	int		channels
);

void	aiffCreateMark(AIFFINFO* aiffinfo, u32 startloop, u32 endloop);
void	aiffWriteMark(AIFFINFO* aiffinfo, FILE* outfile);

int	aiffReadHeader(AIFFINFO* aiffinfo, FILE* infile);
int	aiffGetChannels(AIFFINFO* aiffinfo);
int	aiffGetSamples(AIFFINFO* aiffinfo);
int	aiffGetSampleRate(AIFFINFO* aiffinfo);
int	aiffGetBitsPerSample(AIFFINFO* aiffinfo);
int	aiffGetLoopStart(AIFFINFO* aiffinfo);
int	aiffGetLoopEnd(AIFFINFO* aiffinfo);

#endif
