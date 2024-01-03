#include <string.h>
#include "types.h"
#include "Nxadpcm.h"
#include "endian.h"
#include "dspadpcm.h"

#define	BUFFER_SIZE		1024
#define	ADPCM_BUFFER_SIZE	BYTES_PER_FRAME * BUFFER_SIZE
#define	PCM_BUFFER_SIZE		SAMPLES_PER_FRAME * BUFFER_SIZE
#define	ARAM_BUFFER_SIZE	BUFFER_SIZE * BYTES_PER_FRAME
#define	DMEM_BUFFER_SIZE	BUFFER_SIZE * SAMPLES_PER_FRAME

#define	MMX_FLAG		0x00800000
#define	SSE_FLAG		0x02000000


#if 0
static int check_SSE(void)
{
	int flag = 0;

	_asm {
		pushfd;
		pop		eax;		// eax = EFLAGS;
		xor		eax, 00200000h;
		push		eax;
		popfd;
		pushfd;
		pop		ebx;
		cmp		eax, ebx;
		jne		CHECK_END;	// Not support CPUID

		mov		eax, 0;
		cpuid;
		cmp		eax, 1;
		jl		CHECK_END;

		// Family/Model/Stepping/Feature
		mov		eax, 1;
		cpuid;
		// MMX/SSE
		test		edx, MMX_FLAG;	// Check MMX
		jz		CHECK_END;
		test		edx, SSE_FLAG;	// Check SSE
		jz		CHECK_END;
		mov		flag, 1;

CHECK_END:
	}
	return flag;
}
#else
static int check_SSE(void)
{
	return 0;
}
#endif


void encodeLoop(s16* input_, u32 samples, u8* dst, ADPCMINFO* cxt)
{
	int  i, j, framecnt;
	u32  outoffset, inoffset, insize;
	u16  ps;
	s16* pcmbuffer;
	u8*  adpcmbuffer;
	s16* input;
	u16 (*adpcmEncodeFrameFuncPtr)(s16*, u8*, s16*, u8);


	s16* coeftable = (s16*) cxt;
	s16 coef[16];

	if(check_SSE()) {
		adpcmEncodeFrameFuncPtr = adpcmEncodeFrame_SSE;
		for(i = 0; i < NUM_OF_COEFTABLE; i++) {
			coef[i * 2 + 0] = coeftable[i * 2 + 1];
			coef[i * 2 + 1] = coeftable[i * 2 + 0];
		}
		coeftable = coef;
	} else {
		adpcmEncodeFrameFuncPtr = adpcmEncodeFrame;
	}

	adpcmbuffer = (u8*)  malloc(ADPCM_BUFFER_SIZE + BYTES_PER_FRAME);
	pcmbuffer   = (s16*) malloc((PCM_BUFFER_SIZE + ORDER * MAX_CHANNELS) * sizeof(s16));

	input = input_;

	for(i = 0 ; i < ORDER; i++) {
		pcmbuffer[i + PCM_BUFFER_SIZE] = 0;
	}

	framecnt = 0;

	while(samples > 0) {
		outoffset = 0;

		/* Move last samples to head */
		for(i = 0 ; i < ORDER; i++) {
			pcmbuffer[i] = pcmbuffer[i + PCM_BUFFER_SIZE];
		}

		/* Read PCM data from input file */
		if(samples > PCM_BUFFER_SIZE) {
			insize = PCM_BUFFER_SIZE;
			samples -= PCM_BUFFER_SIZE;
		} else {
			insize = samples;

			for(j = 0 ; j < SAMPLES_PER_FRAME; j++) {
				if(insize + j >= PCM_BUFFER_SIZE) {
					break;
				}

				pcmbuffer[ORDER + insize + j] = 0;
			}

			samples = 0;
		}

		memcpy(pcmbuffer + ORDER, input, insize * sizeof(s16));
		input += insize;

		inoffset = 0;

		/* Encoding Loop */
		while(inoffset < insize) {
			ps = adpcmEncodeFrameFuncPtr(
					pcmbuffer + inoffset,
					adpcmbuffer + outoffset,
					coeftable,
					1);
			outoffset += BYTES_PER_FRAME;
			inoffset  += SAMPLES_PER_FRAME;

			if(!framecnt) {
				cxt->pred_scale = ps;
			}

			framecnt++;
		}

		memcpy(dst, adpcmbuffer, outoffset);
		dst += outoffset;
	}


	free(pcmbuffer);
	free(adpcmbuffer);
}
