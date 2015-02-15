#ifndef __PMX_WEIGHT_H__
#define __PMX_WEIGHT_H__

#include <libpmx/pmx.h>

typedef struct pmx_weight_s {
	pmx_int	type;
} pmx_weight;

pmx_weight *	pmx_weight_create_bdef1();
pmx_weight *	pmx_weight_create_bdef2();
pmx_weight *	pmx_weight_create_bdef4();
pmx_weight *	pmx_weight_create_sdef();
void			pmx_weight_destroy( pmx_weight * const _weight );

#endif