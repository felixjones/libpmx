#ifndef __PMX_VERTEX_H__
#define __PMX_VERTEX_H__

#include <libpmx/pmx.h>
#include <libpmx/pmx_info.h>
#include <libpmx/pmx_weight.h>

enum pmx_vertex_mask_e {
	PMX_VERTEX_POSITION			= 0x1 << 0,
	PMX_VERTEX_NORMAL			= 0x1 << 1,
	PMX_VERTEX_UV				= 0x1 << 2,
	PMX_VERTEX_ADDITIONAL_UV	= 0x1 << 3,
	PMX_VERTEX_WEIGHT_TYPE		= 0x1 << 4,
	PMX_VERTEX_WEIGHT_DEF		= 0x1 << 5,
	PMX_VERTEX_EDGE_SCALE		= 0x1 << 6
};

typedef struct pmx_vertex_s {
	pmx_vec3		position;
	pmx_vec3		normal;
	pmx_vec2		uv;
	pmx_vec4 *		additionalUVs;
	pmx_byte		weightType;
	pmx_weight *	weightDef;
	pmx_float		edgeScale;
} pmx_vertex;

pmx_vertex *	pmx_vertex_create( pmx_info * const _info );
void			pmx_vertex_destroy( pmx_vertex * const _vertex );

#endif