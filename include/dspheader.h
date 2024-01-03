#ifndef __DSPHEADER_H__
#define __DSPHEADER_H__

#define VOICE_TYPE_NOTLOOPED	0x0000	/* sample is not looped */
#define VOICE_TYPE_LOOPED	0x0001	/* sample is indeed looped */

#define DEC_MODE_ADPCM		0x0000	/* ADPCM mode */
#define DEC_MODE_PCM16		0x000A	/* 16-bit PCM mode */
#define DEC_MODE_PCM8		0x0009	/* 8-bit PCM mode (UNSIGNED) */

typedef struct {
	u32	num_samples;
	u32	num_adpcm_nibbles;
	u32	sample_rate;

	u16	loop_flag;
	u16	format;
	u32	sa;	/* loop start address */
	u32	ea;	/* loop end address */
	u32	ca;	/* current address */

	u16	coef[16];

	/* start context */
	u16	gain;   
	u16	ps;
	u16	yn1;
	u16	yn2;

	/* loop context */
	u16	lps;    
	u16	lyn1;
	u16	lyn2;

	u16	pad[11];
} DSPADPCMHEADER;

#endif
