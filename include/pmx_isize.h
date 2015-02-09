#ifndef __PMX_ISIZE_H__
#define __PMX_ISIZE_H__

#include <libpmx/pmx.h>

typedef struct pmx_isize_s {
	pmx_byte	vertex;
	pmx_byte	texture;
	pmx_byte	material;
	pmx_byte	bone;
	pmx_byte	morph;
	pmx_byte	rigidBody;
} pmx_isize;

#endif