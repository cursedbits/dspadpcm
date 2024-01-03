#ifndef __NXADPCM_H__
#define __NXADPCM_H__

#include <stdio.h>
#include <stdlib.h>
#include "types.h"

/* #define DEBUGMSG */

#define	SAMPLES_PER_FRAME		14
#define	NIBBLES_PER_FRAME		16
#define	BYTES_PER_FRAME			8

#define	ORDER				2
#define	MAX_LEVEL			8
#define	MAX_SCALE			12
#define	MAX_CLIP			1

#define	MAX_NUM_OF_COEFTABLE		8
#define	NUM_OF_COEFTABLE		8
#define	COEF_SCALING_BIT		11
#define	COEF_SCALING			2048

#define MAX_CHANNELS			2

/*--------------------------------------------------------------------------*/
/* encframe.c */
u16 adpcmEncodeFrame(s16* inbuffer, u8* outbuffer, s16* coeftable, u8 step);

u16 adpcmEncodeFrame_SSE(s16* inbuffer, u8* outbuffer, s16* coeftable, u8 step);

#endif
