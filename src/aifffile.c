#include <stdio.h>
#include <stdlib.h>
#include "aifffile.h"
#include "endian.h"

const u32* str_form = (const u32*) "FORM";    
const u32* str_aiff = (const u32*) "AIFF";    
const u32* str_aifc = (const u32*) "AIFC";    
const u32* str_fver = (const u32*) "FVER";
const u32* str_comm = (const u32*) "COMM";
const u32* str_ssnd = (const u32*) "SSND";
const u32* str_mark = (const u32*) "MARK";
const u32* str_comt = (const u32*) "COMT";
const u32* str_inst = (const u32*) "INST";
const u32* str_midi = (const u32*) "MIDI";
const u32* str_aesd = (const u32*) "AESD";
const u32* str_appl = (const u32*) "APPL";
const u32* str_name = (const u32*) "NAME";
const u32* str_auth = (const u32*) "AUTH";
const u32* str_copy = (const u32*) "(c) ";
const u32* str_anno = (const u32*) "ANNO";


/*--------------------------------------------------------------------------*/
void aiffCreateHeader(AIFFINFO* aiffinfo, int channels, int samples,
		int bitsPerSample, int sampleRate)
{
	aiffinfo->comm.chunk[0]         = 'C';
	aiffinfo->comm.chunk[1]         = 'O';
	aiffinfo->comm.chunk[2]         = 'M';
	aiffinfo->comm.chunk[3]         = 'M';
	aiffinfo->comm.bytes[0]         = 0;
	aiffinfo->comm.bytes[1]         = 0;
	aiffinfo->comm.bytes[2]         = 0;
	aiffinfo->comm.bytes[3]         = 18;
	aiffinfo->comm.channels[0]      = (u8) (channels >> 8);
	aiffinfo->comm.channels[1]      = (u8) channels;
	aiffinfo->comm.samples[0]       = (u8) (samples >> 24);
	aiffinfo->comm.samples[1]       = (u8) (samples >> 16);
	aiffinfo->comm.samples[2]       = (u8) (samples >> 8);
	aiffinfo->comm.samples[3]       = (u8) samples;
	aiffinfo->comm.bitsPerSample[0] = (u8) (bitsPerSample >> 8);
	aiffinfo->comm.bitsPerSample[1] = (u8) bitsPerSample;
	aiffinfo->comm.samplesPerSec[0] = 0x40;
	aiffinfo->comm.samplesPerSec[1] = 0x1E;
	aiffinfo->comm.samplesPerSec[2] = (u8) (sampleRate >> 24) & 0x7F;
	aiffinfo->comm.samplesPerSec[3] = (u8) (sampleRate >> 16);
	aiffinfo->comm.samplesPerSec[4] = (u8) (sampleRate >> 8);
	aiffinfo->comm.samplesPerSec[5] = (u8) sampleRate;
	aiffinfo->comm.samplesPerSec[6] = 0x00;
	aiffinfo->comm.samplesPerSec[7] = 0x00;
	aiffinfo->comm.samplesPerSec[8] = 0x00;
	aiffinfo->comm.samplesPerSec[9] = 0x00;
}


/*--------------------------------------------------------------------------*/
void aiffWriteHeader(AIFFINFO* aiffinfo, FILE* outfile, int loopStart,
		int loopEnd, int bitsPerSample, int samples, int channels)
{
	u32 size;

	switch(bitsPerSample) {
	case 8:
		size =  reverse_endian_32(sizeof(AIFFINFO) +
				12 + /* AIFF SSND, n bytes */
				(samples * channels) + 8);
		break;

	case 16:
		size =  reverse_endian_32(sizeof(AIFFINFO) +
				12 + /* AIFF SSND, n bytes */
				(samples * channels * 2) + 8);
		break;
	default:
		abort();
		break;
	}

	if(loopEnd) {
		size += sizeof(AIFFMARK) + sizeof(AIFFINST);
	}

	fwrite("FORM", sizeof(char), 4, outfile);
	fwrite(&size, sizeof(char), 4, outfile);
	fwrite("AIFF", sizeof(char), 4, outfile);
	fwrite(&aiffinfo->comm, sizeof(AIFFCOMM), 1, outfile);
	fwrite("SSND", sizeof(char), 4, outfile);

	switch(bitsPerSample) {
	case 8:
		size = reverse_endian_32(
				(samples *	/* n samples */
				channels) +	/* n channels */
				8);		/* fist 8 bytes are 0 */
		break;

	case 16:
		size = reverse_endian_32(
				(samples *	/* n samples */
				2 *		/* 2 bytes per sample */
				channels) +	/* n channels */
				8);		/* fist 8 bytes are 0 */
		break;
	}

	fwrite(&size, sizeof(char), 4, outfile);

	size = 0;

	fwrite(&size, sizeof(char), 4, outfile);
	fwrite(&size, sizeof(char), 4, outfile);
}


/*--------------------------------------------------------------------------*/
void aiffCreateMark(AIFFINFO* aiffinfo, u32 startloop, u32 endloop)
{
	aiffinfo->inst.chunk[0]     = 'I';
	aiffinfo->inst.chunk[1]     = 'N';
	aiffinfo->inst.chunk[2]     = 'S';
	aiffinfo->inst.chunk[3]     = 'T';
	aiffinfo->inst.bytes[0]     = 0;
	aiffinfo->inst.bytes[1]     = 0;
	aiffinfo->inst.bytes[2]     = 0;
	aiffinfo->inst.bytes[3]     = 20;
	aiffinfo->inst.normalKey    = 64;
	aiffinfo->inst.detune       = 0;
	aiffinfo->inst.lowKey       = 0;
	aiffinfo->inst.hiKey        = 127;
	aiffinfo->inst.loVel        = 0;
	aiffinfo->inst.hiVel        = 127;
	aiffinfo->inst.gain[0]      = 0;
	aiffinfo->inst.gain[1]      = 0;
	aiffinfo->inst.playMode0[0] = 0;
	aiffinfo->inst.playMode0[1] = 1;
	aiffinfo->inst.begLoop0[0]  = 0;
	aiffinfo->inst.begLoop0[1]  = 0;
	aiffinfo->inst.endLoop0[0]  = 0;
	aiffinfo->inst.endLoop0[1]  = 1;
	aiffinfo->inst.playMode1[0] = 0;
	aiffinfo->inst.playMode1[1] = 1;
	aiffinfo->inst.begLoop1[0]  = 0;
	aiffinfo->inst.begLoop1[1]  = 0;
	aiffinfo->inst.endLoop1[0]  = 0;
	aiffinfo->inst.endLoop1[1]  = 0;

	aiffinfo->mark.chunk[0]     = 'M';
	aiffinfo->mark.chunk[1]     = 'A';
	aiffinfo->mark.chunk[2]     = 'R';
	aiffinfo->mark.chunk[3]     = 'K';
	aiffinfo->mark.bytes[0]     = 0;
	aiffinfo->mark.bytes[1]     = 0;
	aiffinfo->mark.bytes[2]     = 0;
	aiffinfo->mark.bytes[3]     = 35;
	aiffinfo->mark.count[0]     = 0;
	aiffinfo->mark.count[1]     = 2;
	aiffinfo->mark.id0[0]       = 0;
	aiffinfo->mark.id0[1]       = 0;
	aiffinfo->mark.position0[0] = (u8) (startloop >> 24);
	aiffinfo->mark.position0[1] = (u8) (startloop >> 16);
	aiffinfo->mark.position0[2] = (u8) (startloop >> 8);
	aiffinfo->mark.position0[3] = (u8) startloop;
	aiffinfo->mark.ch0[0]       = 0x08;
	aiffinfo->mark.ch0[1]       = 'b';
	aiffinfo->mark.ch0[2]       = 'e';
	aiffinfo->mark.ch0[3]       = 'g';
	aiffinfo->mark.ch0[4]       = ' ';
	aiffinfo->mark.ch0[5]       = 'l';
	aiffinfo->mark.ch0[6]       = 'o';
	aiffinfo->mark.ch0[7]       = 'o';
	aiffinfo->mark.ch0[8]       = 'p';
	aiffinfo->mark.ch0[9]       = 0;
	aiffinfo->mark.id1[0]       = 0;
	aiffinfo->mark.id1[1]       = 1;
	aiffinfo->mark.position1[0] = (u8) (endloop >> 24);
	aiffinfo->mark.position1[1] = (u8) (endloop >> 16);
	aiffinfo->mark.position1[2] = (u8) (endloop >> 8);
	aiffinfo->mark.position1[3] = (u8) endloop;
	aiffinfo->mark.ch1[0]       = 0x30;
	aiffinfo->mark.ch1[1]       = 0x08;
	aiffinfo->mark.ch1[2]       = 'e';
	aiffinfo->mark.ch1[3]       = 'n';
	aiffinfo->mark.ch1[4]       = 'd';
	aiffinfo->mark.ch1[5]       = ' ';
	aiffinfo->mark.ch1[6]       = 'l';
	aiffinfo->mark.ch1[7]       = 'o';
	aiffinfo->mark.ch1[8]       = 'o';
	aiffinfo->mark.ch1[9]       = 'p';
	aiffinfo->mark.ch1[10]      = 0;
}


/*--------------------------------------------------------------------------*/
void aiffWriteMark(AIFFINFO* aiffinfo, FILE* outfile)
{
	fwrite(&aiffinfo->inst, sizeof(AIFFINST), 1, outfile);
	fwrite(&aiffinfo->mark, sizeof(AIFFMARK), 1, outfile);
}


/*--------------------------------------------------------------------------*/
int aiffReadHeader(AIFFINFO* aiffinfo, FILE* infile)
{
	u32 chunk, length;
	u32 sample_position = 0; 

	if(fseek(infile, 0, SEEK_SET)) {
		return FALSE;
	}

	fread(&chunk, 1, sizeof(u32), infile);
	if(chunk != *str_form) {
		return FALSE;
	}

	fread(&length, 1, sizeof(u32), infile);
	fread(&chunk, 1, sizeof(u32), infile);

	if(chunk != *str_aiff) {
		return FALSE;
	}

	fread(&chunk, 1, sizeof(u32), infile);

	if(chunk != *str_comm) {
		return FALSE;
	}

	fread(&length, 1, sizeof(u32), infile);

	fread(&aiffinfo->comm.channels, 1, sizeof(u16), infile);
	fread(&aiffinfo->comm.samples, 1, sizeof(u32), infile);
	fread(&aiffinfo->comm.bitsPerSample, 1, sizeof(u16), infile);
	fread(&aiffinfo->comm.samplesPerSec[0], 10, sizeof(u8), infile);

	length = reverse_endian_32(length);
	length = 18 - length;

	if(length) {
		fseek(infile, length, SEEK_CUR);
	}

	/* initialize loop markers to 0 */
	aiffinfo->mark.position0[0] = 
	aiffinfo->mark.position0[1] = 
	aiffinfo->mark.position0[2] = 
	aiffinfo->mark.position0[3] = 
	aiffinfo->mark.position1[0] = 
	aiffinfo->mark.position1[1] = 
	aiffinfo->mark.position1[2] =
	aiffinfo->mark.position1[3] = 0;     

	/* get the read position up to the sample data, that's what the */
	/* caller is expecting.. I know... */
	while(fread(&chunk, 1, sizeof(u32), infile) == 4) {        
		u16 count;
		u32 i;

		/* length of chunk */
		fread(&length, 1, sizeof(u32), infile);
		length = reverse_endian_32(length);

		if(feof(infile)) {
			break;
		}

		switch(chunk) {
		case CHUNK_SSND:
			/* first 8 bytes of samples are garbage */
			fread(&chunk, 1, sizeof(u32), infile);
			fread(&chunk, 1, sizeof(u32), infile);

			/* save the position because we are going to go look for
			 * a loop markers */
			sample_position = ftell(infile);
			fseek(infile, length - 8, SEEK_CUR);

			break;

		case CHUNK_MARK:
			fread(&count, 1, sizeof(u16), infile); /* count, n markers */
			count = reverse_endian_16(count);
			length -= 2;

			i = 0;

			while(count) {
				u16 id;
				u32 position;
				u8  ch;

				fread(&id, 1, sizeof(u16), infile);
				fread(&position, 1, sizeof(u32), infile);

				id       = reverse_endian_16(id);
				position = reverse_endian_32(position);

				switch(i) {
				case 0:
					aiffinfo->mark.position0[0] = (u8) (position >> 24);
					aiffinfo->mark.position0[1] = (u8) (position >> 16);
					aiffinfo->mark.position0[2] = (u8) (position >> 8);
					aiffinfo->mark.position0[3] = (u8) position;
					/* printf("begin loop: %d\n", position); */
					break;

				case 1:
					aiffinfo->mark.position1[0] = (u8) (position >> 24);
					aiffinfo->mark.position1[1] = (u8) (position >> 16);
					aiffinfo->mark.position1[2] = (u8) (position >> 8);
					aiffinfo->mark.position1[3] = (u8) position;
					/* printf("end loop: %d\n", position); */
					break;
				}

				/* skip pstring */
				fread(&ch, 1, sizeof(u8), infile);
				fseek(infile, ch, SEEK_CUR);    

				if(ftell(infile) & 1) {
					fread(&ch, 1, sizeof(u8), infile);
				}

				count--;
				i++;
			}

			break;

		case CHUNK_INST: {
			u16 playmode;

			/* skip some stuff we don't care about */
			fseek(infile, 8, SEEK_CUR);
			fread(&playmode, 1, sizeof(u16), infile);
			playmode = reverse_endian_16(playmode);

			if(playmode != 1) {
				aiffinfo->mark.position0[0] = 0;
				aiffinfo->mark.position0[1] = 0;
				aiffinfo->mark.position0[2] = 0;
				aiffinfo->mark.position0[3] = 0;
				aiffinfo->mark.position1[0] = 0;
				aiffinfo->mark.position1[1] = 0;
				aiffinfo->mark.position1[2] = 0;
				aiffinfo->mark.position1[3] = 0;
			}

			fseek(infile, 10, SEEK_CUR);
			}

			break;

		default:
			fseek(infile, length, SEEK_CUR);

			break;
		}
	}

	/* put the read position back to where the samples are */
	fseek(infile, 0, SEEK_SET);
	fseek(infile, sample_position, SEEK_CUR);

	return TRUE;
}


/*--------------------------------------------------------------------------*/
int aiffGetChannels(AIFFINFO* aiffinfo)
{
	return (aiffinfo->comm.channels[0] << 8) | aiffinfo->comm.channels[1];
}


/*--------------------------------------------------------------------------*/
int aiffGetSampleRate(AIFFINFO* aiffinfo)
{
	unsigned long ieeeExponent;
	unsigned long ieeeMantissaHi;

	/* FIXED: sign must be removed from exponent, NOT mantissa. */

	ieeeExponent   = (aiffinfo->comm.samplesPerSec[0] << 8)
			| aiffinfo->comm.samplesPerSec[1];
	ieeeMantissaHi = ((aiffinfo->comm.samplesPerSec[2] << 24) |
			  (aiffinfo->comm.samplesPerSec[3] << 16) |
			  (aiffinfo->comm.samplesPerSec[4] << 8)  |
			   aiffinfo->comm.samplesPerSec[5]);

	ieeeExponent &= 0x7FFF; /* remove sign bit */

	return ieeeMantissaHi >> (16414 - ieeeExponent);
}


/*--------------------------------------------------------------------------*/
int aiffGetSamples(AIFFINFO* aiffinfo)
{
	return (aiffinfo->comm.samples[0] << 24) |
	       (aiffinfo->comm.samples[1] << 16) |
	       (aiffinfo->comm.samples[2] << 8)  |
	        aiffinfo->comm.samples[3];
}


/*--------------------------------------------------------------------------*/
int aiffGetBitsPerSample(AIFFINFO* aiffinfo)
{
	return (aiffinfo->comm.bitsPerSample[0] << 8) |
		aiffinfo->comm.bitsPerSample[1];
}


/*--------------------------------------------------------------------------*/
int aiffGetLoopStart(AIFFINFO* aiffinfo)
{
	return (aiffinfo->mark.position0[0] << 24) |
	       (aiffinfo->mark.position0[1] << 16) |
	       (aiffinfo->mark.position0[2] << 8)  |
	        aiffinfo->mark.position0[3];
}


/*--------------------------------------------------------------------------*/
int aiffGetLoopEnd(AIFFINFO* aiffinfo)
{
	return (aiffinfo->mark.position1[0] << 24) |
	       (aiffinfo->mark.position1[1] << 16) |
	       (aiffinfo->mark.position1[2] << 8)  |
	        aiffinfo->mark.position1[3];
}
