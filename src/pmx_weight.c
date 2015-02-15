#include "pmx_weight.h"

#include <string.h>

#define PMX_WEIGHT_BDEF1	( ( 'B' << 24 ) | ( 'D' << 16 ) | ( 'F' << 8 ) | '1' )
#define PMX_WEIGHT_BDEF2	( ( 'B' << 24 ) | ( 'D' << 16 ) | ( 'F' << 8 ) | '2' )
#define PMX_WEIGHT_BDEF4	( ( 'B' << 24 ) | ( 'D' << 16 ) | ( 'F' << 8 ) | '4' )
#define PMX_WEIGHT_SDEF		( ( 'S' << 24 ) | ( 'D' << 16 ) | ( 'E' << 8 ) | 'F' )

typedef struct pmx_bdef1_s {
	pmx_weight	super;

	pmx_int		boneIndex;
} pmx_bdef1;

typedef struct pmx_bdef2_s {
	pmx_weight	super;

	pmx_int		boneIndex[2];
	pmx_float	weight;
} pmx_bdef2;

typedef struct pmx_bdef4_s {
	pmx_weight	super;

	pmx_int		boneIndex[4];
	pmx_float	weight[4];
} pmx_bdef4;

typedef struct pmx_sdef_s {
	pmx_weight	super;
	
	pmx_int		boneIndex[2];
	pmx_float	weight;
	pmx_vec3	c;
	pmx_vec3	r[2];
} pmx_sdef;

pmx_weight * pmx_weight_create_bdef1() {
	pmx_bdef1 * const bone = ( pmx_bdef1 * )pmx_alloc( PMX_NULL, sizeof( pmx_bdef1 ) );
	memset( bone, 0, sizeof( *bone ) );

	bone->super.type = PMX_WEIGHT_BDEF1;

	return &bone->super;
}

pmx_weight * pmx_weight_create_bdef2() {
	pmx_bdef2 * const bone = ( pmx_bdef2 * )pmx_alloc( PMX_NULL, sizeof( pmx_bdef2 ) );
	memset( bone, 0, sizeof( *bone ) );

	bone->super.type = PMX_WEIGHT_BDEF2;

	return &bone->super;
}

pmx_weight * pmx_weight_create_bdef4() {
	pmx_bdef4 * const bone = ( pmx_bdef4 * )pmx_alloc( PMX_NULL, sizeof( pmx_bdef4 ) );
	memset( bone, 0, sizeof( *bone ) );

	bone->super.type = PMX_WEIGHT_BDEF4;

	return &bone->super;
}

pmx_weight * pmx_weight_create_sdef() {
	pmx_sdef * const bone = ( pmx_sdef * )pmx_alloc( PMX_NULL, sizeof( pmx_sdef ) );
	memset( bone, 0, sizeof( *bone ) );

	bone->super.type = PMX_WEIGHT_SDEF;

	return &bone->super;
}

void pmx_weight_destroy( pmx_weight * const _weight ) {
	pmx_alloc( _weight, 0 );
}
