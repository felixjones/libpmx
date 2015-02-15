#include "pmx_vertex.h"

#include <string.h>

pmx_vertex * pmx_vertex_create( pmx_info * const _info ) {
	pmx_vertex * const vertex = ( pmx_vertex * )pmx_alloc( PMX_NULL, sizeof( pmx_vertex ) );
	memset( vertex, 0, sizeof( *vertex ) );

	vertex->additionalUVs = ( pmx_vec4 * )pmx_alloc( PMX_NULL, sizeof( *vertex->additionalUVs ) * _info->additionalUVCount );

	return vertex;
}

void pmx_vertex_destroy( pmx_vertex * const _vertex ) {
	pmx_alloc( _vertex->additionalUVs, 0 );
	pmx_alloc( _vertex, 0 );
}
