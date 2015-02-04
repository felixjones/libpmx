#ifndef __PMX_INDEX_SIZE_H__
#define __PMX_INDEX_SIZE_H__

#include <libpmx/pmx.h>

typedef struct pmx_isize_s {
	pmx_byte	textTypeEncoding;
	pmx_byte	additionalUVCount;

	struct indexSizes_s {
		pmx_byte	vertex;
		pmx_byte	texture;
		pmx_byte	material;
		pmx_byte	bone;
		pmx_byte	morph;
		pmx_byte	rigid;
	} indexSizes;
} pmx_isize;

#endif