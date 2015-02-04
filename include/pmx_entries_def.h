#ifndef __PMX_ENTRIES_DEF_H__
#define __PMX_ENTRIES_DEF_H__

#include <libpmx/pmx.h>

typedef struct pmx_edef_s {
	pmx_byte	textTypeEncoding;
	pmx_byte	additionalUVCount;
	pmx_byte	vertexIndexSize;
	pmx_byte	textureIndexSize;
	pmx_byte	materialIndexSize;
	pmx_byte	boneIndexSize;
	pmx_byte	morphIndexSize;
	pmx_byte	rigidIndexSize;
} pmx_edef;

pmx_edef *	pmx_edef_create();
void		pmx_edef_destroy( pmx_edef * const _header );

#endif