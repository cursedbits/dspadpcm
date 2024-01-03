#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#include "types.h"

#define NORMAL_ENDIAN   0
#define REVERSE_ENDIAN  1

#define reverse_endian_16(x)	__builtin_bswap16(x)
#define reverse_endian_32(x)	__builtin_bswap32(x)

void reverse_buffer_16(u16* p, int samples);

#endif
