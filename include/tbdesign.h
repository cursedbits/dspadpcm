#ifndef __TBDESIGN_H__
#define __TBDESIGN_H__

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "types.h"
#include "dspadpcm.h"

#define	TINY		1e-10

/* In estimate.c */
void	acf(short* sig, int len, double* ac, int nlags);
int	durbin(double* ac, int order, double* ref, double* taps, double* e2);
void	afromk(double* ref, double* taps, int order);
int	kfroma(double* taps, double* ref, int order);
void	rfroma(double* a, int n, double* r);
double	model_dist(double* ta, double* sa, int order);
int	lud(double** a, int n, s32* indx, s32* d);
void	lubksb(double** a, int n, s32* indx, double b[]);
void	acmat(short* in_buffer, int order, int length, double** a);
void	acvect(short* in_buffer, int order, int length, double* a);

double	model_dist_order2(double* ta, double* sa, int order);
void	rfroma_order2(double* a, int n, double* r);

/* in codebook.c */
void	split(double** codebook, double* dir, int order, int n_entries,
		double delta);
void	refine(double** codebook, int order, int n_entries, double** training,
		int nframes, int iterations, double converge);

/* in tbdesign.c */
void	adpcmCoefTableDesign(s16* input, u32 nsamples, ADPCMINFO* coeftable);

#endif
