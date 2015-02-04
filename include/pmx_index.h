#ifndef __PMX_INDEX_H__
#define __PMX_INDEX_H__

#include <libpmx/pmx.h>

typedef union pmx_index_u {
	pmx_byte	ibyte;
	pmx_short	ishort;
	pmx_int		ilong;
} pmx_index;

#endif