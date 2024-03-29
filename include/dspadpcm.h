#ifndef __DSPADPCM_H__
#define __DSPADPCM_H__

/*---------------------------------------------------------------------------*
    ADPCM info passed to the caller
 *---------------------------------------------------------------------------*/
typedef struct {
	/* start context */
	s16	coef[16];
	u16	gain;
	u16	pred_scale;
	s16	yn1;
	s16	yn2;

	/* loop context */
	u16	loop_pred_scale;
	s16	loop_yn1;
	s16	loop_yn2;
} ADPCMINFO;


/*---------------------------------------------------------------------------*
    exported functions
 *---------------------------------------------------------------------------*/
u32	getBytesForAdpcmBuffer	(u32 samples);
u32	getBytesForAdpcmSamples	(u32 samples);
u32	getBytesForPcmBuffer	(u32 samples);
u32	getBytesForPcmSamples	(u32 samples);
u32	getNibbleAddress	(u32 samples);
u32	getNibblesForNSamples	(u32 samples);
u32	getSampleForAdpcmNibble	(u32 nibble);
u32	getBytesForAdpcmInfo	(void);

void encode(
	s16*		src,	/* location of source samples (16bit PCM signed little endian) */
	u8*		dst,	/* location of destination buffer */
	ADPCMINFO*	cxt,	/* location of adpcm info */
	u32		samples	/* number of samples to encode */
);

void decode(
	u8*		src,	/* location of encoded source samples */
	s16*		dst,	/* location of destination buffer (16 bits / sample) */
	ADPCMINFO*	cxt,	/* location of adpcm info */
	u32		samples	/* number of samples to decode */
);

void getLoopContext(
	u8*		src,	/* location of ADPCM buffer in RAM */
	ADPCMINFO*	cxt,	/* location of adpcminfo */
	u32		samples	/* samples to desired context */
);

#endif
