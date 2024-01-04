#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "endian.h"
#include "dspadpcm.h"
#include "soundfile.h"
#include "dspheader.h"

char*	input_path  = NULL; /* path to input file */
char*	output_path = NULL; /* path to output file */
char*	coef_path   = NULL; /* path to DSPADPCMHEADER text dump file */

FILE*	input_file;
FILE*	output_file;
FILE*	coef_file;

BOOL	loop_flag;     /* TRUE if loop points were specified on commandline */
BOOL	encode_flag;   /* TRUE for encoding */
BOOL	decode_flag;   /* TRUE for decoding */
BOOL	coef_flag;     /* TRUE for text dump of DSPADPCMHEADER to file */
BOOL	verbose_flag;  /* TRUE if user specified verbose mode */
BOOL	ea_flag;       /* TRUE if user specified end address */

u32	loopStart;     /* user specified loop start sample */
u32	loopEnd;       /* user specified loop end sample */
u32	sampleEndAddr; /* user specified end address */

#define	DECODE_WAV	0
#define	DECODE_AIFF	1

u32	decodeFormat;  /* user specified decode fromat */


/*--------------------------------------------------------------------------*/
void print_banner(void)
{
	printf("\n"
	       "DSPADPCM v2.3 - DSP-ADPCM encoder\n"
	       "Copyright 2001 Nintendo. All rights reserved.\n\n");
}


/*--------------------------------------------------------------------------*/
void print_usage(void)
{
	printf("\n"
	       "Usage:\n"
	       "\n"
	       "DSPADPCM -<mode> <inputfile> [<outputfile>] [-opt<argument>]\n"
	       "\n"
	       "Where:\n"
	       "   <mode>.............E for Encode, D for Decode (required)\n"
	       "   <inputfile>........Input file (required)\n"
	       "   [<outputfile>].....Output file (optional)\n"
	       "\n"
	       "Options are:\n"
	       "   -c<coeffile>.......Text dump of coefficients and other data\n"
	       "   -l<start-end>......Sound effect is looped; 'start' is first sample\n"
	       "                      in loop, counting from zero. 'end' is the last\n"
	       "                      sample in the loop, also counting from zero.\n"
	       "   -a<end addr>.......For non-looped sound effects; last sample in\n"
	       "                      the sound effect, counting from zero.\n"
	       "   -h.................This help text.\n"
	       "   -v.................Verbose mode.\n"
	       "   -f.................Decode to AIFF.\n"
	       "   -w.................Decode to WAV (default).\n"
	       "\n"
	       "\n"
	       "This tool generates DSPADPCM data from MONO, 16-bit PCM WAV or AIFF\n"
	       "files. The DSPADPCM output file (*.dsp) has a %lu byte data header\n"
	       "which contains the ADPCM coefficients, loop context (if any),\n"
	       "and other sample info. The format of this header is described\n"
	       "in the DSPADPCM documentation.\n\n", sizeof(DSPADPCMHEADER));
}


/*--------------------------------------------------------------------------*/
void init(void)
{
	input_file   = NULL;
	output_file  = NULL;
	coef_file    = NULL;
	decodeFormat = DECODE_WAV;
}


/*--------------------------------------------------------------------------*/
void clean_up(void)
{
	if(input_file) {
		fclose(input_file);
	}

	if(output_file) {
		fclose(output_file);
	}

	if(coef_file) {
		fclose(coef_file);
	}

	if(output_path) {
		free(output_path);
	}

	if(coef_path) {
		free(coef_path);
	}
}


/*--------------------------------------------------------------------------*/
void coefficients(u16* p)
{
	int i;

	for(i = 0; i < 16; i++) {
		printf("[%d]: 0x%04X\n", i, *p++ & 0xFFFF);
	}
}


/*--------------------------------------------------------------------------
 * findbase() - returns pointer to first character of file basename
 *--------------------------------------------------------------------------*/
char* findbase(char* string)
{
	char* ptr = string;

	while(*ptr++); /* find end of string */

	while((--ptr >= string) && (*ptr != '\\') && (*ptr != '/') && (*ptr != ':'));

	return ++ptr;
}


/*--------------------------------------------------------------------------
 * findext() - returns pointer to start of file extension (including '.')
 *--------------------------------------------------------------------------*/
char* findext(char* string)
{
	char* ptr = string;
	char* end;

	while(*ptr++); /* find end of string */

	end = ptr;

	while((--ptr >= string) && (*ptr != '.'));

	if(ptr <= string) {
		return end;
	} else {
		return ptr;
	}
}


/*--------------------------------------------------------------------------
 * parse_args() - parse the command line arguments
 *--------------------------------------------------------------------------*/
BOOL parse_args(int argc, char* argv[])
{
	u16 i;

	if(argc < 2) {
		/* must specify input file at least */
		printf("\nERROR: Missing parameter\n");

		print_usage();

		return FALSE;
	}

	loop_flag    = FALSE;
	encode_flag  = FALSE;
	decode_flag  = FALSE;
	coef_flag    = FALSE;
	verbose_flag = FALSE;
	ea_flag      = FALSE;

	loopStart     = 0xFFFFFFFF;
	loopEnd       = 0xFFFFFFFF;
	sampleEndAddr = 0x00000000;
	for(i = 1; (i < argc); i++) {
		switch(argv[i][0]) {
		case '?':
			/* user is confused */
			return FALSE;

		case '-':
		case '/':
			/* case '\\': */
			switch(argv[i][1]) {
			case 'f':
			case 'F':
				decodeFormat = DECODE_AIFF;
				break;

			case 'w':
			case 'W':
				decodeFormat = DECODE_WAV;
				break;

			case 'e':
			case 'E':
				if(decode_flag) {
					/* user already asserted decode mode */
					printf("\nERROR: Decode flag already asserted!\n");
					return FALSE;
				} else {
					encode_flag = TRUE;
				}
				break;

			case 'D':
			case 'd':
				if(encode_flag) {
					/* user already asserted encode mode */
					printf("\nERROR: Encode flag already asserted!\n");
					return FALSE;
				} else {
					decode_flag = TRUE;
				}
				break;

			case 'l':
			case 'L':
				loop_flag = TRUE;
				if(sscanf(&argv[i][2], "%d-%d", &loopStart, &loopEnd) == EOF) {
					printf("\nERROR: Unable to parse loop points '%s'\n", &argv[i][2]);
					return FALSE;
				}
				break;

			case '?':
			case 'H':
			case 'h':
				print_usage();
				exit(0);

			case 'a':
			case 'A':
				if(sscanf(&argv[i][2], "%d", &sampleEndAddr) == EOF) {
					printf("\nERROR: Invalid sample end address.\n");
					return FALSE;
				}
				ea_flag = TRUE;
				break;

			case 'c':
			case 'C':
				/* specify coefficient output file */
				if(argv[i][2]) {
					coef_path = strdup(&argv[i][2]);
				}
				coef_flag = TRUE;
				break;

			case 'v':
			case 'V':
				verbose_flag = TRUE;
				break;

			default:
				/* unknown switch */
				if(argv[i][1]) {
					printf("\nERROR: Unknown switch '%c'\n", argv[i][1]);
				} else {
					printf("\nERROR: Unknown switch\n");
				}
				return FALSE;

				break;
			} /* end switch */
			break;

		default:
			if(!input_path) {
				/* input path not specified yet, so argument must be the input path */
				input_path = argv[i];
			} else if(!output_path) {
				/* an input_path has been specified, so this must be the output */
				output_path = strdup(argv[i]);
			} else {
				/* unknown unswitched argument */
				printf("\nERROR: Extraneous argument '%s'\n", argv[i]);
				return FALSE;
			}
			break;
		}
	}

	/* now perform sanity check */
	if(!input_path) {
		/* no input file specified! */
		printf("\nERROR: No input file specified!\n");
		return FALSE;
	}

	if(!output_path) {
		/* output path not specified yet, use default */
		output_path = (char*) malloc(strlen(input_path) + 4);
		strcpy(output_path, findbase(input_path));

		if(decode_flag) {
			if(decodeFormat != DECODE_AIFF) {
				/* add default extension of '.wav' */
				strcpy(findext(output_path), ".wav");
			} else {
				/* add default extension of '.aif' */
				strcpy(findext(output_path), ".aif");
			}
		} else if(encode_flag) {
			/* add default extension of '.dsp' */
			strcpy(findext(output_path), ".dsp");
		} else {
			printf("\nERROR: MODE directive not specified!\n");
			return FALSE;
		}
	}

	/* NOTE: there was a bug in the original dspadpcm which caused it to
	 * always generate a coef file */
	if(!coef_path && coef_flag) {
		/* coefficient output path not specified, use default */
		coef_path = (char*) malloc(strlen(input_path) + 4);
		strcpy(coef_path, findbase(input_path));

		/* add default extension of '.txt' */
		strcpy(findext(coef_path), ".txt");
	}

#define	ARAM_MAX_RANGE	0x4000000	/* 64MB of ARAM - in my dreams */

	if(loop_flag) {
		if(loopStart > loopEnd) {
			printf("\nWARNING: loop-start is greater than loop-end\n");
		}

		if(loopStart > ARAM_MAX_RANGE) {
			printf("\nWARNING: loop-start is beyond valid ARAM range! (0x%08X)\n", loopStart);
		}

		if(loopEnd > ARAM_MAX_RANGE) {
			printf("\nWARNING: loop-end is beyond valid ARAM range! (0x%08X)\n", loopEnd);
		}

		if(ea_flag) {
			printf("\nWARNING: '-a' argument ignored for looped sample.\n");
		}
	}

	if(sampleEndAddr > ARAM_MAX_RANGE) {
		printf("\nWARNING: sample-end address is beyond valid ARAM range! (0x%08X)\n", sampleEndAddr);
	}

	if (verbose_flag) {
		printf("\tinput file : '%s'\n", input_path);
		printf("\toutput file: '%s'\n", output_path);
		printf("\tcoef file  : '%s'\n", coef_path);
		printf("\n");
	}

#if 0
	/* debug */
	printf("\n****************************\n");

	printf("input_path : '%s'\n", input_path);
	printf("output_path: '%s'\n", output_path);
	printf("coef_path  : '%s'\n", coef_path);
	printf("decode_path: '%s'\n", decode_path);

	printf("\n");

	printf("loopStart: %d\n", loopStart);
	printf("loopEnd  : %d\n", loopEnd);

	printf("\n");

	/* exit(0); */
#endif

	return TRUE;

}


/*--------------------------------------------------------------------------
 *      dump DSPHEADER to specified file
 *--------------------------------------------------------------------------*/
void dump_header(DSPADPCMHEADER* h, FILE* handle)
{
	u16 i;
	u16 j;

	fprintf(handle, "\n");

	fprintf(handle, "Header size: %lu bytes\n\n", sizeof(DSPADPCMHEADER));

	fprintf(handle, "Sample     : '%s'\n", input_path);
	fprintf(handle, "Length     : %d samples\n", reverse_endian_32(h->num_samples));
	fprintf(handle, "Num nibbles: %d ADPCM nibbles\n", reverse_endian_32(h->num_adpcm_nibbles));

	fprintf(handle, "Sample Rate: %d Hz\n", reverse_endian_32(h->sample_rate));
	fprintf(handle, "Loop Flag  : %s\n", reverse_endian_16(h->loop_flag) == VOICE_TYPE_LOOPED ? "LOOPED" : "NOT LOOPED");

	fprintf(handle, "\n");

	fprintf(handle, "Start Addr : 0x%08X + ARAM_offset (ADPCM nibble mode)\n", reverse_endian_32(h->sa));
	fprintf(handle, "End Addr   : 0x%08X + ARAM_offset (ADPCM nibble mode)\n", reverse_endian_32(h->ea));
	fprintf(handle, "Curr Addr  : 0x%08X + ARAM_offset (ADPCM nibble mode)\n", reverse_endian_32(h->ca));

	fprintf(handle, "\n");

	for(i = 0, j = 0; i < 16; i += 2, j++) {
		fprintf(handle, "a1[%d]: 0x%04X a2[%d]: 0x%04X \n", j,
				reverse_endian_16(h->coef[i]), j,
				reverse_endian_16(h->coef[i + 1]));
	}

	fprintf(handle, "\n");

	fprintf(handle, "Gain      : 0x%04X\n", reverse_endian_16(h->gain));
	fprintf(handle, "Pred/Scale: 0x%04X\n", reverse_endian_16(h->ps));
	fprintf(handle, "y[n-1]    : 0x%04X\n", reverse_endian_16(h->yn1));
	fprintf(handle, "y[n-2]    : 0x%04X\n", reverse_endian_16(h->yn2));

	fprintf(handle, "\n");

	fprintf(handle, "Loop Pred/Scale: 0x%04X\n", reverse_endian_16(h->lps));
	fprintf(handle, "Loop y[n-1]    : 0x%04X\n", reverse_endian_16(h->lyn1));
	fprintf(handle, "Loop y[n-2]    : 0x%04X\n", reverse_endian_16(h->lyn2));
}


/*--------------------------------------------------------------------------*/
void encode_soundfile(char* path, SOUNDINFO* soundinfo)
{
	DSPADPCMHEADER dspadpcmheader;
	ADPCMINFO adpcminfo;
	void *soundbuffer, *outputbuffer;

	if(verbose_flag) {
		printf(" Done.\nGetting sound samples...");
	}

	soundbuffer  = malloc(soundinfo->bufferLength);
	outputbuffer = malloc(getBytesForAdpcmBuffer(soundinfo->samples));

	if(!soundbuffer || !outputbuffer) {
		if(soundbuffer) {
			free(soundbuffer);
		}
		if(outputbuffer) {
			free(outputbuffer);
		}

		printf("Cannot allocate buffers for encode!\n");
		return;
	}

	memset(&dspadpcmheader, 0, sizeof(DSPADPCMHEADER));

	dspadpcmheader.num_samples       = reverse_endian_32(soundinfo->samples);
	dspadpcmheader.num_adpcm_nibbles = reverse_endian_32(getNibblesForNSamples(soundinfo->samples));
	dspadpcmheader.sample_rate       = reverse_endian_32(soundinfo->sampleRate);

	/* if the user specified loop points on the commandline use them */
	/* or else look for loop points in the adpcminfo */
	if(loop_flag) {
		u32 nibbleLoopStart, nibbleLoopEnd, nibbleCurrent;

		nibbleLoopStart = getNibbleAddress(loopStart);
		nibbleLoopEnd   = getNibbleAddress(loopEnd);
		nibbleCurrent   = getNibbleAddress(0);

		dspadpcmheader.loop_flag = reverse_endian_16(VOICE_TYPE_LOOPED);
		dspadpcmheader.format    = reverse_endian_16(DEC_MODE_ADPCM);
		dspadpcmheader.sa        = reverse_endian_32(nibbleLoopStart);
		dspadpcmheader.ea        = reverse_endian_32(nibbleLoopEnd);
		dspadpcmheader.ca        = reverse_endian_32(nibbleCurrent);
	} else if(soundinfo->loopEnd) { /* the sound info has loops form AIFF */
		u32 nibbleLoopStart, nibbleLoopEnd, nibbleCurrent;

		nibbleLoopStart = getNibbleAddress(soundinfo->loopStart);
		nibbleLoopEnd   = getNibbleAddress(soundinfo->loopEnd - 1); /* AIFF loop end is 1 based */
		nibbleCurrent   = getNibbleAddress(0);

		dspadpcmheader.loop_flag = reverse_endian_16(VOICE_TYPE_LOOPED);
		dspadpcmheader.format    = reverse_endian_16(DEC_MODE_ADPCM);
		dspadpcmheader.sa        = reverse_endian_32(nibbleLoopStart);
		dspadpcmheader.ea        = reverse_endian_32(nibbleLoopEnd);
		dspadpcmheader.ca        = reverse_endian_32(nibbleCurrent);
	} else { /* not looped */
		u32 nibbleLoopStart, nibbleLoopEnd, nibbleCurrent;

		nibbleLoopStart = getNibbleAddress(0);

		/* if the user specified end address use it */
		if(ea_flag) {
			nibbleLoopEnd = getNibbleAddress(sampleEndAddr);
		} else {
			nibbleLoopEnd = getNibbleAddress(soundinfo->samples - 1);
		}

		nibbleCurrent = getNibbleAddress(0);

		dspadpcmheader.loop_flag = reverse_endian_16(VOICE_TYPE_NOTLOOPED);
		dspadpcmheader.format    = reverse_endian_16(DEC_MODE_ADPCM);
		dspadpcmheader.sa        = reverse_endian_32(nibbleLoopStart);
		dspadpcmheader.ea        = reverse_endian_32(nibbleLoopEnd);
		dspadpcmheader.ca        = reverse_endian_32(nibbleCurrent);
	}

	getSoundSamples(path, soundinfo, soundbuffer);

	if(verbose_flag) {
		printf(" Done.\nEncoding samples...");
	}

	encode(soundbuffer, outputbuffer, &adpcminfo, soundinfo->samples);

	/* if the user specified loops get loop context */
	if(loop_flag) {
		getLoopContext(outputbuffer, &adpcminfo, loopStart);
	} else if(soundinfo->loopEnd) { /* see if the aiff file has loop points */
		getLoopContext(outputbuffer, &adpcminfo, soundinfo->loopStart);
	} else { /* no loop make sure loop context is 0 */
		adpcminfo.loop_pred_scale = adpcminfo.loop_yn1 = adpcminfo.loop_yn2 = 0;
	}

	/* put the adpcm info on the dsp_header */
	dspadpcmheader.coef[0]  = reverse_endian_16(adpcminfo.coef[0]);
	dspadpcmheader.coef[1]  = reverse_endian_16(adpcminfo.coef[1]);
	dspadpcmheader.coef[2]  = reverse_endian_16(adpcminfo.coef[2]);
	dspadpcmheader.coef[3]  = reverse_endian_16(adpcminfo.coef[3]);
	dspadpcmheader.coef[4]  = reverse_endian_16(adpcminfo.coef[4]);
	dspadpcmheader.coef[5]  = reverse_endian_16(adpcminfo.coef[5]);
	dspadpcmheader.coef[6]  = reverse_endian_16(adpcminfo.coef[6]);
	dspadpcmheader.coef[7]  = reverse_endian_16(adpcminfo.coef[7]);
	dspadpcmheader.coef[8]  = reverse_endian_16(adpcminfo.coef[8]);
	dspadpcmheader.coef[9]  = reverse_endian_16(adpcminfo.coef[9]);
	dspadpcmheader.coef[10] = reverse_endian_16(adpcminfo.coef[10]);
	dspadpcmheader.coef[11] = reverse_endian_16(adpcminfo.coef[11]);
	dspadpcmheader.coef[12] = reverse_endian_16(adpcminfo.coef[12]);
	dspadpcmheader.coef[13] = reverse_endian_16(adpcminfo.coef[13]);
	dspadpcmheader.coef[14] = reverse_endian_16(adpcminfo.coef[14]);
	dspadpcmheader.coef[15] = reverse_endian_16(adpcminfo.coef[15]);
	dspadpcmheader.gain     = reverse_endian_16(adpcminfo.gain);
	dspadpcmheader.ps       = reverse_endian_16(adpcminfo.pred_scale);
	dspadpcmheader.yn1      = reverse_endian_16(adpcminfo.yn1);
	dspadpcmheader.yn2      = reverse_endian_16(adpcminfo.yn2);
	dspadpcmheader.lps      = reverse_endian_16(adpcminfo.loop_pred_scale);
	dspadpcmheader.lyn1     = reverse_endian_16(adpcminfo.loop_yn1);
	dspadpcmheader.lyn2     = reverse_endian_16(adpcminfo.loop_yn2);

	if(verbose_flag) {
		printf(" Done.\nWriting DSPADPCM file...");
	}

	/* write the output file */
	fwrite(&dspadpcmheader, 1, sizeof(DSPADPCMHEADER), output_file);
	fwrite(outputbuffer, getBytesForAdpcmSamples(soundinfo->samples), sizeof(u8), output_file);

	if(coef_flag) {
		if(verbose_flag) {
			printf(" Done.\nWriting text dump file ...");
		}
		dump_header(&dspadpcmheader, coef_file);
	}

	if(verbose_flag) {
		printf(" Done.\n");
	}

	if(soundbuffer) {
		free(soundbuffer);
	}
	if(outputbuffer) {
		free(outputbuffer);
	}
}


/*--------------------------------------------------------------------------*/
void encode_input_file(void)
{
	SOUNDINFO       soundinfo;

	if(verbose_flag) {
		printf("Getting sound header...");
	}

	switch(getSoundInfo(input_path, &soundinfo)) {
	case SOUND_FILE_SUCCESS:
		if(soundinfo.bitsPerSample != 16) {
			printf("Sound file buffer not 16bit samples!\n");
			return;
		}

		if(soundinfo.channels != 1) {
			printf("Soundfile buffer not mono!\n");
			return;
		}

		encode_soundfile(input_path, &soundinfo);
		break;

	case SOUND_FILE_FORMAT_ERROR:
		printf("Sound file format not supported!\n");
		return;

	case SOUND_FILE_FOPEN_ERROR:
		printf("fopen error for sound file!\n");
		return;
	}
}


/*--------------------------------------------------------------------------*/
void decode_input_file(void)
{
	DSPADPCMHEADER dspadpcmheader;
	u32 samples, sampleRate;
	u8*  inputBuffer;
	s16* outputBuffer;

	if(verbose_flag) {
		printf("Getting DSPADPCM header...");
	}

	/* get DSPADPCMHEADER */
	fread(&dspadpcmheader, 1, sizeof(DSPADPCMHEADER), input_file);

	samples    = reverse_endian_32(dspadpcmheader.num_samples);
	sampleRate = reverse_endian_32(dspadpcmheader.sample_rate);

	/* allocate buffers */
	if((inputBuffer = malloc(getBytesForAdpcmSamples(samples))) != NULL) {
		if(verbose_flag) {
			printf(" Done.\nGetting ADPCM samples...");
		}

		if((outputBuffer = malloc(samples * 2)) != NULL) {
			SOUNDINFO soundinfo;
			ADPCMINFO adpcminfo;

			/* prepare adpcminfo for decoding */
			adpcminfo.coef[0]    = (s16) reverse_endian_16(dspadpcmheader.coef[0]);
			adpcminfo.coef[1]    = (s16) reverse_endian_16(dspadpcmheader.coef[1]);
			adpcminfo.coef[2]    = (s16) reverse_endian_16(dspadpcmheader.coef[2]);
			adpcminfo.coef[3]    = (s16) reverse_endian_16(dspadpcmheader.coef[3]);
			adpcminfo.coef[4]    = (s16) reverse_endian_16(dspadpcmheader.coef[4]);
			adpcminfo.coef[5]    = (s16) reverse_endian_16(dspadpcmheader.coef[5]);
			adpcminfo.coef[6]    = (s16) reverse_endian_16(dspadpcmheader.coef[6]);
			adpcminfo.coef[7]    = (s16) reverse_endian_16(dspadpcmheader.coef[7]);
			adpcminfo.coef[8]    = (s16) reverse_endian_16(dspadpcmheader.coef[8]);
			adpcminfo.coef[9]    = (s16) reverse_endian_16(dspadpcmheader.coef[9]);
			adpcminfo.coef[10]   = (s16) reverse_endian_16(dspadpcmheader.coef[10]);
			adpcminfo.coef[11]   = (s16) reverse_endian_16(dspadpcmheader.coef[11]);
			adpcminfo.coef[12]   = (s16) reverse_endian_16(dspadpcmheader.coef[12]);
			adpcminfo.coef[13]   = (s16) reverse_endian_16(dspadpcmheader.coef[13]);
			adpcminfo.coef[14]   = (s16) reverse_endian_16(dspadpcmheader.coef[14]);
			adpcminfo.coef[15]   = (s16) reverse_endian_16(dspadpcmheader.coef[15]);
			adpcminfo.gain       = reverse_endian_16(dspadpcmheader.gain);
			adpcminfo.pred_scale = 0;
			adpcminfo.yn1        = 0;
			adpcminfo.yn2        = 0;

			/* read adpcm samples into input buffer */
			fread(inputBuffer, getBytesForAdpcmSamples(samples), sizeof(u8), input_file);

			if(verbose_flag) {
				printf(" Done.\nDecoding samples...");
			}

			/* decode samples to buffer */
			decode(inputBuffer, outputBuffer, &adpcminfo, samples);

			soundinfo.bitsPerSample = 16;
			soundinfo.bufferLength  = samples * 2;
			soundinfo.channels      = 1;
			soundinfo.sampleRate    = sampleRate;
			soundinfo.samples       = samples;

			if(dspadpcmheader.loop_flag) {
				soundinfo.loopStart = getSampleForAdpcmNibble(reverse_endian_32(dspadpcmheader.sa));
				soundinfo.loopEnd   = getSampleForAdpcmNibble(reverse_endian_32(dspadpcmheader.ea)) + 1;
			} else {
				soundinfo.loopStart = 0;
				soundinfo.loopEnd   = 0;
			}

			if(verbose_flag) {
				printf(" Done.\nWriting sound file...");
			}

			switch(decodeFormat) {
			case DECODE_WAV:
				writeWaveFile(output_path, &soundinfo, outputBuffer);
				break;

			case DECODE_AIFF:
				writeAiffFile(output_path, &soundinfo, outputBuffer);
				break;
			}
		} else {
			printf("\nERROR: Cannot allocate output buffer!\n");
			clean_up();
			exit(1);
		}
	} else {
		printf("\nERROR: Cannot allocate input buffer!\n");
		clean_up();
		exit(1);
	}

	/* free buffers */
	if(inputBuffer) {
		free(inputBuffer);
	}
	if(outputBuffer) {
		free(outputBuffer);
	}

	/* see if we should write a coefficient file */
	if(coef_flag) {
		if(verbose_flag) {
			printf(" Done.\nWriting text dump file...");
		}
		dump_header(&dspadpcmheader, coef_file);
	}

	if(verbose_flag) {
		printf(" Done.\n");
	}
}


/*--------------------------------------------------------------------------*
 * main()
 *--------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	init();
	print_banner();

	if(!parse_args(argc, argv)) {
		/* don't print usage help, it just adds more junk to log files.
		 * print usage only for missing parameters or explicit requests
		 * for help. */
		return 1;
	}

	/* open files */
	if((input_file = fopen(input_path, "rb")) == NULL) {
		printf("\nERROR: Cannot open %s for reading!\n", input_path);
		clean_up();
		return 1;
	}

	if((output_file = fopen(output_path, "wb")) == NULL) {
		printf("\nERROR: Cannot open %s for writing!\n", output_path);
		clean_up();
		return 1;
	}

	if(coef_path && (coef_file = fopen(coef_path, "w")) == NULL) {
		printf("\nERROR: Cannot open %s for writing!\n", coef_path);
		clean_up();
		return 1;
	}

	/* encode or decode */
	if(encode_flag) {
		encode_input_file();
	}

	if(decode_flag) {
		decode_input_file();
	}

	/* clean up */
	clean_up();

	/* exit with a clean bill of health */
	return 0;
}
