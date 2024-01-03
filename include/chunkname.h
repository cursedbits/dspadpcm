#ifndef __CHUNKNAME_H__
#define __CHUNKNAME_H__

/*--------------------------------------------------------------------------*
    chunk_name macro
 *--------------------------------------------------------------------------*/
#define chunk_name(a,b,c,d) (				\
				 (a & 0xFF)		\
			+	((b & 0xFF) << 8)	\
			+	((c & 0xFF) << 16)	\
			+	((d & 0xFF) << 24))

#endif