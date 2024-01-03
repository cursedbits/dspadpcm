#ifndef __ENCODE_H__
#define __ENCODE_H__

#include "types.h"

void encodeLoop(s16* input, u32 samples, void* dst, ADPCMINFO* cxt);

#endif
