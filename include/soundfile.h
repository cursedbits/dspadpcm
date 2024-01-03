#ifndef __SOUNDFILE_H__
#define __SOUNDFILE_H__

/*--------------------------------------------------------------------------*
    Status
 *--------------------------------------------------------------------------*/
#define	SOUND_FILE_SUCCESS	0
#define	SOUND_FILE_FORMAT_ERROR	1
#define	SOUND_FILE_FOPEN_ERROR	2


/*--------------------------------------------------------------------------*
    SOUNDINFO struct
 *--------------------------------------------------------------------------*/
typedef struct {
	int	channels;	/* Number of channels */
	int	bitsPerSample;	/* Number of bits per sample */
	int	sampleRate;	/* Sample rate in Hz */
	int	samples;	/* Number for samples */
	int	loopStart;	/* 1 based sample index for loop start */
	int	loopEnd;	/* 1 based sample count for loop samples */
	int	bufferLength;	/* buffer length in bytes */
} SOUNDINFO;


/*--------------------------------------------------------------------------*
    Function prototypes
 *--------------------------------------------------------------------------*/
int	getSoundInfo	(char* path, SOUNDINFO* info);
int	getSoundSamples	(char* path, SOUNDINFO* info, void* dest);

int	writeWaveFile	(char* path, SOUNDINFO* info, void* samples);
int	writeAiffFile	(char* path, SOUNDINFO* info, void* samples);

#endif
