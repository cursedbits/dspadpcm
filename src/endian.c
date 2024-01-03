#include "types.h"

/*--------------------------------------------------------------------------*/
void reverse_buffer_16(u16* p, int samples)
{
	u16* data = p;
	while(samples--) {
		*data = __builtin_bswap16(*data);
		data++;
	}
}
